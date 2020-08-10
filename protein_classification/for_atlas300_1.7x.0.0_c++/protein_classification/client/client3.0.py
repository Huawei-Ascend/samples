#!/usr/bin/env python
# -*- coding=utf-8 -*-
# author: liufangxin
# date: 2020.6.8

from tkinter import *
from tkinter.filedialog import askopenfilename
from PIL import Image, ImageTk
import tkinter as tk
import tkinter.messagebox
import numpy as np
import os
import sys
import struct
import time
import socket

# global param
picname=''

try:
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect(('124.70.69.88',3389))
except socket.error as msg:
	print(msg)
	sys.exit(1)
print("connected !")

def process():
	if picname=='':
		tk.messagebox.showerror('', 'Please select one picture!')

	# send image
	fileprefix=picname.split('.')[0]
	filepath='./tmp/'+picname
	if os.path.isfile(filepath):
		filesize=os.stat(filepath).st_size
		s.sendall(bytes(str(filesize),encoding='utf-8'))
		print ('client filepath: {0}'.format(filepath))
		fp = open(filepath, 'rb')
		while 1:
			data = fp.read(1024)
			if not data:
				print ('{0} file send over...\n\n'.format(filepath))
				break
			s.send(data)

		# receive result
		print("start recv")
		while 1:
			buf = s.recv(1024)
			if buf:
				res_size=str(buf,encoding="utf-8")
				new_filename = './result/'+fileprefix+'_result.jpg'
				print ('file new name is {0}, filesize if {1}'.format(new_filename, res_size))

				recvd_size = 0
				res_size=int(res_size)
				fp = open(new_filename, 'wb')
				print ("start receiving...")
				while not recvd_size == res_size:
					if res_size - recvd_size > 1024:
						data = s.recv(1024)
						recvd_size += len(data)
					else:
						data = s.recv(res_size - recvd_size)
						recvd_size += len(data)
						# recvd_size = res_size
					fp.write(data)
				fp.close()
				print ("end receive...\n\n")
				break
		tkinter.messagebox.showinfo('Info','Processing Successfully!')

		# Save and show the result
		if os.path.isfile(new_filename):
			resultimage=Image.open(new_filename)
			resultimage=ImageTk.PhotoImage(resultimage)
			tempapp=tk.Toplevel()
			tempapp.title('Result')
			tempapp.geometry('560x560')
			tempapp.resizable(width=False, height=False)
			label_img = tk.Label(tempapp, image = resultimage)
			label_img.pack()
			tempapp.mainloop()

# Input file selection
def choosepic():
	#global filename
	filename=askopenfilename()
	if filename != '':
		global picname
		picname=filename.split('/')[-1]
		im=Image.open(filename)
		if im.size[0]!=512:
			# im=im.crop((584,0,4088,3504))
			#print(im.size)
			im=im.resize((512,512))
		# im.save('./tmp/'+picname)
		img=ImageTk.PhotoImage(im)
		
		lbPic.config(image=img)
		lbPic.image = img 

# Application GUI
app = tk.Tk()
app.title('HPA')
app.geometry('560x630')
app.iconbitmap('./icofile.ico')
app.resizable(width=False, height=False)

Frame=tk.LabelFrame(app,padx=10,pady=10)
Frame.place(x=10,y=15)

cover=Image.open('./cover.jpg')
coverimg=ImageTk.PhotoImage(cover)
lbPic = tk.Label(Frame,image=coverimg)

lbPic.grid(row=0,column=0)


Frame_button=tk.LabelFrame(app,padx=10,pady=10)
Frame_button.place(x=365,y=565)

Button_file=tk.Button(Frame_button,text='Select',width=10,relief=GROOVE,fg='#5D88B5',command=choosepic)
Button_file.grid(row=0,column=0)

Button_run=tk.Button(Frame_button,text='Process',width=10,relief=GROOVE,fg='#71195C',command=process)
Button_run.grid(row=0,column=2)

app.mainloop()
s.close()