#pragma once
#include <vector>
#include <set>
#include <map>
#include <assert.h>
#include "../common/stlex.h"
#include "util/inverse.h"
#include "core/KtAdjIter.h"
#include "KtWfstHolder.h"


/* 
 * Minimization is an algorithm designed to minimize the number of states for any DFA.
 * The minimization for WFSTs is described in [Moh09], and consists of two steps:
 *   1. Push weights and output labels to the initial states in the WFST, and
 *   2. Minimize the WFST using a classical minimization algorithm assumin that
 *      the triplet "input:output/weight" on each transition is one single label.
 *
 * classical minimization algorithm basically obtain a partition of the set of states 
 * in the automation, i.e., hte set of states is divided into non-overlapping and 
 * non-empty blocks. The partition is determined so that each block includes equivalent
 * states that are not distinguished from each other, i.e., they accept the identical
 * set of symbol sequences along the paths from those states to some final states.
 *
 * Once the partition is obtained, all the states are replaced with a new set of states
 * each of which corresponds to a block of equivalent states in the partition.
 * Transitons outgoing from all states in a block are bound by their labels, and they 
 * are redirected for the new states, where the state indices are replaced with those
 * for the blocks to which the original states belong.
 *
 * Hopcrofts's algorithm is a well-known algorithm for efficiently minimizing DFAs, whose
 * computational complexity is O(|E|log|Q|).
 *
 *
 * WFST-Minimization(T)
 * 1   P <- W <- empty ; P is parition, W is a queue containing the blocks used to split each block in P 
 * 2   for each �� in { ��(f) | f in F }
 * 3     F' <- { f | ��(f) = ��, f in F } ;F'��ʾȨֵ��ͬ����̬��  
 * 4     P <- P + { F' } ;���F'Ϊ�»���
 * 5     Enqueue(W, F') ;��F'ѹ����������
 * 6   P <- P + { Q-F } ;��ӷ���̬��Ϊ�»���
 * 7   while W not empty
 * 8     S <- Head(W), Dequeue(W)
 * 9     for each (i, o, w) in { (i[e], o[e], w[e]) | e in E^-1[S] } ;E^-1[S]����S״̬����ߵļ���
 * 10      R(i, o, w) <- { p(e) | i[e] = i, o[e] = o, w[e] = w, e in E^-1[S] } ;��(i, o, w)��Ԫ����ͬ�ıߵ����ۺ�ΪR(i, o, w), ˵����R��(i,o,w)ӳ�䵽{p[e]}
 * 11      for each B in P such that B & R(i, o, w) != empty and B !include in R(i, o, w) 
 * 12        B1 <- B & R(i, o, w)
 * 13        B2 <- B - B1 
 * 14        P <- (P - {B}) + {B1, B2}
 * 15        if B in W
 * 16          Erase(W, B), Enqueue(W, B1), Enqueue(W, B2)
 * 17        else
 * 18          if |B1| <= |B2|
 * 19            Enqueue(W, B1)
 * 20          else
 * 21            Enqueue(W, B2)
 * 22  for each e in E
 * 23    E' <- E' + {(B(p[e]), i[e], o[e], w[e], B(n[e]))} ;�����µı߼�
 * 24  for each S in Q' such that S include in F
 * 25    F' <- F' + {S} ;�����µ���̬��
 * 26    ��'(S) <- ��(q) for some q in S
 * 27  return T' = (Q', F', E', ��')
 *
 */

template<typename FST>
class KtMinimization : public kPrivate::KtWfstHolder<FST>
{
public:
	using state_type = std::set<state_index_t>;


	KtMinimization(const FST& fst) : infst_(fst) {

		/// �Ȼ�����ֹ״̬
		decltype(auto) F = fst.finals();
		std::map<weight_type, state_type> Fp; // ��Ȩֵ������ֹ״̬
		for (auto& f : F)
			Fp[f.second].insert(f.first); // F' <- { f | ��(f) = ��, f in F }

		P_.reserve(fst.order()); // ȷ��P_�㹻��
		std::set<state_type> W;
		for (auto& fp : Fp)
			P_.push_back(fp.second); // P <- P + { F' }
		W.insert(P_.begin(), P_.end()); // W = P

		state_type nonFinals; 
		for (state_index_t s = 0; s < fst.order(); s++)
			if (!fst.isFinal(s))
				nonFinals.insert(s);
		P_.push_back(nonFinals); // P <- P + { Q-F }

		auto gR = inverse(fst);
		while (!W.empty()) {
			auto S = *W.begin();  W.erase(W.begin());
			std::map<edge_type, state_type> R; // R(i, o, w)
			for (auto& s : S) {
					auto iter = KtAdjIter(gR, s); // ����E^-1[S]
					for (; !iter.isEnd(); ++iter) {
						auto& iow = iter.edge();
						auto pe = *iter;
						R[iow].insert(pe); // // R(i, o, w) <-{ p(e) | i[e] = i, o[e] = o, w[e] = w, e in E^-1[S] }
					}
			}
			
			for (auto& r : R) {
				for (unsigned i = 0; i < P_.size(); i++) {
					state_type& B = P_[i];
					state_type B1 = stdx::intersect(B, r.second);
					if (!B1.empty() && !stdx::is_sub_of(B, r.second)) { // for each B in P such that B & R(i, o, w) != empty and B !include in R(i, o, w) 
						state_type B2 = stdx::diff(B, B1);
						P_.erase(P_.begin() + i--); P_.push_back(B1); P_.push_back(B2); // P <- (P - {B}) + {B1, B2}
						if (W.find(B) != W.end())  // if B in W
							W.erase(B), W.insert(B1), W.insert(B2);
						else 
							W.insert(B1.size() <= B2.size() ? B1 : B2);
					}
				}
			}
		}

		/// ���춥���ӳ��:state[infst_] -> state[wfst_]
		std::vector<unsigned> idxMap(infst_.order(), 0);
		for (unsigned i = 0; i < infst_.order(); i++)
			for (unsigned j = 0; j < P_.size(); j++)
				if (P_[j].find(i) != P_[j].end()) {
					assert(idxMap[i] == 0);
					idxMap[i] = j;
				}

		/// ����wfst_
		wfst().reset(static_cast<unsigned>(P_.size()));

		// E' <- E' + { (B(p[e]), i[e], o[e], w[e], B(n[e])) }
		auto eiter = infst_.transIter();
		for (; !eiter.isEnd(); ++eiter) {
			auto from = idxMap[eiter.from()];
			auto to = idxMap[*eiter];
			auto& val = eiter.edge();
			if(!wfst().hasTrans(from, to, val))
				wfst().addTrans(from, to, val);
		}

		// ���ó�ʼ״̬����ֹ״̬
		for (unsigned i = 0; i < P_.size(); i++) {
			auto& S = P_[i];
			auto iter = S.begin();
			if (infst_.isInitial(*iter)) {
				assert(S.size() == 1);
				wfst().setInitial(i, infst_.initialWeight(*iter));
			}
			else if (infst_.isFinal(*iter)) {
				bool allFinal(true);
				while (++iter != P_[i].end()) {
					if (!infst_.isFinal(*iter)) {
						allFinal = false;
						break;
					}
					assert(infst_.finalWeight(*S.begin()) == infst_.finalWeight(*iter));
				}

				if(allFinal)
					wfst().setFinal(i, infst_.finalWeight(*S.begin()));
			}
		}
	}

private:
	const FST& infst_;
	std::vector<state_type> P_;
};

