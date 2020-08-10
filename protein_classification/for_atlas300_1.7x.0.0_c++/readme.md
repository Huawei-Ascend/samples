## 基于荧光显微图像的蛋白质亚细胞定位预测系统部署说明

*Author：雷后超  Date: 2020.06.15*



本系统分为client和server两个部分，其中client部署在本地电脑上，server部署在搭载Atlas300的云服务器上

### Client 部署

客户端采用python3实现，运行所需的库主要有tkinter，PIL，numpy和socket，运行前需要确认这些库的安装，并需要修改 client3.0.py 文件中连接的服务器端的IP地址和端口号（以下代码中的xxx）。

```python
try:
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect(('xxx.xx.x.xxx',xxxx))
except socket.error as msg:
	print(msg)
	sys.exit(1)
```

然后通过python运行此文件即可，运行方式: python client3.0.py。

### Server 部署

server文件夹下是服务器端部署的全部文件，需要将server目录下的所有文件拷贝到 /home/HwHiAiUser/HIAI_PROJECTS/hpa/ 目录下。

服务器程序也通过python3实现，用到的库主要有socket，threading，PIL和numpy。

运行前也需要修改代码 server.py 文件中的IP地址和端口号，注意此处的IP地址为服务器网卡的IP地址，而不是云服务器绑定的公网IP。

```
try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(('xxx.xxx.x.xxx', xxxx))
    s.listen(10)
except socket.error as msg:
    print(msg)
    sys.exit(1)
```

为了方便编译运行，特提供一键式脚本 run.sh，该脚本包含模型转换、程序编译、程序运行等所有操作，仅需给该脚本设置权限: chmod 777 run.sh，然后运行该脚本即可完成服务端的部署，运行方式如下: ./run.sh