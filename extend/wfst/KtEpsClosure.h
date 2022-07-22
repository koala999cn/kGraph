#pragma once
#include <vector>
#include <assert.h>
#include "GraphX.h"
#include "core/KtWeightor.h"
#include "KtSpSingleSource.h"


// ���ݵ�Դ���·���㷨������WFST��epsilon�հ�

template<typename WFST, typename CLS_GRAPH = DigraphSx<typename WFST::weight_type>>
class KtEpsClosure : public KtHolder<CLS_GRAPH>
{
public:
	using weight_type = typename WFST::weight_type;
	using closure_graph_t = CLS_GRAPH;
	using super_ = KtHolder<CLS_GRAPH>;


	KtEpsClosure(const WFST& fst) : super_(fst.order()) {
		std::vector<bool> flags(fst.order(), false); // ���ڱ�Ƕ���v�Ƿ���eps����

		// ����fst�е�eps�ߵ�cg_
		auto iter = fst.transIter();
		for (; !iter.isEnd(); ++iter) {
			auto& e = iter.edge();
			if (e.isym == WFST::eps && e.osym == WFST::eps) {
				auto v = iter.from();
				flags[v] = true;
				inside().addEdge(v, *iter, e.wt);
			}
		}

		auto epsg = inside(); // ֻ����eps�ߵ�ͼ

		// ͨ����Դ���·���㷨����հ�
		for (unsigned v = 0; v < flags.size(); v++) {
			if (!flags[v]) continue;

			KtSpSingleSource<closure_graph_t, KtWeightSelf<weight_type>> sp(epsg, v);
			for (unsigned w = 0; w < fst.order(); w++)
				if (sp.reachable(w))
					if (!epsg.hasEdge(v, w))
						inside().addEdge(v, w, sp.distance(w));
					else
						inside().setEdge(v, w, sp.distance(w));
		}
	}
};
