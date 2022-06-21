#pragma once
#include <vector>
#include <assert.h>
#include "core/KtAdjIter.h"
#include "util/inverse.h"


/* 
 * the weight pushing operation moves the weights distributed over all the paths
 * to the initial states in a WFSA/WFST without changing its function.
 * the effect of weight look-ahead caused by pushing weights accelerates the
 * search process since the unpromising paths can be eliminated in the early 
 * stage of the search, and therefore the total processing time can be reduced.
 *
 * A general weight pushing algorithm comprises two steps.
 *   1st step: computes a potential for each state, which is obtained as the sum of 
 *             weights over all the paths originating from that state to some final states.
 *   2ed step: the weight for each transition is modified with the potential difference 
 *             between the source and destination states of the transition.
 *
 *
 *
 * WFST-Potential(T)
 * 1   for each q in Q
 * 2     if q in F
 * 3       V[q] <- r[q] <- ¦Ñ(q)
 * 4     else
 * 5       V[q] <- r[q] <- 0
 * 6   S <- F
 * 7   while S not empty
 * 8     q <- Head(s)
 * 9     Dequeue(S)
 * 10    R <- r[q] /// r[q] means the sum of the path weigths added since the last time q was popped
 * 11    r[q] <- 0
 * 12  for each e in E^-1[q] /// for each transition e incoming to state q
 * 13    if V[p[e]] != V[p[e]] + (R * w[e]) /// p[e] means the previous state of e
 * 14      V[p[e]] <- V[p[e]] + (R * w[e])
 * 15      r[p[e]] <- r[p[e]] + (R * w[e])
 * 16      if p[e] not in S
 * 17        Enqueue(S, p[e])
 * 18  return V
 *
 *
 * WFST-WeightPushing(T)
 * 1  V <- WFST-Potential(T)
 * 2  for each q in Q
 * 3    if q in I
 * 4      ¦Ë(q) <- ¦Ë(q) * V[q]
 * 5    for each e in E[q] 
 * 6      w[e] <- V[q]^-1 * w[e] * V[n[e]]
 * 7    if q in F 
 * 8      ¦Ñ(q) <- V[q]^-1 * ¦Ñ(q)
 *
 *
 */

template<typename WFST>
class KtWeightPushing
{
public:
	using weight_type = typename WFST::weight_type;
	using state_index_t = typename WFST::state_index_t;


	KtWeightPushing(WFST& fst) { // pushing weights in-place
		auto V = potential_(fst);
		for (state_index_t q = 0; q < V.size(); q++) {
			if (fst.isInitial(q))
				fst.setInitial(q, fst.initialWeight(q) * V[q]);

			auto iter = fst.adjIter(q);
			for (; !iter.isEnd(); ++iter) {
				// w[e] <- V[q]^-1 * w[e] * V[n[e]]
				auto w = iter.edge();
				w.wt = w.wt.leftDiv(V[q]) * V[*iter];
				iter.reedge(w);
			}
	
			if (fst.isFinal(q))
				fst.setFinal(q, fst.finalWeight(q).leftDiv(V[q]));
		}
	}


private:
	
	// TODO: use KtShortestPath
	std::vector<weight_type> potential_(WFST& fst) {

		std::vector<weight_type> V(fst.order(), weight_type::zero());
		std::vector<state_index_t> S;

		decltype(auto) F = fst.finals();
		for (const auto& f : F) {
			S.push_back(f.first);
			V[f.first] = f.second;
		}

		std::vector<weight_type> r(V);

		auto gR = inverse(fst);
		while (!S.empty()) {
			auto q = S.back(); S.pop_back();
			auto R = r[q]; r[q] = weight_type::zero();

			auto iterR = KtAdjIter(gR, q);
			for (; !iterR.isEnd(); ++iterR) { // for each e in E^-1[q]
				auto delta = R * iterR.edge().wt;
				auto pe = *iterR;
				auto Vn = V[pe] + delta;
				if(Vn != V[pe]) {
					V[pe] = Vn;
					r[pe] = r[pe] + delta;

					if (std::find(S.begin(), S.end(), pe) == S.end())
						S.push_back(pe);
				}
			}
		}

		return V;
	}
};

