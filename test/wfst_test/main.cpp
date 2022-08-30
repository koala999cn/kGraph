#include <stdio.h>
#include <stdlib.h>
#include <time.h>


extern void composite_test();
extern void determinize_test();
extern void weight_pushing_test();
extern void minimization_test();
extern void epsilon_closure_test();
extern void epsilon_removal_test();
extern void synchronize_test();
extern void support_test();


void test_fst()
{
	composite_test(); printf("\n");
	determinize_test(); printf("\n");
	weight_pushing_test(); printf("\n");
	minimization_test(); printf("\n");
	epsilon_closure_test(); printf("\n");
	epsilon_removal_test(); printf("\n");
	synchronize_test(); printf("\n");
}


int main(int argc, char const *argv[])
{
	srand(static_cast<unsigned int>(time(nullptr)));

	test_fst();

	printf(" :) All passed! press any key to exit.\n");
	getchar();

	return 0;
}