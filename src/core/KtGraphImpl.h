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


    // �������Ĺ��캯��
    using GRAPH_BASE::GRAPH_BASE;

	// ���������йس�Ա����
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
					if (!isDigraph() && g.isDigraph()) // ֧������ͼת����ͼ
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


    // һ������ͼ��DAG�����ҽ�����ʹ��DFS���ÿ����ʱδ�����κλر�
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
        // ɾ���ر�. �Ի�Ҳ�ǻر�
		dfs_iter<true, true> iter(*this, 0);
        while(!iter.isEnd()) {
            if(iter.isBack())
                eraseEdge(iter.from(), *iter);
            ++iter;
        }
    }


	// ���ر߼���
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


    // ���㵱ǰͼ����
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


	// �ж��Ƿ���ͨͼ
	bool isConnected() const {
		bfs_iter<> iter(*this, 0);
		unsigned V(0);
		for (; !iter.isEnd(); ++iter)
			++V;

		return V == order();
	}



    // ��Ҳ��Ϊ�ؽڱ�(articulation edge)�����ɾ�������߽���һ����ͨͼ�ֽ�Ϊ���ཻ��������ͼ��
    // û���ŵ�ͼ��Ϊ����ͨ��
    // ���κ�DFS���У�һ������v-w��һ���ţ������ǵ��ҽ��������ڻر߽�w��һ��������w��һ������������
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


	// ���(cut-point)Ҳ�ƹؽڵ�(articulation point)�����ɾ���ö��㣬����һ����ͨͼ�ֽ�Ϊ�����������ཻ����ͼ
	// û�и���ͼ��Ϊ����ͨ(Biconnected)�򶥵���ͨ
	// ����ͨͼ�е�ÿһ�Զ��㣬�����������ཻ��·������
	auto cutpoints() const {
		assert(isConnected());
		unsigned root(0);
		std::vector<unsigned> res;
		unsigned sons_of_root(0); // ���ڵ����������
		KtDfsIterX<KtGraphImpl, false> iter(*this, root);
		while (!iter.isEnd()) {
			unsigned p = iter.from();
			if (p == root && iter.isTree()) // ����popping״̬ 
				++sons_of_root;

			if (p != -1 && p != root && iter.isPopping()) {
				unsigned w = *iter;
				if (iter.lowIndex(w) >= iter.pushIndex(p))
					res.push_back(p);
			}

			++iter;
		}

		if (sons_of_root > 1) res.push_back(root);  // ������ڵ��ж�����������ڵ�Ϊ���
		std::sort(res.begin(), res.end());
		res.erase(std::unique(res.begin(), res.end()), res.end());

		return res;
	}
};

