#pragma once
#include <algorithm>


/* 
  ����Warshall�㷨�Ĵ��ݱհ�ʵ��.
  Warshall�㷨�����ڲ�������Ĵ��ݱհ�ʵ��ʱ�临�Ӷȴ�V^3*log2V����ΪV^3
 */
template<typename SRC_GRAPH, typename CLS_GRAPH = SRC_GRAPH>
class KtTransitiveClosureWar
{
public:
    KtTransitiveClosureWar(const SRC_GRAPH& g) : g_(g) {
        unsigned V = g.order();
        for(size_t v = 0; v < V; v++)
            g_.addEdge(v, v); // ����Ի�

        // Warshall�㷨
        for(unsigned i = 0; i < V; i++)
            for(unsigned s = 0; s < V; s++)
                if(g_.hasEdge(s, i))
                    for(unsigned t = 0; t < V; t++)
                        if(g_.hasEdge(i, t))
                            g_.addEdge(s, t);
    }

    bool isReachable(unsigned v, unsigned w) const {
        return g_.hasEdge(v, w);
    }

private:
    CLS_GRAPH g_;
};


/* 
  ����DFS�Ĵ��ݱհ�ʵ��.
  ����G�ĸ���������Ϊ��ʼ���һ��������DFS��������ɴ�Ķ��㼯���Դ˼��㴫�ݱհ���
 */
#include "KtDfsIter.h"
template<typename SRC_GRAPH, typename CLS_GRAPH = SRC_GRAPH>
class KtTransitiveClosureDfs
{
public:
    KtTransitiveClosureDfs(const SRC_GRAPH& g) : g_(g.order()) {
        unsigned V = g.order();
        for(unsigned i = 0; i < V; i++)
            g_.addEdge(i, i);
                    
        for(size_t v = 0; v < V; v++) {
            KtDfsIter<SRC_GRAPH> iter(g, v);
            assert(iter.from() == -1);
            ++iter; // skip v-self
            while(!iter.isEnd()) {
                if(iter.isTree()) {
                    unsigned w = *iter;
                    g_.addEdge(v, w);
                }
                ++iter;
            }
        }
    }

    bool isReachable(unsigned v, unsigned w) const {
        return g_.hasEdge(v, w);
    }

private:
    CLS_GRAPH g_;
};


// DAG�Ĵ��ݱհ�������DFS�㷨ʵ�֡�
// ���KtClosureDfs����Ҫ����DAG�����Խ����Ż�.
//   1.û�лر�
//   2.�±߿��Ժ��ԣ������õݹ�չ����Ҳ���û��ݴ��ݱհ�
//   3.��߲��õݹ�չ����ֱ�ӻ��ݼ���
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
        while(!iter.isEnd()) {
            unsigned p = iter.from();
            if(p == -1) {
                ++iter; // skip edge(-1, v)
                continue;
            }

            unsigned v = *iter;
            if(!iter.isPopping()) 
                g_.addEdge(p, v);

            assert(!iter.isBack()); // DAGû�лر�

            if(iter.isCross() || iter.isPopping()) 
                 for(size_t w = 0; w < V; w++) 
                    if(g_.hasEdge(v, w)) g_.addEdge(p, w);

            ++iter;
        }     
    }

    bool isReachable(unsigned v, unsigned w) const {
        return g_.hasEdge(v, w);
    }

private:
    CLS_DAG g_;
};


/* 
  ����ǿ�����Ĵ��ݱհ�ʵ��.
 */
#include "KtStronglyConnected.h"
template<typename GRAPH>
class KtTransitiveClosureScc
{
public:
    KtTransitiveClosureScc(const GRAPH& g) : scc_(g) {
        // ��ÿ��ǿ��ͨ����Ϊһ�����㹹��DAG
        GRAPH K(scc_.count()); // TODO��use bit graph
        for(unsigned v = 0; v < g.order(); v++) {
            typename GRAPH::adj_vertex_iter iter(g, v);
            while(!iter.isEnd()) {
                if(scc_[v] != scc_[*iter]) // �����Ի�
                    K.addEdge(scc_[v], scc_[*iter]);
                ++iter;
            }
        }

        // ʹ�û���DAG�Ĵ��ݱհ��Ż��㷨
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
