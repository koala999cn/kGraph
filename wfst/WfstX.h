#pragma once
#include "KtWfst.h"
#include "KtSemiring.h"
#include "trans_traits.h"


namespace kPrivate
{
	// fst的转移类型
	template<typename SEMIRING, typename ALPHA_TYPE>
	struct KpStdTrans_
	{
		using weight_type = SEMIRING;
		using alpha_type = ALPHA_TYPE;

		ALPHA_TYPE isym; // 输入符号
		ALPHA_TYPE osym; // 输出符号
		SEMIRING wt; // 权重


		bool operator == (const KpStdTrans_& rhs) const {
			return isym == rhs.isym && osym == rhs.osym && wt == rhs.wt;
		}

		bool operator < (const KpStdTrans_& rhs) const {
			return isym < rhs.isym ||
				isym == rhs.isym && osym < rhs.osym ||
				isym == rhs.isym && osym == rhs.osym && wt < rhs.wt;
		}
	};
}



// 使用稀疏图实现的标准wfst
template<typename SEMIRING, typename ALPHA_TYPE = int>
using StdWfst = KtWfst<kPrivate::KpStdTrans_<SEMIRING, ALPHA_TYPE>>;

template<typename WEIGHT, typename ALPHA_TYPE = int>
using StdWfstLog = StdWfst<KtLogSemiring<WEIGHT>, ALPHA_TYPE>;

template<typename WEIGHT, typename ALPHA_TYPE = int>
using StdWfstTrop = StdWfst<KtTropicalSemiring<WEIGHT>, ALPHA_TYPE>;


// 使用扁平图实现的标准wfst
template<typename SEMIRING, typename ALPHA_TYPE = int>
using StdFlatWfst = KtWfst<kPrivate::KpStdTrans_<SEMIRING, ALPHA_TYPE>, 
	trans_traits<kPrivate::KpStdTrans_<SEMIRING, ALPHA_TYPE>>,
    GraphFx<kPrivate::KpStdTrans_<SEMIRING, ALPHA_TYPE>>>;

template<typename WEIGHT, typename ALPHA_TYPE = int>
using StdFlatWfstLog = StdFlatWfst<KtLogSemiring<WEIGHT>, ALPHA_TYPE>;

template<typename WEIGHT, typename ALPHA_TYPE = int>
using StdFlatWfstTrop = StdFlatWfst<KtTropicalSemiring<WEIGHT>, ALPHA_TYPE>;

