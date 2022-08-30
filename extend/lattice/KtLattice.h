#pragma once
#include "KtLatWeight.h"
#include "../wfst/WfstX.h"
#include "../wfst/trans_traits.h"
#include "../../graph/core/KtAdjIter.h"



// Lattice��һ���޻�WFSA����������hmm״̬��hmm�����أ����ʣ�
// ÿ���ڵ���һ����Ƶ��ĳ��ʱ��Ķ��롣

// ��ѵ���õ���ѧģ�͡��ֳɵ�����ģ�ͺͷ����ֵ乹���������磨wfst����
// �����ȡ�Ĳ��Լ������������������Ͻ������磬�õ�����ṹ��lattice��
// ������ʶ������

// for detail see https://www.danielpovey.com/files/2012_icassp_lattices.pdf

/*
	Lattice are used for all word-level networks
*/

// ����Ǵʣ�����ת�Ƹ��ʡ�ת¼�Ľ���Ǳ���Lattic��õ�����ߵ÷ִ����С�

template<typename WEIGHT_TYPE>
class KtLattice : public StdWfst<WEIGHT_TYPE>
{
public:
	using super_ = StdWfst<WEIGHT_TYPE>;
	using super_::super_;

    using trans_traits_ = trans_traits<typename super_::trans_type>;

	constexpr static bool isCompact() {
		return WEIGHT_TYPE::isCompact();
	}

    // ��������nbest-lattice��������lattice�����outdegree����1
    std::vector<double> acousticCostsPerFrame() const;


private:

};


template<typename WEIGHT_TYPE>
std::vector<double> KtLattice<WEIGHT_TYPE>::acousticCostsPerFrame() const
{
    assert(super_::initials().size() == 1);

    std::vector<double> loglikes;
    loglikes.reserve(super_::size());

    auto cur_state = super_::initials().begin()->first;
    double eps_acwt = 0.0; // �ʼ��epsת�Ƶ��ۼ�acȨֵ
    while (true) {
        auto w = super_::weight_type::zero();
        if (super_::isFinal(cur_state))
            w = super_::finalWeight(cur_state);
        if (w != super_::weight_type::zero()) {
            assert(super_::outdegree(cur_state) == 0);
            break;
        }
        else {
            assert(super_::outdegree(cur_state) == 1);
            auto iter = KtAdjIter(*this, cur_state);
            auto& arc = iter.edge();
            auto& wt = trans_traits_::weight(arc);
            double acwt = 0;
            if constexpr (isCompact())
                acwt = wt.weight().value1();
            else
                acwt = wt.value1();

            if (trans_traits_::isym(arc) != trans_traits_::eps) { // isym != eps
                if (eps_acwt > 0) {
                    assert(loglikes.empty());
                    acwt += eps_acwt; 
                    eps_acwt = 0.0;
                }
                loglikes.push_back(acwt); // ѹ��acwt
            }
            else if (acwt == acwt) { // isym == eps����acwt != nan
                if (loglikes.empty()) { // �Ƿ�ǰ׺��epsת��
                    eps_acwt += acwt; // �ۼ�acȨֵ����ʱ����eps_acwt
                }
                else { 
                    loglikes.back() += acwt; // // �ۼ�acȨֵ��ǰ��֡
                }
            }
            cur_state = iter.to();
        }
    }

    return loglikes;
}


using kLatticed = KtLattice<KtLatWeight<double>>;
using kLatticef = KtLattice<KtLatWeight<float>>;

using kCompactLatticed32 = KtLattice<KtCompactLatWeight<KtLatWeight<double>, std::int32_t>>;
using kCompactLatticef32 = KtLattice<KtCompactLatWeight<KtLatWeight<float>, std::int32_t>>;
using kCompactLatticed64 = KtLattice<KtCompactLatWeight<KtLatWeight<double>, std::int64_t>>;
using kCompactLatticef64 = KtLattice<KtCompactLatWeight<KtLatWeight<float>, std::int64_t>>;
