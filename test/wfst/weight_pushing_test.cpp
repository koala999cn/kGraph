#include "WfstX.h"
#include "KtWeightPushing.h"
#include "../graph/test_util.h"


// 对val的n位小数四舍五入
template<typename T>
T round(const T& val, int n)
{
	int factor = 1;
	while (n-- > 0)
		factor *= 10;

	auto r = val * factor;
	r = round(r);
	r /= factor;
	return r;
}


void test_tropical_()
{
	using fst_type = StdWfstTrop<float, char>;
	fst_type fst(4);
	fst.setInitial(0, 0);
	fst.setFinal(3, 0.5f);
	fst.addTrans(0, 1, 'a', 'A', 1);
	fst.addTrans(0, 2, 'b', 'C', 0);
	fst.addTrans(1, 1, 'd', 'D', 1.2f);
	fst.addTrans(1, 3, 'c', 'B', 1);
	fst.addTrans(2, 3, 'e', 'E', 3);

	KtWeightPushing<fst_type> wp(fst);

	auto& g = fst;

	if (fst.initialWeight(0).value() != 2.5f ||
		fst.finalWeight(3).value() != 0 || 
		g.getEdge(0, 1).wt.value() != 0 ||
		g.getEdge(0, 2).wt.value() != 1 ||
		g.getEdge(1, 1).wt.value() != 1.2f ||
		g.getEdge(1, 3).wt.value() != 0 ||
		g.getEdge(2, 3).wt.value() != 0)
		test_failed(g, true);

	printf("  > passed\n"); fflush(stdout);
}


void test_log_()
{
	using fst_type = StdWfstLog<float, char>;
	fst_type fst(4);
	fst.setInitial(0, 0);
	fst.setFinal(3, 0.5f);
	fst.addTrans(0, 1, 'a', 'A', 1);
	fst.addTrans(0, 2, 'b', 'C', 0);
	fst.addTrans(1, 1, 'd', 'D', 1.2f);
	fst.addTrans(1, 3, 'c', 'B', 1);
	fst.addTrans(2, 3, 'e', 'E', 3);

	KtWeightPushing<fst_type> wp(fst);

	auto& g = fst;

	if (round(fst.initialWeight(0).value(), 2) != 1.91f ||
		fst.finalWeight(3).value() != 0 ||
		round(g.getEdge(0, 1).wt.value(), 3) != 0.229f ||
		round(g.getEdge(0, 2).wt.value(), 2) != 1.59f ||
		round(g.getEdge(1, 1).wt.value(), 1) != 1.2f ||
		round(g.getEdge(1, 3).wt.value(), 3) != 0.358f ||
		g.getEdge(2, 3).wt.value() != 0)
		test_failed(g, true);

	printf("  > passed\n"); fflush(stdout);
}


void weight_pushing_test()
{
	printf("weight-pushing test...\n");
	fflush(stdout);

	printf("   tropical semering");
	fflush(stdout);
	test_tropical_();

	printf("   log semering");
	fflush(stdout);
	test_log_();
}

