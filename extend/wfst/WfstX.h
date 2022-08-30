#pragma once
#include "KtWfst.h"
#include "KtSemiring.h"
#include "trans_traits.h"


namespace kPrivate
{
	// fst��ת������
	template<typename SEMIRING, typename ALPHA_TYPE>
	struct KpStdTrans_
	{
		using weight_type = SEMIRING;
		using alpha_type = ALPHA_TYPE;

		ALPHA_TYPE isym; // �������
		ALPHA_TYPE osym; // �������
		SEMIRING wt; // Ȩ��

		KpStdTrans_() = default;
		KpStdTrans_(const KpStdTrans_&) = default;
		KpStdTrans_(KpStdTrans_&&) = default;
		KpStdTrans_(ALPHA_TYPE isym_, ALPHA_TYPE osym_, SEMIRING wt_) {
			isym = isym_, osym = osym_;
			wt = wt_;
		}

		KpStdTrans_& operator=(const KpStdTrans_& rhs) {
			isym = rhs.isym, osym = rhs.osym;
			wt = rhs.wt;
			return *this;
		}

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



// ʹ��ϡ��ͼʵ�ֵı�׼wfst
template<typename SEMIRING, typename ALPHA_TYPE = int>
using StdWfst = KtWfst<kPrivate::KpStdTrans_<SEMIRING, ALPHA_TYPE>>;

template<typename WEIGHT, typename ALPHA_TYPE = int>
using StdWfstLog = StdWfst<KtLogSemiring<WEIGHT>, ALPHA_TYPE>;

template<typename WEIGHT, typename ALPHA_TYPE = int>
using StdWfstTrop = StdWfst<KtTropicalSemiring<WEIGHT>, ALPHA_TYPE>;


// ʹ�ñ�ƽͼʵ�ֵı�׼wfst
template<typename SEMIRING, typename ALPHA_TYPE = int>
using StdFlatWfst = KtWfst<kPrivate::KpStdTrans_<SEMIRING, ALPHA_TYPE>, 
	trans_traits<kPrivate::KpStdTrans_<SEMIRING, ALPHA_TYPE>>,
    GraphFx<kPrivate::KpStdTrans_<SEMIRING, ALPHA_TYPE>>>;

template<typename WEIGHT, typename ALPHA_TYPE = int>
using StdFlatWfstLog = StdFlatWfst<KtLogSemiring<WEIGHT>, ALPHA_TYPE>;

template<typename WEIGHT, typename ALPHA_TYPE = int>
using StdFlatWfstTrop = StdFlatWfst<KtTropicalSemiring<WEIGHT>, ALPHA_TYPE>;

