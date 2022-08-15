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


// 读取kaldi的hmm-topology文件
// kaldi为每个phone定义一个hmm模型。从图论的角度看，每个hmm模型相当于一个子图，
// 图的顶点对应hmm模型的状态，图的边对应hmm模型的转移概率。
// hmm-topo定义了每个子图的拓扑结构，大部分hmm模型的拓扑结构是相同的，
// 只是边（转移概率）和顶点（发射概率）不同。
// 边的数值在TransitionModel给出。

class KgHmmTopo
{
public:
	struct KpPdfClass {
		std::int32_t forward; std::int32_t self_loop;
	};

	// 顶点对象的类型为KpPdfClass，代表各Hmm状态的<forward_pdf_class, self_loop_pdf_class>
	// 边的类型为float，代表Hmm转移概率
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

