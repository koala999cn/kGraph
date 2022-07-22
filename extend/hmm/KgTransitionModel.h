#pragma once
#include <istream>
#include "KgHmmTopo.h"


// 所有hmm模型的边的集合。
// 每个Tuple(phone, hmm_state, forward_pdf, self_loop_pdf)对应有一条边
// kaldi为每条边指定唯一的transition_id，用于状态转移的标识
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
	int numTransitions() const { return trans2state_.size() - 1; }

	/// Returns the total number of transition-states (note, these are one-based).
	int numStates() const { return tuples_.size(); }

	// NumPdfs() actually returns the highest-numbered pdf we ever saw, plus one.
	// In normal cases this should equal the number of pdfs in the system, but if you
	// initialized this object with fewer than all the phones, and it happens that
	// an unseen phone has the highest-numbered pdf, this might be different.
	int numPdfs() const { return num_pdfs_; }

	// Transition-parameter-getting functions:
	float getTransitionProb(int trans_id) const;
	float getTransitionLogProb(int trans_id) const;

	int pairToTransitionId(int trans_state, int trans_index) const;

	// returns the self-loop transition-id, or zero if this state doesn't have a self-loop.
	int selfLoopOf(int trans_state) const;  

	int transitionId2State(int trans_id) const {
		return trans2state_[trans_id];
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

	struct KpTuple {
		std::int32_t phone;
		std::int32_t hmm_state;
		std::int32_t forward_pdf;
		std::int32_t self_loop_pdf;

		bool operator < (const KpTuple&other) const {
			if (phone < other.phone) return true;
			else if (phone > other.phone) return false;
			else if (hmm_state < other.hmm_state) return true;
			else if (hmm_state > other.hmm_state) return false;
			else if (forward_pdf < other.forward_pdf) return true;
			else if (forward_pdf > other.forward_pdf) return false;
			else return (self_loop_pdf < other.self_loop_pdf);
		}

		bool operator == (const KpTuple&other) const {
			return (phone == other.phone && hmm_state == other.hmm_state
				&& forward_pdf == other.forward_pdf && self_loop_pdf == other.self_loop_pdf);
		}
	};

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

