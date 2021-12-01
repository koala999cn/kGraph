#pragma once
#include <algorithm>


template<typename ITER_TYPE>
class KtRange
{
public:
	using iter_type = ITER_TYPE;
	using deref_type = decltype(*std::declval<iter_type>());
	using const_deref_type = decltype(*std::declval<std::add_const_t<iter_type>>());
	using value_type = std::remove_reference_t<deref_type>;
	constexpr static bool is_const = std::is_const<deref_type>::value;


	KtRange() {}

	KtRange(const iter_type& first, const iter_type& last) :
		first_(first), last_(last) {}

	KtRange(const iter_type& first, unsigned count) :
		first_(first), last_(std::next(first, count)) {}

	KtRange(const KtRange& rhs) : 
		first_(rhs.first_), last_(rhs.last_) {}


	bool operator==(const KtRange& rhs) const {
		return first_ == rhs.first_ && last_ == rhs.last_;
	}


	// 作为容器使用

	iter_type& begin() { return first_; }
	iter_type& end() { return last_; }

	const iter_type& begin() const { return first_; }
	const iter_type& end() const { return last_; }

	auto size() const { return std::distance(first_, last_); }

	KtRange subRange(unsigned offset, unsigned size) const { 
		auto b = std::next(first_, offset);
		return KtRange(b, std::next(b, size));
	}

	
	// 作为迭代器使用

	iter_type& operator++() { return ++first_; }

	void advance(unsigned n) { std::advance(first_, n); }

	deref_type operator*() { return *first_; }
	const_deref_type operator*() const { return *first_; }

	bool empty() const { return first_ == last_; }


	// advance first_ until Pred(first_)
	template<typename Pred>
	void skipUntil(Pred pred) {	
		while (!empty() && !pred(first_))
			++first_;
	}


	// 对一些常用std算法的封装

	void sort() { std::sort(first_, last_); }
	template<class Compare> void sort(Compare comp) { std::sort(first_, last_, comp); }

	void reverse() { std::reverse(first_, last_); }

	void fill(const value_type& val) { std::fill(first_, last_, val); }

	auto count(const value_type& val) const { return std::count(first_, last_, val); }


private:
	iter_type first_, last_;
};

