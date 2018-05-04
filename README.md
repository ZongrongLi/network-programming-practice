# network-programming-practice
网络编程实践


ttcp：

网络性能测试工具，带宽，吞吐量，延迟，阻塞io



netcat：

聊天工具 各有两个线程用一条tcp互相通信 标准输入到tcp 到标准输出


dummyload：

假负载 给cpu加负载画出cos曲线 开环无反馈，简单粗暴的pv操作
把一秒分为100份，决定每一份的是闲还是忙
在100个数中均匀的标记出 pecent个忙点：Bresenham's line algorithm


procman:
进程状态查看，画cpu曲线，使用html作为界面 容易聚合数据 可以查看多级


待优化：

stat文件 每次打开不用关闭，动态共享

统计每个线程的CPU使用

内存的图表

监控整台机器


