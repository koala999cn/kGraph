#pragma once
#include <vector>
#include "KvProbModel.h"


// 高斯模型实现类

class KcGaussPdf : public KvProbModel
{
public:
	KcGaussPdf(unsigned obsDim);

	// 返回观察值obs的输出概率(log值)
	double prob(const double* obs) const override;

	unsigned dim() const override { return mean_.size(); }

	bool isDiagnal() const { return mean_.size() == invCov_.size(); }

	// 返回均值适量
	const std::vector<double>& mean() const { return mean_; }

	// 返回协方差矩阵的逆
	const std::vector<double>& invCovar() const { return invCov_; }

	// 计算并返回协方差矩阵
	std::vector<double> covar() const;
;

	void setMean(const double* mean);

	// 若bDiag为真，cov为对角阵，尺寸为dim；否则，cov为dim*dim的对称阵，仅读取下三角阵数据
	// 函数同步更新gconst
	// 若协方差矩阵求逆失败，返回false
	bool setCovar(const double* cov, bool bDiag);

	// gconst兼容htk，与kaldi不同
	double gconst() const { return gconst_; }

private:
	std::vector<double> mean_;
	std::vector<double> invCov_; // cov[i] = sigma[i]^2, 表示x[i]的方差. 当存储矩阵时，row-major

	double gconst_; // 用于加速概率计算的预计算常值，与cov有关，随cov设置而更新
};

