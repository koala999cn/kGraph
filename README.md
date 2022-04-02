# kGraph

### 介绍

基于C++模板实现的图基本算法

核心代码参考Algorithms in C++ Part 5: Graph Algorithms, Robert Sedgewick, 3rd Edition.


### 代码结构

base\，基础支持数据结构和算法

core\，核心图算法

util\，一些实用函数

test\，测试代码

GraphX.h，定义了一些基础的图类型


### 图结构

图的结构布局与算法相对独立实现。

纵向上有稠密图、稀疏图、扁平图3类基本结构；

横向上有无向/有向、单边/多边、始终排序3类配置参数。

两者结合可构造丰富多样的底层结构。


### 图算法

1.  邻接顶点迭代器
2.  深度优先（Dfs）、广度优先（Bfs）和基于优先队列（Pfs）的迭代器
3.  连通、强连通检测
4.  二分图检测
5.  桥、割点检测
6.  拓扑排序：基于源点队列, 基于Dfs的逆排序
7.  传递闭包：Warshall, Dfs, Dag-Dfs, Strongly-Connected
8.  最小生成树：Prim, Kruskal, Boruvka, Pfs
9. 最优路径：单源（Dijkstra, Pfs, TS, Bellman-Ford），全源（Dijkstra, Pfs, Dfs, Floyd）
10. 最大流：Path-Augment, Pfs, Edmonds-Karp/Bfs, Preflow


### 编译

编译器须支持c++17。核心代码全C++模板实现，不需编译，测试代码通过MSVC2019和GCC11编译。


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

