#pragma once
#include <vector>
#include <string>
#include <assert.h>
#include "../test/test_util.h"
#include "KtDfsIterX.h" 
#include "KtBfsIter.h"
#include "KtWeightor.h"


template<typename GRAPH_BASE> 
class KtGraphImpl : public GRAPH_BASE
{
public:

    using super_ = GRAPH_BASE;
    using typename super_::edge_type;
    using typename super_::vertex_index_t;
    using typename super_::adj_vertex_iter;
    using typename super_::const_adj_vertex_iter;
    using typename super_::adj_vertex_iter_r;
    using typename super_::const_adj_vertex_iter_r;

    template<bool fullGraph = false, bool modeEdge = false, bool stopAtPopping = false>
    using dfs_iter = KtDfsIter<KtGraphImpl, fullGraph, modeEdge, stopAtPopping>;

    template<bool fullGraph = false, bool modeEdge = false, bool stopAtPopping = false>
    using const_dfs_iter = KtDfsIter<const KtGraphImpl, fullGraph, modeEdge, stopAtPopping>;

    template<bool fullGraph = false, bool modeEdge = false>
    using bfs_iter = KtBfsIter<KtGraphImpl, fullGraph, modeEdge>;

    template<bool fullGraph = false, bool modeEdge = false>
    using const_bfs_iter = KtBfsIter<const KtGraphImpl, fullGraph, modeEdge>;

    using edge_iter = bfs_iter<true, true>; 
    using const_edge_iter = const_bfs_iter<true, true>;


    // 导入基类的构造函数
    using super_::super_;

    // 导入基类的有关成员函数
    using super_::order;
    using super_::size;
    using super_::addEdge;
    using super_::hasEdge;
    using super_::getEdge;
    using super_::eraseEdge;
    using super_::isDigraph;
    using super_::indegree;
    using super_::outdegree;



    // 返回4个版本的邻接顶点迭代器
    auto adjIter(vertex_index_t v) const {
        return const_adj_vertex_iter(*this, v);
    }

    auto adjIter(vertex_index_t v) {
        return adj_vertex_iter(*this, v);
    }

    auto adjIterR(vertex_index_t v) const {
        return const_adj_vertex_iter_r(*this, v);
    }

    auto adjIterR(vertex_index_t v) {
        return adj_vertex_iter_r(*this, v);
    }


    // @GRAPH: 输出的图类型
    // @WEIGHTOR: 边权值转换函数子
    template<typename GRAPH, typename WEIGHTOR = KtWeightSelf<edge_type>>
    GRAPH copy() const {
        GRAPH g(order()); g.reserve(order(), size());
        const_edge_iter iter(*this, 0);
        for (; !iter.isEnd(); ++iter)
            g.addEdge(iter.from(), *iter, WEIGHTOR{}(iter.edge()));

        assert(g.size() == size());
        return g;
    }



    bool hasSelfloop() const {
        auto V = order();
        for(unsigned i = 0; i < V; i++) 
            if(hasEdge(i, i)) 
                return true;

        return false;
    }


    // 删除自环
    void eraseSelfloop() {
        auto V = order();
        for(unsigned i = 0; i < V; i++) 
            eraseEdge(i, i); 
    }


    // 一个有向图是DAG，当且仅当在使用DFS检查每条边时未遇到任何回边
    bool hasLoop() const {
        const_dfs_iter<true, true, true> iter(*this, 0);
        while(!iter.isEnd()) {
            if(iter.isBack())
                return true;
            ++iter;
        }       

        return false;
    }
    

    // 删除环
    void eraseLoop() {
        // 删除回边. 自环也是回边
        dfs_iter<true, true> iter(*this, 0);
        while(!iter.isEnd()) {
            if(iter.isBack())
                iter.erase();
            else
                ++iter;
        }
    }


    // 边集
    template<typename WEIGHTOR>
    auto edges() const {
        using edge_type = std::pair<std::pair<unsigned, unsigned>, typename WEIGHTOR::weight_type>;
        std::vector<edge_type> es; es.reserve(size());
        const_bfs_iter<true, true> iter(*this, 0); 
        for(; !iter.isEnd(); ++iter) 
            es.push_back({ { iter.from(), *iter }, WEIGHTOR{}(iter.edge()) });

        assert(es.size() == size());
        return es;
    }


    // 计算当前图的逆
    template<typename GRAPH>
    GRAPH inverse() const {
        GRAPH gR(order()); gR.reserve(order(), size());
        const_bfs_iter<true, true> iter(*this, 0);
        for (; !iter.isEnd(); ++iter)
            gR.addEdge(*iter, iter.from(), iter.edge());

        return gR;
    }


    // 有向无环图
    bool isDag() const { return isDigraph() && !hasLoop(); }


    // 判断是否连通图
    bool isConnected() const {
        const_bfs_iter<> iter(*this, 0);
        unsigned V(0);
        for (; !iter.isEnd(); ++iter)
            ++V;

        return V == order();
    }


    // 顶点v是否可达w，即是否存在一条从v到w的路径
    bool isReachable(vertex_index_t v, vertex_index_t w) const {
        const_bfs_iter<> iter(*this, v);
        for (; !iter.isEnd(); ++iter)
            if (*iter == w)
                return true;

        return false;
    }


    // 返回源点集合
    auto sources() const {
        std::vector<unsigned> s;
        for (unsigned v = 0; v < order(); v++)
            if (indegree(v) == 0)
                s.push_back(v);

        return s;
    }


    // 返回汇点集合
    auto sinks() const {
        std::vector<unsigned> s;
        for (unsigned v = 0; v < order(); v++)
            if (outdegree(v) == 0)
                s.push_back(v);

        return s;
    }


    // 桥也称关节边(articulation edge)，如果删除这条边将把一个连通图分解为不相交的两个子图。
    // 没有桥的图称为边连通。
    // 在任何DFS树中，一条树边v-w是一个桥，条件是当且仅当不存在回边将w的一个子孙与w的一个祖先相连接
    auto bridges() const {
        std::vector<std::pair<unsigned, unsigned>> res;
        KtDfsIterX<const KtGraphImpl, true> iter(*this, 0);
        while(!iter.isEnd()) { 
            if (iter.isBridge()) {
                unsigned from = iter.from();
                if(from != -1)
                    res.push_back({ from, *iter });
            }

            ++iter;
        }
            
        return res;
    }


    // 割点(cut-point)也称关节点(articulation point)，如果删除该顶点，将把一个连通图分解为至少两个不相交的子图
    // 没有割点的图称为重连通(Biconnected)或顶点连通
    // 重连通图中的每一对顶点，都由两条不相交的路径相连
    auto cutpoints() const {
        assert(isConnected());
        unsigned root(0);
        std::vector<unsigned> res;
        unsigned sons_of_root(0); // 根节点的子树数量
        KtDfsIterX<const KtGraphImpl, false> iter(*this, root);
        while (!iter.isEnd()) {
            unsigned p = iter.from();
            if (p == root && iter.isTree()) // 忽略popping状态 
                ++sons_of_root;

            if (p != -1 && p != root/*根节点在后面处理*/ && iter.isPopping()) {
                unsigned w = *iter;
                if (iter.lowIndex(w) >= iter.pushIndex(p))
                    res.push_back(p);
            }

            ++iter;
        }

        if (sons_of_root > 1) res.push_back(root);  // 如果根节点有多个子树，根节点为割点
        std::sort(res.begin(), res.end());
        res.erase(std::unique(res.begin(), res.end()), res.end());

        return res;
    }
};

