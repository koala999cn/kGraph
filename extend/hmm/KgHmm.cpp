#include "KgHmm.h"
#include <memory>
#include "KvProbModel.h"
#include "../../graph/GraphX.h"
#include "KtuMath.h"


namespace kPrivate
{
	using kHmm = DigraphSx<double, std::shared_ptr<KvProbModel>>;
}


KgHmm::KgHmm(unsigned numStates)
{
	dptr_ = new kPrivate::kHmm(numStates + 2); // ¼ÓÉÏ³õÌ¬ºÍÖÕÌ¬
}


KgHmm::~KgHmm()
{
	delete (kPrivate::kHmm*)dptr_;
}


unsigned KgHmm::numStates() const
{
	return ((kPrivate::kHmm*)dptr_)->order() - 2;
}


bool KgHmm::hasTrans(unsigned s1, unsigned s2) const
{
	return ((kPrivate::kHmm*)dptr_)->hasEdge(s1, s2);
}


bool KgHmm::isTee() const
{
	return hasTrans(0, numStates() + 1);
}


double KgHmm::transProb(unsigned s1, unsigned s2) const
{
	return ((const kPrivate::kHmm*)dptr_)->getEdge(s1, s2);
}


void KgHmm::setTransProb(unsigned s1, unsigned s2, double p)
{
	((kPrivate::kHmm*)dptr_)->setEdge(s1, s2, p);
}


void KgHmm::eraseTrans(unsigned s1, unsigned s2)
{
	((kPrivate::kHmm*)dptr_)->eraseEdge(s1, s2);
}


double KgHmm::emissionProb(unsigned s, const double* obs) const
{
	return ((const kPrivate::kHmm*)dptr_)->vertexAt(s)->prob(obs);
}
