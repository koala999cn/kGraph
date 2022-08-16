#include "KuKaldiGmm.h"
#include <assert.h>
#include <fstream>
#include "KuKaldiIO.h"
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
        KtuMath<double>::mul(means_invvar.data(), var.data(), mean.data(), static_cast<unsigned>(N));
        return mean;
    }
}


bool KuKaldiGmm::loadGmm(stdx::istreamx& strm, kGmm& gmm)
{
    std::string tok;
    strm >> tok;
    if (tok != "<DiagGMM>") // <FullGMM>
        return false;

    strm >> tok;
    std::vector<float> gconsts;
    if (tok == "<GCONSTS>") {  // The gconsts are optional.
        KuKaldiIO::readFloatVector(strm, gconsts); // ignore it
        strm >> tok;
    }

    if (tok != "<WEIGHTS>")
        return false;
    std::vector<double> weights;
    KuKaldiIO::readFloatVector(strm, weights);

    strm >> tok;
    if (tok != "<MEANS_INVVARS>")
        return false;
    std::vector<std::vector<double>> means_invvars;
    KuKaldiIO::readFloatMatrix(strm, means_invvars);

    strm >> tok;
    if (tok != "<INV_VARS>")
        return false;
    std::vector<std::vector<double>> inv_vars;
    KuKaldiIO::readFloatMatrix(strm, inv_vars);

    strm >> tok;
    if (tok != "</DiagGMM>")
        return false;

    if (weights.size() != inv_vars.size() ||
        weights.size() != means_invvars.size())
        return false;

    gmm.resize(static_cast<unsigned>(weights.size()));
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
                KtuMath<double>::dot(means_invvars[i].data(), mean.data(), 
                    static_cast<unsigned>(mean.size())), 0.001));
        }

        gmm.modelAt(i) = gauss;
    }

    return true;
}


std::vector<std::shared_ptr<kGmm>> KuKaldiGmm::loadGmms(stdx::istreamx& strm)
{
    std::vector<std::shared_ptr<kGmm>> ms;
    std::int32_t numPdfs, dim;

    std::string tok;
    strm >> tok;
    if (tok != "<DIMENSION>")
        return decltype(ms)();

    KuKaldiIO::readBasicType(strm, dim);

    strm >> tok;
    if (tok != "<NUMPDFS>")
        return decltype(ms)();

    KuKaldiIO::readBasicType(strm, numPdfs);
    assert(numPdfs > 0);

    ms.resize(numPdfs);
    for (std::int32_t i = 0; i < numPdfs; i++) {
        auto gmm = std::make_shared<kGmm>();
        if (!loadGmm(strm, *gmm) || gmm->dim() != dim)
            return decltype(ms)();
        ms[i] = gmm;
    }

    return ms;
}


std::vector<std::shared_ptr<kGmm>> KuKaldiGmm::loadGmms(const char* filePath)
{
    std::ifstream ifs(filePath, std::ios_base::binary);
    if (!ifs)
        return { 0 };

    auto bin = KuKaldiIO::binaryTest(ifs);
    if (!bin)
        return { 0 }; // 暂不支持text模式

    stdx::istreamx strm(ifs, bin);
    return loadGmms(strm);
}