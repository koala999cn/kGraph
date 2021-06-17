#pragma once
#include <vector>
#include <string>
#include <assert.h>
#include "../test/test_util.h"
#include "KtDfsIterX.h" 
#include "KtBfsIter.h"


template<typename GRAPH_BASE> 
class KtGraphImpl : public GRAPH_BASE
{
public:

    using typename GRAPH_BASE::value_type;
    using typename GRAPH_BASE::adj_vertex_iter;

    template<bool fullGraph = false, bool modeEdge = false, bool stopAtPopping = false>
    using dfs_iter = KtDfsIter<KtGraphImpl, fullGraph, modeEdge, stopAtPopping>;

    template<bool fullGraph = false, bool modeEdge = false>
    using bfs_iter = KtBfsIter<KtGraphImpl, fullGraph, modeEdge>;


    // 导入基类的构造函数
    using GRAPH_BASE::GRAPH_BASE;

	// 导入基类的有关成员函数
    using GRAPH_BASE::order;
    using GRAPH_BASE::size;
    using GRAPH_BASE::addEdge;
    using GRAPH_BASE::hasEdge;
    using GRAPH_BASE::getEdge;
    using GRAPH_BASE::isDigraph;


	template<typename GRAPH>
	GRAPH copy() const {
		GRAPH g(order());

		auto V = order();
		for (unsigned i = 0; i < V; i++) {
			unsigned jMax = isDigraph() ? V : i + 1;
			for (unsigned j = 0; j < jMax; j++)
				if (hasEdge(i, j)) {
					const auto& val = getEdge(i, j);
					g.addEdge(i, j, val);
					if (!isDigraph() && g.isDigraph()) // 支持无向图转有向图
						g.addEdge(j, i, val);
				}
		}

		return g;
	}



    bool hasSelfloop() const {
        auto V = order();
        for(unsigned i = 0; i < V; i++) 
            if(hasEdge(i, i)) 
                return true;

        return false;
    }


    void eraseSelfloop() {
        auto V = order();
        for(unsigned i = 0; i < V; i++) 
            if(hasEdge(i, i)) 
                eraseEdge(i, i);
    }


    // 一个有向图是DAG，当且仅当在使用DFS检查每条边时未遇到任何回边
    bool hasLoop() const {
        dfs_iter<true, true, true> iter(*this, 0);
        while(!iter.isEnd()) {
            if(iter.isBack())
                return true;
            ++iter;
        }       

        return false;
    }
    

    void eraseLoop() {
        // 删除回边. 自环也是回边
		dfs_iter<true, true> iter(*this, 0);
        while(!iter.isEnd()) {
            if(iter.isBack())
                eraseEdge(iter.from(), *iter);
            ++iter;
        }
    }


	// 返回边集合
	template<typename WEIGHTOR>
    auto edges() const {
		using edge_type = std::pair<std::pair<unsigned, unsigned>, typename WEIGHTOR::weight_type>;
        std::vector<edge_type> es; es.reserve(size());
        bfs_iter<true, true> iter(*this, 0); 
        for(; !iter.isEnd(); ++iter) 
			es.push_back({ { iter.from(), *iter }, WEIGHTOR{}(iter.value()) });

        assert(es.size() == size());
        return es;
    }


    // 计算当前图的逆
	template<typename GRAPH>
	GRAPH reverse() const {
		GRAPH gR(order());
        for(unsigned v = 0; v < order(); v++) {
            adj_vertex_iter iter(*this, v);
            while(!iter.isEnd()) {
                auto w = *iter;
                gR.addEdge(w, v, iter.value());
                ++iter;
            }
        }

		return gR;
    }


	// 判断是否连通图
	bool isConnected() const {
		bfs_iter<> iter(*this, 0);
		unsigned V(0);
		for (; !iter.isEnd(); ++iter)
			++V;

		return V == order();
	}



    // 桥也成为关节边(articulation edge)，如果删除这条边将把一个连通图分解为不相交的两个子图。
    // 没有桥的图称为边连通。
    // 在任何DFS树中，一条树边v-w是一个桥，条件是当且仅当不存在回边将w的一个子孙与w的一个祖先相连接
    auto bridges() const {
        std::vector<std::pair<unsigned, unsigned>> res;
        KtDfsIterX<KtGraphImpl, true> iter(*this, 0);
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
		KtDfsIterX<KtGraphImpl, false> iter(*this, root);
		while (!iter.isEnd()) {
			unsigned p = iter.from();
			if (p == root && iter.isTree()) // 忽略popping状态 
				++sons_of_root;

			if (p != -1 && p != root && iter.isPopping()) {
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

