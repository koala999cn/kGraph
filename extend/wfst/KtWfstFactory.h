#pragma once
#include <vector>
#include <assert.h>
#include "KtWfstHolder.h"


namespace kPrivate {

// wfst一些优化算法有两方面通用需求：
// 一是构造复合state
// 二是根据状态栈依次构建新的fst
// 本类主要对以上两方面进行抽象，提供一致的使用接口
// @STATE_TYPE代表复合state的类型
template<typename WFST, typename STATE_TYPE>
class KtWfstFactory : public KtWfstHolder<WFST>
{
public:
	using state_type = STATE_TYPE;


	KtWfstFactory() {}
	KtWfstFactory(const WFST& fst) : KtWfstHolder<WFST>(fst) {}


	state_index_t addState(const state_type& s) {
		auto idx = inside().addState();
		assert(idx == Q_.size());  // wfst_的顶点始终跟着Q增长并保持同步。
		Q_.push_back(s);
		S_.push_back({ s, idx });
		return idx;
	}


	state_index_t addInitial(const state_type& s, const weight_type& wt = weight_type::one()) {
		auto idx = addState(s);
		inside().setInitial(idx, wt);
		return idx;
	}

	state_index_t addFinal(const state_type& s, const weight_type& wt = weight_type::one()) {
		auto idx = addState(s);
		inside().setFinal(idx, wt);
		// TODO: 对于终止状态，是否必要压入S队列？
		return idx;
	}

	void setFinal(state_index_t s, const weight_type& wt) {
		inside().setFinal(s, wt);
	}


	// convert state_index_t to state_type instance
	const state_type& state(state_index_t idx) const {
		return Q_[idx];
	}

	// convert state_type instance to state_index_t
	state_index_t index(const state_type& s) const {
		auto pos = std::find(Q_.begin(), Q_.end(), s);
		return static_cast<state_index_t>(std::distance(Q_.begin(), pos));
	}


	state_index_t addTrans(state_index_t fromIdx, const state_type& toState,
		const alpha_type& isym, const alpha_type& osym, const weight_type& wt) {
		auto toIdx = index(toState);
		if (toIdx >= Q_.size()) // to是一个新状态
			toIdx = addState(toState);

		// 更新转移集E
		inside().addTrans(fromIdx, toIdx, isym, osym, wt);

		return toIdx;
	}


	bool done() const {
		return S_.empty();
	}


	auto next() {
		auto x = S_.back();
		S_.pop_back();
		return x;
	}


private:
	std::vector<state_type> Q_;
	std::vector<std::pair<state_type, state_index_t>> S_; // 增加1个state_index_t元素，用于快速定位顶点序号
};

}
