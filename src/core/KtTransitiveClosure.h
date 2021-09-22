#pragma once
#include <algorithm>


/* 
  基于Warshall算法的传递闭包实现.
  Warshall算法将基于布尔矩阵的传递闭包实现时间复杂度从V^3*log2V降低为V^3
 */
template<typename SRC_GRAPH, typename CLS_GRAPH = SRC_GRAPH>
class KtTransitiveClosureWar
{
public:
    KtTransitiveClosureWar(const SRC_GRAPH& g) : g_(g) {
        unsigned V = g.order();
        for(size_t v = 0; v < V; v++)
            if(!g_.hasEdge(v, v)) 
                g_.addEdge(v, v); // 添加自环

        // Warshall算法
        for(unsigned i = 0; i < V; i++)
            for(unsigned s = 0; s < V; s++)
                if(g_.hasEdge(s, i))
                    for(unsigned t = 0; t < V; t++)
                        if(g_.hasEdge(i, t) && !g_.hasEdge(s, t))
                            g_.addEdge(s, t);
    }

    bool isReachable(unsigned v, unsigned w) const {
        return g_.hasEdge(v, w);
    }

private:
    CLS_GRAPH g_;
};


/* 
  基于DFS的传递闭包实现.
  它以G的各个顶点作为开始完成一个单独的DFS，计算其可达的顶点集，以此计算传递闭包。
 */
#include "KtDfsIter.h"
template<typename SRC_GRAPH, typename CLS_GRAPH = SRC_GRAPH>
class KtTransitiveClosureDfs
{
public:
    KtTransitiveClosureDfs(const SRC_GRAPH& g) : g_(g.order()) {
        unsigned V = g.order();
        for(unsigned i = 0; i < V; i++)
            if(!g_.hasEdge(i, i))
                g_.addEdge(i, i);
                    
        for(size_t v = 0; v < V; v++) {
            KtDfsIter<SRC_GRAPH> iter(g, v);
            assert(iter.from() == -1);
            ++iter; // skip v-self
            for (; !iter.isEnd(); ++iter)
                if(iter.isTree() && !g_.hasEdge(v, *iter)) 
                    g_.addEdge(v, *iter);
        }
    }

    bool isReachable(unsigned v, unsigned w) const {
        return g_.hasEdge(v, w);
    }

private:
    CLS_GRAPH g_;
};


// DAG的传递闭包，基于DFS算法实现。
// 相比KtClosureDfs，主要利用DAG的特性进行优化.
//   1.没有回边
//   2.下边可以忽略，即不用递归展开，也不用回溯传递闭包
//   3.跨边不用递归展开，直接回溯即可
template<typename SRC_DAG, typename CLS_DAG = SRC_DAG>
class KtTransitiveClosureDag
{
public:
    KtTransitiveClosureDag(const SRC_DAG& g) : g_(g.order()) {
        assert(!g.hasLoop());
        unsigned V = g.order();
        for(unsigned i = 0; i < V; i++)
            g_.addEdge(i, i);

        KtDfsIter<SRC_DAG, true, true, true> iter(g, 0); 
        for (; !iter.isEnd(); ++iter) {
            unsigned p = iter.from();
            if(p == -1) 
                continue; // skip edge(-1, v)

            unsigned v = *iter;
            if(!iter.isPopping() && !g_.hasEdge(p, v))
                g_.addEdge(p, v);

            assert(!iter.isBack()); // DAG没有回边

            if(iter.isCross() || iter.isPopping()) 
                 for(size_t w = 0; w < V; w++) 
                    if(g_.hasEdge(v, w) && !g_.hasEdge(p, w)) 
                        g_.addEdge(p, w);
        }     
    }

    bool isReachable(unsigned v, unsigned w) const {
        return g_.hasEdge(v, w);
    }

private:
    CLS_DAG g_;
};


/* 
  基于强分量的传递闭包实现.
 */
#include "KtStronglyConnected.h"
template<typename GRAPH>
class KtTransitiveClosureScc
{
public:
    KtTransitiveClosureScc(const GRAPH& g) : scc_(g) {
        // 以每个强连通分量为一个顶点构建DAG
        GRAPH K(scc_.count()); // TODO：use bit graph
        for(unsigned v = 0; v < g.order(); v++) {
            typename GRAPH::adj_vertex_iter iter(g, v);
            while(!iter.isEnd()) {
                auto x = scc_[v];
                auto y = scc_[*iter];
                if(x != y && !K.hasEdge(x, y)) // 消除自环
                    K.addEdge(x, y);
                ++iter;
            }
        }

        // 使用基于DAG的传递闭包优化算法
        dagCls_ = new KtTransitiveClosureDag<GRAPH>(K);
    }

    ~KtTransitiveClosureScc() { delete dagCls_; }

    bool isReachable(unsigned v, unsigned w) const {
        return dagCls_->isReachable(scc_[v], scc_[w]);
    }

private:
    KtStronglyConnectedTar<GRAPH> scc_;
    KtTransitiveClosureDag<GRAPH>* dagCls_;
};
