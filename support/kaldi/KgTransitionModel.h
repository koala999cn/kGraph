#pragma once
#include <istream>
#include <tuple>
#include "KgHmmTopo.h"


// ����hmmģ�͵ıߵļ��ϡ�
// ÿ��Tuple(phone, hmm_state, forward_pdf, self_loop_pdf)��Ӧ��һ����
// kaldiΪÿ����ָ��Ψһ��transition_id������״̬ת�Ƶı�ʶ
class KgTransitionModel
{
public:

	bool load(const std::string& fileName);
	bool load(std::istream &strm, bool binary);

	const std::string& errorText() const { return err_; }

	// returns true if this trans_id goes to the final state
	// (which is bound to be nonemitting).
	bool isFinal(int trans_id) const;  								 

	// return true if this trans_id corresponds to a self-loop.
	bool isSelfLoop(int trans_id) const;  

	/// Returns the total number of transition-ids (note, these are one-based).
	auto numTrans() const { return trans2state_.size() - 1; }

	/// Returns the total number of transition-states (note, these are one-based).
	auto numStates() const { return tuples_.size(); }

	// NumPdfs() actually returns the highest-numbered pdf we ever saw, plus one.
	// In normal cases this should equal the number of pdfs in the system, but if you
	// initialized this object with fewer than all the phones, and it happens that
	// an unseen phone has the highest-numbered pdf, this might be different.
	auto numPdfs() const { return num_pdfs_; }

	// Transition-parameter-getting functions:
	float getTransProb(int trans_id) const;
	float getTransLogProb(int trans_id) const;

	int pairToTransId(int trans_state, int trans_index) const;

	// returns the self-loop transition-id, or zero if this state doesn't have a self-loop.
	int selfLoopOf(int trans_state) const;  

	int transId2State(int trans_id) const {
		return trans2state_[trans_id];
	}

	int transId2Pdf(int trans_id) const {
		return trans2pdf_[trans_id];
	}

private:
	void computeDerived();  // called from constructor and Read function: computes state2trans_ and trans2state_.
	void computeDerivedOfProbs();  // computes quantities derived from log-probs (currently just
								   // non_self_loop_log_probs_; called whenever log-probs change.
	void initializeProbs();  // called from constructor.
	void check() const;
	bool isHmm() const;

private:
	std::string err_;

	KgHmmTopo topo_;

	// KpTuple<0>: phone;
	// KpTuple<1>: hmm_state;
	// KpTuple<2>: forward_pdf;
	// KpTuple<3>: self_loop_pdf;
	using KpTuple = std::tuple<std::int32_t, std::int32_t, std::int32_t, std::int32_t>;

	/// Tuples indexed by transition state minus one;
	/// the tuples are in sorted order which allows us to do the reverse mapping from
	/// tuple to transition state
	std::vector<KpTuple> tuples_;

	/// Gives the first transition_id of each transition-state; indexed by
	/// the transition-state.  Array indexed 1..num-transition-states+1 (the last one
	/// is needed so we can know the num-transitions of the last transition-state.
	std::vector<int> state2trans_;

	/// For each transition-id, the corresponding transition
	/// state (indexed by transition-id).
	std::vector<int> trans2state_;

	std::vector<int> trans2pdf_;

	/// For each transition-id, the corresponding log-prob.  Indexed by transition-id.
	std::vector<float> log_probs_;

	/// For each transition-state, the log of (1 - self-loop-prob).  Indexed by
	/// transition-state.
	std::vector<float> non_self_loop_log_probs_;

	/// This is actually one plus the highest-numbered pdf we ever got back from the
	/// tree (but the tree numbers pdfs contiguously from zero so this is the number
	/// of pdfs).
	int num_pdfs_;
};

