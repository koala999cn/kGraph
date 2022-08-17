#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <fstream>
#include "../support/kaldi/KuKaldiIO.h"
#include "../support/kaldi/KuKaldiGmm.h"
#include "../common/istreamx.h"
#include "../hmm/KcMixModel.h"


void test_hmm()
{
	auto gmmlist = KuKaldiGmm::loadGmms("../test/data/gmmlist.mdl");
	if (gmmlist.empty()) {
		printf("failed to load 'test/data/gmmlist.mdl'");
		abort();
	}
	printf("loaded %d gmms from 'test/data/gmmlist.mdl'\n", int(gmmlist.size()));


	std::string key;
	std::vector<std::vector<double>> table;
	std::ifstream ark("../test/data/feature.39.ark", std::ios_base::binary);
	if (!ark) {
		printf("failed to open 'test/data/feature.39.ark'");
		abort();
	}

	// 此处istreamx的bin标志设为false，因为文件以key字符串开头，
	// 读取key之后，readTable将根据后续内容自动设置istreamx的bin标记
	if (!KuKaldiIO::readTable(stdx::istreamx(ark, false), key, table)) {
		printf("failed to read table from 'test/data/feature.39.ark'");
		abort();
	}
	printf("loaded %d features<dim = %d> from 'test/data/feature.39.ark'\n", 
		int(table.size()), int(table[0].size()));

	printf("calculating output probability of features...  ");
	std::vector<std::vector<double>> feature(table.size());
	for (unsigned i = 0; i < table.size(); i++) {
		feature[i].resize(gmmlist.size());
		for (unsigned j = 0; j < gmmlist.size(); j++)
			feature[i][j] = gmmlist[j]->prob(table[i].data());
	}
	printf("ok\n");

	// 加载比照数据
	std::vector<std::vector<double>> answer;
	std::ifstream loglikes("../test/data/loglikes.ark", std::ios_base::binary);
	if (!loglikes) {
		printf("failed to open 'test/data/loglikes.ark'");
		abort();
	}

	if (!KuKaldiIO::readTable(stdx::istreamx(loglikes, false), key, answer)) {
		printf("failed");
		abort();
	}

	assert(answer.size() == feature.size());

	for (unsigned i = 0; i < feature.size(); i++)
		for (unsigned j = 0; j < feature[i].size(); j++) {
			auto delta = feature[i][j] - answer[i][j];
			if (std::abs(delta) > 0.001) {
				printf("failed");
				abort();
			}
		}
}


int main(int argc, char const* argv[])
{
	printf("testing hmm...  \n");
	test_hmm();
	printf(" :) passed! press any key to exit.\n");
	getchar();

	return 0;
}