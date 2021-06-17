#pragma once
#include <vector>
#include <iostream>
#include "../base/union_find_set.h"


// 最小生成树算法


template<typename GRAPH, typename WEIGHTOR = default_wtor<GRAPH>>
class KtMst
{
    static_assert(!GRAPH::isDigraph(), "KtMst cannot instantiated with Digraph.");

public:
    KtMst(const GRAPH& g) : 
        graph_(g),
        dist_(0.0) {
        assert(g.isConnected());
    }


    // 是否成功生成mst
    bool ok() const { return mst_.size() == graph_.order() - 1; }


    // 获取mst的第i条边，i < V - 1
    auto operator[](unsigned i) const {
        return mst_[i];
    }


    // mst的累加权值
    double dist() const {
        return dist_;
    }


    // 生成并返回MST的图表示，图类型为G
    template<typename G>
    G toGraph() const {
        G g(mst_.size() + 1);
        for(const auto& e : mst_) {
            unsigned v = e.first, w = e.second;
            g.addEdge(v, w, graph_.getEdge(v, w));
        }

        return g;
    }


protected:
    const GRAPH& graph_;
	std::vector<std::pair<unsigned, unsigned>> mst_; // mst的V-1条边
    double dist_; // mst的权值累计
};


// Prim是最简单的MST算法，而且对于稠密图也是首选方法。
// 算法的核心思想：维护一个图的剪切，它由树顶点（选择放入MST）和非树顶点（尚未选择放入MST）组成，
// 通过在MST中放置任意一个顶点，以此作为开始，再在MST中放入一条最小交叉边（它将非树顶点调整为树顶点），
// 并重复此操作V-1次，从而将所有顶点都放在树上。
// Prim算法的运行时间与V^2成正比，因此对于稠密图是线性的。
// @WEIGHTOR：从value到weight的转换子
// @minimum：为true则取最小权值，否则取最大权值
template<typename GRAPH, class WEIGHTOR = default_wtor<GRAPH>>
class KtMstPrim : public KtMst<GRAPH, WEIGHTOR>
{
	using weight_type = typename WEIGHTOR::weight_type;
    using super_ = KtMst<GRAPH, WEIGHTOR>;
	using super_::mst_;
	using super_::dist_;


public:
    KtMstPrim(const GRAPH& g) : super_{g} {
		WEIGHTOR wtor;
        unsigned min = -1;
        unsigned V = g.order();
        assert(super_::mst_.size() == 0);
        mst_.reserve(V - 1);
        std::vector<bool> non_mst(V, true); // 元素i标记顶点i是否不在mst树中
        std::vector<unsigned> fr(V, -1); // fr[i]存储与顶点i连接的最小权值边的顶点
		std::vector<weight_type> wt(V, WEIGHTOR{}.worst_weight); // wt[i]存储与顶点i连接的边的最小权值

        for(unsigned v = 0; min != 0; v = min) { // v表示新加入mst顶点集的顶点
            min = 0; // 用来存储此轮搜索中，与mst顶点集相连的最小权值交叉边顶点
            for(unsigned w = 1; w < g.order(); w++) {
                if(non_mst[w]) { // 遍历不在mst顶点集中的顶点，找到最小交叉边
                    if(g.hasEdge(v, w)) { // 只需判断新加入mst顶点v与w的权值是否更小
                        auto e = g.getEdge(v, w);
                        double p = wtor(e);
                        if(wtor.comp(p, wt[w]))
                            wt[w] = p, fr[w] = v;
                    }

                    if(wtor.comp(wt[w], wt[min])) min = w; // 发现一条更小的权值边，记录它
                }
            }

            if(min) {
                non_mst[min] = false;
                mst_.push_back({fr[min], min});
                dist_ = wtor.acc(dist_, wt[min]);
            }
        }
    }
};


// Kruskal遍历边来生成MST：
//   1. 首先将V个顶点看作V颗单顶点树;
//   2. 选择权值最小的边，合并两颗树；
//   3. 若构成环，则舍弃改条边;
//   4. 重复2，直至找到组成MST的V-1条边
// 算法时间复杂度E*lgE，其中排序开销占主导地位
template<typename GRAPH, typename WEIGHTOR = default_wtor<GRAPH>>
class KtMstKruskal : public KtMst<GRAPH, WEIGHTOR>
{
	using weight_type = typename WEIGHTOR::weight_type;
	using super_ = KtMst<GRAPH, WEIGHTOR>;
    using super_::mst_;
    using super_::dist_;

public:
    KtMstKruskal(const GRAPH& g) : super_{g} {
        auto all_edges = g.edges<WEIGHTOR>();

        // 降序排序，权值最小的边在尾部，方便出栈
        std::sort(begin(all_edges), end(all_edges),
			[](const auto& x1, const auto& x2) { return WEIGHTOR{}.comp(x2.second, x1.second); });

        const unsigned V = g.order();
        const unsigned E = g.size();
        union_find_set uf(V);
        assert(super_::mst_.size() == 0);
        mst_.reserve(V - 1);

        for(unsigned i = 0; i < E && mst_.size() < V - 1; i++) {
            const auto& e = all_edges.back();
            unsigned v = e.first.first, w = e.first.second;
            if(uf.unite(v, w)) { // 若合并成功，则一定无环
                mst_.push_back(e.first);
				dist_ = WEIGHTOR{}.acc(dist_, e.second);
            }
			all_edges.pop_back();
        }
    }
};



// Boruvka与Kruskal算法类似，都是从森林开始构建mst。
// 不同点在于Kruskal每次增加1条最小跨边，而Boruvka每次增加1批。
// 算法时间复杂度E*lgV
template<typename GRAPH, typename WEIGHTOR = default_wtor<GRAPH>>
class KtMstBoruvka : public KtMst<GRAPH, WEIGHTOR>
{
	using weight_type = typename WEIGHTOR::weight_type;
	using super_ = KtMst<GRAPH, WEIGHTOR>;
    using super_::mst_;
    using super_::dist_;
	using edges_type = decltype(GRAPH{}.edges<WEIGHTOR>());
	using edge_type = typename edges_type::value_type;

public:
    KtMstBoruvka(const GRAPH& g) : super_{g} {
		assert(super_::mst_.size() == 0);

        const unsigned V = g.order();
        auto edges = g.edges<WEIGHTOR>();
        mst_.reserve(V - 1);
        unsigned N; // 有效边数量
        std::vector<const edge_type*> b;
        union_find_set uf(V);

        // 从单顶点子树开始
        for(unsigned E = edges.size(); E != 0 && mst_.size() < V - 1; E = N) { 
            b.assign(V, nullptr);
            N = 0;

            // 搜索连接每两颗子树的最小跨边，存储到数组b中。
            for(unsigned h = 0; h < E; h++) { 
                const auto& e = edges[h];
                unsigned i = uf.find(e.first.first), j = uf.find(e.first.second);
                if(i == j) continue;
                edges[N] = e; // 复制有效边，等同于删除无效边
				if (b[i] == nullptr || WEIGHTOR{}.comp(e.second, b[i]->second)) b[i] = &edges[N];
				if (b[j] == nullptr || WEIGHTOR{}.comp(e.second, b[j]->second)) b[j] = &edges[N];
                ++N;
            }

            // 将存储在b中的最小跨边，按照查并集的模式添加到mst。
            for(unsigned h = 0; h < V; h++)
                if(b[h] != nullptr && uf.unite(b[h]->first.first, b[h]->first.second)) 
					mst_.push_back(b[h]->first), dist_ = WEIGHTOR{}.acc(dist_, b[h]->second);
        }
    }
};