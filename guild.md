前言：该实验来自黑书[《深入理解计算机系统》](https://hansimov.gitbook.io/csapp)。它很好的从代码实践的角度讲解了计算机系统常用知识。陈皓曾这样评价这本书：“为程序员描述计算机系统的实现细节，帮助其在大脑中构造一个层次型的计算机系统。从最底层的数据在内存中的表示到流水线指令的构成，到虚拟存储器，到编译系统，到动态加载库，到最后的用户态应用。通过掌握程序是如何映射到系统上，以及程序是如何执行的，你能够更好地理解程序的行为为什么是这样的，以及效率低下是如何造成的。”

虽然本书所设计的实验距离当下主流的应用开发还有一段距离，但他就好比手动挡的汽车驾照，虽然一般大家都开自动挡的车，但了解如何驾驶手动挡的你可以在自动系统失灵时，仍能够驱动项目运行起来。

## 实验简介

实现一个并发缓存（concurrent caching）的 Web 代理，该代理位于本地的浏览器和其他万维网之间。该实验让学生接触到有趣的网络编程世界，并将课程中的许多概念联系在一起，例如字节序（byte ordering）、缓存、进程控制、信号、信号处理、文件 I/O、并发和同步。

## 实验内容

### Part 1：处理代理请求

代理请求其实与普通的web请求类似，只不过请求头会额外包含一个链接，指向目的服务器的资源。如果不了解普通的web请求有哪几类，分别有哪几部分，可以先自行学习了解。（注：请注意查看实验要求，提前设想程序要处理哪几种web请求，具体把控哪些细节。建议结合各种请求的作用效果思考）



#### 解析请求头



### Part 2：处理请求资源



### Part 3：维护缓存



### Part 4：测试与问题处理

### 

