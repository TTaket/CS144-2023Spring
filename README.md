# cs144

c 语言实现 TCP/IP 协议栈并替换掉 linux 中的 tcpip 协议栈

共分为 Lab0 - Lab7

solution 分支放置最新更新
各个分支为各个分实验
实验思路： 自顶向下

#### Lab0

完成时间：2023/10/3 -15.05

完成抽象：

- 最上层应用层： webget 我们可以通过封装好的 TCPSocket 和 Address 和内存中的字节流 向网络发起请求 我们的实验内容是慢慢实现封装好的这三个部分
- 内存中的字节流： 手动实现完成 - 字节在”输入”端写入，并可以从”输出”端以同样的顺序读取

中文参考文档： http://doraemonzzz.com/2022/01/30/2022-1-30-CS144-Lab0%E7%BF%BB%E8%AF%91/

#### Lab1

完成时间：2023/10/7 -1.30

完成抽象：

我们得到了一层重组器抽象： 无序 - > 有序

    输入： 包的序列号 包信息 目标流

    输出： 向目标流中输出有序的包的信息

    保证： 向目标流中输出的信息不会超过目标流的容量上限

中文参考文档：http://doraemonzzz.com/2022/01/30/2022-1-30-CS144-Lab1%E7%BF%BB%E8%AF%91/

#### Lab2

完成时间：2023/10/9 - 18.40

完成抽象：

    我们得到了 tcp_receiver 到现在位置 我们已经处理好了 tcp 在接受信息的时候传输层 接收端 应该做的事情（比较粗略 没有包含各种意外情况的处理 只有简单的应答和窗口数据）

中文参考文档：http://doraemonzzz.com/2021/12/27/2021-12-27-CS144-Lab2%E7%BF%BB%E8%AF%91

---

原网站：https://cs144.github.io/

Keith Winstein 大佬的仓库（startcode)：https://gitee.com/kangyupl

Doraemonzzz 大佬的网站（中文翻译）：http://doraemonzzz.com/
