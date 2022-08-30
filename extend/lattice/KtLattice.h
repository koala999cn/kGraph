#pragma once
#include "../wfst/WfstX.h"
#include "KtLatWeight.h"


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

private:

};


using kLatticed = KtLattice<KtLatWeight<double>>;
using kLatticef = KtLattice<KtLatWeight<float>>;

using kCompactLatticed32 = KtLattice<KtCompactLatWeight<KtLatWeight<double>, std::int32_t>>;
using kCompactLatticef32 = KtLattice<KtCompactLatWeight<KtLatWeight<float>, std::int32_t>>;
using kCompactLatticed64 = KtLattice<KtCompactLatWeight<KtLatWeight<double>, std::int64_t>>;
using kCompactLatticef64 = KtLattice<KtCompactLatWeight<KtLatWeight<float>, std::int64_t>>;
