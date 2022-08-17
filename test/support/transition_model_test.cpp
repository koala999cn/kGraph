#include "kaldi/KgTransitionModel.h"
#include "kaldi/KuKaldiIO.h"
#include <assert.h>
#include <fstream>


void transition_model_test()
{
	printf("test kaldi transition model...  ");

	KgTransitionModel tm;
	
	std::ifstream ifs("../test/data/final.mdl", std::ios_base::binary);
	bool bin = KuKaldiIO::binaryTest(ifs);
	if (!tm.load(ifs, bin) || tm.numPdfs() != 1656 || tm.numTrans() != 3502
		|| tm.numStates() != 1747) {
		printf("failed");
		abort();
	}

	printf("passed\n");
}