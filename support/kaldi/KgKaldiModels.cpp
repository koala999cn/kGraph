#include "KgKaldiModels.h"
#include <fstream>
#include "KuKaldiIO.h"
#include "../extend/hmm/KcMixModel.h"


KgKaldiModels::KgKaldiModels(const std::string& path)
{
	std::string ampath = path + "/final.mdl";
	std::ifstream ifs(ampath, std::ios_base::binary);
	if (!ifs) return;

	auto bin = KuKaldiIO::binaryTest(ifs);
	if (!tm_.load(ifs, bin))
		return;

	stdx::istreamx isx(ifs, bin);
	pdfs_ = KuKaldiGmm::loadGmms(isx);
	if (pdfs_.empty() || pdfs_.size() != tm_.numPdfs())
		return;

	std::string hclg = path + "/HCLG.fst";
	wfst_.reset(KuFstIO::readMmap(hclg));
	if (!wfst_) return;

	std::string words = path + "/words.txt";
	st_.reset(KuKaldiIO::loadWordIdPair(words.c_str()));
	if (!st_) return;
}


KgKaldiModels::operator bool() const
{
	return st_ != nullptr;
}


double KgKaldiModels::prob(unsigned transId, const double* feat)
{
	assert(transId > 0 && transId <= tm_.numTrans());
	auto pdfId = tm_.transId2Pdf(transId);
	assert(pdfId >= 0 && pdfId < pdfs_.size());
	return pdfs_[pdfId]->prob(feat) * 0.1; // TODO: 0.1为ac_scale，调整为可配置
}


std::string KgKaldiModels::dump(const char* prefix) const
{
	std::ostringstream ostr;
	ostr << prefix << "pdfs: " << tm_.numPdfs()
		<< ", states: " << tm_.numStates()
		<< ", transitions: " << tm_.numTrans() << "\n";


	ostr << prefix << "states: " << wfst_->order()
		<< ", arcs: " << wfst_->size() << "\n";

	ostr << prefix << "symbols: " << st_->size() << "\n";

	return ostr.str();
}
