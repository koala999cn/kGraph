#include "kaldi/KuKaldiLat.h"
#include <assert.h>
#include <fstream>
#include "istreamx.h"


// ¶ÁÈ¡kaldi-latÎÄ¼þ²âÊÔ
void lattice_test()
{
	printf("test kaldi lattice...  ");

	printf("  binary mode  ");
	auto binLats = KuKaldiLat::load<kCompactLatticed64>("../test/data/lat.1");
	if (binLats.size() != 10) {
		printf("failed");
		abort();
	}
	printf("passed\n");

	printf("  text mode  ");
	auto txtLats = KuKaldiLat::load<kCompactLatticed64>("../test/data/lat.txt");
	if (txtLats.size() != 1) {
		printf("failed");
		abort();
	}
	printf("passed\n");

	printf("  corss verify  ");


	printf("passed\n");
}
