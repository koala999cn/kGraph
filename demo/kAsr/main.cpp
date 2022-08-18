#include <iostream>
#include <thread>
#include <conio.h>
#include "KtViterbiBeam.h"
#include "kaldi/KgKaldiModels.h"
#include "../../kAsrFront/src/KgAsrFrontFrame.h"


int main()
{
    std::cout << ">> loading kaldi models...\n";

	KgKaldiModels models("./data/");
	if (!models) {
		std::cout << "  failed to load models from path './data/'\n";
		return 1;
	}
	std::cout << models.dump("  ");

	auto viterbi = KtViterbiBeam(models.wfst());

	auto voice_notify = []() {
		std::cout << '-';
	};

	auto decode = [&viterbi, &models](const std::vector<std::vector<double>>& feats) {

		std::cout << "\n" << ">> picked " << feats.size() << " frames, now decoding...\n";

		int numIter(0); // 最多重试5次
		auto pdf = [&models](unsigned transId, const std::vector<double>& feat) {
			return models.prob(transId, feat.data()); };

		while (!viterbi.search<std::vector<double>>(feats.data(), feats.size(), pdf)
			&& ++numIter < 5) {
			viterbi.setBeam(2 * viterbi.beam());
		}

		auto path = viterbi.bestPath();

		if (path.empty()) {
			std::cout << "   search failed at 5 retries\n";
		}
		else {			
			std::cout << "   hitted at " << numIter << " retries.";
			std::cout << " Overall log-likelihood per frame is "
				<< viterbi.totalWeight() / feats.size() << "\n";

			std::cout << "   result: ";
			auto words = models.getWords(path.cbegin(), path.cend());
			for (auto s : words)
				std::cout << s << " ";
			std::cout << "\n";
		}
	};

	KgAsrFrontFrame front("front-end.json");
	if (!front.run(decode, voice_notify)) {
		std::cout << "  failed to run front-end\n";
		return 1;
	}

	std::cout << ">> start listening...\n";
	while (!_kbhit())
		std::this_thread::yield();

	return 0;
}
