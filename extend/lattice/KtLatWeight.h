#pragma once
#include "../wfst/KtSemiring.h"


// lattice-weight由2个浮点数构成，分别代表graph cost和acoustic cost

template<typename T>
class KtLatWeight : public KtPairSemiring<KtTropicalSemiring<T>, KtTropicalSemiring<T>>
{
public:
	using super_ = KtPairSemiring<KtTropicalSemiring<T>, KtTropicalSemiring<T>>;
	using super_::super_;

	constexpr static bool isCompact() { return false; } // TODO: 其他更好的实现方法？

	static const KtLatWeight& one() {
		return (const KtLatWeight&)super_::one();
	}

	static const KtLatWeight& zero() {
		return (const KtLatWeight&)super_::zero();
	}


	// 重实现+操作符
	KtLatWeight operator+(const KtLatWeight& rhs) const {
		T cost0 = T(super_::value0()) + T(super_::value1());
		T cost1 = T(rhs.value0()) + T(rhs.value1());
		
		if (cost0 < cost1)
			return *this;
		else if (cost0 > cost1)
			return rhs;
		
		// cost0 == cost1
		return (super_::value0() + rhs.value0()) == super_::value0() ? *this : rhs;
	}

	template<typename U>
	KtLatWeight<T>& operator=(const KtLatWeight<U>& other) {
		auto val0 = U(other.value0());
		auto val1 = U(other.value1());

		super_::value0() = T(val0);
		super_::value1() = T(val1);
		return *this;
	}
};



// 参考kaldi实现的compact-lattic-weight
// 主要在KtLatWeight基础上，增加了一个整数序列，代表transition-ids

template<typename WT_TYPE, typename INT_TYPE>
class KtCompactLatWeight : public KtPairSemiring<WT_TYPE, std::vector<INT_TYPE>>
{
public:
	using super_ = KtPairSemiring<WT_TYPE, std::vector<INT_TYPE>>;
	using super_::super_;

	constexpr static bool isCompact() { return true; }

	KtCompactLatWeight(const WT_TYPE& wt) : super_{ wt, {0} } {}

	static const KtCompactLatWeight& zero() {
		static auto z = KtCompactLatWeight{ WT_TYPE::zero(), std::vector<INT_TYPE>() };
		return z;
	}

	static const KtCompactLatWeight& one() {
		static auto o = KtCompactLatWeight{ WT_TYPE::one(), std::vector<INT_TYPE>() };
		return o;
	}

	const WT_TYPE& weight() const { return super_::value0(); }
	WT_TYPE& weight() { return super_::value0(); }

	const std::vector<INT_TYPE>& string() const { return super_::value1(); }
	std::vector<INT_TYPE>& string() { return super_::value1(); }

	KtCompactLatWeight operator+(const KtCompactLatWeight& rhs) const {
		auto newstr = string(); 
		newstr.insert(newstr.end(), rhs.string().begin(), rhs.string().end());
		return KtCompactLatWeight{ weight() + rhs.weight(), newstr };
	}

	KtCompactLatWeight reverse() const {
		std::vector<INT_TYPE> rstr(string().size());
		std::reverse_copy(string().begin(), string().end(), rstr.begin());
		return { weight(), rstr };
	}

	template<typename WT1, typename INT1>
	KtCompactLatWeight& operator=(const KtCompactLatWeight<WT1, INT1>& other) {
		weight() = other.weight();
		string().assign(other.string().cbegin(), other.string().cend());
		return *this;
	}

	template<typename WT1>
	KtCompactLatWeight& operator=(const WT1& other) {
		weight() = other;
		string().clear();
		return *this;
	}

	operator const WT_TYPE& () const {
		return weight();
	}
};

