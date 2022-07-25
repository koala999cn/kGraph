#pragma once
#include "istreamx.h"
#include <vector>
#include <memory>

class KcMixModel;
using kGmm = KcMixModel;

// �ṩkalid-gmm�Ķ�ȡ֧��

class KuGmm
{
public:

	// ��ȡkaldi�ĵ���gmmģ��
	static bool loadSingle(stdx::istreamx& strm, kGmm& gmm);

	// ��ȡkaldi��gmmģ������
	static std::vector<std::shared_ptr<kGmm>> loadList(stdx::istreamx& strm);

};