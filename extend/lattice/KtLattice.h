#pragma once
#include "KtLatWeight.h"
#include "../wfst/WfstX.h"
#include "../wfst/trans_traits.h"
#include "../../graph/core/KtAdjIter.h"



// Lattice是一个无环WFSA，结点可以是hmm状态、hmm（音素）、词，
// 每个节点是一段音频在某个时间的对齐。

// 用训练好的声学模型、现成的语言模型和发音字典构建解码网络（wfst），
// 最后将提取的测试集的语音特征输入以上解码网络，得到网络结构（lattice）
// 的语音识别结果。

// for detail see https://www.danielpovey.com/files/2012_icassp_lattices.pdf

/*
	Lattice are used for all word-level networks
*/

// 结点是词，边是转移概率。转录的结果是遍历Lattic后得到的最高得分词序列。

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

    // 仅适用于nbest-lattice，即线性lattice，最大outdegree等于1
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
    double eps_acwt = 0.0; // 最开始的eps转移的累计ac权值
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
                loglikes.push_back(acwt); // 压入acwt
            }
            else if (acwt == acwt) { // isym == eps，且acwt != nan
                if (loglikes.empty()) { // 是否前缀的eps转移
                    eps_acwt += acwt; // 累加ac权值到临时变量eps_acwt
                }
                else { 
                    loglikes.back() += acwt; // // 累加ac权值到前序帧
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
