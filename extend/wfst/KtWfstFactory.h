#pragma once
#include <vector>
#include <assert.h>
#include "KtWfstHolder.h"


namespace kPrivate {

// wfstһЩ�Ż��㷨��������ͨ������
// һ�ǹ��츴��state
// ���Ǹ���״̬ջ���ι����µ�fst
// ������Ҫ��������������г����ṩһ�µ�ʹ�ýӿ�
// @STATE_TYPE������state������
template<typename WFST, typename STATE_TYPE>
class KtWfstFactory : public KtWfstHolder<WFST>
{
public:
	using state_type = STATE_TYPE;


	KtWfstFactory() {}
	KtWfstFactory(const WFST& fst) : KtWfstHolder<WFST>(fst) {}


	state_index_t addState(const state_type& s) {
		auto idx = inside().addState();
		assert(idx == Q_.size());  // wfst_�Ķ���ʼ�ո���Q����������ͬ����
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
		// TODO: ������ֹ״̬���Ƿ��Ҫѹ��S���У�
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
		if (toIdx >= Q_.size()) // to��һ����״̬
			toIdx = addState(toState);

		// ����ת�Ƽ�E
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
	std::vector<std::pair<state_type, state_index_t>> S_; // ����1��state_index_tԪ�أ����ڿ��ٶ�λ�������
};

}
