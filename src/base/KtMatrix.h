#pragma once
#include <vector>
#include <assert.h>
#include "KtRange.h"


// 一个简单的矩阵模板实现
template<typename T>
class KtMatrix
{
public:
    using container = std::vector<T>;
    using value_type = typename container::value_type;
    using reference = typename container::reference;
    using const_reference = typename container::const_reference;

    using row_element_iter = typename container::iterator;
    using const_row_element_iter = typename container::const_iterator;
    using row_range = KtRange<row_element_iter>;
    using const_row_range = KtRange<const_row_element_iter>;
    
  
    KtMatrix() = default;
    KtMatrix(const KtMatrix&) = default;
    KtMatrix(KtMatrix&& m) = default;
    ~KtMatrix() = default;
    KtMatrix& operator=(const KtMatrix&) = default;
    KtMatrix& operator=(KtMatrix&& m) = default;

    KtMatrix(unsigned rows, unsigned cols, const_reference initValue) :
        rows_(rows), cols_(cols) {
        data_.assign(rows * cols, initValue);
    }


    // 访问矩阵的第row行、第col列元素
    const_reference operator()(unsigned row, unsigned col) const {
        assert(row < rows_ && col < cols_);
        return row_major ? data_[row * cols_ + col] : data_[col * rows_ + row];
    }

    reference operator()(unsigned row, unsigned col) {
        assert(row < rows_ && col < cols_);
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
        return KtRange<row_element_iter>(start, cols_);
    }


    auto row(unsigned idx) const {
        const_row_element_iter start = std::next(data_.cbegin(), idx * cols_);
        return KtRange<const_row_element_iter>(start, cols_);
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
        auto src = data_.data() + (rows_ - 1) * cols_;
        auto dst = data_.end() - 1;
        for (unsigned r = 1; r < rows_; r++) { // 首行不移动，所以r从1起
            std::copy_backward(src, src + cols_, dst);
            *dst = val; // 新增列赋值
            src -= cols_;
            dst -= cols_ + 1;
        }

        *(src + cols_) = val; // 首行的新增列赋值

        ++cols_;
    }


private:
    unsigned rows_, cols_;
    container data_;
};
