#include "KcGroupModel.h"
#include "KtuMath.h"


KcGroupModel::KcGroupModel(unsigned numGroups, const unsigned* subDims)
{
	subDims_.assign(subDims, subDims + numGroups);
	
	weights_.assign(numGroups, 0); // 初始权值置log(1) 

	models_.resize(numGroups);

	totalDim_ = 0;
	std::for_each(subDims, subDims + numGroups, [this](unsigned x) {
		totalDim_ += x;
		});
}


KcGroupModel::KcGroupModel(const std::vector<unsigned>& subDims)
	: KcGroupModel(static_cast<unsigned>(subDims.size()), subDims.data())
{

}


void KcGroupModel::setWeights(const double w[])
{
	weights_.assign(w, w + weights_.size());
}


void KcGroupModel::setWeight(unsigned idx, double w)
{
	assert(idx < weights_.size());
	weights_[idx] = w;
}


double KcGroupModel::prob(const double* obs) const
{
	double prob(0.0);
	for (unsigned i = 0; i < weights_.size(); i++) {
		assert(models_[i] && models_[i]->dim() == subDims_[i]);
		prob += weights_[i] * models_[i]->prob(obs); // 确保此处返回log概率
		obs += subDims_[i];
	}

	return prob;
}


double KcGroupModel::prob(const double* obs_seq[], unsigned len, unsigned pos) const
{
	double prob(0.0);
	for (unsigned i = 0; i < weights_.size(); i++) {
		assert(models_[i] && models_[i]->dim() == subDims_[i]);
		prob += weights_[i] * models_[i]->prob(obs_seq, len, pos);
		for (unsigned i = 0; i < len; i++)
			obs_seq[i] += subDims_[i];
	}

	return prob;
}
