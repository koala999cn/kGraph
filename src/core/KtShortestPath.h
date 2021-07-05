#pragma once
#include<vector>
#include "KtWeightor.h"


// 最短路径算法
// 此类算法的关键是顶点处理的顺序，对应有顺序遍历、Dfs、Bfs、Pfs、Ts等方法。
// 不同实现的时间复杂度如下：
// 单源：
//   Dijkstra，适用于非负权值，O(V^2)，稠密图最优
//   PFS，适用于非负权值，O(E*lgV)，保守边界值
//   TS，适用于无环图，O(E)，最优
//   Bellman-Ford，适用于无负环图，O(V*E)，有改进余地吗？
// 全源：
//    Dijkstra，适用于非负权值，O(V^3)，对所有图均相同
//    PFS，适用于非负权值，O(V*E*lgV)，保守边界值
//    DFS，适用于无环图，O(V*E)，对所有图均相同
//    Floyd，适用于无负环图，O(V^3)，对所有图均相同
//    Johnson，适用于无负环图，O(V*E*lgV)，保守边界值
// 注：由于权值的零值和偏移转换难以统一，Johnson尚未实现



// 单源最短路径的基类：计算起始顶点v0（源点）到图g中其他各顶点的最短路径
template<typename GRAPH, class WEIGHTOR>
class KtSpt
{
    static_assert(GRAPH::isDigraph(), "KvSpt must be instantiated with Digraph.");
    using weight_type = typename WEIGHTOR::weight_type;

public:
    KtSpt(const GRAPH& g, unsigned v0) :
        v0_(v0),
        spt_(g.order(), -1),
        dist_(g.order(), WEIGHTOR{}.worst_weight) {}

    // 返回从源点到顶点v的最短路径(逆序)
    auto pathR(unsigned v) const {
        std::vector<unsigned> p;
        unsigned s = v;
        do {
            p.push_back(s);
            s = spt_[s];
        } while (s != v0_ && s != v && s != -1);

        // s == -1，表示没有环路，路径入栈成功
        // s == v0，表示v0存在环路，需要手动将v0入栈
        // s == v，表示存在经过v的负环，这时丢失了从v0->v的最短路径，栈中路径退化为以v为起止点的负环。
        if(s != -1) p.push_back(s);

        return p;
    }


    // 返回从源点到顶点v的最短距离
    weight_type dist(unsigned v) const {
        return dist_[v];
    }


protected:

    // 边松弛. 判断v0经过边(v, w)到w的路径是否比当前到w的路径更好
    // wt为边(v, w)的权值
    bool relax_(unsigned v, unsigned w, weight_type wt) {
        if (spt_[v] != -1) {
            if (v != v0_)
                wt = WEIGHTOR{}.acc(wt, dist_[v]);
        }
        else if (v != v0_)
            return false;

        if(WEIGHTOR{}.comp(wt, dist_[w])) {
            dist_[w] = wt, spt_[w] = v;
            return true;
        }

        return false;
    }

protected:
    unsigned v0_; // 源点
    std::vector<unsigned> spt_; // (spt_[i], i)表示从源点v0_到顶点i的最短路径上的最后一条边
    std::vector<weight_type> dist_; // dist_[i]表示从源点v0_到顶点i的最短距离
};


// 基于广度优先搜索的单源最短路径实现
// 处理E较大的有环图时，速度非常慢
// 处理DAG较有优势，貌似比TS方法还快
// NOTE: 当有负环的时候，会陷入无限递归
#include "KtBfsIter.h"
template<typename GRAPH, class WEIGHTOR = default_wtor<GRAPH>>
class KtSptBfs : public KtSpt<GRAPH, WEIGHTOR>
{
public:
    KtSptBfs(const GRAPH& g, unsigned v0) : KtSpt<GRAPH, WEIGHTOR>(g, v0) {
        bfs_(g, v0);
    }

private:

    void bfs_(const GRAPH& g, unsigned v) {
        KtBfsIter<GRAPH, false, true> iter(g, v);
        for (; !iter.isEnd(); ++iter) {
            unsigned v = iter.from(), w = *iter;

            // 如果relax_更新了已出栈的顶点信息，则需要对出栈顶点重新bfs
            if (KtSpt<GRAPH, WEIGHTOR>::relax_(v, w, WEIGHTOR{}(iter.value())) && iter.isPopped(w))
                bfs_(g, w); 
        }
    }
};


// 速度比bfs还慢，慢很多，弃疗
#include "KtDfsIter.h"
template<typename GRAPH, class WEIGHTOR = default_wtor<GRAPH>>
class KtSptDfs : public KtSpt<GRAPH, WEIGHTOR>
{
public:
    KtSptDfs(const GRAPH& g, unsigned v0) : KtSpt<GRAPH, WEIGHTOR>(g, v0) {
        dfs_(g, v0);
    }

private:
    void dfs_(const GRAPH& g, unsigned v) {
        KtDfsIter<GRAPH, false, true> iter(g, v);
        for (; !iter.isEnd(); ++iter) {
            unsigned v = iter.from(), w = *iter;

            if (KtSpt<GRAPH, WEIGHTOR>::relax_(v, w, WEIGHTOR{}(iter.value())))
                if (!iter.isTree() && w != v)
                    dfs_(g, w); // TODO: 区分不同类型的边（跨边，下边，回边），进一步优化
        }
    }
};



// 基于Dijkstra算法的单源最短路径实现
// NOTE: 该算法不支持负环，也不支持负权值，因为一旦出现负权值，
// 可能会出现一条比之前更短的路径，这会破坏始终延着最短路径生长的准则
// 也就是说，我们无法判断当前的最短路径是不是真的最短，因为后面出现的负权值可能会让其他路径更短
template<typename GRAPH, class WEIGHTOR = default_wtor<GRAPH>>
class KtSptDijkstra : public KtSpt<GRAPH, WEIGHTOR>
{
    using super_ = KtSpt<GRAPH, WEIGHTOR>;
    using super_::v0_;
    using super_::spt_;
    using super_::dist_;

public:
    KtSptDijkstra(const GRAPH& g, unsigned v0) : super_(g, v0) {

        std::vector<bool> vis(g.order(), false); // 用于标记源点v0到顶点i的最短路径是否已计算

        unsigned v = v0;
        while (v != -1) { 
            vis[v] = true;

            // 边松弛
            for (unsigned w = 0; w < g.order(); w++)
                if ((!vis[w] || w == v0_/*允许计算v0到自身环路的最短路径*/) && g.hasEdge(v, w))
                    KtSpt<GRAPH, WEIGHTOR>::relax_(v, w, WEIGHTOR{}(g.getEdge(v, w)));


            // 在vis[i]等于false的集合中，寻找距离v0路径最优的顶点. TODO: 使用优先队列实现
            v = -1;
            for (unsigned w = 0; w < g.order(); w++)
                if (!vis[w] && (v == -1 || WEIGHTOR{}.comp(dist_[w], dist_[v])))
                    v = w;

            if (v == -1 || spt_[v] == -1/*与v0不连通*/) break; // all done
        }
    }
};



template<typename GRAPH, class WEIGHTOR = default_wtor<GRAPH>>
class KtSptPfs : public KtSpt<GRAPH, WEIGHTOR>
{
    using super_ = KtSpt<GRAPH, WEIGHTOR>;
    using super_::v0_;
    using super_::dist_;

public:
    KtSptPfs(const GRAPH& g, unsigned v0) : super_(g, v0) {
        using element_type = std::pair<unsigned, typename WEIGHTOR::weight_type>;
        struct Comp {
            bool operator()(const element_type& a, const element_type& b) {
                return WEIGHTOR{}.comp(b.second, a.second); // 优先队列的比较函数需要反过来比较，才能保证最优元素在队列顶端
            }
        };

        using priority_queue = std::priority_queue<element_type, std::vector<element_type>, Comp>;
        priority_queue pq; pq.emplace(v0, dist_[v0]);
        std::vector<bool> vis(g.order(), false); // 用于标记源点v0到顶点i的最短路径是否已计算

        while (!pq.empty()) {

            unsigned v = pq.top().first; pq.pop();
            if (vis[v]) continue;
            vis[v] = true;

            // 边松弛
            for (unsigned w = 0; w < g.order(); w++)
                if ((!vis[w] || w == v0_/*允许计算v0到自身环路的最短路径*/) && g.hasEdge(v, w))
                    if (KtSpt<GRAPH, WEIGHTOR>::relax_(v, w, WEIGHTOR{}(g.getEdge(v, w))))
                        pq.emplace(w, dist_[w]);
        }
    }
};


// 基于拓扑排序算法的单源最短路径实现，仅适用于DAG.
// 运行速度没有想像中快，效率还比不上bfs方法
#include "KtTopologySort.h"
template<typename DAG, class WEIGHTOR = default_wtor<DAG>>
class KtSptTs : public KtSpt<DAG, WEIGHTOR>
{
public:
    KtSptTs(const DAG& g, unsigned v0) : KtSpt<DAG, WEIGHTOR>(g, v0) {
        assert(!g.hasLoop());
        KtTopologySort<DAG> ts(g);
        unsigned j = ts.relabel(v0); // j之前的顶点可以忽略，因为按照拓扑排序，v0与它们没有可达路径
        for(unsigned v = ts[j++]; j < g.order(); v = ts[j++]) {
            typename DAG::adj_vertex_iter iter(g, v);
            while(!iter.isEnd()) {
                KtSpt<DAG, WEIGHTOR>::relax_(v, *iter, WEIGHTOR{}(iter.value()));
                ++iter;
            }
        }
    }
};



// 基于Bellman-Ford算法的单源最短路径实现
// NOTE: 可用来解决包含负权值的单源最短路径问题，时间复杂度O(VE)。
// 对于稠密图，其运行时间不比Floyd算法更好，Floyd算法则要找出全部最短路径，而不仅仅是单源。
// 对于稀疏图，其运行时间最多比Floyd算法快V倍，但对于无负权值的图，其运行时间比Dijkstra算法慢约V倍。
template<typename GRAPH, class WEIGHTOR = default_wtor<GRAPH>>
class KtSptBellmanFord : public KtSpt<GRAPH, WEIGHTOR>
{
public:
    KtSptBellmanFord(const GRAPH& g, unsigned v0) : KtSpt<GRAPH, WEIGHTOR>(g, v0) {
        std::queue<unsigned> q;
        unsigned V = g.order();
        q.push(v0); q.push(V); // 标记值V将当前一批顶点与下一批顶点分隔，并使得可以在V遍处理后终止。
        unsigned N = 0;
        while (!q.empty()) {
            unsigned v = q.front(); q.pop();
            while (v == V) {
                if (N++ > V) return; // 存在一条长度超过V的最短路径，这表示肯定存在负环。
                q.push(V);
                v = q.front(); q.pop();
            }

            typename GRAPH::adj_vertex_iter iter(g, v);
            for (; !iter.isEnd(); ++iter)
                if (KtSpt<GRAPH, WEIGHTOR>::relax_(v, *iter, WEIGHTOR{}(iter.value()))
                    && *iter != v/*忽略自环顶点，避免重复展开*/)
                    q.push(*iter);
        }
    }
};



// 全源最短路径的基类：计算任意顶点到图中其他各顶点的最短路径
template<typename GRAPH, class WEIGHTOR>
class KtAllSpt
{
    using weight_type = typename WEIGHTOR::weight_type;

public:
    KtAllSpt(const GRAPH& g) 
        : spt_(g.order(), std::vector<unsigned>(g.order(), -1)),
        dst_(g.order(), std::vector<weight_type>(g.order(), WEIGHTOR{}.worst_weight)) {}


    weight_type dist(unsigned v, unsigned w) const {
        return dst_[v][w];
    }


    // 返回v到w的最短路径(逆序)
    auto pathR(unsigned v, unsigned w) const {
        std::vector<unsigned> p;
        unsigned s = w;
        do {
            p.push_back(s);
            s = spt_[v][s];
        } while (s != v && s != w && s != -1 );
        if (s != -1) p.push_back(s);

        return p;
    }


protected:

    // 路径松弛. 判断s经过x再到t的路径是否比当前s到t的路径更好
    bool relax_(unsigned s, unsigned t, unsigned x) {
        assert(x != s && x != t); // s可能等于t，用于计算最短环路
        const auto dst = WEIGHTOR{}.acc(dst_[s][x], dst_[x][t]);
        if (WEIGHTOR{}.comp(dst, dst_[s][t])) { // dst比dst_[s][t]更优
            dst_[s][t] = dst;
            spt_[s][t] = spt_[x][t];
            return true;
        }

        return false;
    }


protected:

    template<typename T> using vector = std::vector<T>;
    vector<vector<unsigned>> spt_;
    vector<vector<weight_type>> dst_;
};



// Floyd实现全源最短路径
// 时间复杂度O(V^3)，适用于稠密图
// 对于稀疏图，可逐个顶点用Dijkstra算法计算单源最短路径，从而得到全源最短路径
// NOTE: 该算法未对权值作任何假设，因此即使存在负权值也是有效的。
// 如果不存在负环，则会计算出最短路径；否则，只能找出一条不包过负环的最短路径。
template<typename GRAPH, class WEIGHTOR = default_wtor<GRAPH>>
class KtAllSptFloyd : public KtAllSpt<GRAPH, WEIGHTOR>
{
    using super_ = KtAllSpt<GRAPH, WEIGHTOR>;
    using super_::spt_;
    using super_::dst_;

public:
    KtAllSptFloyd(const GRAPH& g) : super_(g) {

        // 按边初始化spt_和dst_
        KtBfsIter<GRAPH, true, true> iter(g, 0);
        for (; !iter.isEnd(); ++iter) {
            unsigned v = iter.from(), w = *iter;
            spt_[v][w] = v;
            dst_[v][w] = WEIGHTOR{}(iter.value());
        }

        const unsigned V = g.order();

        // 判定路径s->x->t是否比路径s->t更优
        for (unsigned x = 0; x < V; x++)
            for (unsigned s = 0; s < V; s++)
                if (spt_[s][x] != -1 && s != x) // 若s->x不通，或者s==x，则略过
                    for (unsigned t = 0; t < V; t++)
                        if(t != x) KtAllSpt<GRAPH, WEIGHTOR>::relax_(s, t, x);
    }
};



// DFS实现全源最短路径，仅适用于DAG
// 时间复杂度O(V*E)
// 速度比TS方法快很多
template<typename GRAPH, class WEIGHTOR = default_wtor<GRAPH>>
class KtAllSptDfs : public KtAllSpt<GRAPH, WEIGHTOR>
{
    using super_ = KtAllSpt<GRAPH, WEIGHTOR>;
    using super_::spt_;
    using super_::dst_;

public:
    KtAllSptDfs(const GRAPH& g) : super_(g), done_(g.order(), false) {
        assert(!g.hasLoop());

        for (unsigned v = 0; v < g.order(); v++)
            if(!done_[v]) dfs_(g, v);
    }

private:
    void dfs_(const GRAPH& g, unsigned v) {
        done_[v] = true;

        typename GRAPH::adj_vertex_iter iter(g, v);
        for (; !iter.isEnd(); ++iter) {
            unsigned w = *iter;
            auto wt = WEIGHTOR{}(iter.value());
            if (WEIGHTOR{}.comp(wt, dst_[v][w])) {
                dst_[v][w] = wt;
                spt_[v][w] = v;
            }

            if (!done_[w]) dfs_(g, w);

            if (w != v) {
                for (unsigned i = 0; i < g.order(); i++)
                    if (spt_[w][i] != -1 && w != i)
                        KtAllSpt<GRAPH, WEIGHTOR>::relax_(v, i, w);
            }
        }
    }

private:
    std::vector<bool> done_;
};

