#pragma once
#include <vector>
#include <assert.h>
#include "KtWeightor.h"
#include "KtShortestPath.h"
#include "../GraphX.h"


// �����������һ��st-�����ҳ�һ�������Ӷ�ʹ�ô�s��t���κ��������������и������ֵ��
// ��Ҫ���������㷨��һ������·��������㷨������Ԥ��-ѹ��������㷨

// �����������㷨�������ڡ���������residual network����������һ���������һ��������������������ԭ����ͬ�Ķ��㣬
// ����ԭ���еĸ����ߣ��������н���1����2���ߣ������¶��壺����ԭ���еĸ�����v-w��
//    ��fΪ������cΪ������ ��ô��
//    �����f != c�������������а���һ������Ϊc-f��ǰ��v-w�����������������������߻��������ѹ���������
//    �����f != 0�������������а���һ������Ϊf�Ļر�w-v����������ǰ���������������е�������Ҳ���ɻ��˵�������


// ������Ļ��࣬ʵ����������������
template<typename GRAPH, typename WEIGHTOR>
class KtMaxFlow
{
protected:
	using flow_type = typename WEIGHTOR::weight_type;
	using RGRAPH = DigraphSx<flow_type>; // ����������


	// ������g�����ƣ�
	//   1. gΪst���������ҽ���1��Դ�㡢1����㣻
	//   2. ��v-wΪg�ıߣ���w-v������g�ıߡ�
	KtMaxFlow(const GRAPH& g, unsigned s, unsigned t)
		: graph_(g), // ����ԭͼ����
		  rg_(g.template copy<RGRAPH, WEIGHTOR>()) { // ����������
		assert(s != t);
	}


public:

	// ���ر�v-w��������
	auto cap(unsigned v, unsigned w) const {
		return WEIGHTOR{}(graph_.getEdge(v, w));
	}


	// ���ر�v-w��������
	auto flow(unsigned v, unsigned w) const {
		assert(graph_.hasEdge(v, w));
		return rg_.getEdge(w, v);
	}


	// ���ر�v-w�������� := cap - flow
	auto residual(unsigned v, unsigned w) const {
		assert(graph_.hasEdge(v, w));
		return rg_.getEdge(v, w);
	}


	// ���ض���v��������
	auto outflow(unsigned v) const {
		flow_type f(0);

		auto iter = graph_.adjIter(v);
		for (; !iter.isEnd(); ++iter)
			f += flow(v, *iter);

		return f;
	}


	// ���ض���v��������
	auto inflow(unsigned v) const {
		flow_type f(0);

		for (unsigned w = 0; w < graph_.order(); w++)
			if (graph_.hasEdge(w, v))
				f += flow(w, v);

		return f;
	}


	// ���ض���v�ľ����� = ������ - ������ 
	auto netflow(unsigned v) const {
		return inflow(v) - outflow(v);
	}


protected:

	// ���������ı�v-w��������delta
	void addFlow_(unsigned v, unsigned w, flow_type delta) {
		assert(rg_.hasEdge(v, w));
		flow_type f = rg_.getEdge(v, w);
		f -= delta; // �����ߵĿ�������������delta
		if (f == 0)
			rg_.eraseEdge(v, w);
		else
			rg_.setEdge(v, w, f);

		// ��Ӧ�رߵĿ�������������delta
		if (rg_.hasEdge(w, v))
			rg_.setEdge(w, v, rg_.getEdge(w, v) + delta);
		else
			rg_.addEdge(w, v, delta);
	}


protected:
	const GRAPH& graph_; // ԭͼ
	RGRAPH rg_; // ������
};



// ����·��������㷨�Ļ���
// ����·��������㷨Ҳ��Ford-Fulkerson�㷨
// ����һ�����Ŵ�Դ�㵽����·������������ͨ�÷���
// ��������������ǰ�����޿յĻرߵ�·��������ͨ������ǰ���е�����ͬʱ���ٻر��е������������е�������
// �������ӵ������ɻر��е�����ǰ����δ����������Сֵ�����ơ�
// ���µ����У�����·������һ��ǰ�߱�������������һ���ر߱�Ϊ�ա�

// �����֮���������ı�Ȩֵ���ǿ����������ֵ����������������s��t��·������Ϊ����·����

// ����·�������㷨������2�֣�
//   һ�������������·���������ſ���ʹ���õ����������·�����䡣
//   �����������·�����������ű������ٵ�·�����䡣

// Ford-Fulkerson�㷨�κ�ʵ�����������·����Ŀ������V*M,
// �ҳ�һ�����������ʱ��ΪO(V*E*M)������ϡ������ΪO(V^2*M)�����У�MΪ�������ı�������
template<typename GRAPH, class WEIGHTOR>
class KtPathAugment : public KtMaxFlow<GRAPH, WEIGHTOR>
{
protected:
	using super_ = KtMaxFlow<GRAPH, WEIGHTOR>;
	using super_::super_; // ���빹�캯��
	using super_::rg_;
	using super_::addFlow_;
	using typename super_::flow_type;
	using typename super_::RGRAPH;


protected:

	// ������������·��pathR�Ŀ�������������·���ϵ���С��
	flow_type minFlow_(const std::vector<unsigned>& pathR) const {
		auto iter = pathR.begin();
		unsigned w = *iter++;
		unsigned v = *iter++;
		flow_type d = rg_.getEdge(v, w);

		while (iter != pathR.end()) {
			w = v;
			v = *iter++;
			flow_type x = rg_.getEdge(v, w);
			if (x < d) d = x;
		}

		return d;
	}


	// ����·��pathR��������
	void augment_(const std::vector<unsigned>& pathR, flow_type delta) {
		// ����·������delta
		auto iter = pathR.begin();
		auto w = *iter++;
		unsigned v;
		for (; iter != pathR.end(); w = v, ++iter) {
			v = *iter;
			addFlow_(v, w, delta);
		}
	}
};



// �������ȶ��������������������·���㷨
// ���㷨���������·����Ŀ������2*E*lgM���ҳ�ϡ������һ�����������ʱ��ΪO(V^2*lgM*lgV)��
// ������ʵ������������������·���㷨���õĵ���ԶԶС���������·���㷨���õĵ�����
// ���������ҵ�����·���Ĺ�������Ҫһ���ԸߵĿ�����
template<typename GRAPH, class WEIGHTOR = default_wtor<GRAPH>>
class KtMaxFlowPfs : public KtPathAugment<GRAPH, WEIGHTOR>
{
public:
	using super_ = KtPathAugment<GRAPH, WEIGHTOR>;
	using super_::rg_;
	using super_::augment_;
	using super_::minFlow_;	
	using typename super_::flow_type;
	using typename super_::RGRAPH;


	KtMaxFlowPfs(const GRAPH& g, unsigned s, unsigned t) : super_(g, s, t) {
		
		// ����������Ϊ·���ϵ���Сֵ
		struct adder {
			auto operator()(const flow_type& f1, const flow_type& f2) const {
				return std::min(f1, f2);
			}
		};

		// ȡ��������������·��
		using weightor = KtWeightorMax<KtWeightSelf<flow_type>, adder>;

		while (true) {
			KtSptPfs<RGRAPH, weightor> pfs(rg_, s);
			auto path = pfs.pathR(t);
			if (path.size() < 2)
				break;
			assert(path.back() == s && path.front() == t);

			// ����·����������������Ȩֵ
			assert(pfs.dist(t) == minFlow_(path));
			augment_(path, pfs.dist(t));
		}
	}
};



// ���ڹ�������������������·���㷨��Ҳ��Edmonds-Karp�㷨
// ���㷨���������·����Ŀ������V*E/2���ҳ�ϡ������һ�����������ʱ��ΪO(V^3)��
template<typename GRAPH, class WEIGHTOR = default_wtor<GRAPH>>
class KtMaxFlowBfs : public KtPathAugment<GRAPH, WEIGHTOR>
{
public:
	using super_ = KtPathAugment<GRAPH, WEIGHTOR>;
	using super_::rg_;
	using super_::augment_;
	using super_::minFlow_;
	using typename super_::flow_type;
	using typename super_::RGRAPH;


	KtMaxFlowBfs(const GRAPH& g, unsigned s, unsigned t) : super_(g, s, t) {

		// ȡ�������ٵ�·��
		using weightor = KtWeightorMin<KtWeightUnit<flow_type>, KtAdder<typename KtWeightUnit<flow_type>::weight_type>>;

		while (true) {
			KtSptPfs<RGRAPH, weightor> bfs(rg_, s);
			auto path = bfs.pathR(t);
			if (path.size() < 2)
				break;
			assert(path.back() == s && path.front() == t);

			// ����·����������������Ȩֵ
			augment_(path, minFlow_(path));
		}
	}
};



// Ԥ��-ѹ��������㷨
// ��Ԥ��(preflow)�������������������������������ļ��ϣ����������ϵ���С�ڵ��ڸñ�������
//    ���Ҷ���ÿ���ڲ����㣬���������ڵ�����������
// �������(active vertex)���������������������ڲ����㣨Լ��Դ��ͻ���ǲ���ģ���
// ��������(excess flow)����������������������֮�
// ������߶ȡ�ÿ�����㱻����һ���߶�ֵ�����У����h(t) = 0���������е�ÿ����u-v����h(u) <= h(v) + 1��
// ���ϸ�ߡ�������������h(u) = h(v) + 1��һ����u-v��

// �㷨������£����ڱߣ���
// -- ��ʼ����������߶Ⱥ�����ʼ�����ҳ������ӵ�Դ��ı߳������������⣬����������Ϊ0����
// -- ѭ�����²��裬ֱ��û�л����Ϊֹ��
//    1. ѡ��һ������㣻
//    2. ���뿪�ö���ĺϸ�ߣ�����еĻ���ѹ������
//    3. ���û�кϸ�ߣ������Ӷ���ĸ߶ȡ�
// �㷨ʱ�临�Ӷ�ΪO(V^2*E)
// �����㷨��û��ָ���߶Ⱥ�����ʲô�����ѡ�����㡢���ѡ��ϸ�ߡ�ÿ��ѹ��������ȵȣ�
// ͨ��������ѡ��Ĳ�ͬѡ�񣬿ɵõ���ͬ���㷨ʵ�֡�

// ���ڶ�����㷨ѡ��
//    1. ѡ��һ�������󣬳���ѡ�����ĺϸ�ߣ�ֱ���ö����Ϊ�ǻ�����޺ϸ��Ϊֹ��
//    2. ÿ�ξ�����ѹ����������

// ������һ�����ڶ����㷨��ʵ�֣�����һ�����廯ѡ�
//    1. ʹ��FIFO��������ѡ�����㡣
//    2. Դ��߶ȳ�ʼ��ΪV������Ϊ0��

// TODO: 1. ʹ�û��ڸ߶�ֵ�����ȶ��д���FIFO���У��ܻ�ø������ܡ�
//       2. �������Ĵ���ʵ�֡� 
template<typename GRAPH, class WEIGHTOR = default_wtor<GRAPH>>
class KtMaxFlowPre : public KtMaxFlow<GRAPH, WEIGHTOR>
{
public:
	using super_ = KtMaxFlow<GRAPH, WEIGHTOR>;
	using super_::rg_;
	using super_::addFlow_;
	using super_::netflow;
	using typename super_::flow_type;


	KtMaxFlowPre(const GRAPH& g, unsigned s, unsigned t) : super_(g, s, t) {

		// ������ĸ߶�
		std::vector<unsigned> height(g.order(), 0);	
		height[s] = g.order();

		// ������ĳ�����
		// Դ��ĳ�������ʼ��Ϊ�㹻���Ա��ڵ�һ��ѭ��ʱ���������ѹ��ϸ�ߡ�
		std::vector<flow_type> excessFlow(g.order(), 0);
		excessFlow[s] = std::numeric_limits<flow_type>::max();

		// ����㼯����ʼ��ΪԴ��
		std::queue<unsigned> activeVert; 
		activeVert.push(s);


		// Ԥ��-ѹ���㷨��ѭ��
		while (!activeVert.empty()) {

			unsigned v = activeVert.front(); activeVert.pop();
			if (excessFlow[v] == 0)
				continue;

			auto iter = rg_.adjIter(v);

			// ����addFlow_�ᶯ̬ɾ���ߣ��⽫�ƻ����������ݣ�������ͳһ�ռ��ڽӵ�
			std::vector<std::pair<unsigned, flow_type>> adjs;
			for (; !iter.isEnd(); ++iter)
				adjs.emplace_back(*iter, iter.value());

			for(const auto& i : adjs) {
				unsigned w = i.first;
				auto pushFlow = std::min(i.second, excessFlow[v]);

				// �Ժϸ�߽�����ѹ�����
				if (pushFlow > 0 && (v == s || height[v] == height[w] + 1)) {
					addFlow_(v, w, pushFlow);
					excessFlow[v] -= pushFlow, excessFlow[w] += pushFlow;
					if (w != s && w != t) {
						assert(netflow(w) == excessFlow[w]);
						activeVert.push(w);
					}
				}
			}

			// ���v���г��������ض�û�кϸ�ߣ�������v�ĸ߶ȣ������½�v�ӵ�����㼯
			if (v != s && v != t && excessFlow[v] > 0) {
				assert(netflow(v) == excessFlow[v]);
				height[v]++;
				activeVert.push(v);
			}
		}
	}
};

