#pragma once
#include <vector>
#include <map>
#include "KtWfstFactory.h"



/*
 * A WFST is said to be deterministic or subsequential if it has a unique initial state and 
 * if no two transitions leaving any state share the same input label.
 *
 * 算法参考"Speech Recognition Algorithms Using Weighted Finite-State Transducers", Takaaki Hori, Atsushi Nakamura.
 * 
 * Weighted-Determinization(T)
 * 1  i' ← {(i, eps, λ(i)) : i∈I}
 * 2  λ'(i') ← 1
 * 3  Q' ← S ← {i'}
 * 4  while !S.empty() do
 * 5      p' ← Head(S)
 * 6      Dequeue(S)
 * 7      for each x∈{i[e]=x, e∈E[p], p∈Q[p']} do
 * 8          y' ← ∧{z?y}, (p,z,v)∈p', (p,x,y,w,q)∈E, 下同  /// compute new arc osym  
 * 9          w' ← +{v*w}   /// compute new arc weight      
 * 10         q' ← {(q, y'^-1?z?y, +{w'^?1*v*w})}  /// compute new states and leftover osym & residual weight    
 * 11         E' ← E'∪{(p', x, y', w', q')}
 * 12         if q' !∈ Q' then
 * 13             Q' ← Q'∪{q'}
 * 14             if Q[q']∩F != empty then
 * 15                 F' ← F'∪{q'}               
 * 16                 ρ(q') ← {(z, +{v*ρ(q)|(q,z,v)∈q', q∈F})|(q,z,v)∈q', q∈F}   /// compute new final weight
 * 17             Enqueue(S, q')
 * 18 return T'
 *
 * y' is obtained as the longest common prefix of z & y, denoted by ∧ in line 8. 
 * However, since y' should be a single alpha, we can just pick at most one common head symbol o them. 
 * 
 * p' is a weighted subset of Q that (q, x) ∈ Q×Κ. Q[p'] is the set of states q of p'. 
 * E[Q[p']] represents the set of transitions leaving Q[p'], and i[E[Q[p']]] the set of 
 * input labels of these transitions.
 */

template<typename FST>
class KtDeterminize
{
public:

	// determinized FST涉及的基本类型
	using wfst_type = FST;
	using alpha_type = typename wfst_type::alpha_type;
	using weight_type = typename wfst_type::weight_type;
	using state_index_t = typename wfst_type::state_index_t;

	// determinized FST的状态类型，由三元组序列构成，即(q, y'^-1?z?y, +{w'^?1*v*w})
	// 每个tuple的第1个元素为原FST的状态序号
	// 第2个元素为该状态的leftover osym
	// 第3个元素为该状态的residual weight
	using state_element_type = std::tuple<state_index_t, std::vector<alpha_type>, weight_type>;
	using state_type = std::vector<state_element_type>;

	// ρ(q') ← { (z, +{v*ρ(q)}) }. 由此，终止状态的权值是一个二元组序列.
	using final_weight_type = std::vector<std::pair<std::vector<alpha_type>, weight_type>>;


	KtDeterminize(const FST& fst) : infst_(fst) {

		/// NOTE: 初始状态的权值设置为λ(i)，而不是算法中第二行所示的1. 其实两者是等价的
		for (const auto& i : fst.initials())
			factory_.addInitial({ state_element_type{ i.first, {}, i.second } }, i.second); // (i, eps, λ(i))


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

	// determinize的核心代码，主要执行算法中的7-17行
	//   @pC:  待展开的复合状态
	//   @idx: pC在新wfst中的序号
	void expandState_(const state_type& pC, state_index_t idx);


	// 展开finalWeights_
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
	// 对E[Q[p']]相同的isym累积计算(q', y', w')，使用map缓存中间结果：isym -> (q', y', w')
	std::map<alpha_type, std::tuple<state_type, alpha_type, weight_type>> temp; 

	// pC = p' = { (p, z, v) }
	for (auto& i : pC) {
		auto iter = infst_.adjIter(std::get<0>(i)); // std::get<0>(p)∈Q[p']
		for (; !iter.isEnd(); ++iter) {
			const auto& e = iter.edge(); // e = E[Q[p']]
			auto zy = std::get<1>(i);
			if (e.osym != FST::eps) zy.push_back(e.osym); // z.y
			weight_type vw = std::get<2>(i) * e.wt; // v*w
			state_element_type se{ *iter, zy, vw };

			auto& t = temp[e.isym]; // e.isym = x ∈ i[E[Q[p']]]
			std::get<0>(t).push_back(se);
			if (std::get<0>(t).size() == 1) { // 首个元素
				std::get<1>(t) = zy.empty() ? FST::eps : zy.front();
				std::get<2>(t) = vw;
			}
			else { // 非首个元素
				std::get<1>(t) = std::get<1>(t) == zy.front() ? zy.front() : FST::eps; // 累计计算y' = ∧{z?y}
				std::get<2>(t) = std::get<2>(t) + vw; // 累计计算w' = +{v*w}
			}
		}
	}

	// 使用边权值w'对determinized状态中的各residual weight归一化
	for (auto& i : temp) {
		auto& q = std::get<0>(i.second); // state_type
		const auto& y = std::get<1>(i.second); // y'
		const auto& w = std::get<2>(i.second); // w'

		final_weight_type finalWt;
		for (auto& j : q) { // j为st中的state_element_type，即三元组(p, z, v)
			auto& p = std::get<0>(j); // state_index_t
			auto& z = std::get<1>(j); // std::vector<alpha_type>
			auto& v = std::get<2>(j); // weight_type

			if (!z.empty() && z.front() == y) z.erase(z.begin()); // y'^-1?z?y
			v = v.leftDiv(w); // = +{w'^?1*v*w}. 实际此处计算的是(w'^?1) * (+{v*w})
			                  // TODO: 若+{w'^?1*v*w} != (w'^?1)*(+{v*w})，以上实现会有问题

			// 同步更新终止状态权值
			if (infst_.isFinal(p)) {
				auto x = finalWt.begin();
				for (; x != finalWt.end(); ++x)
					if (x->first == z) // 找到z对应的权值
						break;

				if (x != finalWt.end())  // 累积更新 w += v*ρ(q)
					x->second = x->second + v * infst_.finalWeight(p);
				else // 新插入
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
				// 若是最后一个字符，则链接到superFinal，否则新增一个中间状态
				state_index_t to = (n == sym_seq.size() - 1) ? superFinal : wfst.addState();

				// 若是第一个字符，则权值为j.second，否则权值为1
				weight_type wt = n == 0 ? j.second : weight_type::one();

				// 增加转移
				wfst.addTrans(i.first, to, wfst_type::eps, sym_seq[n], wt);
			}
		}
	}
}
