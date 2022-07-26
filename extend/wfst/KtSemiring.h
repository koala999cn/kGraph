#pragma once
#include <cmath>
#include <algorithm>
#include <limits>
#include "../graph/base/KtHolder.h"


// �뻷��ģ�������

template<typename T>
class KtSemiring : public KtHolder<T>
{
public:
	using super_ = KtHolder<T>;
	using super_::super_;
	using typename super_::value_type;
	using super_::inside;

	value_type value() const { return inside(); }


	// ��ֵ��һֵ�Ķ���
	static const KtSemiring& zero() { 
		static const KtSemiring z(0);
		return z; 
	}

	static const KtSemiring& one() { 
		static const KtSemiring o(1);
		return o;
	}

	// �ӷ��ͳ˷��Ķ���
	KtSemiring operator+(const KtSemiring& rhs) const { return inside() + rhs.inside(); }
	KtSemiring operator*(const KtSemiring& rhs) const { return inside() * rhs.inside(); }
};



template<typename T>
struct is_kind_of_semiring
{
	template<typename TT>
	static TT check(KtSemiring<TT>);
	static void check(...);

	using tt_type = decltype(check(std::declval(T)()));

	enum {
		value = !std::is_void<tt_type>::value
	};
};



// ����ΪһЩ�����뻷��ʵ��


// ��bool���͵��ػ�
template<>
class KtSemiring<bool> : public KtHolder<bool>
{
public:
	using super_ = KtHolder<bool>;
	using super_::super_;
	using typename super_::value_type;

	value_type value() const { return inside(); }


	// ��ֵ��һֵ�Ķ���
	static const KtSemiring& zero() {
		static const KtSemiring<bool> z(false);
		return z;
	}

	static const KtSemiring& one() {
		static const KtSemiring<bool> o(true);
		return o;
	}

	// �ӷ��ͳ˷��Ķ���
	KtSemiring operator+(const KtSemiring& rhs) const { return inside() + rhs.inside(); }
	KtSemiring operator*(const KtSemiring& rhs) const { return inside() * rhs.inside(); }
};



// ԭ�����ļӷ����㶨�壺x + y = -log(e^-x + e^-y), ��ֵΪinf.
template<typename T>
class KtLogSemiring : public KtSemiring<T>
{
public:
	using super_ = KtSemiring<T>;
	using super_::super_;
	using super_::value;

	static const KtLogSemiring& zero() {
		static const KtLogSemiring z(std::numeric_limits<T>::infinity());
		return z;
	}

	static const KtLogSemiring& one() {
		static const KtLogSemiring o(T(0));
		return o;
	}


	KtLogSemiring operator+(const KtLogSemiring& rhs) const { 
		if (value() == zero())
			return rhs.value();
		else if (rhs.value() == zero())
			return value();
		
		return -std::log(std::exp(-value()) + std::exp(-rhs.value()));
	}

	KtLogSemiring operator*(const KtLogSemiring& rhs) const { return value() + rhs.value(); }

	KtLogSemiring operator/(const KtLogSemiring& rhs) const { return value() - rhs.value(); }

	// compute s^-1 * this
	KtLogSemiring leftDiv(const KtLogSemiring& s) const { return value() - s.value(); }
};



// ��ֵ�����log�뻷�������ڸ���logֵ����
// x + y = log(e^x + e^y), ��ֵΪ-inf.
template<typename T>
class KtNegLogSemiring : public KtSemiring<T>
{
public:
	using super_ = KtSemiring<T>;
	using super_::super_;
	using super_::value;

	static const KtNegLogSemiring& zero() {
		static const KtNegLogSemiring z(-std::numeric_limits<T>::infinity());
		return z;
	}

	static const KtNegLogSemiring& one() {
		static const KtNegLogSemiring o(T(0));
		return o;
	}


	KtNegLogSemiring operator+(const KtNegLogSemiring& rhs) const {
		if (value() == zero())
			return rhs.value();
		else if (rhs.value() == zero())
			return value();
		else if (value() > rhs.value())
			return value() + std::log(1 + std::exp(rhs.value() - value()));
		else
			return rhs.value() + std::log(1 + std::exp(value() - rhs.value()));
	}

	KtNegLogSemiring operator*(const KtNegLogSemiring& rhs) const { return value() + rhs.value(); }

	KtNegLogSemiring operator/(const KtNegLogSemiring& rhs) const { return value() - rhs.value(); }

	// compute s^-1 * this
	KtNegLogSemiring leftDiv(const KtNegLogSemiring& s) const { return value() - s.value(); }
};


/* 
 * Convenient for shortest-path algorithms because global path weights are
 * guaranteed to be monotonic non-decreasing, viterbi approximation is fast
*/
template<typename T, bool doMax = false>
class KtTropicalSemiring : public KtSemiring<T>
{
public:
	using super_ = KtSemiring<T>;
	using super_::super_;
	using super_::value;

	static const KtTropicalSemiring& zero() {
		static const KtTropicalSemiring z(doMax ? -std::numeric_limits<T>::infinity() :
													std::numeric_limits<T>::infinity());
		return z;
	}

	static const KtTropicalSemiring& one() {
		static const KtTropicalSemiring o(T(0));
		return o;
	}


	KtTropicalSemiring operator+(const KtTropicalSemiring& rhs) const { 
		return doMax ? std::max(value(), rhs.value()) : std::min(value(), rhs.value());
	}

	KtTropicalSemiring operator*(const KtTropicalSemiring& rhs) const { return value() + rhs.value(); }

	KtTropicalSemiring operator/(const KtTropicalSemiring& rhs) const { return value() - rhs.value(); }

	// compute s^-1 * this
	KtTropicalSemiring leftDiv(const KtTropicalSemiring& s) const { return value() - s.value(); }
};

