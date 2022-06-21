#pragma once
#include "base/KtHolder.h"


namespace kPrivate {

	// 将KtWfst对象置内的封装

	template<typename WFST>
	class KtWfstHolder : public KtHolder<WFST>
	{
	public:
		using super_ = KtHolder<WFST>;
		using wfst_type = typename super_::value_type;
		using edge_type = typename wfst_type::edge_type;
		using vertex_index_t = typename wfst_type::vertex_index_t;
		using weight_type = typename wfst_type::weight_type;
		using trans_type = typename wfst_type::trans_type;
		using state_index_t = typename wfst_type::state_index_t;
		using alpha_type = typename wfst_type::alpha_type;


		// 导入构造函数
		using super_::super_;

		WFST& wfst() { return super_::inside(); }
		const WFST& wfst() const { return super_::inside(); }
	};

}