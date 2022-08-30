#pragma once
#include "../../graph/util/copy.h"


// wfst�Ŀ�������Ҫ��graph�����Ļ����ϣ�����initials��finals�Ŀ���
// @WTOR: ��Ҫ����weight_type��ת��������trans_type
template<typename SRC, typename DST, typename WTOR = KtWeightSelf<typename SRC::edge_type>>
DST wfst_copy(const SRC& src)
{
	struct TRANS_OR_ {
		using trans1_t = typename SRC::trans_type;
		using trans2_t = typename DST::trans_type;
		trans2_t operator()(const trans1_t& trans) {
			return trans_traits<trans2_t>::construct(
				trans_traits<trans1_t>::isym(trans),
				trans_traits<trans1_t>::osym(trans),
				WTOR{}(trans_traits<trans1_t>::weight(trans)));
		}
	};

    auto dst = copy<SRC, DST, TRANS_OR_>(src);

    for (auto& i : src.initials())
        dst.setInitial(i.first, WTOR{}(i.second));

    for (auto& f : src.finals())
        dst.setFinal(f.first, WTOR{}(f.second));

    return dst;
}


template<typename SRC, typename DST, typename WTOR = KtWeightSelf<typename SRC::edge_type>>
void wfst_copy(const SRC& src, DST& dst)
{
    dst = wfst_copy<SRC, DST, WTOR>(src);
}