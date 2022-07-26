#include "KcMixModel.h"
#include "KtuMath.h"
#include <assert.h>


void KcMixModel::resize(unsigned numMix)
{
	assert(numMix > 0);
	weights_.resize(numMix, log(1.0 / numMix));
	models_.resize(numMix);

}

double KcMixModel::prob(const double* obs) const
{
	assert(weights_.size() == models_.size());

	double prob(KtuMath<double>::neginf);
	for (unsigned i = 0; i < numMix(); i++)
		prob = KtuMath<double>::addLog(prob, models_[i]->prob(obs) + weights_[i]);

	return prob;
}


unsigned KcMixModel::dim() const
{
	if (models_.empty() || models_[0] == nullptr)
		return 0;

	return models_[0]->dim();
}


void KcMixModel::setWeights(const double w[/*numMix*/], bool wlog)
{
	weights_.assign(w, w + weights_.size());
	if (!wlog) {
		assert(KtuMath<double>::almostEqualRel(KtuMath<double>::sum(w, numMix()), 1));
		KtuMath<double>::applyLog(weights_.data(), weights_.size());
	}
}


void KcMixModel::setWeight(unsigned idx, double w, bool wlog)
{
	assert(idx < weights_.size());
	weights_[idx] = wlog ? w : log(w);
}


unsigned KcMixModel::getBestModel(const double* obs) const
{
	unsigned bestm = 0;
	unsigned M = numMix();
	if (M > 1) {
		double bestP = KtuMath<double>::neginf; bestm = -1;
		for (unsigned m = 0; m < M; m++) {
			double p = models_[m]->prob(obs);
			if (p > bestP) {
				bestP = p; bestm = m;
			}
		}
	}

	return bestm;
}
