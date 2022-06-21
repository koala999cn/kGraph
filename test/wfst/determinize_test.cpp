#include "WfstX.h"
#include "KtDeterminize.h"
#include "../graph/test_util.h"


using fst_type = StdWfstTrop<float, char>;


void test_determinize_()
{
	//fst_type fst(4);
	//fst.setInitial(0);
	//fst.setFinal(3, fst_type::weight_type::zero());
	//fst.addTrans(0, 1, 'a', 'b', 1);
	//fst.addTrans(0, 2, 'a', 'b', 2);
	//fst.addTrans(1, 1, 'b', 'b', 3);
	//fst.addTrans(2, 2, 'b', 'b', 3);
	//fst.addTrans(1, 3, 'c', 'b', 5);
	//fst.addTrans(2, 3, 'd', 'a', 6);

	fst_type fst(7);
	fst.setInitial(0, 0.2f);
	fst.setFinal(4, 0.3f);
	fst.setFinal(5, 0.1f);
	fst.setFinal(6, 0);

	fst.addTrans(0, 1, 'a', 'X', 0.5f);
	fst.addTrans(0, 2, 'a', 'Y', 1.2f);
	fst.addTrans(1, 4, 'b', '\0', 0);
	fst.addTrans(1, 5, 'c', 'Z', 0);
	fst.addTrans(2, 6, 'b', '\0', 1);
	fst.addTrans(2, 6, 'd', 'Z', 0);

	printf("   specific wfst |S| = %d, |T| = %d", fst.order(), fst.size()); 
	fflush(stdout);
	KtDeterminize<fst_type> det(fst);
	if (det->order() != 6 || det->size() != 8)
		test_failed(det.wfst());

	printf("  > passed\n"); fflush(stdout);
}


void determinize_test()
{
	printf("determinize test...\n");
	fflush(stdout);

	test_determinize_();
}

