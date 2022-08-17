#include "kaldi/KgKaldiModels.h"
#include "kaldi/KuKaldiIO.h"
#include "../extend/wfst/KtViterbiBeam.h"
#include <fstream>
#include "istreamx.h"


void decode_test()
{
	printf("test kaldi models & viterbi-beam decoder...  ");

	KgKaldiModels models("./data/");
	if (!models) {
		printf("  failed to load models from path './data/'\n");
		abort();
	}

	std::string key;
	std::vector<std::vector<double>> table;
	std::ifstream feats("./data/test-mfcc-delta.ark", std::ios_base::binary);
	auto isx = stdx::istreamx(feats, false);
	if (!KuKaldiIO::readTable(isx, key, table)) {
		printf("failed to load 'data/test-mfcc-delta.ark'\n");
		abort();
	}

	auto viterbi = KtViterbiBeam(models.wfst());
	int maxIter(5); // 最多重试5次
	auto pdf = [&models](unsigned transId, const std::vector<double>& feat) {
		return models.prob(transId, feat.data()); };

	while (!viterbi.search<std::vector<double>>(table.data(), table.size(), pdf)
		&& maxIter-- > 0) {
		viterbi.setBeam(2 * viterbi.beam());
	}

	if (maxIter > 0) {

		printf("\n");
		printf("  Overall log-likelihood per frame is %f over %d frames.\n",
			viterbi.totalWeight() / table.size(), table.size());

		auto path = viterbi.bestPath();
		auto words = models.getWords(path.cbegin(), path.cend());

		printf("  ");
		for (auto i : path)
			printf("%d ", i); 
		printf("\n");

		printf("  ");
		for (auto s : words)
			printf("%s ", s.c_str());
		printf("\n");
	}
}
