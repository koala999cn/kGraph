#pragma once
#include "../common/istreamx.h"
#include <vector>
#include <memory>

class KcMixModel;
using kGmm = KcMixModel;

// 提供kalid-gmm的读取支持

class KuKaldiGmm
{
public:

	// 读取kaldi的单个gmm模型
	static bool loadGmm(stdx::istreamx& strm, kGmm& gmm);

	// 读取kaldi的gmm模型序列
	static std::vector<std::shared_ptr<kGmm>> loadGmms(stdx::istreamx& strm);

	static std::vector<std::shared_ptr<kGmm>> loadGmms(const char* filePath);

};