一个疏漏百出、功能极简的KV数据库，粗略实现了ACID、MVCC，李国良老师《大数据分析与内存计算》课程作业，欢迎参考和改进。

### 环境：python3、C++11

### 依赖：flask、requests、pthread.h

### 简介：
service.py是flask服务，运行时调用KVDB.exe，KVDB.cpp是数据库引擎，runner.py是客户端，从标准输入获取指令。

项目详情可见doc/doc.pdf

支持的指令例子：

PUT A 1
GET A
DEL A
PUT A (A+1)
PUT A (A-1)
BEGIN
COMMIT
ABORT

