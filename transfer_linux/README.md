# 编译和执行

执行`./make`即可编译程序

为了能够执行程序，需要配置transfer_file.ini文件，指定该机器上接收数据和发送数据的网卡名，该配置文件在A和B上均需要配置。transfer_file.ini文件格式如下：

```
[netcard]
send=eth0
receive=eth1
```

编译出来的可执行文件需要在A和B上同时运行，假设A是要发送数据的机器，在A上执行`./transfer_linux -f filename`，在B上执行`./transfer_linux`，这样A就可以发送文件给B了。B接收到文件名添加了后缀.recv。

需要注意的是，需要使用root用户来运行程序。

