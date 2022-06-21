#pragma once
#include <vector>
#include <map>
#include "KtWfstFactory.h"



/*
 * A WFST is said to be deterministic or subsequential if it has a unique initial state and 
 * if no two transitions leaving any state share the same input label.
 *
 * �㷨�ο�"Speech Recognition Algorithms Using Weighted Finite-State Transducers", Takaaki Hori, Atsushi Nakamura.
 * 
 * Weighted-Determinization(T)
 * 1  i' �� {(i, eps, ��(i)) : i��I}
 * 2  ��'(i') �� 1
 * 3  Q' �� S �� {i'}
 * 4  while !S.empty() do
 * 5      p' �� Head(S)
 * 6      Dequeue(S)
 * 7      for each x��{i[e]=x, e��E[p], p��Q[p']} do
 * 8          y' �� ��{z?y}, (p,z,v)��p', (p,x,y,w,q)��E, ��ͬ  /// compute new arc osym  
 * 9          w' �� +{v*w}   /// compute new arc weight      
 * 10         q' �� {(q, y'^-1?z?y, +{w'^?1*v*w})}  /// compute new states and leftover osym & residual weight    
 * 11         E' �� E'��{(p', x, y', w', q')}
 * 12         if q' !�� Q' then
 * 13             Q' �� Q'��{q'}
 * 14             if Q[q']��F != empty then
 * 15                 F' �� F'��{q'}               
 * 16                 ��(q') �� {(z, +{v*��(q)|(q,z,v)��q', q��F})|(q,z,v)��q', q��F}   /// compute new final weight
 * 17             Enqueue(S, q')
 * 18 return T'
 *
 * y' is obtained as the longest common prefix of z & y, denoted by �� in line 8. 
 * However, since y' should be a single alpha, we can just pick at most one common head symbol o them. 
 * 
 * p' is a weighted subset of Q that (q, x) �� Q����. Q[p'] is the set of states q of p'. 
 * E[Q[p']] represents the set of transitions leaving Q[p'], and i[E[Q[p']]] the set of 
 * input labels of these transitions.
 */

template<typename FST>
class KtDeterminize
{
public:

	// determinized FST�漰�Ļ�������
	using wfst_type = FST;
	using alpha_type = typename wfst_type::alpha_type;
	using weight_type = typename wfst_type::weight_type;
	using state_index_t = typename wfst_type::state_index_t;

	// determinized FST��״̬���ͣ�����Ԫ�����й��ɣ���(q, y'^-1?z?y, +{w'^?1*v*w})
	// ÿ��tuple�ĵ�1��Ԫ��ΪԭFST��״̬���
	// ��2��Ԫ��Ϊ��״̬��leftover osym
	// ��3��Ԫ��Ϊ��״̬��residual weight
	using state_element_type = std::tuple<state_index_t, std::vector<alpha_type>, weight_type>;
	using state_type = std::vector<state_element_type>;

	// ��(q') �� { (z, +{v*��(q)}) }. �ɴˣ���ֹ״̬��Ȩֵ��һ����Ԫ������.
	using final_weight_type = std::vector<std::pair<std::vector<alpha_type>, weight_type>>;


	KtDeterminize(const FST& fst) : infst_(fst) {

		/// NOTE: ��ʼ״̬��Ȩֵ����Ϊ��(i)���������㷨�еڶ�����ʾ��1. ��ʵ�����ǵȼ۵�
		for (const auto& i : fst.initials())
			factory_.addInitial({ state_element_type{ i.first, {}, i.second } }, i.second); // (i, eps, ��(i))


		while (!factory_.done()) {
			auto t = factory_.next(); 
			assert(t.second < factory_->order());

			// t.first = p' = { (p, z, v) }
			// t.second = index of p'
			expandState_(t.first, t.second);
		}

		expandFinalWeight_();
		makeSuperInitial_();
	}

	decltype(auto) wfst() { return factory_.wfst(); }
	decltype(auto) wfst() const { return factory_.wfst(); }

	auto operator->() { return factory_.operator->(); }
	auto operator->() const { return factory_.operator->(); }


private:

	// determinize�ĺ��Ĵ��룬��Ҫִ���㷨�е�7-17��
	//   @pC:  ��չ���ĸ���״̬
	//   @idx: pC����wfst�е����
	void expandState_(const state_type& pC, state_index_t idx);


	// չ��finalWeights_
	void expandFinalWeight_();


	void makeSuperInitial_() { factory_.wfst().makeSuperInitial(); }


private:
	const FST& infst_;
	kPrivate::KtWfstFactory<wfst_type, state_type> factory_;
	std::map<state_index_t, final_weight_type> finalWeights_;
};



template<typename FST>
void KtDeterminize<FST>::expandState_(const state_type& pC, state_index_t idx)
{
	// ��E[Q[p']]��ͬ��isym�ۻ�����(q', y', w')��ʹ��map�����м�����isym -> (q', y', w')
	std::map<alpha_type, std::tuple<state_type, alpha_type, weight_type>> temp; 

	// pC = p' = { (p, z, v) }
	for (auto& i : pC) {
		auto iter = infst_.adjIter(std::get<0>(i)); // std::get<0>(p)��Q[p']
		for (; !iter.isEnd(); ++iter) {
			const auto& e = iter.edge(); // e = E[Q[p']]
			auto zy = std::get<1>(i);
			if (e.osym != FST::eps) zy.push_back(e.osym); // z.y
			weight_type vw = std::get<2>(i) * e.wt; // v*w
			state_element_type se{ *iter, zy, vw };

			auto& t = temp[e.isym]; // e.isym = x �� i[E[Q[p']]]
			std::get<0>(t).push_back(se);
			if (std::get<0>(t).size() == 1) { // �׸�Ԫ��
				std::get<1>(t) = zy.empty() ? FST::eps : zy.front();
				std::get<2>(t) = vw;
			}
			else { // ���׸�Ԫ��
				std::get<1>(t) = std::get<1>(t) == zy.front() ? zy.front() : FST::eps; // �ۼƼ���y' = ��{z?y}
				std::get<2>(t) = std::get<2>(t) + vw; // �ۼƼ���w' = +{v*w}
			}
		}
	}

	// ʹ�ñ�Ȩֵw'��determinized״̬�еĸ�residual weight��һ��
	for (auto& i : temp) {
		auto& q = std::get<0>(i.second); // state_type
		const auto& y = std::get<1>(i.second); // y'
		const auto& w = std::get<2>(i.second); // w'

		final_weight_type finalWt;
		for (auto& j : q) { // jΪst�е�state_element_type������Ԫ��(p, z, v)
			auto& p = std::get<0>(j); // state_index_t
			auto& z = std::get<1>(j); // std::vector<alpha_type>
			auto& v = std::get<2>(j); // weight_type

			if (!z.empty() && z.front() == y) z.erase(z.begin()); // y'^-1?z?y
			v = v.leftDiv(w); // = +{w'^?1*v*w}. ʵ�ʴ˴��������(w'^?1) * (+{v*w})
			                  // TODO: ��+{w'^?1*v*w} != (w'^?1)*(+{v*w})������ʵ�ֻ�������

			// ͬ��������ֹ״̬Ȩֵ
			if (infst_.isFinal(p)) {
				auto x = finalWt.begin();
				for (; x != finalWt.end(); ++x)
					if (x->first == z) // �ҵ�z��Ӧ��Ȩֵ
						break;

				if (x != finalWt.end())  // �ۻ����� w += v*��(q)
					x->second = x->second + v * infst_.finalWeight(p);
				else // �²���
					finalWt.push_back({ z, v * infst_.finalWeight(p) });
			}
		}

		auto toIdx = factory_.addTrans(idx, q, i.first, y, w); // i.first = i[e]

		if (!finalWt.empty())
			finalWeights_.insert({ toIdx, finalWt });
	}
}


template<typename FST>
void KtDeterminize<FST>::expandFinalWeight_() 
{
	wfst_type& wfst = factory_.inside();
	assert(!wfst.hasFinal());
	state_index_t superFinal = wfst.addFinal();

	for (auto& i : finalWeights_) { // i = final_weight_type = std::vector<std::pair<std::vector<alpha_type>, weight_type>>;
		for (auto& j : i.second) { // j = std::pair<std::vector<alpha_type>, weight_type>
			auto& sym_seq = j.first;
			for (auto n = 0u; n < sym_seq.size(); n++) {
				// �������һ���ַ��������ӵ�superFinal����������һ���м�״̬
				state_index_t to = (n == sym_seq.size() - 1) ? superFinal : wfst.addState();

				// ���ǵ�һ���ַ�����ȨֵΪj.second������ȨֵΪ1
				weight_type wt = n == 0 ? j.second : weight_type::one();

				// ����ת��
				wfst.addTrans(i.first, to, wfst_type::eps, sym_seq[n], wt);
			}
		}
	}
}
