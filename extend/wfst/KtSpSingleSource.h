#pragma once
#include <vector>
#include <unordered_set>
#include "core/KtAdjIter.h"


// 对WFST重新实现最短路径算法
// 参考：Weighted Automata Algorithms, Mehryar Mohri, 2009


/*
 * 单源最短路径算法
 *
 * GEN-SINGLE-SOURCE(G, s)
 * 1  for each q in Q
 * 2    d[q] <- r[q] <- 0
 * 3  d[s] <- r[s] <- 1
 * 4  S <- {s}
 * 5  while S not empty
 * 6    q <- Head(s)
 * 7    Dequeue(S)
 * 8    R <- r[q] /// r[q] means the sum of the path weigths added since the last time q was popped
 * 9    r[q] <- 0
 * 10   for each e in E[q] /// for each transition e outgoing from state q
 * 11     if d[n[e]] != d[n[e]] + (R * w[e]) /// n[e] means the "to"(next) state of e
 * 12       d[n[e]] <- d[n[e]] + (R * w[e])
 * 13       r[n[e]] <- r[n[e]] + (R * w[e])
 * 14       if n[e] not in S
 * 15         Enqueue(S, n[e])
 *
 */

template<typename GRAPH, typename WEIGHTOR>
class KtSpSingleSource
{
public:
	using vertex_index_t = typename GRAPH::vertex_index_t;
	using weight_type = typename GRAPH::edge_type;


	KtSpSingleSource(const GRAPH& g, vertex_index_t s)
		: dist_(g.order(), weight_type::zero())
	    , s_(s) {

		dist_[s] = weight_type::one(); // TODO: dist_[s]有必要置1吗
		std::vector<weight_type> r(dist_);

		std::unordered_set<vertex_index_t> S;
		S.insert(s);

		while (!S.empty()) {
			auto pos = S.begin();
			auto q = *pos; S.erase(pos);
			auto R = r[q]; r[q] = weight_type::zero();

			auto iter = KtAdjIter(g, q);
			for (; !iter.isEnd(); ++iter) { // for each e in E[q]
				auto delta = R * WEIGHTOR{}(iter.edge());
				auto ne = *iter; // n[e]
				auto dne = dist_[ne] + delta; // dne =d[n[e]]
				if (dne != dist_[ne]) {
					dist_[ne] = dne;
					r[ne] = r[ne] + delta;

					if (S.find(ne) == S.end())
						S.insert(ne);
				}
			}
		}
	}


	const weight_type& distance(vertex_index_t t) const {
		return dist_[t];
	}


	// 是否存在从s到t的路径
	bool reachable(vertex_index_t t) const {
		return dist_[t] != weight_type::zero() && (dist_[t] != weight_type::one() || t != s_);
	}

private:
	vertex_index_t s_;
	std::vector<weight_type> dist_;
};
