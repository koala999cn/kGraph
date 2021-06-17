#pragma once
#include <vector>

// 一个简单的矩阵模板实现
template<typename T, bool row_major = true>
class KtMatrix
{
public:
    typedef T value_type;
    typedef typename std::vector<value_type>::iterator element_iterator;
    typedef typename std::vector<value_type>::const_iterator const_element_iterator;


    KtMatrix() : rows_(0), cols_(0) {}
    KtMatrix(const KtMatrix&) = default;
    KtMatrix(KtMatrix&&) = default;
    KtMatrix& operator=(const KtMatrix&) = default;
    KtMatrix& operator=(KtMatrix&&) = default;

    KtMatrix(unsigned rows, unsigned cols, const T& initValue) :
        rows_(rows), cols_(cols) {
        data_.assign(rows * cols, initValue);
    }

    const T& operator()(unsigned row, unsigned col) const {
        return row_major ? data_[row * cols_ + col] : data_[col * rows_ + row];
    }

    T& operator()(unsigned row, unsigned col) {
        return row_major ? data_[row * cols_ + col] : data_[col * rows_ + row];
    }


    void reset(unsigned rows, unsigned cols, const T& val) {
        rows_ = rows, cols_ = cols;
        data_.assign(rows * cols, val);
    }


    auto rowBegin(unsigned row) const { return std::next(data_.begin(), row * cols_); }
    auto rowEnd(unsigned row) const { return std::next(data_.begin(), (row + 1) * cols_); }

	void eraseRow(unsigned row) { data_.erase(rowBegin(row), rowEnd(row)); --rows_; }
	void eraseCol(unsigned col) {
		--cols_;
		auto iter = std::next(data_.begin(), col);
		for (unsigned r = 0; r < rows_; r++) {
			iter = data_.erase(iter);
			if(r != rows_-1)
				iter = std::next(iter, cols_);
		}
	}


private:
    unsigned rows_, cols_;
    std::vector<value_type> data_;
};
