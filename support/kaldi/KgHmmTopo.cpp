#include "KgHmmTopo.h"
#include "istreamx.h"
#include "KuLex.h"
#include "KuStrUtil.h"
#include "../graph/core/KtAdjIter.h"
#include "KuKaldiIO.h"
#include <vector>
#include <fstream>
#include <assert.h>


bool KgHmmTopo::loadText_(std::istream &strm)
{
	stdx::istreamx isx(strm, false);
	std::string tok;
	isx >> tok;
	if (tok != "<Topology>") {
		err_ = "Reading HmmTopology object, expected <Topology>, got " + tok;
			return false;
	}

	while (isx >> tok) {
		if (tok == "</Topology>")
			break;

		if (tok != "<TopologyEntry>") {
			err_ = "Reading HmmTopology object, expected </Topology> or <TopologyEntry>, got " + tok;
			return false;
		}

		isx >> tok;
		if (tok != "<ForPhones>") {
			err_ = "Reading HmmTopology object, expected <ForPhones>, got " + tok;
			return false;
		}

		std::vector<std::int32_t> phones;
		while (true) {
			if (!(isx >> tok)) {
				err_ = "Reading HmmTopology object, unexpected end of file while expecting phones.";
				return false;
			}

			if (tok == "</ForPhones>") break;

			auto phone = KuLex::parseInt(tok);
			if (!phone.first) {
				err_ = "Reading HmmTopology object, expected integer, got instead " + tok;
				return false;
			}

			phones.push_back(phone.second);
		}

		isx >> tok;
		kHmm hmm;
		while (tok != "</TopologyEntry>") {
			if (tok != "<State>") {
				err_ = "Expected </TopologyEntry> or <State>, got instead " + tok;
				return false;
			}

			std::int32_t state;
			if (!(isx >> state)) {
				err_ = "Read state index(int32) failed at position " + KuStrUtil::toString(isx->tellg());
				return false;
			}

			while (state >= static_cast<std::int32_t>(hmm.order()))
				hmm.addVertex(KpPdfClass{ KuKaldiIO::kNoPdf, KuKaldiIO::kNoPdf });

			isx >> tok;
			std::int32_t forward_pdf_class = KuKaldiIO::kNoPdf;  // -1 by default, means no pdf.
			if (tok == "<PdfClass>") {
				if (!(isx >> forward_pdf_class)) {
					err_ = "Read PdfClass id(int32) failed at position " + KuStrUtil::toString(isx->tellg());
					return false;
				}

				hmm.vertexAt(state) = { forward_pdf_class, forward_pdf_class };
				isx >> tok;
				if (tok == "<SelfLoopPdfClass>") {
					err_ = "pdf classes should be defined using <PdfClass> "
						"or <ForwardPdfClass>/<SelfLoopPdfClass> pair";
					return false;
				}
			}
			else if (tok == "<ForwardPdfClass>") {
				std::int32_t self_loop_pdf_class = KuKaldiIO::kNoPdf;
				if (!(isx >> forward_pdf_class)) {
					err_ = "Read ForwardPdfClass id(int32) failed at position "
						+ KuStrUtil::toString(isx->tellg());
					return false;
				}
				isx >> tok;
				if (tok != "<SelfLoopPdfClass>") {
					err_ = "Expected <SelfLoopPdfClass>, got instead " + tok;
					return false;
				}

				if (!(isx >> self_loop_pdf_class)) {
					err_ = "Read SelfLoopPdfClass id(int32) failed at position "
						+ KuStrUtil::toString(isx->tellg());
					return false;
				}

				hmm.vertexAt(state) = { forward_pdf_class, self_loop_pdf_class };
				isx >> tok;
			}
			else {
				hmm.vertexAt(state) = { forward_pdf_class, forward_pdf_class };
			}

			while (tok == "<Transition>") {
				std::int32_t to_state;
				if (!(isx >> to_state)) {
					err_ = "Read to_state(int32) failed at position "
						+ KuStrUtil::toString(isx->tellg());
					return false;
				}

				float trans_prob;
				if (!(isx >> trans_prob)) {
					err_ = "Read trans_prob(float) failed at position "
						+ KuStrUtil::toString(isx->tellg());
					return false;
				}

				while (to_state >= static_cast<std::int32_t>(hmm.order()))
					hmm.addVertex(KpPdfClass{ KuKaldiIO::kNoPdf, KuKaldiIO::kNoPdf });

				hmm.addEdge(state, to_state, trans_prob);

				isx >> tok;
			}

			if (tok != "</State>") {
				err_ = "Expected </State>, got instead " + tok;
			}

			isx >> tok;
		}

		auto my_index = entries_.size();
		entries_.push_back(hmm);

		for (size_t i = 0; i < phones.size(); i++) {
			auto phone = phones[i];
			if (static_cast<std::int32_t>(phone2idx_.size()) <= phone)
				phone2idx_.resize(phone + 1, -1);  // -1 is invalid index.
			assert(phone > 0);
			if (phone2idx_[phone] != -1) {
				err_ = "Phone with index " + KuStrUtil::toString(i)
					+ " appears in multiple topology entries.";
				return false;
			}
			phone2idx_[phone] = static_cast<int>(my_index);
			phones_.push_back(phone);
		}
	}

	std::sort(phones_.begin(), phones_.end());

	return true;
}


bool KgHmmTopo::loadBin_(std::istream &strm)
{
	stdx::istreamx isx(strm, true);

	std::string tok;
	isx >> tok;
	if (tok != "<Topology>") {
		err_ = "Reading HmmTopology object, expected <Topology>, got " + tok;
		return false;
	}

	if (!KuKaldiIO::readIntVector(isx, phones_) ||
		!KuKaldiIO::readIntVector(isx, phone2idx_))
		return false;

	std::int32_t sz;
	if (!KuKaldiIO::readBasicType(isx, sz))
		return false;

	bool is_hmm = true;
	if (sz == -1) {
		is_hmm = false;
		std::int32_t sz;
		if (!KuKaldiIO::readBasicType(isx, sz))
			return false;
	}
	entries_.resize(sz);
	for (std::int32_t i = 0; i < sz; i++) {
		std::int32_t thist_sz;
		if (!KuKaldiIO::readBasicType(isx, thist_sz))
			return false;
		auto& hmm = entries_[i];
		hmm.reset(thist_sz);
		for (std::int32_t j = 0; j < thist_sz; j++) {
			std::int32_t forward_pdf_class, self_loop_pdf_class;
			if (!KuKaldiIO::readBasicType(isx, forward_pdf_class))
				return false;

			if (is_hmm)
				self_loop_pdf_class = forward_pdf_class;
			else if (!KuKaldiIO::readBasicType(isx, self_loop_pdf_class))
				return false;

			hmm.vertexAt(j) = { forward_pdf_class, self_loop_pdf_class };

			std::int32_t thiss_sz;
			if (!KuKaldiIO::readBasicType(isx, thiss_sz))
				return false;
			hmm.reserve(j, thiss_sz);
			for (std::int32_t k = 0; k < thiss_sz; k++) {
				std::int32_t to;
				float prob;;
				if (!KuKaldiIO::readBasicType(isx, to) ||
					!KuKaldiIO::readBasicType(isx, prob))
					return false;
				hmm.addEdge(j, to, prob);
			}
		}
	}

	isx >> tok;
	if (tok != "</Topology>") {
		err_ = "Reading HmmTopology object, expected </Topology>, got " + tok;
		return false;
	}

	std::sort(phones_.begin(), phones_.end());

	return true;
}


bool KgHmmTopo::load(const std::string& fileName)
{
	std::ifstream ifs(fileName, std::ios_base::binary);
	if (!ifs.is_open())
		return false;

	return load(ifs, KuKaldiIO::binaryTest(ifs));
}


bool KgHmmTopo::load(std::istream &strm, bool binary)
{
	phones_.clear();
	phone2idx_.clear();
	entries_.clear();

	return binary ? loadBin_(strm) : loadText_(strm);
}


const KgHmmTopo::kHmm* KgHmmTopo::topologyForPhone(int phone) const
{
	if (phone < 0 || phone >= int(phone2idx_.size()) || phone2idx_[phone] == -1)
		return nullptr;

	return &entries_[phone2idx_[phone]];
}


int KgHmmTopo::numPdfClasses(int phone) const
{
	auto entry = topologyForPhone(phone);
	if (entry == nullptr)
		return 0;

	int max_pdf_class = 0;
	for (unsigned i = 0; i < entry->order(); i++) {
		max_pdf_class = std::max(max_pdf_class, entry->vertexAt(i).forward);
		max_pdf_class = std::max(max_pdf_class, entry->vertexAt(i).self_loop);
	}
	return max_pdf_class + 1;
}



// TODO: 使用kGraph的最短路径实现
// weightor的输入由edge改为adj_iter
int KgHmmTopo::shortestLength(int phone) const
{
	auto entry = topologyForPhone(phone);
	if (entry == nullptr)
		return -1;

	// min_length[state] gives the minimum length for sequences up to and
	// including that state.
	unsigned order = entry->order();
	std::vector<int> min_length(order, std::numeric_limits<int>::max());
	assert(!entry->isEmpty());

	min_length[0] = entry->vertexAt(0).forward == KuKaldiIO::kNoPdf ? 0 : 1;
	bool changed = true;
	while (changed) {
		changed = false;
		for (unsigned s = 0; s < order; s++) {
			decltype(auto) this_state = entry->vertexAt(s);
			auto iter = KtAdjIter(*entry, s);
			for (; !iter.isEnd(); ++iter) {
				auto next_state = iter.to();
				int next_state_min_length = min_length[s] +
					(entry->vertexAt(next_state).forward == KuKaldiIO::kNoPdf ? 0 : 1);
				if (next_state_min_length < min_length[next_state]) {
					min_length[next_state] = next_state_min_length;
					if (next_state < s)
						changed = true;
					// the test of 'next_state < s' is an optimization for speed.
				}
			}
		}
	}

	assert(min_length.back() != std::numeric_limits<int>::max());
	// the last state is the final-state.
	return min_length.back();
}