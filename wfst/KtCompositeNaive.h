#pragma once
#include "KtWfst.h"
#include "KtWfstFactory.h"



/* Navie Implementation of WFST's composition.
 * see Mehryar Mohri's <Weighted Automata Algorithms> for detail.
 *
 * 定义：
 *   T = Composite(T1, T2) = +[b∈B]{ T1(a, b)*T2(b, c) }
 *     - T1 = WFST(A, B, Q1, I1, F1, E1, λ1, ρ1).
 *     - T2 = WFST(B ,C, Q2, I2, F2, E2, λ2, ρ2).
 *     - T = WFST(A, C, Q, I, F, E, λ, ρ).
 *     - a∈A, b∈B, c∈C.
 *     - Q ∈ Q1×Q2.
 *     - I = I1×I2.
 *     - F = Q∩(F1×F2).
 *     - E = { (q1, q1'), a, c, w1*w2, (q2, q2') }, 式中{q1, a, b, w1, q2}∈E1, {q1', b, c, w2, q2'}∈E2
 *
 * 算法(epsilon-free版本)：
 * Weighted-Composition(T1, T2)
 * 1  Q ← I1×I2
 * 2  S ← I1×I2
 * 3  while !S.empty() do
 * 4      q = (q1, q2) ← Head(S)
 * 5      Dequeue(S)
 * 6      if q ∈ I1×I2 then
 * 7          I ← I∪{q}
 * 8          λ(q) ← λ1(q1)*λ2(q2)
 * 9      if q ∈ F1×F2 then
 * 10         F ← F∪{q}
 * 11         ρ(q) ← ρ1(q1)*ρ2(q2)
 * 12     for each (e1, e2)∈E1[q1]×E2[q2] such that o[e1]==i[e2] do
 * 13         if (q' = (n[e1], n[e2]) !∈ Q) then
 * 14             Q ← Q∪{q'}
 * 15             Enqueue(S, q')
 * 16         E ← E∪{(q, i[e1], o[e2],w[e1]*w[e2], q')}
 * 17 return T
 *
 * 上述代码中，n[e]表示转移e的next_state，即边的to点，o[e]表示e的输出符号，i[e]表示e的输入符号
 *
 *
 * 当T1或T2包含epsilon时，需要特殊考虑，可分三种情况：
 * (1)当o[e1]=eps时，(q1,q2)可以通过epsilon跳转到(n(e1),q2);
 * (2)当i[e2]=eps时，(q1,q2)可以通过epsilon跳转到(q1,n(e2));
 * (3)当o[e1]=i[e2]=eps时，(q1,q2)则可以跳转到3个状态：(n(e1),q2), (q1,n(e2)), (n(e1),n(e2)).
 * 对于(3)种情况，将会出现冗余路径，这时就需要过滤。
 * 最常用的过滤算法，是优先选择(n(e1),n(e2))，过滤掉(n(e1),q2), (q1,n(e2))，即优先走对角线.
 *
 */


template<typename FST1, typename FST2>
class KtCompositeNaive
{
	static_assert(std::is_same_v<typename FST1::weight_type, typename FST2::weight_type>, 
		"semiring not match for composition");
	static_assert(std::is_same_v<typename FST1::alpha_type, typename FST2::alpha_type>, 
		"alpha type not match for composition");
	static_assert(FST1::eps == FST2::eps, "epsilon not match for composition");

public:
	using state_index_t = typename FST1::state_index_t;
	using alpha_type = typename FST1::alpha_type;
	using composited_state_t = std::pair<state_index_t, state_index_t>;


	// 在构造函数中执行组合操作
	KtCompositeNaive(const FST1& fst1, const FST2& fst2) {
		
		auto I1s = fst1.initials(); 
		auto I2s = fst2.initials(); 
		
		for (auto I1 : I1s)
			for (auto I2 : I2s)
				// 增加初始状态{ I1, I2 }
				factory_.addInitial({ I1.first, I2.first }, I1.second * I2.second);

		while (!factory_.done()) {
			auto q = factory_.next();
			assert(q.second < factory_->order());

			auto iter1 = fst1.adjIter(q.first.first);
			for (; !iter1.isEnd(); ++iter1) {
				auto& t1 = iter1.edge();

				auto iter2 = fst2.adjIter(q.first.second);
				for (; !iter2.isEnd(); ++iter2) {
					auto& t2 = iter2.edge();

					state_index_t toIdx{};
					if (t1.osym == t2.isym)  // o[e1]=i[e2], 跳转至(n(e1), n(e2))
						toIdx = factory_.addTrans(q.second, { *iter1, *iter2 }, t1.isym, t2.osym, t1.wt * t2.wt);
					else if (t2.isym == FST2::eps) // i[e2]=eps, 跳转至(q1, n(e2))
						toIdx = factory_.addTrans(q.second, { q.first.first, *iter2 }, t2.isym, t2.osym, t2.wt);
					else if (t1.osym == FST1::eps) // o[e1]=eps, 跳转至(n(e1),q2);
						toIdx = factory_.addTrans(q.second, { *iter1, q.first.second }, t1.isym, t1.osym, t1.wt); // 可能会加入多次

					// 判断是否为final状态
					if (toIdx == factory_->order() - 1) { // 只检测可能的新增状态
						auto& s = factory_.state(toIdx);
						if (fst1.isFinal(s.first) && fst2.isFinal(s.second))
							factory_.setFinal(toIdx, fst1.finalWeight(s.first) * fst2.finalWeight(s.second));
					}
				}
			}
		}
	}

	// 组合是否成功？
	bool ok() const { return factory_.wfst().hasFinal(); }


	decltype(auto) wfst() { return factory_.wfst(); }
	decltype(auto) wfst() const { return factory_.wfst(); }

	auto operator->() { return factory_.operator->(); }
	auto operator->() const { return factory_.operator->(); }

private:
	kPrivate::KtWfstFactory<FST1, composited_state_t> factory_;
};

