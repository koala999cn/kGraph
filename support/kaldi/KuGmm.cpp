#include "KuGmm.h"
#include <assert.h>
#include "KuBasicIO.h"
#include "KtuMath.h"
#include "../extend/hmm/KcMixModel.h"
#include "../extend/hmm/KcGaussPdf.h"


namespace kPrivate
{
    static std::vector<double> get_mean(
        const std::vector<double>& means_invvar, const std::vector<double>& var)
    {
        assert(var.size() == means_invvar.size()); // TODO: handle covar

        auto N = means_invvar.size();
        std::vector<double> mean(N);
        KtuMath<double>::mul(means_invvar.data(), var.data(), mean.data(), N);
        return mean;
    }
}


bool KuGmm::loadSingle(stdx::istreamx& strm, kGmm& gmm)
{
    std::string tok;
    strm >> tok;
    if (tok != "<DiagGMM>") // <FullGMM>
        return false;

    strm >> tok;
    std::vector<float> gconsts;
    if (tok == "<GCONSTS>") {  // The gconsts are optional.
        KuBasicIO::readFloatVector(strm, gconsts); // ignore it
        strm >> tok;
    }

    if (tok != "<WEIGHTS>")
        return false;
    std::vector<double> weights;
    KuBasicIO::readFloatVector(strm, weights);

    strm >> tok;
    if (tok != "<MEANS_INVVARS>")
        return false;
    std::vector<std::vector<double>> means_invvars;
    KuBasicIO::readFloatMatrix(strm, means_invvars);

    strm >> tok;
    if (tok != "<INV_VARS>")
        return false;
    std::vector<std::vector<double>> inv_vars;
    KuBasicIO::readFloatMatrix(strm, inv_vars);

    strm >> tok;
    if (tok != "</DiagGMM>")
        return false;

    if (weights.size() != inv_vars.size() ||
        weights.size() != means_invvars.size())
        return false;

    gmm.resize(weights.size());
    gmm.setWeights(weights.data());
    auto dim = static_cast<unsigned>(inv_vars[0].size());
    for (unsigned i = 0; i < gmm.numMix(); i++) {
        assert(inv_vars[i].size() == dim);
        auto gauss = std::make_shared<KcGaussPdf>(dim);
        KtuMath<double>::recip(inv_vars[i].data(), dim); // invvar -> var
        auto mean = kPrivate::get_mean(means_invvars[i], inv_vars[i]);
        gauss->setMean(mean.data());
        gauss->setCovar(inv_vars[i].data(), true); // TODO
        
        // gconst verify
        if (!gconsts.empty()) {
            assert(KtuMath<double>::almostEqual(-2 * (gconsts[i] - gmm.weight(i)) - gauss->gconst(),
                KtuMath<double>::dot(means_invvars[i].data(), mean.data(), mean.size()), 0.0001));
        }

        gmm.modelAt(i) = gauss;
    }

    return true;
}


std::vector<std::shared_ptr<kGmm>> KuGmm::loadList(stdx::istreamx& strm)
{
    std::vector<std::shared_ptr<kGmm>> ms;
    std::int32_t numPdfs, dim;

    std::string tok;
    strm >> tok;
    if (tok != "<DIMENSION>")
        return decltype(ms)();

    KuBasicIO::readBasicType(strm, dim);

    strm >> tok;
    if (tok != "<NUMPDFS>")
        return decltype(ms)();

    KuBasicIO::readBasicType(strm, numPdfs);
    assert(numPdfs > 0);

    ms.resize(numPdfs);
    for (std::int32_t i = 0; i < numPdfs; i++) {
        auto gmm = std::make_shared<kGmm>();
        if (!loadSingle(strm, *gmm) || gmm->dim() != dim)
            return decltype(ms)();
        ms[i] = gmm;
    }

    return ms;
}
