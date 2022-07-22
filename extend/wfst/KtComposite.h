#pragma once
#include <tuple>
#include <vector>
#include "KtWfst.h"
#include "KtWfstFactory.h"
#include "KtAdjIterPlus.h"



/* 
 * KtNavieComposite的加强版，实现基于滤波的composite通用算法
 * 具体参考google研究论文A Filter-based Algorithm for Efficient Composition of Finite-State Transducers.
 *
 * 算法(filter版本)：
 * Weighted-Composition(T1, T2, Φ)
 * 1  Q ← I ← S ← I1×I2×{i3}
 * 2  for each q = (q1, q2, i3) ∈ I do λ(q) ← λ(q1)*λ2(q2)
 * 3  while !S.empty() do
 * 4      q = (q1, q2, q3) ← Head(S)
 * 5      Dequeue(S)
 * 6      if q ∈ F1×F2×Q3 and ρ3(q3) != 0 then
 * 7          F ← F∪{q}
 * 8          ρ(q) ← ρ1(q1)*ρ2(q2)*ρ3(q3)
 * 9      M ← {(e1',e2',q3') = ψ(e1,e2,q3) | e1∈E^L[q1], e2∈E^L[q2], q3'!=⊥}
 * 10     for each (e1',e2',q3')∈M do
 * 11         if (q' = (n[e1'], n[e2'], q3') !∈ Q) then
 * 12             Q ← Q∪{q'}
 * 13             Enqueue(S, q')
 * 14         E ← E∪{(q, i[e1'], o[e2'], w[e1']*w[e2'], q')}
 * 15 return T
 *
 */

template<typename FST1, typename FST2, typename FILTER>
class KtComposite
{
	static_assert(std::is_same_v<typename FST1::weight_type, typename FST2::weight_type>, 
		"semiring mismatch for composition");
	static_assert(std::is_same_v<typename FST1::weight_type, typename FILTER::weight_type>, 
		"filter's semiring mismatch");
	static_assert(std::is_same_v<typename FST1::alpha_type, typename FST2::alpha_type>, 
		"alpha type mismatch for composition");
	static_assert(FST1::eps == FST2::eps, "epsilon mismatch for composition");

public:

	using composited_state_t = std::tuple<typename FST1::state_index_t, 
										typename FST2::state_index_t, 
										typename FILTER::state_index_t>; 


	KtComposite(const FST1& fst1, const FST2& fst2, FILTER& filter) {
		auto I1s = fst1.initials();
		auto I2s = fst2.initials();
		auto i3 = filter.initial();

		for (auto& I1 : I1s)
			for (auto& I2 : I2s)
				// 增加初始状态{ I1, I2, i3 }
				factory_.addInitial({ I1.first, I2.first, i3 }, I1.second * I2.second);
			

		while (!factory_.done()) {
			auto q = factory_.next();
			assert(q.second < factory_.wfst().order());

			auto q1 = std::get<0>(q.first);
			auto q2 = std::get<1>(q.first);
			auto q3 = std::get<2>(q.first);

			typename FST1::trans_type loop1{ FST1::eps, FILTER::epsL, FST1::weight_type::one() };

			KtAdjIterPlus<const FST1> iter1(fst1, q1, { q1, loop1 }); // 构建带虚拟自环的迭代器
			for (; !iter1.isEnd(); ++iter1) {
				auto& t1 = iter1.edge();

				typename FST2::trans_type loop2{ FILTER::epsL, FST2::eps, FST2::weight_type::one() };

				KtAdjIterPlus<const FST2>  iter2(fst2, q2, { q2, loop2 }); // 构建带虚拟自环的迭代器
				for (; !iter2.isEnd(); ++iter2) {
					auto& t2 = iter2.edge();

					auto q3N = filter(q3, t1, t2);

					if (!filter.blocking(q3N)) {
						auto toIdx = factory_.addTrans(q.second, { *iter1, *iter2, q3N }, t1.isym, t2.osym, t1.wt * t2.wt);

						// 判断是否为final状态
						if (toIdx == factory_->order() - 1) { // 只检测可能的新增状态
							auto& s = factory_.state(toIdx);
							auto q1 = std::get<0>(s);
							auto q2 = std::get<1>(s);
							auto q3 = std::get<2>(s);
							if (fst1.isFinal(q1) && fst2.isFinal(q2) && filter.weight(q3) != FILTER::weight_type::zero())
								factory_.setFinal(toIdx, fst1.finalWeight(q1) * fst2.finalWeight(q2) * filter.weight(q3));
						}
					}
				}
			}
		} // while (!S_.empty())


		// trimming co-accessible

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