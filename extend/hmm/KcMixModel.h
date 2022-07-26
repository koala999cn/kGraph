#pragma once
#include <vector>
#include <memory>
#include "KvProbModel.h"


// 通用混合模型：P(x) = w[0]*P(x[0]) + ... + w[n-1]*P(x[n-1])
// 当成员models_的元素均使用gauss时，退化为gmm
class KcMixModel : public KvProbModel
{
public:
	KcMixModel() = default;

	void resize(unsigned numMix); // 初始化KvProbModel对象为nullptr

	unsigned dim() const override;

	// 输入概率(log值)
	double prob(const double* obs) const override;

	unsigned numMix() const { 
		return static_cast<unsigned>(models_.size()); 
	}


	// 获取第idx个混合分量
	std::shared_ptr<KvProbModel> modelAt(unsigned idx) const {
		return models_[idx];
	}

	std::shared_ptr<KvProbModel>& modelAt(unsigned idx) {
		return models_[idx];
	}

	// 若wlog为真，表示w为log值
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

