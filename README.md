# cabinet
由于这不仅是一个编程项目    
还是我个人的毕业设计    
所以readme里面放着比较重要的东西
包括且不限于:    
1. 当前任务及优先级        
2. 项目当前进展情况        

项目的需求及迭代步骤 请见 [需求文档](https://github.com/dszhengyu/cabinet/blob/master/%E9%9C%80%E6%B1%82%E6%96%87%E6%A1%A3.md)    
项目的设计图 [请点我](https://www.processon.com/diagrams) (未分享)    

###2016-2-27-v3.0  
吃喝玩乐好几天, 疯狂看足球==脚痒痒了~    
进度: v3.0基本设计完成    
接下来任务: 重构代码    
重构内容:    
1. 提取Client父类    
2. 提取Cabinet父类    
3. 适当修改EventPoll类, 支持新的类结构    
4. 写一点Util的内容    
重构之后, 开火!    

###2016-2-21-v3.0    
大病一场真是浪费时间...    
v2.0已经完成(3小时?)    
v1.1也完成    
最困难的v3.0, raft集群   
目前为设计阶段   
此阶段可以做:     
P0 研读ratf相关论文, 书籍    
P0 思考raft实现     
P1 参考他人代码, 测试框架(单元测试, 冒烟测试)    
P2 一些代码重构, 框架基础设施搭建(eventloop重构, 配置开机加载)    
raft论文里面提到用了2k行c++代码实现    
应该不会太难    

###2016-2-12-v1.1    
v1.1可以做的有:    
1. 记录客户连接的ip和端口(简单)--done    
2. cli实现作业控制, 后台转前台时候, 打印终端提示符--done        
3. server关闭时获取信号, 打印日志, 清理数据--deley    
4. 客户端检查保留关键字符, 如   $ # \n \r * \0  --no-need, binary safe    
5. 花哨的开机日志--done    

###2016-2-12
项目进展: stage1 完成 包括服务器以及命令行客户端    
此为v2.0 分支    
进行迭代二, 需求思考中, 未详细设计, 未进入代码阶段      
需要对迭代三设计进行大致勾画    


###2016-2-11
项目进展: stage1 完成 包括服务器以及命令行客户端    
此为v1.1 分支    
第一个1为迭代号, 第二个1标志迭代1的附加版本号    
此分支负责对迭代一进行修补工作, 比如增加命令, 修改细节等    
接下来的开发不允许主干开发    
开发流程为:
1. 根据版本号新建分支    
2. 分支开发    
3. 打tag    
4. 合并分支到主干    
5. 删除分支    

###2016-2-5
项目进展: stage 1 cabinet-cli代码,基本框架完成, 具体命令及数据库没有支持 cabinet-cli尚未开始
当前任务及优先级
P0 尽快完成cabinet-cli的设计及代码编写, 求简, 复用Client代码, 尽快与cabinet-server进行测试
P0.5 完成cabinet-server的编写 
P1 思考迭代二需要做什么

###2016-1-24 
项目进展: stage 1 设计阶段    
当前任务及优先级    
P0 进入第一步迭代的代码阶段    
P1 确定如何第一步迭代之后, 如何支持aof, 2pc (阅读论文查看现有实现) 确定流程        
P1 阅读APUE重要相关章节    
P2 进一步了解raft协议    
P3 一致性hash    
P3 广泛阅读论文, 寻找思路    


###2016-1-15 当前任务及优先级
P0-阅读论文及网页, 弄清楚这些但是不仅这些协议, 寻找实现例子(raft gossip 心跳/租约 2PC 一致性hash CAP)    
P1-(50%已经阅读大致流程)-阅读redis源码, 确定程序框架
P1-确定论文主调, 思考创新点(不要闭门造车)    
P2-(50%)阅读unix网络编程第三部分(选读)    
P2-选读APUE    
P2-paxos及zookeeper    
P3-单测, 日志, make工具等    
