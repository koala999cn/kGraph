#include <stdio.h>
#include <stdlib.h>


extern void hmm_topo_test();
extern void kaldi_table_test();
extern void transition_model_test();
extern void decode_test();

int main(int argc, char const* argv[])
{
	hmm_topo_test();
	kaldi_table_test();
	transition_model_test();
	decode_test();

	printf(" :) All passed! press any key to exit.\n");
	getchar();

	return 0;
}
