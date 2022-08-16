#pragma once
#include <vector>
#include <string>
#include <memory>
#include "KuKaldiGmm.h"
#include "KgTransitionModel.h"
#include "../openfst/KuFstIO.h"
#include "../extend/wfst/WfstX.h"


// 封装kaldi的语音解码有关模型：包括声学模型、解码网络、字母表

class KgKaldiModels
{
public:
	// path目录下须有如下文件:
	//    final.mdl - hmm声学模型
	//    HCLG.fst - wfst解码网络
	//    words.txt - 字母表映射
	KgKaldiModels(const std::string& path);

	// 计算边transId（转移概率）的起始顶点（隐藏状态）输出feat的概率
	double prob(unsigned transId, const double* feat);

	// ok???
	operator bool() const;

	const MmapWfst& wfst() const { return *wfst_; }

	std::string dump(const char* prefix) const;

	// 输入id序列，返回symbol序列
	template<typename ID_ITER>
	auto getWords(ID_ITER begin, ID_ITER end) const {
		std::vector<std::string> words;
		words.reserve(std::distance(begin, end));
		for (auto iter = begin; iter != end; ++iter)
			words.push_back(st_->getSymbol(*iter));
		return words;
	}

private:
	KgTransitionModel tm_;
	std::vector<std::shared_ptr<kGmm>> pdfs_;
	std::unique_ptr<MmapWfst> wfst_;
	std::unique_ptr<KgSymbolTable> st_;
};
