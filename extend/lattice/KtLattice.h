#pragma once
#include "../wfst/WfstX.h"
#include "KtLatWeight.h"


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

private:

};


using kLatticed = KtLattice<KtLatWeight<double>>;
using kLatticef = KtLattice<KtLatWeight<float>>;

using kCompactLatticed32 = KtLattice<KtCompactLatWeight<KtLatWeight<double>, std::int32_t>>;
using kCompactLatticef32 = KtLattice<KtCompactLatWeight<KtLatWeight<float>, std::int32_t>>;
using kCompactLatticed64 = KtLattice<KtCompactLatWeight<KtLatWeight<double>, std::int64_t>>;
using kCompactLatticef64 = KtLattice<KtCompactLatWeight<KtLatWeight<float>, std::int64_t>>;
