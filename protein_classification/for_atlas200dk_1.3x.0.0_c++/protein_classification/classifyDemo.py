#coding=utf-8

import hiai
from hiai.nn_tensor_lib import DataType
import imageNetClasses
import jpegHandler
import os
import numpy as np
import time

resnet18OmFileName='./models/HPA.om'
srcFileDir = './ImageNetRaw/'
dstFileDir = './HPAResult/'


def CreateGraphWithoutDVPP(model):

	#调用get_default_graph获取默认Graph，再进行流程编排
	myGraph = hiai.hiai._global_default_graph_stack.get_default_graph()
	if myGraph is None :
		print 'get defaule graph failed'
		return None

	nntensorList=hiai.NNTensorList()

	# 不调用DVPP 缩放图片，使用opencv 缩放图片
	resultInference = hiai.inference(nntensorList, model, None)

	if ( hiai.HiaiPythonStatust.HIAI_PYTHON_OK == myGraph.create_graph()):
		print 'create graph ok !!!!'
		return myGraph
	else :
		print 'create graph failed, please check Davinc log.'
		return None


def GraphInference(graphHandle,inputTensorList):
	if not isinstance(graphHandle,hiai.Graph) :
		print "graphHandle is not Graph object"
		return None

	resultList = graphHandle.proc(inputTensorList)
	return resultList

def Resnet18PostProcess(resultList, srcFilePath, dstFilePath, fileName):

	if resultList is not None :
		# resultList 是一个list对象。每个item 也是一个 array对象的list
		resultArray = resultList[0]
		batchNum = resultArray.shape[0]
		confidenceNum = resultArray.shape[3]
		confidenceList = resultArray[0,0,0,:]

		confidenceArray = np.array(confidenceList)
		confidenceIndex = np.argsort(-confidenceArray)

		firstClass = confidenceIndex[0]
		firstConfidence = confidenceArray[firstClass]
		firstLabel = imageNetClasses.imageNet_classes[firstClass]
		print fileName + '    ' + '%.2f%%' % (firstConfidence*100) + '\t' + firstLabel

		Class2 = confidenceIndex[1]
		Confidence2 = confidenceArray[Class2]
		Label2 = imageNetClasses.imageNet_classes[Class2]
		print fileName + '    ' + '%.2f%%' % (Confidence2 * 100) + '\t' + Label2

		Class3 = confidenceIndex[2]
		Confidence3 = confidenceArray[Class3]
		Label3 = imageNetClasses.imageNet_classes[Class3]
		print fileName + '    ' + '%.2f%%' % (Confidence3 * 100) + '\t' + Label3

		dstFileName = os.path.join('%s%s' % (dstFilePath, fileName))
		srcFileName = os.path.join('%s%s' % (srcFilePath, fileName))

		jpegHandler.putText(srcFileName, dstFileName, '%.2f%%' % (firstConfidence*100) + ':' + firstLabel, 100)
		jpegHandler.putText(dstFileName, dstFileName, '%.2f%%' % (Confidence2 * 100) + ':' + Label2, 150)
		jpegHandler.putText(dstFileName, dstFileName, '%.2f%%' % (Confidence3 * 100) + ':' + Label3, 200)

		return None
	else :
		print 'graph inference failed '
		return None

def main():
	inferenceModel = hiai.AIModelDescription('restnet18',resnet18OmFileName)
	
	# we will resize the jpeg to 256*224 to meet resnet18 requirement via opencv,
	# so DVPP resizing is not needed  	
	myGraph = CreateGraphWithoutDVPP(inferenceModel)
	if myGraph is None :
		print "CreateGraph failed"
		return None

	# in this sample demo, the resnet18 model requires 256*224 images
	dvppInWidth = 224
	dvppInHeight = 224

	start = time.time()
	jpegHandler.mkdirown(dstFileDir)

	pathDir =  os.listdir(srcFileDir)
	for allDir in pathDir :
		child = os.path.join('%s%s' % (srcFileDir, allDir))
		if( not jpegHandler.is_img(child) ):
			print '[info] file : ' + child + ' is not image !'
			continue 

		# read the jpeg file and resize it to required w&h, than change it to YUV format.	
		input_image = jpegHandler.jpeg2yuv(child, dvppInWidth, dvppInHeight)

		inputImageTensor = hiai.NNTensor(input_image,dvppInWidth,dvppInHeight,3,'testImage',DataType.UINT8_T, dvppInWidth*dvppInHeight*3/2) 
		nntensorList=hiai.NNTensorList(inputImageTensor)

		resultList = GraphInference(myGraph,nntensorList)
		if resultList is None :
			print "graph inference failed"
			continue

		Resnet18PostProcess(resultList, srcFileDir, dstFileDir, allDir)

	end = time.time()
	print 'cost time ' + str((end-start)*1000) + 'ms'		
	

	hiai.hiai._global_default_graph_stack.get_default_graph().destroy()



	print '-------------------end'


if __name__ == "__main__":
	main()
