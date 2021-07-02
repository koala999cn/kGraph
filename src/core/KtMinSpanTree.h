#pragma once
#include <vector>
#include <iostream>
#include "../base/union_find_set.h"


// ��С����������Ȩͼ����С��������һ������������Ȩ�����бߵ�Ȩֵ֮�ͣ�������������κ���������Ȩ��


template<typename GRAPH, typename WEIGHTOR = default_wtor<GRAPH>>
class KtMst
{
    static_assert(!GRAPH::isDigraph(), "KtMst cannot instantiated with Digraph.");

public:
    KtMst(const GRAPH& g) : 
        graph_(g),
        dist_(0.0) {
        assert(g.isConnected());
		mst_.reserve(g.order() - 1);
    }


    // �Ƿ�ɹ�����mst
    bool ok() const { return mst_.size() == graph_.order() - 1; }


    // ��ȡmst�ĵ�i���ߣ�i < V - 1
    auto operator[](unsigned i) const {
        return mst_[i];
    }


    // mst���ۼ�Ȩֵ
    double dist() const {
        return dist_;
    }


    // ���ɲ�����MST��ͼ��ʾ��ͼ����ΪG
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
	std::vector<std::pair<unsigned, unsigned>> mst_; // mst��V-1����
    double dist_; // mst��Ȩֵ�ۼ�
};


// Prim����򵥵�MST�㷨�����Ҷ��ڳ���ͼҲ����ѡ������
// �㷨�ĺ���˼�룺ά��һ��ͼ�ļ��У����������㣨ѡ�����MST���ͷ������㣨��δѡ�����MST����ɣ�
// ͨ����MST�з�������һ�����㣬�Դ���Ϊ��ʼ������MST�з���һ����С����ߣ����������������Ϊ�����㣩��
// ���ظ��˲���V-1�Σ��Ӷ������ж��㶼�������ϡ�
// Prim�㷨������ʱ����V^2�����ȣ���˶��ڳ���ͼ�����Եġ�
// @WEIGHTOR����value��weight��ת����
// @minimum��Ϊtrue��ȡ��СȨֵ������ȡ���Ȩֵ
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
        assert(mst_.size() == 0);
        std::vector<bool> non_mst(V, true); // Ԫ��i��Ƕ���i�Ƿ���mst����
        std::vector<unsigned> fr(V, -1); // fr[i]�洢�붥��i���ӵ���СȨֵ�ߵĶ���
		std::vector<weight_type> wt(V, WEIGHTOR{}.worst_weight); // wt[i]�洢�붥��i���ӵıߵ���СȨֵ

        for(unsigned v = 0; min != 0; v = min) { // v��ʾ�¼���mst���㼯�Ķ���
            min = 0; // �����洢���������У���mst���㼯��������СȨֵ����߶���
            for(unsigned w = 1; w < g.order(); w++) {
                if(non_mst[w]) { // ��������mst���㼯�еĶ��㣬�ҵ���С�����
                    if(g.hasEdge(v, w)) { // ֻ���ж��¼���mst����v��w��Ȩֵ�Ƿ��С
                        auto e = g.getEdge(v, w);
                        double p = wtor(e);
                        if(wtor.comp(p, wt[w]))
                            wt[w] = p, fr[w] = v;
                    }

                    if(wtor.comp(wt[w], wt[min])) min = w; // ����һ����С��Ȩֵ�ߣ���¼��
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


// Kruskal������������MST��
//   1. ���Ƚ�V�����㿴��V�ŵ�������;
//   2. ѡ��Ȩֵ��С�ıߣ��ϲ���������
//   3. �����ɻ���������������;
//   4. �ظ�2��ֱ���ҵ����MST��V-1����
// �㷨ʱ�临�Ӷ�E*lgE������������ռ������λ
template<typename GRAPH, typename WEIGHTOR = default_wtor<GRAPH>>
class KtMstKruskal : public KtMst<GRAPH, WEIGHTOR>
{
	using weight_type = typename WEIGHTOR::weight_type;
	using super_ = KtMst<GRAPH, WEIGHTOR>;
    using super_::mst_;
    using super_::dist_;

public:
    KtMstKruskal(const GRAPH& g) : super_{g} {
        auto all_edges = g.template edges<WEIGHTOR>();

        // ��������Ȩֵ��С�ı���β���������ջ
        std::sort(std::begin(all_edges), std::end(all_edges),
			[](const auto& x1, const auto& x2) { return WEIGHTOR{}.comp(x2.second, x1.second); });

        const unsigned V = g.order();
        const unsigned E = g.size();
        union_find_set uf(V);
        assert(super_::mst_.size() == 0);

        for(unsigned i = 0; i < E && mst_.size() < V - 1; i++) {
            const auto& e = all_edges.back();
            unsigned v = e.first.first, w = e.first.second;
            if(uf.unite(v, w)) { // ���ϲ��ɹ�����һ���޻�
                mst_.push_back(e.first);
				dist_ = WEIGHTOR{}.acc(dist_, e.second);
            }
			all_edges.pop_back();
        }
    }
};



// Boruvka��Kruskal�㷨���ƣ����Ǵ�ɭ�ֿ�ʼ����mst��
// ��ͬ������Kruskalÿ������1����С��ߣ���Boruvkaÿ������1����
// �㷨ʱ�临�Ӷ�E*lgV
template<typename GRAPH, typename WEIGHTOR = default_wtor<GRAPH>>
class KtMstBoruvka : public KtMst<GRAPH, WEIGHTOR>
{
	using weight_type = typename WEIGHTOR::weight_type;
	using super_ = KtMst<GRAPH, WEIGHTOR>;
    using super_::mst_;
    using super_::dist_;
	using edges_type = decltype(GRAPH{}.template edges<WEIGHTOR>());
	using edge_type = typename edges_type::value_type;

public:
    KtMstBoruvka(const GRAPH& g) : super_{g} {
		assert(mst_.size() == 0);

        const unsigned V = g.order();
        auto edges = g.template edges<WEIGHTOR>();
        unsigned N; // ��Ч������
        std::vector<const edge_type*> b;
        union_find_set uf(V);

        // �ӵ�����������ʼ
        for(unsigned E = edges.size(); E != 0 && mst_.size() < V - 1; E = N) { 
            b.assign(V, nullptr);
            N = 0;

            // ��������ÿ������������С��ߣ��洢������b�С�
            for(unsigned h = 0; h < E; h++) { 
                const auto& e = edges[h];
                unsigned i = uf.find(e.first.first), j = uf.find(e.first.second);
                if(i == j) continue;
                edges[N] = e; // ������Ч�ߣ���ͬ��ɾ����Ч��
				if (b[i] == nullptr || WEIGHTOR{}.comp(e.second, b[i]->second)) b[i] = &edges[N];
				if (b[j] == nullptr || WEIGHTOR{}.comp(e.second, b[j]->second)) b[j] = &edges[N];
                ++N;
            }

            // ���洢��b�е���С��ߣ����ղ鲢����ģʽ���ӵ�mst��
            for(unsigned h = 0; h < V; h++)
                if(b[h] != nullptr && uf.unite(b[h]->first.first, b[h]->first.second)) 
					mst_.push_back(b[h]->first), dist_ = WEIGHTOR{}.acc(dist_, b[h]->second);
        }
    }
};


#include "KtPfsIter.h"
template<typename GRAPH, typename WEIGHTOR = default_wtor<GRAPH>>
class KtMstPfs : public KtMst<GRAPH, WEIGHTOR>
{
	using value_type = typename GRAPH::value_type;
	using weight_type = typename WEIGHTOR::weight_type;
	using super_ = KtMst<GRAPH, WEIGHTOR>;
	using super_::mst_;
	using super_::dist_;
	using edges_type = decltype(GRAPH{}.template edges<WEIGHTOR>());
	using edge_type = typename edges_type::value_type;

public:
	KtMstPfs(const GRAPH& g) : super_{ g } {
		assert(super_::mst_.size() == 0);
		struct Prior {
			auto operator()(unsigned, unsigned, const value_type& val) {
				return WEIGHTOR{}(val);
			}
		};

		struct Comp {
			auto operator()(const weight_type& a, const weight_type& b) {
				return WEIGHTOR{}.comp(a, b);
			}
		};


		KtPfsIter<GRAPH, Prior, Comp> iter(g, 0);
		++iter; // skip v0
		for (; !iter.isEnd() ; ++iter) {
			if (!iter.isPopped(*iter)) {
				mst_.push_back(iter.edge());
				dist_ = WEIGHTOR{}.acc(dist_, iter.prior());

				if (mst_.size() == g.order() - 1)
					break;
			}
		}
	}
};