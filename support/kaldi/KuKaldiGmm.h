#pragma once
#include "../common/istreamx.h"
#include <vector>
#include <memory>

class KcMixModel;
using kGmm = KcMixModel;

// �ṩkalid-gmm�Ķ�ȡ֧��

class KuKaldiGmm
{
public:

	// ��ȡkaldi�ĵ���gmmģ��
	static bool loadSingle(stdx::istreamx& strm, kGmm& gmm);

	// ��ȡkaldi��gmmģ������
	static std::vector<std::shared_ptr<kGmm>> loadList(stdx::istreamx& strm);

	static std::vector<std::shared_ptr<kGmm>> loadList(const char* filePath);

};