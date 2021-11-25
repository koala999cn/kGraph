# kGraph

### 介绍

基于C++模板实现的图基本算法

核心代码参考Algorithms in C++ Part 5 Graph Algorithms, Robert Sedgewick, 3rd Edition.


### 代码结构

base\，基础支持数据结构和算法

core\，核心图算法

util\，一些实用函数

test\，测试代码

GraphX.h，定义了一些基础的图类型


### 功能

1.  稠密图、稀疏图、平行图（一对顶点多条边）和带顶点对象的图、嵌套子图等多类图结构
2.  邻接顶点迭代器、逆邻接顶点迭代器
3.  深度优先、广度优先和基于优先队列的迭代器
4.  连通、强连通检测
5.  二分图检测
6.  桥、割点检测
7.  拓扑排序
8.  传递闭包：Warshall, Dfs, Dag-Dfs, Strongly-Connected
9.  最小生成树：Prim, Kruskal, Boruvka, Pfs
10. 最优路径：单源（Dijkstra,, Pfs, TS, Bellman-Ford），全源（Dijkstra, Pfs, Dfs, Floyd）
11. 最大流：Path-Augment, Pfs, Edmonds-Karp/Bfs, Preflow


### 软件架构

全C++模板实现，通过模板类的继承和特化提供多种图类型。

最底层的类为KtGraphBase，提供ADJ_MATRIX、direction、parallel三个模板参量，分别实现稠密/稀疏、有向/无向、平行/非平行图。

次一级为KtGraphDense和KtGraphSparse，分别为稠密矩阵和稀疏矩阵对KtGraphBase的特化，同时实现各自的邻接顶点迭代器。

再次一级为KtGraphImpl，通过模板参量GRAPH_BASE特化，继承自KtGraphDense或KtGraphSparse，提供基于一致接口的常用函数实现。

再次一级为KtGraphVoa，通过继承自特化的KtGraphImpl，扩展实现带顶点对象的图类型。

最上一层为KtGraphSub，实现嵌套图类型。

具体参见下图，嵌套图未画出。

![输入图片说明](https://images.gitee.com/uploads/images/2021/1102/100032_d62dde5c_8396825.jpeg "arch.jpg")


### 编译

编译器须支持c++17。核心代码不需编译，测试代码通过vc和gcc编译测试。


### 使用说明

创建有向稠密图的示例代码：

```
    #include "GraphX.h"

    DigraphDd g(6);
    g.addEdge(0, 1, 0.41), g.addEdge(0, 5, 0.29); 
    g.addEdge(1, 2, 0.51), g.addEdge(1, 4, 0.32); 
    g.addEdge(2, 3, 0.50);
    g.addEdge(3, 0, 0.45), g.addEdge(3, 5, 0.38);
    g.addEdge(4, 2, 0.32), g.addEdge(4, 3, 0.36);
    g.addEdge(5, 1, 0.29), g.addEdge(5, 4, 0.20);
```

对图g进行深度优先迭代的示例代码：

```
    #include "../core/KtDfsIter.h"

    KtDfsIter<DigraphDd> dfs(g, 0);
    std::vector<unsigned> v;
    for (; !dfs.isEnd(); ++dfs) 
        v.push_back(*dfs); // *dfs表示当前遍历的顶点序号
```


更多示例代码参考test目录下的测试代码。

