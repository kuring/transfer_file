该工程为Linux下的单向传输文件程序，适合在两台机器之间仅能单向通信，不能建立tcp连接的情况。出于安全考虑，两台机器之间可以安装物理隔离或网闸装置来阻止机器之间进行直接的通信，两台机器之间要想能够传输文件仅能通过A向网卡上发数据包，B从网卡上接收数据包的方式来进行通信。B要发数据到A，仅能通过另外两个网卡发送数据包的形式实现。

该工程仅仅是研究了A和B之间传输文件的可行性，要想使用该工程，需要跟具体的业务相结合，直接将其中的代码合并到业务工程中即可，合并工作并不复杂。

该工程分为Linux版本和Windows版本，其中transfer_linux为Linux版本，transfer_windows为Windows版本。

# 工程实现原理

该工程的Linux版本用到libpcap作为发送数据包和读取数据包的库，Windows版本使用winpcap作为发送数据包和读取数据包的库。当发送文件时，将要发送的文件拆分成1000字节的数据包，每个数据包封装成udp数据包格式进行发送。A不断向B发送udp数据包，直到发送完毕。B在接收到A发送的第一个数据包时，会在内存中申请和文件大小相同的内存，后续接收到文件的数据包直接将数据复制到已经申请好的内存中。当B接收完A发送的数据包时，向A发送ack包，告诉A文件已经正确接收完毕，同时B将文件从内存中保存到磁盘上。

# 异常处理

本工程已经充分考虑到了网络的不稳定性和丢包情况。

将要发送文件的数据包进行了编号处理，B接收到数据包如果跟上次接收到的数据包不连续，B直接丢弃，并给A发送FILE_WRONG命令。A接收到FILE_WRONG命令后停止该文件后续数据包的发送，并重新发送文件。

A如果连续发送三次都没有发送成功，停止对文件的发送。

# 工程中用到的线程

* 读取文件发送队列并发送文件线程。
* 读取数据块发送队列并将数据块发送到网卡线程。
* 定期检测发送完成文件map线程。

# 效率

经在普通机器上测试，文件传输速度在10M以上，效率还可以。