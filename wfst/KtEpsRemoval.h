#pragma once
#include "KtEpsClosure.h"
#include "KtWfstHolder.h"
#include "core/KtAdjIter.h"


/*
 * Epsilon removal is an operation for converting an eps-NFA to its equivalent NFA without eps transitions.
 *
 * WFST-EpsilonRemoval(T)
 * 1   for each p in Q 
 * 2     E' <- { e in E[p] | (i(e), o(e)) != (eps, eps) } ;E'��ʼ��Ϊ���з�(eps, eps)�߼�
 * 3     for each (q, w') in EpsilonClosure(p)
 * 4       E' <- E' + { (p, x, y, w'*w, r) | (p, x, y, w, r) in E[q], (x, y) != (eps, eps) }
 * 5       if p in F 
 * 6         F' <- F' + { p }
 * 7         ��'(p) <- ��(p)
 * 8       if q in F
 * 9         if p not in F
 * 10          F <- F + { p }
 * 11        ��'(p) <- ��'(p) + (w' * ��(q))
 * 12  return T' = (F', E', ��')
 *
 */

template<typename FST>
class KtEpsRemoval : public kPrivate::KtWfstHolder<FST>
{
public:

	KtEpsRemoval(const FST& fst) : kPrivate::KtWfstHolder<FST>(fst.order()) {

		KtEpsClosure<FST> ec(fst); // ��ǰ����fst��epsilon�հ�
		std::vector<bool> visited(fst.order(), false); // ���ڱ���ѷ��ʵĽڵ�

		for (unsigned p = 0; p < fst.order(); p++) { // for each p in Q
			
			if (fst.isInitial(p)) 
				wfst().setInitial(p, fst.initialWeight(p));

			if (fst.isFinal(p)) {
				assert(!wfst().isFinal(p));
				wfst().setFinal(p, fst.finalWeight(p));
			}

			if (visited[p]) continue;

			auto iter1 = fst.adjIter(p);
			for (; !iter1.isEnd(); ++iter1) {
				auto& e1 = iter1.edge();
				if (!isEps(e1))
					wfst().addTrans(p, *iter1, e1);

				auto eci = KtAdjIter(ec.inside(), p);
				for (; !eci.isEnd(); ++eci) { // for each q in EpsilonClosure(p)
					auto q = *eci;

					assert(q != p); // TODO: ��δ���eps�Ի�,���ڳ���������ӵ�����

					if (visited[q]) continue; // ��֤ÿ����ֻ����һ��
					visited[q] = true;

					if (fst.isFinal(q)) {
						if(!wfst().isFinal(p))
							wfst().setFinal(p, weight_type::zero());

						wfst().setFinal(p,  wfst().finalWeight(p) + (eci.edge() * fst.finalWeight(q)));
					}

					auto iter2 = fst.adjIter(q);
					for (; !iter2.isEnd(); ++iter2) {
						auto& e2 = iter2.edge();
						if (!isEps(e2))
							wfst().addTrans(p, *iter2, e2.isym, e2.osym, e2.wt * eci.edge());
					}
				}// for each q in EpsilonClosure(p)
			} // for each e in E[p]
		}// for each p in Q
	}


private:

	static bool isEps(const edge_type& e) {
		return e.isym == wfst_type::eps && e.osym == wfst_type::eps;
	}
};

