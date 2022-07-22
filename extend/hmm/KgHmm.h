#pragma once

// Hmm模型涉及：
// 3个研究对象：一是状态集合，二是观测集合，三是观测序列。
// 3个基本要素：一是状态转移概率矩阵(状态i->状态j)，二是观测概率矩阵（状态i->观测j），三是初始概率向量（P{状态i|t=0}）。
// 3个基本问题：一是概率计算问题。已知观测序列和Hmm模型，计算观测序列出现的概率。（向前/向后算法）
//              二是学习问题。已知观测序列，估计Hmm模型参数。（EM算法，也叫Baum-Welch算法）
//              三是预测问题，也称解码问题。已知观测序列和模型Hmm，计算观测序列对应的状态序列。（Viterbi算法）

// 概率均为log(P)
class KgHmm
{
public:
	// @numStates: 构造含有numStates个状态的hmm. 不包含初始和终止2个状态
	KgHmm(unsigned numStates);
	~KgHmm();

	// 返回hmm包含的状态数，不包含初始和终止2个状态
	unsigned numStates() const; 

	// 从状态s1到状态s2是否存在转移概率
	// 0表示初态
	bool hasTrans(unsigned s1, unsigned s2) const;

	// return true if a0F != 0
	bool isTee() const;

	// 状态s1 -> s2的转移概率
	double transProb(unsigned s1, unsigned s2) const;

	// 置转移概率
	// @p: 转移概率的log值
	void setTransProb(unsigned s1, unsigned s2, double p);

	// 删除状态s1 -> s2的转移弧
	void eraseTrans(unsigned s1, unsigned s2);

	// 状态s的发射概率
	double emissionProb(unsigned s, const double* obs) const;

private:
	void* dptr_;
};

