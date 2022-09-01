#pragma once
#include "KtLatWeight.h"
#include "../wfst/WfstX.h"
#include "../wfst/trans_traits.h"
#include "../../graph/core/KtAdjIter.h"


// Lattice通常作为音频解码的输出。
// 
// Lattice是一个无环WFSA，结点可以是hmm状态、hmm（音素）、词，边是转移概率.
// 每个节点是一段音频在某个时间的对齐。
// 转录的结果是遍历Lattic后得到的最高得分词序列。
//
// for detail see https://www.danielpovey.com/files/2012_icassp_lattices.pdf
//

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
    // 返回各帧的acoustic权值（对应lat-weight的value1），其元素个数等于this-lat中isym不等于eps的状态数量
    // 对于isym等于eps的状态，其acoustic权值累计到前序状态；若无前序状态，则累计到下一个状态。
    std::vector<double> acousticCostsPerFrame() const;

    // 仅适用于经过topo排序的lattice.
    // 返回各节点的时间（以帧数表示）。
    // 对于non-compact-lattice，返回的数组长度等于lattice的节点（状态）数
    // 对于compact-lattice，返回的数组长度等于lattice的节点（状态）数 + 1，最后一个元素累加了终态的时间
    std::vector<int> stateTimes() const;

private:

};


template<typename WEIGHT_TYPE>
std::vector<double> KtLattice<WEIGHT_TYPE>::acousticCostsPerFrame() const
{
    // TODO: assert(isLinear(*this));
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
                    loglikes.back() += acwt; // 累加ac权值到前序帧
                }
            }
            cur_state = iter.to();
        }
    }

    return loglikes;
}


template<typename WEIGHT_TYPE>
std::vector<int> KtLattice<WEIGHT_TYPE>::stateTimes() const
{
    // assert(isTopoSorted(*this));
    assert(super_::initials().size() == 1);
    assert(super_::initials().begin()->first == 0);

    if (super_::isEmpty())
        return {};

    std::vector<int> times(super_::numStates(), -1);
    if constexpr (isCompact())
        times.push_back(-1); // 添加终态权值的string长度

    times.front() = 0;

    for (unsigned s = 0; s < super_::numStates(); s++) {
        auto iter = KtAdjIter(*this, s);
        for (; !iter.isEnd(); ++iter) {
            auto& trans = iter.edge();
            int delta;
            if constexpr (!isCompact())
                delta = trans_traits_::isym(trans) == trans_traits_::eps ? 0 : 1;
            else
                delta = trans_traits_::weight(trans).string().size();

            auto to = iter.to();
            if (times[to] == -1)
                times[to] = times[s] + delta;
            else
                assert(times[to] == times[s] + delta);
        }

        if constexpr (isCompact()) {
            if (super_::isFinal(s)) {
                auto utt_len = times[s] + finalWeight(s).string().size();
                if (times.back() == -1)
                    times.back() = utt_len;
                else
                    assert(times.back() == utt_len); // TODO: 是否允许不一致？
            }
        }
    }
}


using kLatticed = KtLattice<KtLatWeight<double>>;
using kLatticef = KtLattice<KtLatWeight<float>>;

using kCompactLatticed32 = KtLattice<KtCompactLatWeight<KtLatWeight<double>, std::int32_t>>;
using kCompactLatticef32 = KtLattice<KtCompactLatWeight<KtLatWeight<float>, std::int32_t>>;
using kCompactLatticed64 = KtLattice<KtCompactLatWeight<KtLatWeight<double>, std::int64_t>>;
using kCompactLatticef64 = KtLattice<KtCompactLatWeight<KtLatWeight<float>, std::int64_t>>;
