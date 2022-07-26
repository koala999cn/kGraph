#pragma once
#include <unordered_map>
#include <map>
#include <assert.h>
#include "../graph/GraphX.h"
#include "../graph/core/KtBfsIter.h"
#include "../graph/core/KtAdjIter.h"
#include "../graph/core/vertex_traits.h"
#include "trans_traits.h"


// 基本定义和实现算法参考Mehryar Mohri的系列论文

// WFST基于半环(semiring)Κ，主要由8要素构成：
//   ∑ -- 输入字母表
//   Ω -- 输出字母表
//   Ｑ -- 状态集
//   Ｉ -- 初始状态集，包含于Ｑ
//   Ｆ -- 终止状态集，包含于Ｑ
//   Ｅ -- 转移集，包含于Ｑ×(∑∪{ε})×(Ω ∪{ε})×Κ×Ｑ
//   λ -- 初始加权函数，Ｉ->Κ
//   ρ -- 终止加权函数，Ｆ->Κ

// weights are assumed to be cumulated by multiplication along each path and 
// addition over different paths.

/*
  A WFSA is simply a WFST where the output labels have been omitted.
  Similarly, FSAs and FSTs lack weights on the arcs or states.
*/


/*
 * Baisc operations:
 *  - composition
 *  - determinization
 *  - minimization
 *  - epsilon-removal
 *  - epsilon-normalization
 *  - synchronization
 *  - weight-pushing
 *  - reversal
 *  - projection
 *  - shortest-path
 *  - connection
 *  - closure
 *  - concatenation
 *  - pruning
 *  - re-weighting
 *  - union
 */


template<typename TRANS_TYPE, 
	typename TRANS_TRAITS = trans_traits<TRANS_TYPE>, 
	typename GRAPH_TYPE = DigraphPx<TRANS_TYPE>>
class KtWfst : public GRAPH_TYPE
{
public:
	using super_ = GRAPH_TYPE;
	using graph_type = GRAPH_TYPE;
	using trans_type = TRANS_TYPE; // 转移类型
	using traits_type = TRANS_TRAITS;
	using weight_type = std::decay_t<decltype(traits_type::weight(std::declval<trans_type>()))>; // 半环，相当于权重
	using alpha_type = std::decay_t<decltype(traits_type::isym(std::declval<trans_type>()))>; // 字母类型
	using typename super_::vertex_index_t;
	using typename super_::vertex_type;
	using typename super_::edge_type;


	// 声明wfst特有名称
	using state_index_t = vertex_index_t;
	using trans_iter = typename graph_type::edge_iter;
	using const_trans_iter = typename graph_type::const_edge_iter;
	using initial_set_type = std::unordered_map<state_index_t, weight_type>; // 初始状态集合的类型
	using final_set_type = std::conditional_t<has_weight_v<vertex_type>, void*, initial_set_type>; // 终止状态集合的类型
	

	constexpr static const auto eps = traits_type::eps;
	constexpr static state_index_t null_state = -1;

	using super_::super_;
	using super_::order;

	// 状态是否具有权值属性？
	constexpr static bool hasStateWeight() {
		return has_weight_v<vertex_type>;
	}

	auto transIter() { return super_::edgeIter(); }
	auto transIter() const { return super_::edgeIter(); }


	state_index_t addState() { return super_::addVertex(); }

	void eraseState(state_index_t s) {
		super_::eraseVertex(s);

		// 更新初始/终止状态序号
		updateStateIdx_(initials_, s);

		if constexpr (hasStateWeight())
		    updateStateIdx_(finals_, s);
	}


	// 新增1个转移
	void addTrans(state_index_t s1, state_index_t s2,
		const alpha_type& isym, const alpha_type& osym, const weight_type& wt, bool merge = false) {

		if (merge) {
			// 查找是否存在<isym, osym>的转移，若存在则需要合并，否则新增
			auto iter = adjIter(s1);
			for (; !iter.isEnd(); ++iter) {
				if (*iter == s2) {
					auto& trans = iter.edge();
					if (traits_type::isym(trans) == isym && traits_type::osym(trans) == osym) { // 合并
						iter.reedge(traits_type::construct(isym,  osym, traits_type::weight(trans) + wt));
						return;
					}
				}
			}
		}

		// 新增
		addEdge(s1, s2, traits_type::construct(isym, osym, wt));
	}

	void addTrans(state_index_t s1, state_index_t s2, const trans_type& trans) {
		addEdge(s1, s2, trans);
	}


	// 删除s1到s2的转移
	void eraseTrans(state_index_t s1, state_index_t s2) {
		eraseEdge(s1, s2);
	}


	bool hasTrans(state_index_t s1, state_index_t s2) const {
		return hasEdge(s1, s2);
	}

	bool hasTrans(state_index_t s1, state_index_t s2, const trans_type& trans) const {
		return hasEdge(s1, s2, trans);
	}


	auto numInitials() const { return initials_.size(); }

	// 返回初始状态集
	const initial_set_type& initials() const { return initials_; }

	// 是否设置初始状态
	bool hasInitial() const { return !initials_.empty(); }


	// 状态i是否为初始状态
	bool isInitial(state_index_t i) const { 
		return initials_.find(i) != initials_.end(); 
	}

	// 返回初始状态i的权值，该方法断言i必定为初始状态
	const weight_type& initialWeight(state_index_t i) const {
		assert(isInitial(i));
		return initials_.at(i);
	}


	// 设置已有状态i为初始状态，或更改初始状态i的权值为wt
	void setInitial(state_index_t i, const weight_type& wt = weight_type::one()) {
		assert(i < order());
		initials_.insert_or_assign(i, wt);
	}

	void unsetInitial(state_index_t i) {
		assert(isInitial(i));
		initials_.erase(i);
	}

	// 增加一个新状态，并将其置为初始状态，权值为wt。
	state_index_t addInitial(const weight_type& wt = weight_type::one()) {
		state_index_t i = addState();
		initials_.insert({ i, wt });
		return i;
	}

	// 返回超级初始状态的序号，若失败则返回null_state
	state_index_t makeSuperInitial() {
		if (!hasInitial())
			return null_state;
		else if (initials().size() == 1)
			return initials().begin()->first;
		else {
			auto idx = addState();
			for (auto& i : initials())
				addTrans(idx, i.first, eps, eps, i.second);

			initials_.clear();
			setInitial(idx);

			return idx;
		}
	}

	// 返回终止状态集
	decltype(auto) finals() const { 
		if constexpr (hasStateWeight()) {
			std::vector<std::pair<state_index_t, weight_type>> fs;
			for (state_index_t i = 0; i < order(); i++)
				if (isFinal(i))
					fs.push_back({ i, finalWeight(i) });
			return fs;
		}
		else
			return finals_;
	}

	template<typename T = vertex_type, typename = std::enable_if_t<has_weight_v<T>>>
	decltype(auto) stateWeight(state_index_t f) const {
		return vertex_traits<T>::weight(vertexAt(f));
	}

	template<typename T = vertex_type, typename = std::enable_if_t<has_weight_v<T>>>
	decltype(auto) stateWeight(state_index_t f) {
		return vertex_traits<T>::weight(vertexAt(f));
	}

	// 状态f是否为终止状态
	bool isFinal(state_index_t f) const {
		if constexpr (hasStateWeight()) {
			if constexpr (std::is_scalar_v<std::decay_t<decltype(stateWeight(f))>>)
			    return weight_type::zero().value() != stateWeight(f);
			else
				return weight_type::zero() != stateWeight(f);
		}
		else
			return finals_.find(f) != finals_.end();
	}

	auto numFinals() const {
		if constexpr (hasStateWeight()) {
			state_index_t num(0);
			for (state_index_t i = 0; i < order(); i++)
				if (isFinal(i)) num++;
			return num;
		}
		else
			return finals_.size();
	}


	// 是否设置终止状态
	bool hasFinal() const { 
		if constexpr (hasStateWeight()) {
			state_index_t num(0);
			for (state_index_t i = 0; i < order(); i++)
				if (isFinal(i))
					return true;
			return false;
		}
		else
			return !finals_.empty();
	}

	
	// 返回终止状态f的权值，该方法断言f必定为终止状态
	decltype(auto) finalWeight(state_index_t f) const {
		assert(isFinal(f));
		if constexpr (hasStateWeight())
			return stateWeight(f);
		else
			return finals_.at(f);
	}


	// 设置已有状态f为终止状态，或更改终止状态f的权值为wt
	void setFinal(state_index_t f, const weight_type& wt = weight_type::one()) {
		assert(f < order());
		if constexpr (hasStateWeight())
			stateWeight(f) = wt;
		else
			finals_.insert_or_assign(f, wt);
	}

	void unsetFinal(state_index_t f) {
		assert(isFinal(f));
		if constexpr (hasStateWeight())
			stateWeight(f) = weight_type::zero();
		else
			finals_.erase(f);
	}

	// 增加一个新状态，并将其置为终止状态，权值为wt。
	state_index_t addFinal(const weight_type& wt = weight_type::one()) {
		state_index_t f = addState();
		if constexpr (hasStateWeight())
			stateWeight(f) = wt;
		else 
			finals_.insert({ f, wt });
		return f;
	}

	/*
	// 返回超级终止状态的序号，若失败则返回null_state
	state_index_t makeSuperFinal() {
		if (!hasFinal())
			return null_state;
		else if (finals().size() == 1)
			return finals().begin()->first;
		else {
			auto idx = addState();
			for (auto& f : finals())
				addTrans(f.first, idx, eps, eps, f.second);

			finals_.clear();
			setFinal(idx);

			return idx;
		}
	}*/


	/*
    // 将所有边（状态转移）反转，即交换每个转移的输入状态与输出状态。同时交换初始状态与终止状态。
	KtWfst reverse() const {
		KtWfst rwfst;
		rwfst.inside() = inverse<graph_type>();
		rwfst.initials_ = finals_;
		rwfst.finals_ = initials_;
		return rwfst;
	}


	// 将输入输出反转
	KtWfst invert() const {
		struct Weightor_ {
			auto operator()(const trans_type& trans) {
				trans_traits traits;
				return traits.construct(traits.osym(trans), traits.isym(trans), traits.weight(trans)); // 反转
			}
		};

		KtWfst rwfst;
		rwfst.inside() = copy<graph_type, Weightor_>();
		rwfst.initials_ = initials_;
		rwfst.finals_ = finals_;
	}*/

	/// 几个优化算法


	// 删除不在初始状态到终止状态路径上的顶点
	void eraseDeadStates() {
		std::vector<bool> dead(order(), true);

		// TODO: 一个简单的实现，不完备
		for (auto& i : initials()) {
			auto bfs = KtBfsIter(*this, i.first);
			for (; !bfs.isEnd(); ++bfs)
				dead[*bfs] = false;
		}

		for (state_index_t s = order() - 1; s != -1; s--) // 逆向遍历删除，以便保持图的顶点编号顺序
			if (dead[s]) eraseState(s);
	}


	// 合并重边中输入/输出相同的转移
	void mergeSameTrans() {

		for (state_index_t s = 0; s < order(); s++) {

			struct KpTrans {
				state_index_t to;
				alpha_type ilabel;
				alpha_type olabel;

				bool operator < (const KpTrans& other) const {
					return to < other.to || (to == other.to && ilabel < other.ilabel) || 
						(to == other.to && ilabel == other.ilabel && olabel < other.olabel);
				}
			};

			std::map<KpTrans, super_::adj_iter> trans;
			auto iter = adjIter(s);	
			while (!iter.isEnd()) {
				auto& edge = iter.edge();
				KpTrans t{ *iter, traits_type::isym(edge), traits_type::osym(edge) };
				auto miter = trans.find(t);
				if (miter != trans.end()) { // 需要合并
					miter->second.reedge(traits_type::construct(traits_type::isym(edge), traits_type::osym(edge),
						traits_type::weight(edge) + traits_type::weight(miter->second.edge())));
					iter.erase();
				}
				else {
					trans.insert({ t, iter });
					++iter;
				}
			}
		}
	}


private:

	// 删除states中键值为s的元素，并调整states中所有大于s的键值为s-1
	static void updateStateIdx_(initial_set_type& states, state_index_t s) {
		initial_set_type newStates;
		newStates.reserve(states.size());
		for (auto& f : states) {
			if (f.first < s)
				newStates.insert({ f.first, f.second });
			else if (f.first > s)
				newStates.insert({ f.first - 1, f.second });
		}
		std::swap(states, newStates);
	}

private:
	initial_set_type initials_; // 初始状态集及其权值
	final_set_type finals_; // 终止状态集及其权值
};
