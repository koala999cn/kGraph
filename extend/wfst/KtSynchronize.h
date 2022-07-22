#pragma once
#include <vector>
#include "KtWfst.h"
#include "KtWfstFactory.h"


/*
 * 同步操作：消除WFST中输入符号或者输出符号为eps的转移，
 * 剩下的转移中，要么输入、输出符号都为eps，要么都不是eps
 *
 * Synchronize(T)
 * 1   F' <- Q' <- E' <- empty
 * 2   S <- i' <- { (i, eps, eps) : i in I }
 * 3   while S not empty
 * 4     p' = (q, x, y) <- HEAD(S)
 * 5     DEQUEUE(S)
 * 6     if( q in F && |x| + |y| = 0 ) ;|x|代表字符串x的长度
 * 7       F' <- F' + {p'}; 
 * 8       ρ'(p') <- ρ(q)
 * 9     else if( q in F && |x| + |y| > 0 )
 * 10      q' <- (f, cdr(x), cdr(y))  ;cdr(x)代表字符串x除去首字符余下的部分
 * 11      E' <- E' + { p', car(x), car(y), ρ(q), q' } ;car(x)代表字符串x的首字符
 * 12      if( q' not in Q' )
 * 13        Q' <- Q' + {q'}; 
 * 14        ENQUEUE(S, q');
 * 15    for each e in E[q]
 * 16      if(|x i[e]| > 0 && |y o[e]| > 0)
 * 17        q' <- (n[e], cdr(x i[e]), cdr(y o[e]))
 * 18        E' <- E' + {(p', car(x i[e]), car(y o[e]), w[e], q')}
 * 19      else
 * 20        q' <- (n[e], x i[e], y o[e])
 * 21        E' <- E' + {(p', eps, eps, w[e], q')}
 * 22      if(q' not in Q')
 * 23        Q' <- Q' + {q'}
 * 24        ENQUEUE(S, q')
 * 25  return T'
 *
 */

template<typename WFST>
class KtSynchronize
{
public:
	using wfst_type = WFST;
	using alpha_type = typename wfst_type::alpha_type;
	using weight_type = typename wfst_type::weight_type;
	using trans_type = typename wfst_type::trans_type;
	using state_index_t = typename wfst_type::state_index_t;
	using state_type = std::pair<alpha_type, std::pair<alpha_type, alpha_type>>;

	KtSynchronize(const wfst_type& fst) {
	
		std::vector<state_type> S;

		// S <- i' <- { (i, eps, eps) : i in I }
		for (auto& i : fst.initials())
			factory_.addInitial(makeState_(i.first, wfst_type::eps, wfst_type::eps), i.second);

		while (!factory_.done()) {
			auto t = factory_.next();
			assert(t.second < factory_->order());
			auto s = t.first;
			auto p_ = t.second;
			alpha_type q = s.first, x = s.second.first, y = s.second.second;

			if (fst.isFinal(q)) {
				if (length_(x) + length_(y) == 0) { // if( q in F && |x| + |y| = 0 )
					factory_.setFinal(p_, fst.finalWeight(q));
				}
				else { // else if( q in F && |x| + |y| > 0 )
					state_type q_ = makeState_( q, cdr_(x), cdr_(y)); // q' <- (f, cdr(x), cdr(y)) 
					factory_.addTrans(p_, q_, car_(x), car_(y), fst.finalWeight(q));
				}
			}

			auto iter = fst.adjIter(q);
			for (; !iter.isEnd(); ++iter) { // for each e in E[q]
				auto& e = iter.edge();
				if (length_(x) + length_(e.isym) > 0 &&
					length_(y) + length_(e.osym) > 0) { // if(|x i[e]| > 0 && |y o[e]| > 0)
					state_type q_ = makeState_(*iter, cdr_(x, e.isym), cdr_(y, e.osym));
					factory_.addTrans(p_, q_, car_(x, e.isym), car_(y, e.osym), e.wt);
				}
				else {
					state_type q_ = makeState_(*iter, mix_(x, e.isym), mix_(y, e.osym));
					factory_.addTrans(p_, q_, wfst_type::eps, wfst_type::eps, e.wt);
				}
			}
		}
	}


	wfst_type& wfst() { return factory_; }
	const wfst_type& wfst() const { return factory_; }

	wfst_type* operator->() { return factory_.operator->(); }
	const wfst_type* operator->() const { return factory_.operator->(); }


private:

	static state_type makeState_(const alpha_type& a, const alpha_type& b, const alpha_type& c) {
		return state_type{ a, { b, c } };
	}


	// 以下函数假定alpha_type为单字符

	// |a|
	static unsigned length_(const alpha_type& a) {
		return a == wfst_type::eps ? 0 : 1;
	}

	// car(a)
	static alpha_type car_(const alpha_type& a) {
		return a == wfst_type::eps ? WFST::eps : a;
	}

	// car(ab)
	static alpha_type car_(const alpha_type& a, const alpha_type& b) {
		return a == wfst_type::eps ? b : a;
	}


	// cdr(a)
	static alpha_type cdr_(const alpha_type& a) {
		return wfst_type::eps;
	}

	// cdr(ab)
	static alpha_type cdr_(const alpha_type& a, const alpha_type& b) {
		return a == wfst_type::eps ? cdr_(b) : b;
	}


	// ab
	static alpha_type mix_(const alpha_type& a, const alpha_type& b) {
		assert(length_(a) + length_(b) < 2);
		return a == wfst_type::eps ? b : a;
	}

private:
	kPrivate::KtWfstFactory<wfst_type, state_type> factory_;
};
