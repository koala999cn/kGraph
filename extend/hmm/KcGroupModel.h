#pragma once
#include <vector>
#include <memory>
#include "KvProbModel.h"


// �ο�htkʵ�ֵķ���ģ�ͣ��������൱��htk�е�stream
// P(x) = P(x[0])^w[0] * ... * P(x[n-1])^w[n-1]
// ==> log(P(x)) = w[0]*log(P(x[0])) + ... + w[n-1]*log(P(x[n-1]))
class KcGroupModel : public KvProbModel
{
public:
	KcGroupModel(unsigned numGroups, const unsigned* subDims);
	KcGroupModel(const std::vector<unsigned>& subDims);


	// ������Ŀ
	unsigned numGroups() const { return subDims_.size(); }

	// ������������ά��
	unsigned dim() const { return totalDim_; }

	// ��idx�������ά��
	unsigned subDim(unsigned idx) const { return subDims_[idx]; }

	// ��ȡ��idx���������
	std::shared_ptr<KvProbModel> modelAt(unsigned idx) const {
		return models_[idx];
	}

	std::shared_ptr<KvProbModel>& modelAt(unsigned idx) {
		return models_[idx];
	}


	void setWeight(unsigned idx, double w);
	void setWeights(const double w[/*numGroups*/]);

	// return log(w)
	double weight(size_t idx) const { return weights_[idx]; }

	double prob(const double* obs) const override;

	double prob(const double* obs_seq[], unsigned len, unsigned pos) const override;

private:
	std::vector<unsigned> subDims_; // �������ά��
	std::vector<double> weights_;
	std::vector<std::shared_ptr<KvProbModel>> models_;
	unsigned totalDim_; // = SUM(subDims_[i])
};

