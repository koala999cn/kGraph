#pragma once
#include <vector>
#include "KtRange.h"


// 一个简单的矩阵模板实现
template<typename T, bool row_major = true>
class KtMatrix
{
    static_assert(row_major, "col-major not supported yet");

public:
    using container = std::vector<T>;
    using value_type = typename container::value_type;
    using reference = typename container::reference;
    using const_reference = typename container::const_reference;

    using row_element_iter = typename container::iterator;
	using const_row_element_iter = typename container::const_iterator;
	using row_range = KtRange<row_element_iter>;
	using const_row_range = KtRange<const_row_element_iter>;
	
private:

	template<typename ELE_ITER>
	class col_element_iter_
	{
	public:
		using deref_type = decltype(*std::declval<ELE_ITER>());
		using const_deref_type = const deref_type;

		col_element_iter_(const ELE_ITER& iter, const ELE_ITER& end, unsigned offset) :
			iter_(iter), offset_(offset) {}

		col_element_iter_& operator++() {
			if (std::distance(iter_, end_) < offset_)
				std::advance(iter_, offset_);
			else
				iter_ = end_;

			return *this;
		}


		deref_type operator*() { return *iter_; }
		const_deref_type operator*() const { return *iter_; }


		bool operator==(const col_element_iter_& rhs) const {
			assert(end_ == rhs.end_ && offset_ == rhs.offset_);
			return iter_ == rhs.iter_;
		}


	private:
		ELE_ITER iter_, end_;
		unsigned offset_; // 一次前进的偏移量
	};


public:

	using col_element_iter = col_element_iter_<typename container::iterator>;
	using const_col_element_iter = col_element_iter_<typename container::const_iterator>;
	using col_range = KtRange<col_element_iter>;
	using const_col_range = KtRange<const_col_element_iter>;

	
	KtMatrix() : rows_(0), cols_(0) {}
    KtMatrix(const KtMatrix&) = default;
	KtMatrix(KtMatrix&& m) : data_(std::move(m.data_)) {
		rows_ = m.rows_;
		cols_ = m.cols_;
	}

    KtMatrix& operator=(const KtMatrix&) = default;
	KtMatrix& operator=(KtMatrix&& m) {
		rows_ = m.rows_;
		cols_ = m.cols_;
		std::swap(data_, m.data_);
	}

    KtMatrix(unsigned rows, unsigned cols, const_reference initValue) :
        rows_(rows), cols_(cols) {
        data_.assign(rows * cols, initValue);
    }


	// 访问矩阵的第row行、第col列元素
    const_reference operator()(unsigned row, unsigned col) const {
        return row_major ? data_[row * cols_ + col] : data_[col * rows_ + row];
    }

    reference operator()(unsigned row, unsigned col) {
        return row_major ? data_[row * cols_ + col] : data_[col * rows_ + row];
    }


    void resize(unsigned rows, unsigned cols, const_reference init_val) {
        rows_ = rows, cols_ = cols;
        data_.assign(rows * cols, init_val);
    }

    void reserve(unsigned rows, unsigned cols) { data_.reserve(rows * cols); }

    unsigned rows() const { return rows_; } // 行数
    unsigned cols() const { return cols_; } // 列数


    void insert(unsigned row, unsigned col, const_reference val) {
        data_[row * cols_ + col] = val;
    }


	// 返回第idx行的迭代范围
	auto row(unsigned idx) {
		row_element_iter start = std::next(data_.begin(), idx * cols_);
		return KtRange<row_element_iter>(start, std::next(start, rows_));
	}


	auto row(unsigned idx) const {
		const_row_element_iter start = std::next(data_.cbegin(), idx * cols_);
		return KtRange<const_row_element_iter>(start, std::next(start, rows_));
	}


	// 返回第idx列的迭代范围
	auto col(unsigned idx) {
		auto b = std::next(data_.begin(), idx);
		return col_range(col_element_iter(b, data_.end(), cols_), 
						col_element_iter(data_.end(), data_.end(), cols_));
	}

	auto col(unsigned idx) const {
		auto b = std::next(data_.cbegin(), idx);
		return const_col_range(const_col_element_iter(b, data_.cend(), cols_),
							const_col_element_iter(data_.cend(), data_.cend(), cols_));
	}


	// 将第row行所有数据置为val
	void assignRow(unsigned rowIdx, const_reference val) {
		row(rowIdx).fill(val);
	}

	// 将第col列所有数据置为val
	void assignCol(unsigned colIdx, const_reference val) {
		col(colIdx).fill(val);
	}


    void eraseRow(unsigned idx) { 
		auto range = row(idx);
        data_.erase(range.begin(), range.end());
        --rows_; 
    }

    void eraseCol(unsigned idx) {
		--cols_;
        auto iter = std::next(data_.begin(), idx);
        for (unsigned r = 0; r < rows_; r++) {
            iter = data_.erase(iter);
            if(r != rows_-1)
                std::advance(iter, cols_);
        }
    }


    // @val: 新增行的初始值
    void appendRow(const_reference val) {
        data_.resize(data_.size() + cols_, val);
        ++rows_;
    }


    // @val: 新增列的初始值
    void appendCol(const_reference val) {
        data_.resize(data_.size() + rows_);
        value_type* p = data_.data();
        for (unsigned r = rows_; r > 1/*第一行数据不用移动*/; r--) {
            value_type* src = p + r * cols_;
            value_type* dst = src + cols_;
            *--dst = val;
            for (unsigned c = 0; c < cols_; c++)
                *--dst = *--src;
        }

        ++cols_;
    }


private:
    unsigned rows_, cols_;
    container data_;
};
