#pragma once
#include <vector>
#include <string>
#include <memory>
#include "KuKaldiGmm.h"
#include "KgTransitionModel.h"
#include "../openfst/KuFstIO.h"
#include "../extend/wfst/WfstX.h"


// ��װkaldi�����������й�ģ�ͣ�������ѧģ�͡��������硢��ĸ��

class KgKaldiModels
{
public:
	// pathĿ¼�����������ļ�:
	//    final.mdl - hmm��ѧģ��
	//    HCLG.fst - wfst��������
	//    words.txt - ��ĸ��ӳ��
	KgKaldiModels(const std::string& path);

	// �����transId��ת�Ƹ��ʣ�����ʼ���㣨����״̬�����feat�ĸ���
	double prob(unsigned transId, const double* feat);

	// ok???
	operator bool() const;

	const MmapWfst& wfst() const { return *wfst_; }

	std::string dump(const char* prefix) const;

	// ����id���У�����symbol����
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
