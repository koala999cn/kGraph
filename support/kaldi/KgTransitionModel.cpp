#include "KgTransitionModel.h"
#include "istreamx.h"
#include "KuLex.h"
#include "KuStrUtil.h"
#include "../graph/core/KtAdjIter.h"
#include "KuKaldiIO.h"
#include <vector>
#include <fstream>
#include <assert.h>


bool KgTransitionModel::load(const std::string& fileName)
{
	std::ifstream ifs(fileName, std::ios_base::binary);
	if (!ifs.is_open())
		return false;

	return load(ifs, KuKaldiIO::binaryTest(ifs));
}


bool KgTransitionModel::load(std::istream &strm, bool binary)
{
	stdx::istreamx isx(strm, binary);

	std::string tok;
	isx >> tok;
	if (tok != "<TransitionModel>") {
		err_ = "Reading TransitionModel object, expected <TransitionModel>, got " + tok;
		return false;
	}

	if (!topo_.load(strm, binary))
		return false;

	isx >> tok;

	std::int32_t tupleSize;
	if (!KuKaldiIO::readBasicType(isx, tupleSize))
		return false;
	tuples_.resize(tupleSize);

	for (std::int32_t i = 0; i < tupleSize; i++) {
		KuKaldiIO::readBasicType(isx, std::get<0>(tuples_[i]));
		KuKaldiIO::readBasicType(isx, std::get<1>(tuples_[i]));
		KuKaldiIO::readBasicType(isx, std::get<2>(tuples_[i]));

		if (tok == "<Tuples>")
			KuKaldiIO::readBasicType(isx, std::get<3>(tuples_[i]));
		else if (tok == "<Triples>")
			std::get<3>(tuples_[i]) = std::get<2>(tuples_[i]);
	}
	isx >> tok;
	assert(tok == "</Triples>" || tok == "</Tuples>");

	computeDerived();

	isx >> tok;
	if (tok != "<LogProbs>") {
		err_ = "Reading TransitionModel object, expected <LogProbs>, got " + tok;
		return false;
	}

	if (!KuKaldiIO::readFloatVector(isx, log_probs_)) {
		err_ = "Reading TransitionModel object, failed read log_probs";
		return false;
	}

	isx >> tok;
	if (tok != "</LogProbs>") {
		err_ = "Reading TransitionModel object, expected </LogProbs>, got " + tok;
		return false;
	}

	isx >> tok;
	if (tok != "</TransitionModel>") {
		err_ = "Reading TransitionModel object, expected </TransitionModel>, got " + tok;
		return false;
	}

	computeDerivedOfProbs();
	check();

	return true;
}


bool KgTransitionModel::isFinal(int trans_id) const
{
	assert(false);
	return false;
}


bool KgTransitionModel::isSelfLoop(int trans_id) const
{
	assert(static_cast<size_t>(trans_id) < trans2state_.size());
	auto from_state = trans2state_[trans_id];
	auto trans_index = trans_id - state2trans_[from_state];
	auto &tuple = tuples_[from_state - 1];
	int phone = std::get<0>(tuple), hmm_state = std::get<1>(tuple);
	auto hmm = topo_.topologyForPhone(phone);
	assert(static_cast<unsigned>(hmm_state) < hmm->order()
		&& static_cast<unsigned>(trans_index) < hmm->outdegree(hmm_state));

	auto iter = KtAdjIter(*hmm, hmm_state);
	while (trans_index-- > 0) {
		++iter;
		assert(!iter.isEnd());
	}

	return iter.to() == hmm_state;
}


int KgTransitionModel::pairToTransId(int trans_state, int trans_index) const
{
	assert(static_cast<size_t>(trans_state) <= tuples_.size());
	assert(trans_index < state2trans_[trans_state + 1] - state2trans_[trans_state]);
	return state2trans_[trans_state] + trans_index;
}


float KgTransitionModel::getTransProb(int trans_id) const
{
	return exp(log_probs_[trans_id]);
}


float KgTransitionModel::getTransLogProb(int trans_id) const
{
	return log_probs_[trans_id];
}


int KgTransitionModel::selfLoopOf(int trans_state) const
{
	assert(static_cast<size_t>(trans_state - 1) < tuples_.size());
	auto &tuple = tuples_[trans_state - 1];
	int phone = std::get<0>(tuple), hmm_state = std::get<1>(tuple);
	auto entry = topo_.topologyForPhone(phone);
	assert(static_cast<size_t>(hmm_state) < entry->order());

	int trans_index = 0;
	auto iter = KtAdjIter(*entry, hmm_state);
	for (; !iter.isEnd(); ++iter, ++trans_index) {
		if(iter.to() == hmm_state)
			return pairToTransId(trans_state, trans_index);
	}

	return 0;  // invalid transition id.
}


void KgTransitionModel::computeDerived()
{
	state2trans_.resize(tuples_.size() + 2);  // indexed by transition-state, which
										   // is one based, but also an entry for one past end of list.

	int cur_transition_id = 1;
	num_pdfs_ = 0;
	for (int tstate = 1;
	tstate <= int(tuples_.size() + 1);  // not a typo.
		tstate++) {
		state2trans_[tstate] = cur_transition_id;
		if (static_cast<size_t>(tstate) <= tuples_.size()) {
			int phone = std::get<0>(tuples_[tstate - 1]),
				hmm_state = std::get<1>(tuples_[tstate - 1]),
				forward_pdf = std::get<2>(tuples_[tstate - 1]),
				self_loop_pdf = std::get<3>(tuples_[tstate - 1]);
			num_pdfs_ = std::max(num_pdfs_, 1 + forward_pdf);
			num_pdfs_ = std::max(num_pdfs_, 1 + self_loop_pdf);
			auto hmm = topo_.topologyForPhone(phone);
			assert(hmm);
			int cur_num_trans = int(hmm->outdegree(hmm_state));
			cur_transition_id += cur_num_trans;  // # trans out of this state.
		}
	}

	trans2state_.resize(cur_transition_id);   // cur_transition_id is #transition-ids+1.
	trans2pdf_.resize(cur_transition_id);
	for (int tstate = 1; tstate <= int(tuples_.size()); tstate++) {
		for (int tid = state2trans_[tstate]; tid < state2trans_[tstate + 1]; tid++) {
			trans2state_[tid] = tstate;
			if (isSelfLoop(tid))
				trans2pdf_[tid] = std::get<3>(tuples_[tstate - 1]);
			else
				trans2pdf_[tid] = std::get<2>(tuples_[tstate - 1]);
		}
	}

	// The following statements put copies a large number in the region of memory
	// past the end of the trans2pdf_ array, while leaving the aray as it was
	// before.  The goal of this is to speed up decoding by disabling a check
	// inside TransitionIdToPdf() that the transition-id was within the correct
	// range.
	int num_big_numbers = std::min<int>(2000, cur_transition_id);
	trans2pdf_.resize(cur_transition_id + num_big_numbers,
		std::numeric_limits<int>::max());
	trans2pdf_.resize(cur_transition_id);
}


void KgTransitionModel::computeDerivedOfProbs()
{
	non_self_loop_log_probs_.resize(numStates() + 1);  // this array indexed
													   //  by transition-state with nothing in zeroth element.

	for (int tstate = 1; tstate <= numStates(); tstate++) {
		int tid = selfLoopOf(tstate);
		if (tid == 0) {  // no self-loop
			non_self_loop_log_probs_[tstate] = 0;  // log(1.0)
		}
		else {
			float self_loop_prob = exp(getTransLogProb(tid)),
				non_self_loop_prob = 1.0f - self_loop_prob;
			if (non_self_loop_prob <= 0) 
				non_self_loop_prob = 1.0e-10f;  // just so we can continue...
			non_self_loop_log_probs_[tstate] = log(non_self_loop_prob);  // will be negative.
		}
	}
}


void KgTransitionModel::initializeProbs()
{

}


void KgTransitionModel::check() const
{

}


bool KgTransitionModel::isHmm() const
{
	assert(false);
	return true;
}