#include "kaldi/KuKaldiIO.h"
#include <assert.h>
#include <fstream>
#include "istreamx.h"


void kaldi_table_test()
{
	printf("test kaldi table...  ");

	std::string key;
	std::vector<std::vector<float>> table;	
	std::ifstream ark("../test/data/test-mfcc.ark", std::ios_base::binary);
	auto isx = stdx::istreamx(ark, false);
	if (!KuKaldiIO::readTable(isx, key, table) ||
		key != "TEST" ||
		table.size() != 979 || table[0].size() != 13) {
		printf("failed");
		abort();
	}

	printf("passed\n");
}
