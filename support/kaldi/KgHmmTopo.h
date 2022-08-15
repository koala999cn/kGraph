#pragma once
#include <istream>
#include "../graph/GraphX.h"


#define DECLARE_DEFAULT(class_name) \
    class_name() = default; \
	class_name(const class_name&) = default; \
    class_name(class_name&&) = default; \
    class_name& operator=(const class_name&) = default; \
    class_name& operator=(class_name&&) = default; \
    ~class_name() = default;


// ��ȡkaldi��hmm-topology�ļ�
// kaldiΪÿ��phone����һ��hmmģ�͡���ͼ�۵ĽǶȿ���ÿ��hmmģ���൱��һ����ͼ��
// ͼ�Ķ����Ӧhmmģ�͵�״̬��ͼ�ı߶�Ӧhmmģ�͵�ת�Ƹ��ʡ�
// hmm-topo������ÿ����ͼ�����˽ṹ���󲿷�hmmģ�͵����˽ṹ����ͬ�ģ�
// ֻ�Ǳߣ�ת�Ƹ��ʣ��Ͷ��㣨������ʣ���ͬ��
// �ߵ���ֵ��TransitionModel������

class KgHmmTopo
{
public:
	struct KpPdfClass {
		std::int32_t forward; std::int32_t self_loop;
	};

	// ������������ΪKpPdfClass�������Hmm״̬��<forward_pdf_class, self_loop_pdf_class>
	// �ߵ�����Ϊfloat������Hmmת�Ƹ���
	using kHmm = DigraphSx<float, KpPdfClass>;

	DECLARE_DEFAULT(KgHmmTopo)

	bool load(const std::string& fileName);
	bool load(std::istream &strm, bool binary);
	
	const std::string& errorText() const { return err_; }

	const kHmm* topologyForPhone(int phone) const;

	int numPdfClasses(int phone) const;

	// return -1 for error
	int shortestLength(int phone) const;

private:
	bool loadText_(std::istream &strm);
	bool loadBin_(std::istream &strm);

private:
	std::string err_;

	std::vector<std::int32_t> phones_;  // list of all phones we have topology for.  Sorted, uniq.  no epsilon (zero) phone.
	std::vector<std::int32_t> phone2idx_;  // map from phones to indexes into the entries vector (or -1 for not present).
	std::vector<kHmm> entries_;
};

