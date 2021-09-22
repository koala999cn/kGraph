#pragma once
#include <vector>


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
    using element_iterator = typename container::iterator;
    using const_element_iterator = typename container::const_iterator;


    KtMatrix() : rows_(0), cols_(0) {}
    KtMatrix(const KtMatrix&) = default;
    KtMatrix(KtMatrix&&) = default;
    KtMatrix& operator=(const KtMatrix&) = default;
    KtMatrix& operator=(KtMatrix&&) = default;

    KtMatrix(unsigned rows, unsigned cols, const_reference initValue) :
        rows_(rows), cols_(cols) {
        data_.assign(rows * cols, initValue);
    }

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


    auto rowBegin(unsigned row) const { return std::next(data_.begin(), row * cols_); }
    auto rowBegin(unsigned row) { return std::next(data_.begin(), row * cols_); }

    auto rowEnd(unsigned row) const { return std::next(data_.begin(), (row + 1) * cols_); }
    auto rowEnd(unsigned row) { return std::next(data_.begin(), (row + 1) * cols_); }

    // 将第row行赋值为val
    void assignRow(unsigned row, const_reference val) {
        std::fill(rowBegin(row), rowEnd(row), val);
    }

    // 将第col列赋值为val
    void assignCol(unsigned col, const_reference val) {
        auto iter = rowBegin(0);
        std::advance(iter, col);
        for (auto r = 1u; r < rows(); r++) {
            *iter = val;
            std::advance(iter, rows());
        } 

        *iter = val;
    }


    void eraseRow(unsigned row) { 
        data_.erase(rowBegin(row), rowEnd(row)); 
        --rows_; 
    }

    void eraseCol(unsigned col) {
        --cols_;
        auto iter = std::next(data_.begin(), col);
        for (unsigned r = 0; r < rows_; r++) {
            iter = data_.erase(iter);
            if(r != rows_-1)
                iter = std::next(iter, cols_);
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
