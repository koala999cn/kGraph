#pragma once
#include <vector>
#include <memory>
#include "KvProbModel.h"


// ͨ�û��ģ�ͣ�P(x) = w[0]*P(x[0]) + ... + w[n-1]*P(x[n-1])
// ����Աmodels_��Ԫ�ؾ�ʹ��gaussʱ���˻�Ϊgmm
class KcMixModel : public KvProbModel
{
public:
	KcMixModel() = default;

	void resize(unsigned numMix); // ��ʼ��KvProbModel����Ϊnullptr

	unsigned dim() const override;

	// �������(logֵ)
	double prob(const double* obs) const override;

	unsigned numMix() const { 
		return static_cast<unsigned>(models_.size()); 
	}


	// ��ȡ��idx����Ϸ���
	std::shared_ptr<KvProbModel> modelAt(unsigned idx) const {
		return models_[idx];
	}

	std::shared_ptr<KvProbModel>& modelAt(unsigned idx) {
		return models_[idx];
	}

	// ��wlogΪ�棬��ʾwΪlogֵ
	void setWeight(unsigned idx, double w, bool wlog = false);
	void setWeights(const double w[/*numMix*/], bool wlog = false);

	// return log(w)
	double weight(unsigned idx) const { return weights_[idx]; }

	// find most likely mix component for obs
	unsigned getBestModel(const double* obs) const;


protected:
	std::vector<double> weights_; // store as log(w)
	std::vector<std::shared_ptr<KvProbModel>> models_;
};

