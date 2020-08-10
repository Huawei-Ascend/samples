##########################################################
# 
# 本文件的功能:
#    调用caffe模型,进行 finetune 训练
# 
##########################################################
import caffe 
import numpy as np 
caffe.set_mode_gpu()
caffe.set_device(0)
from sklearn.metrics import recall_score, f1_score, precision_score
import warnings 
warnings.filterwarnings("ignore")
import matplotlib.pyplot as plt;
import os


# 训练
def train():
    # 1,定义solver
    solver = caffe.get_solver("./model/solver.prototxt")

    # 2,定义参数
    weights = "./model/resnet-18.caffemodel"
    train_preds = []
    train_gts   = []
    epochs = 82620      # 迭代次数30 epochs
    display_epochs = 100

    # 3,加载预训练模型的参数
    solver.net.copy_from(weights) 
    print("\n\n\n\n************** start training ***********************\n\n")

    # 4,开始训练
    for it in range(1, epochs + 1):

        # (1)执行一次前向计算和反向传播
        solver.step(1)
        # print("loss: ", solver.net.blobs['loss'].data)
        
        # (2)获取预测值和真实值
        train_gt   = (solver.net.blobs['label'].data)[:,:,0,0]
        train_pred = solver.net.blobs['score'].data
        train_pred[train_pred >= 0] = 1
        train_pred[train_pred <  0] = 0

        train_gts.extend(train_gt)
        train_preds.extend(train_pred)

        # (3) 进行100次之后统计计算 准确率，召回率，f1值等指标
        if it % display_epochs == 0:
            train_gts       = np.array(train_gts)
            train_preds     = np.array(train_preds)
            train_precision = precision_score(train_gts, train_preds, average="macro")
            train_recall    = recall_score(train_gts, train_preds, average="macro")
            train_f1        = f1_score(train_gts, train_preds, average="macro")
            print("\n\n******************** train集指标 ********************************************************")
            print("*")
            print("*")
            print ('* iteration:%d\n\t查准率: %.4f\t查全率: %.4f\tF1值: %.4f' %(it, train_precision, train_recall, train_f1))
            train_gts = []
            train_preds = []
            print("*")
            print("*")
            print("******************************************************************************************\n\n")
    
    print("\n\n************** end training ***********************\n\n")


# 定义测试函数,计算训练好的模型的测试性能指标
def test():
    # 1,定义网络
    net = caffe.Net("./model/test.prototxt", "./saved_model/solver_iter_70000.caffemodel", caffe.TEST)
    print("\n\n\n************************** start testing ********************************")

    # 2, 定义计算指标所需参数
    num_batches = 683
    ys = []
    preds = []
    for _ in range(num_batches):
        # 3,前向计算
        net.forward()

        # 4,取出预测输出和真实值
        y = net.blobs['label'].data[:, :, 0, 0]
        pred = net.blobs['score'].data
        pred[pred >= 0] = 1
        pred[pred <  0] = 0

        ys.extend(y)
        preds.extend(pred)

    # 5,计算指标
    ys = np.array(ys)
    preds = np.array(preds)
    test_precision = precision_score(ys, preds, average="macro")
    test_recall    = recall_score(ys, preds, average="macro")
    test_f1        = f1_score(y, pred, average="macro")
    print("\n\n查准率: %.4f\n" % test_precision)
    print("查全率: %.4f\n"% test_recall)
    print("F1值:  %.4f\n" % test_f1)

    print("\n\n************************** end testing ********************************\n\n\n")

# 测试函数,可视化一张图片的推断结果
def visualize():

    # label和id之间的对应关系
    # id_2_label = {
    #     "1": "Mitochondria",
    #     "2": "Nucleus",
    #     "3": "Endoplasmic reticulum",
    #     "4": "Nuclear speckles",
    #     "5": "Plasma membrane",

    #     "6": "Nucleoplasm",
    #     "7": "Cytosol",
    #     "8": "Nucleoli",
    #     "9": "Vesicles",
    #     "10": "Golgi apparatus",
    # }
    id_2_label = [
        "Mitochondria", "Nucleus", "Endoplasmic reticulum", "Nuclear speckles", 
        "Plasma membrane", "Nucleoplasm", "Cytosol", "Nucleoli",
        "Vesicles", "Golgi apparatus"
    ]

    # 1,拿到推断的图片的名字和推断结果
    results = os.listdir("./hpa_result")
    for result in results:
        
        # 拿到图片的名字
        img_name = (result.strip().split(".")[0])[8:] + ".jpeg"
        img = plt.imread("./data/test/" + img_name)

        # 拿到图片的推断结果
        img_label = ((open("./hpa_result/" + result).read()).strip().split(" "))[7:]
        # print("image name: ", img_name, "; ", img_label)

        # 可视化图片
        plt.imshow(img)
        font_size = 12
        
        top1 = np.argmax(img_label)
        label = id_2_label[top1] + " : " + str(float(img_label[top1]) * 100) + "%"
        img_label[top1] = 0
        plt.text(0, 150, label, fontdict = {"fontsize" : font_size, "color":"white"})

        top2 = np.argmax(img_label)
        label = id_2_label[top2] + " : " + str(float(img_label[top2]) * 100) + "%"
        img_label[top2] = 0
        plt.text(0, 300, label, fontdict = {"fontsize" : font_size, "color":"white"})

        top3 = np.argmax(img_label)
        label = id_2_label[top3] + " : " + str(float(img_label[top3]) * 100) + "%"
        img_label[top3] = 0
        plt.text(0, 450, label, fontdict = {"fontsize" : font_size, "color":"white"})
        # plt.show()
        plt.savefig("./hpa_result/" + img_name)
        plt.close()
    print("Finished...")

#  主函数
def main():
    # 1,训练
    # train()

    # 2,测试
    test()

    # 3,可视化图片的推理结果
    # visualize()

main()