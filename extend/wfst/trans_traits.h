#pragma once


// wfst转移的traits. 用户可自定义，但成员须与trans_traits保持一致
template<typename TRANS_TYPE>
struct trans_traits
{
	using trans_type = TRANS_TYPE;
	using weight_type = typename TRANS_TYPE::weight_type;
	using alpha_type = typename TRANS_TYPE::alpha_type;
	constexpr static alpha_type eps = alpha_type{};

	//  根据输入、输出和权重构造转移对象
	static trans_type construct(const alpha_type& isym, const alpha_type& osym, const weight_type& wt) {
		return { isym, osym, wt };
	};

	static const weight_type& weight(const trans_type& trans) {
		return trans.wt;
	}

	static weight_type& weight(trans_type& trans) {
		return trans.wt;
	}

	static const alpha_type& isym(const trans_type& trans) {
		return trans.isym;
	}

	static alpha_type& isym(trans_type& trans) {
		return trans.isym;
	}

	static const alpha_type& osym(const trans_type& trans) {
		return trans.osym;
	}

	static alpha_type& osym(trans_type& trans) {
		return trans.osym;
	}
};
