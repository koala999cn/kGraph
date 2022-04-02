#pragma once
#include "KtHolder.h"


// KtHolder的拓展，封装内、外两个类型的基本操作
//  

namespace kPrivate
{
	template<typename OUTSIDE_TYPE, typename INSIDE_TYPE, bool trivial>
	class KtHolder2Impl_ : public KtHolder<INSIDE_TYPE>
	{
	public:
		using super_ = KtHolder<INSIDE_TYPE>;
		using outside_value_t = OUTSIDE_TYPE;
		using const_outside_value_t = std::add_const_t<outside_value_t>;
		using inside_value_t = INSIDE_TYPE;
		using const_inside_value_t = std::add_const_t<inside_value_t>;
		using super_::inside;

		KtHolder2Impl_() : oval_{}, super_{} {}
		KtHolder2Impl_(const KtHolder2Impl_& h) : oval_(h.oval_), super_{h} {}
		KtHolder2Impl_(KtHolder2Impl_&& h) : oval_(std::move(h.oval_)), super_{ std::move(h.inside()) } {}
		KtHolder2Impl_(const inside_value_t& h) : oval_{}, super_{ h } {}
		KtHolder2Impl_(inside_value_t&& h) : oval_{}, super_{ std::move(h) } {}


		KtHolder2Impl_& operator=(const KtHolder2Impl_& h) {
			oval_ = h.oval_; 
			inside() = h;
			return *this;
		}

		KtHolder2Impl_& operator=(KtHolder2Impl_&& h) {
			oval_ = std::move(h.oval_); 
			inside() = std::move(h.inside());
			return *this;
		}


		KtHolder2Impl_& operator=(const inside_value_t& ival) {
			inside() = ival;
			return *this;
		}

		KtHolder2Impl_& operator=(inside_value_t&& ival) {
			inside() = std::move(ival);
			return *this;
		}


		bool operator==(const KtHolder2Impl_& h) const { 
			return oval_ == h.oval_ && inside() == h.inside(); 
		}
		bool operator!=(const KtHolder2Impl_& h) const { 
			return oval_ != h.oval_ || inside() != h.inside();
		}


		outside_value_t& outside() { return oval_; }
		const_outside_value_t& outside() const { return oval_; }


	private:
		outside_value_t oval_;
	};


	template<typename OUTSIDE_TYPE, typename INSIDE_TYPE>
	class KtHolder2Impl_<OUTSIDE_TYPE, INSIDE_TYPE, true> : public KtHolder<INSIDE_TYPE>
	{
	public:
		using super_ = KtHolder<INSIDE_TYPE>;
		using outside_value_t = OUTSIDE_TYPE;
		using const_outside_value_t = std::add_const_t<outside_value_t>;
		using inside_value_t = INSIDE_TYPE;
		using const_inside_value_t = std::add_const_t<inside_value_t>;
		using super_::inside;

		KtHolder2Impl_() = default;
		KtHolder2Impl_(const KtHolder2Impl_& h) = default;
		KtHolder2Impl_(KtHolder2Impl_&& h) = default;
		~KtHolder2Impl_() = default;
		KtHolder2Impl_(const inside_value_t& h) : oval_{}, super_{ h } {}
		KtHolder2Impl_(inside_value_t&& h) : oval_{}, super_{ std::move(h) } {}

		KtHolder2Impl_& operator=(const KtHolder2Impl_& h) = default;
		KtHolder2Impl_& operator=(KtHolder2Impl_&& h) = default;

		KtHolder2Impl_& operator=(const inside_value_t& ival) {
			inside() = ival;
			return *this;
		}

		KtHolder2Impl_& operator=(inside_value_t&& ival) {
			inside() = std::move(ival);
			return *this;
		}

		bool operator==(const KtHolder2Impl_& h) const {
			return oval_ == h.oval_ && inside() == h.inside();
		}
		bool operator!=(const KtHolder2Impl_& h) const {
			return oval_ != h.oval_ || inside() != h.inside();
		}


		outside_value_t& outside() { return oval_; }
		const_outside_value_t& outside() const { return oval_; }


	private:
		outside_value_t oval_;
	};
}


template<typename OUTSIDE_TYPE, typename INSIDE_TYPE>
using KtHolder2 = kPrivate::KtHolder2Impl_<OUTSIDE_TYPE, INSIDE_TYPE, 
	std::is_trivial_v<OUTSIDE_TYPE> && std::is_trivial_v<INSIDE_TYPE>>;