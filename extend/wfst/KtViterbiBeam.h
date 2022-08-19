#pragma once
#include <vector>
#include <map>
#include <deque>
#include <memory>
#include <algorithm>
#include <functional>
#include <assert.h>


// wfst��viterbi��������
// single-WFST-ViterbiBeamSearch(N, X)
//   @N: WFST = (X, Z, Q, I, F, E, r, p)
//   @X: ������/ʶ�����������

// �㷨�ο�:
// 1. Speech Recognition Algorithms Using Weighted Finite-State Transducers, Takaaki Hori, Atsushi Nakamura, 2013
// 2. Token Passing: a Simple Conceptual Model for Connected Speech Rcognition Systems, S.J. Young, 1989

/* �㷨��ܣ�����1��
 * 1  S <- initialize(I, r)
 * 2  A <- {}
 * 3  for t = 1 to T do
 * 4    S <- transition_with_epsilon(E, S, t - 1)
 * 5    <S, A> <- transition_with_input(E, S, A, x[t], t)
 * 6    prune(S, A, t)
 * 7  end for
 * 8  B <- final_transition(E, F, p, S, T)
 * 9  Y <- backtrack(B)
 * 10 return Y
 */

/* ʵ��ϸ�ڣ�
   1. ����1ʹ��2����ε�ǰ��������һ��wfst״̬��Σ�����hmm״̬��Ρ�
   ����ʵ���мٶ�hmm״̬��ȫչ��Ϊwfst״̬������ת��hmm״̬
   2. ��������2��ʹ��token���ǰ�������
   3. ʵ�־���ʹ��std�⣬δ�����㷨�Ż�
 */

template<typename WFST>
class KtViterbiBeam
{
	using state_index_t = typename WFST::state_index_t;
	using state_vector = std::vector<typename WFST::state_index_t>;
	using alpha_type = typename WFST::alpha_type;
	using weight_type = typename WFST::weight_type;
	using trans_type = typename WFST::trans_type;
	using traits_type = typename WFST::traits_type;
	using trans_id = const trans_type*;
	using frame_index_t = unsigned;

public:
	KtViterbiBeam(const WFST& wfst) : wfst_(wfst) {
		beam_ = 32.0;
		maxActive_ = 7000;
		minActive_ = 20; // This decoder mostly used for alignment, use small default.
	}

	double beam() const { return beam_; }
	void setBeam(double beam) { beam_ = decltype(beam_)(beam); }

	// @pdf: ��ѧģ�ͣ��൱��Acoustic matching transducer,
	//       ��Ҫ����������ģ��id����÷֣�score = AC_MODEL(X[i], transId)
	template<typename FEAT_TYPE>
	using pdf = std::function<double(unsigned transId, const FEAT_TYPE& xi)>;

	// ����true��ʾ�ҵ�һ������ֹ״̬������·��
	template<typename FEAT_TYPE>
	bool search(const FEAT_TYPE X[], frame_index_t T, pdf<FEAT_TYPE> ac);

	std::deque<alpha_type> bestPath() const;

	double totalWeight() const {
		if (best_)
			return best_->weight_;

		return std::numeric_limits<double>::infinity();
	}

private:
	static trans_id transId(const trans_type& trans) {
		return &trans;
	}

	// ��ʼ��atl
	void initialize_();

    // ����atl������ǰ�Ƹ�״̬��epsת�ƣ�����atl
	void transEps_(frame_index_t t, bool finalTrans = false);

	template<typename FEAT_TYPE>
	void transInput_(const FEAT_TYPE& x, frame_index_t t, pdf<FEAT_TYPE> ac);

	void prune_();
	
	void finalTrans_(frame_index_t t);

	// �Ƚ�����weight�Ĵ�С
	static bool comp_(const weight_type& w1, const weight_type& w2) {
		//return w1 != w2 && w1 == w1 + w2; // TODO: �����ʱ�޷�����ͨ��
		return w1.value() < w2.value();
	}

private:
	const WFST& wfst_;

	struct KpToken {
		frame_index_t t_;
		trans_id trans_;
		weight_type weight_;
		std::shared_ptr<KpToken> prev_;


		// ��ʼ״̬��token����
		KpToken(weight_type initWeight) : trans_{}, prev_(nullptr), t_(0) {
			weight_ = initWeight;
		}

		// epsת�Ƶ�token����
		KpToken(frame_index_t t, trans_id trans, std::shared_ptr<KpToken> prev)
			: t_(t), trans_(trans), prev_(prev) {
			weight_ = traits_type::weight(*trans) * prev->weight_;
		}

		// non-epsת�Ƶ�token����
		KpToken(frame_index_t t, trans_id trans, weight_type ac_cost, std::shared_ptr<KpToken> prev)
			: t_(t), trans_(trans), prev_(prev) {
			weight_ = traits_type::weight(*trans) * ac_cost * prev->weight_;
		}

		// ��ֹ״̬��token����
		KpToken(frame_index_t t, weight_type fianlWeight, std::shared_ptr<KpToken> prev)
			: t_(t), trans_{}, prev_(prev) {
			weight_ = fianlWeight * prev->weight_;
		}

		KpToken& operator=(const KpToken& rhs) {
			t_ = rhs.t_;
			trans_ = rhs.trans_;
			weight_ = rhs.weight_;
			prev_ = rhs.prev_;
			return *this;
		}

		~KpToken() {
			// �����ƶ��й���ʱ����800+���ݴ��ͷ�prev_���ܻ���ɶ�ջ���
			// �ڴ�ʵ��һ���ֶ��ͷŵĴ���
			if (prev_ && prev_.use_count() == 1) {
				auto tok = prev_; prev_.reset(); // �ӹ�prev_
				while (tok->prev_ && tok->prev_.use_count() == 1) {
					auto prev = tok->prev_; tok->prev_.reset();
					tok = prev; // �ӹ�ǰ��prev_, ͬʱ�ͷ�tok
				}
			}
		}


		bool operator < (const KpToken& rhs) {
			return comp_(weight_, rhs.weight_);
		}
	};

	std::map<state_index_t, std::shared_ptr<KpToken>> atl0_, atl1_; // active token list
	std::shared_ptr<KpToken> best_; // finalTrans_�Ľ��

private:

	/// ��֦��ز���
	typename weight_type::value_type beam_; // Decoding beam.  Larger->slower, more accurate
	int maxActive_; // Decoder max active states.  Larger->slower; more accurate
	int minActive_; // Decoder min active states (don't prune if #active less than this)
};


template<typename WFST> template<typename FEAT_TYPE>
bool KtViterbiBeam<WFST>::search(const FEAT_TYPE X[], frame_index_t T, pdf<FEAT_TYPE> ac)
{
	initialize_();

	for (unsigned t = 0; t < T; t++) {
		transEps_(t);
		transInput_(X[t], t, ac);
		prune_();
	}

	finalTrans_(T);
	return best_ != nullptr;
}


/* initialize(I, r)  # rΪwfst�ĳ�ʼ״̬Ȩֵ����
 * 1  for each i : I do
 * 2    a(0, i) <- r(i)
 * 3    B(0, i) <- <0, 0>
 * 4    Enqueue(S, i)
 * 5  end for
 * 6  return S
 */
template<typename WFST>
void KtViterbiBeam<WFST>::initialize_()
{
	atl0_.clear(), atl1_.clear(); best_.reset();

	for (auto& i : wfst_.initials()) // for each i : I
		atl0_[i.first] = std::make_shared<KpToken>(i.second);
}


/* transition_with_eps(E, S, t)
 * 1  S' <- {}
 * 2  while S != {} do
 * 3    s <- Head(S)
 * 4    Dequeue(S)
 * 5    for each e in E(s, eps) do
 * 6      a' <- a(t,s)*w[e]
 * 7      if a(t,n[e])+a' != a' then  # a'����
 * 8        a(t,n[e]) <- a'
 * 9        B(t,n[e]) <- <t,e>
 * 10       if n[e] !in S then
 * 11         Enqueue(S, n[e])
 * 12       endif
 * 13     endif
 * 14   end for
 * 15   if s !in S' and {e|e in E(s), i[e] != eps} != {} then
 * 16     Enqueue(S', s)
 * 17   end if
 * 18 end while
 * 19 return S'
 */

template<typename WFST> 
void KtViterbiBeam<WFST>::transEps_(frame_index_t t, bool finalTrans)
{
	assert(atl1_.empty());

	while (!atl0_.empty()) {
		auto cur = *atl0_.begin();
		atl0_.erase(atl0_.begin());

		auto iter = wfst_.adjIter(cur.first);
		bool hasNonEps(false); // ���s״̬�Ƿ���з�eps��ת��
		for (; !iter.isEnd(); ++iter) {
			auto& trans = iter.edge();
			if (traits_type::isym(trans) == WFST::eps) { // for each e in E(s, eps)
				auto to = *iter;
				auto tok = std::make_shared<KpToken>(cur.second->t_, transId(trans), cur.second);
				auto pos = atl0_.find(to);
				if (pos == atl0_.end()) {
					pos = atl1_.find(to); // ����atl1_
					if (pos == atl1_.end()) {
						atl0_.insert({ to, tok });
						continue;
					}
				}

				if (*tok < *pos->second) // tok����
					*pos->second = *tok;
			}
			else {
				hasNonEps = true;
			}
		}

		if ((hasNonEps || finalTrans) && atl1_.count(cur.first) == 0)
			atl1_.insert(cur);
	}
}


/* transition_with_input(E, S, A, x, t)
 * 1  for each s in S do
 * 2    for each e in E(s) such that i[e] != eps do
 * 3      a(t-1, e, ie) <- alpha(t - 1, s) * w[e]  # ie����e��Ӧhmm�ĳ�ʼ״̬
 * 4      b(t-1, e, ie) <- t - 1
 * 5      if <e, ie> !in A then
 * 6        A <- A + {<e, ie>}
 * 7      end if
 * 8    end for
 * 9  end for
 * 10 S' <- A' <- {}
 * 11 while A != {} do
 * 12   <e, j> <- Head(A)
 * 13   Dequeue(A)
 * 14   for each k in Adj(j) such that k != fe do  # fe����e��Ӧhmm����ֹ״̬
 * 15     a' <- a(t-1, e, j) * w(x, k | i[e], j)  # w��ʾhmmģ�͵�ת������=a[jk]*b[k](x)��aΪ״̬ת�Ƹ��ʣ�bΪ�������
 * 16     if a(t, e, k) * a' != a' then
 * 17       a(t, e, k) <- a'
 * 18       b(t, e, k) <- b(t-1, e, j)
 * 19       if <e, k> !in A' then
 * 20         Enqueue(A', <e, k>)
 * 21       end if
 * 22     end if
 * 23   end for
 * 24 end while
 * 25 for each <e, k> in A' such that fe in Adj(k) do
 * 26   a' <- a(t, e, k) * w(eps, fe | i[e], k)  # �˴�x=eps����Ӧw=a[jk]
 * 27   if alpha(t, n[e]) + a' != a' then
 * 28     alpha(t, n[e]) <- a'
 * 29     beta(t, n[e]) <- <b(t, e, k), e>
 * 30     if n[e] !in S' then
 * 31       Enqueue(S', n[e])
 * 32     end if
 * 33   end if
 * 34 end for
 * 35 return <A', S'>
 */
template<typename WFST> template<typename FEAT_TYPE>
void KtViterbiBeam<WFST>::transInput_(const FEAT_TYPE& x, frame_index_t t, pdf<FEAT_TYPE> ac)
{
	assert(atl0_.empty());

	while (!atl1_.empty()) {
		auto cur = *atl1_.begin();
		atl1_.erase(atl1_.begin());

		auto iter = wfst_.adjIter(cur.first);
		for (; !iter.isEnd(); ++iter) {
			auto& trans = iter.edge();
			auto isym = traits_type::isym(trans);
			if (isym != WFST::eps) { // for each e in E(s, eps)	
				auto tok = std::make_shared<KpToken>(t, transId(trans), 
					static_cast<typename weight_type::value_type>(-ac(isym, x)), cur.second);
				if (cur.second->trans_ == transId(trans)) { // ���ת����ͬ��ʹ��ԭtoken��tֵ��prevֵ
					tok->t_ = cur.second->t_;
					tok->prev_ = cur.second->prev_;
				}

				auto to = *iter;
				auto pos = atl0_.find(to);
				if (pos == atl0_.end())
					atl0_[to] = tok;
				else if (*tok < *pos->second) 
					*pos->second = *tok;
			}
		}
	}
}


/* prune(S, A, t)
 * 1   w(t, best) = { +alpha(t, s) for each s in S} + { +a(t, e, j) for <e, j> in A }
 * 2   w(t, th) = beam * w(t, best)  # �൱�ڼ��ϼ������
 * 3   for each s in S do
 * 4     if alpha(t, s) + w(t, th) = w(t, th) then
 * 5       erase(S, s)
 * 6     end if
 * 7   end for
 * 8   for each <e, j> in A do
 * 9     if a(t, e, j) + w(t, th) = w(t, th) then
 * 10      erase(A, <e, j>)
 * 11    end if
 * 12  end for
 * 13  return <S, A>
 */

template<typename WFST>
void KtViterbiBeam<WFST>::prune_()
{
	// ȷ����alt0_���м�֦
	assert(!atl0_.empty() && atl1_.empty());

	if (atl0_.size() < static_cast<decltype(atl0_.size())>(minActive_))
		return;

	// ����wbest
	weight_type wbest = 0; // TODO:  weight_type::zero();
	for (auto& i : atl0_)
		if(comp_(i.second->weight_, wbest))
			wbest = i.second->weight_;

	weight_type wth = wbest.value() + beam_; //  weight_type(beam_)* wbest; // TODO:

	using ele_type = std::pair<state_index_t, std::shared_ptr<KpToken>>;
	std::vector<ele_type> allToks;
	allToks.reserve(atl0_.size());
	for (auto iter = atl0_.begin(); iter != atl0_.end(); iter++)
		allToks.push_back(*iter);

	std::sort(allToks.begin(), allToks.end(), 
		[](const ele_type& e1, const ele_type& e2) {
		    return *e1.second < *e2.second;
		});

	auto pos = std::lower_bound(allToks.begin(), allToks.end(), wth, 
		[this](const ele_type& e, const weight_type& w) {
		    return comp_(e.second->weight_, w);
		});

	auto diff = std::distance(allToks.begin(), pos);
	if (diff < minActive_)
		pos = std::next(allToks.begin() + minActive_);
	else if(diff > maxActive_)
		pos = std::next(allToks.begin() + maxActive_);

	for (; pos != allToks.end(); pos++) {
		assert(atl0_.count(pos->first) == 1);
		atl0_.erase(pos->first);
	}
}


template<typename WFST>
void KtViterbiBeam<WFST>::finalTrans_(frame_index_t t)
{
	transEps_(t, true); 
	assert(atl0_.empty());

	best_.reset();
	for (auto& i : atl1_) {
		if (wfst_.isFinal(i.first) && (best_ == nullptr || *i.second < *best_))
			best_ = i.second;
	}

	atl1_.clear();
}


template<typename WFST>
std::deque<typename KtViterbiBeam<WFST>::alpha_type> KtViterbiBeam<WFST>::bestPath() const
{
	std::deque<alpha_type> path;

	if (best_) {
		auto tok = best_->prev_;
		while (tok)	{
			if (tok->trans_) {
				auto osym = traits_type::osym(*tok->trans_);
				if (osym != WFST::eps)
					path.push_front(osym);
			}
			tok = tok->prev_;
		}
	}

	return path;
}

