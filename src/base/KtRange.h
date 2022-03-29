#pragma once
#include <algorithm>


template<typename ITER_TYPE>
class KtRange
{
public:
    using iterator = ITER_TYPE;
    using const_iterator = std::add_const_t<iterator>;
    using deref_type = decltype(*std::declval<iterator>());
    using const_deref_type = decltype(*std::declval<const_iterator>());
    using value_type = std::remove_reference_t<deref_type>;
    constexpr static bool is_const = std::is_const<deref_type>::value;


    KtRange() {}

    KtRange(const iterator& first, const iterator& last) :
        first_(first), last_(last) {}

    KtRange(const iterator& first, unsigned count) :
        first_(first), last_(std::next(first, count)) {}

    KtRange(const KtRange& rhs) : 
        first_(rhs.first_), last_(rhs.last_) {}


    bool operator==(const KtRange& rhs) const {
        return first_ == rhs.first_ && last_ == rhs.last_;
    }


    // 作为容器使用

	iterator& begin() { return first_; }
	iterator& end() { return last_; }

    const iterator& begin() const { return first_; }
    const iterator& end() const { return last_; }

    auto size() const { return std::distance(first_, last_); }

    KtRange subrange(unsigned offset, unsigned size) const { 
        auto b = std::next(first_, offset);
        return KtRange(b, std::next(b, size));
    }

    deref_type at(unsigned idx) { return *std::next(first_, idx); }
    const_deref_type at(unsigned idx) const { return *std::next(first_, idx); }


    // 作为迭代器使用

	iterator& operator++() { return ++first_; }

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
	iterator first_, last_;
};
