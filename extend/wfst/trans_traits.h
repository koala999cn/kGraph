#pragma once


// wfstת�Ƶ�traits. �û����Զ��壬����Ա����trans_traits����һ��
template<typename TRANS_TYPE>
struct trans_traits
{
	using trans_type = TRANS_TYPE;
	using weight_type = typename TRANS_TYPE::weight_type;
	using alpha_type = typename TRANS_TYPE::alpha_type;
	constexpr static alpha_type eps = alpha_type{};

	//  �������롢�����Ȩ�ع���ת�ƶ���
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
