#pragma once
#include <vector>
#include "KvProbModel.h"


// ��˹ģ��ʵ����

class KcGaussPdf : public KvProbModel
{
public:
	KcGaussPdf(unsigned obsDim);

	// ���ع۲�ֵobs���������(logֵ)
	double prob(const double* obs) const override;

	unsigned dim() const override { return mean_.size(); }

	bool isDiagnal() const { return mean_.size() == invCov_.size(); }

	// ���ؾ�ֵ����
	const std::vector<double>& mean() const { return mean_; }

	// ����Э����������
	const std::vector<double>& invCovar() const { return invCov_; }

	// ���㲢����Э�������
	std::vector<double> covar() const;
;

	void setMean(const double* mean);

	// ��bDiagΪ�棬covΪ�Խ��󣬳ߴ�Ϊdim������covΪdim*dim�ĶԳ��󣬽���ȡ������������
	// ����ͬ������gconst
	// ��Э�����������ʧ�ܣ�����false
	bool setCovar(const double* cov, bool bDiag);

	// gconst����htk����kaldi��ͬ
	double gconst() const { return gconst_; }

private:
	std::vector<double> mean_;
	std::vector<double> invCov_; // cov[i] = sigma[i]^2, ��ʾx[i]�ķ���. ���洢����ʱ��row-major

	double gconst_; // ���ڼ��ٸ��ʼ����Ԥ���㳣ֵ����cov�йأ���cov���ö�����
};

