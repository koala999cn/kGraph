#pragma once


// 概率模型的接口类

class KvProbModel
{
public:

	// 输入特征向量的维度
	virtual unsigned dim() const = 0;

	// 根据输入obs计算输出概率(log值)
	virtual double prob(const double* obs) const = 0;

	// 计算obs_seq[pos]观察向量的输出概率
	// obs_seq的其他信息提供上下文，len为obs_seq的尺寸
	// 即obs_seq的规格为[len][dim]
	virtual double prob(const double* obs_seq[], unsigned len, unsigned pos) const {
		return prob(obs_seq[pos]); // 缺省实现退化为上下文无关输出概率
	}
};
