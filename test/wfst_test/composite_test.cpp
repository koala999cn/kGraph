#include "WfstX.h"
#include "KtComposite.h"
#include "KtCompositeNaive.h"
#include "KtCompositeFilter.h"
#include "../graph/test_util.h"


using fst_type = StdWfstTrop<float, char>;


void test_without_eps_()
{
	fst_type fst1(4);
	fst1.setInitial(0);
	fst1.setFinal(3);
	fst1.addTrans(0, 1, 'a', 'b', 0.1f);
	fst1.addTrans(1, 0, 'a', 'b', 0.2f);
	fst1.addTrans(1, 2, 'b', 'b', 0.3f);
	fst1.addTrans(1, 3, 'b', 'b', 0.4f);
	fst1.addTrans(2, 3, 'a', 'b', 0.5f);
	fst1.addTrans(3, 3, 'a', 'a', 0.6f);
	
	fst_type fst2(4);
	fst2.setInitial(0);
	fst2.setFinal(3);
	fst2.addTrans(0, 1, 'b', 'b', 0.1f);
	fst2.addTrans(1, 1, 'b', 'a', 0.2f);
	fst2.addTrans(1, 2, 'a', 'b', 0.3f);
	fst2.addTrans(1, 3, 'a', 'b', 0.4f);
	fst2.addTrans(2, 3, 'b', 'a', 0.5f);
	

	auto com = KtCompositeNaive(fst1, fst2);
	if (!com.ok() || com->size() != 8 || com->order() != 7)
		test_failed(com.wfst());

	printf("  > passed\n"); fflush(stdout);
}


void test_with_eps_()
{
	fst_type fst1(4);
	fst1.setInitial(0);
	fst1.addTrans(0, 1, 'a', 'e', 1);
	fst1.addTrans(1, 2, 'b', 0, 2);
	fst1.addTrans(2, 3, 'c', 'f', 0.5f);
	fst1.setFinal(3);

	fst_type fst2(5);
	fst2.setInitial(0);
	fst2.addTrans(0, 1, 'e', 'A', 2);
	fst2.addTrans(1, 2, 0, 'B', 0.2f);
	fst2.addTrans(2, 3, 0, 'C', 1);
	fst2.addTrans(3, 4, 'f', 'D', 0.5);
	fst2.setFinal(4);


	auto com = KtCompositeNaive(fst1, fst2);
	if (!com.ok() || com->size() != 4 || com->order() != 5)
		test_failed(com.wfst());
	
	KtEpsMatchFilter<fst_type, fst_type, '#'> filterM;
	auto comM = KtComposite(fst1, fst2, filterM);
	if (!comM.ok()) // TODO: 更全面的测试
		test_failed(comM.wfst());

	KtEpsSeqFilter<fst_type, fst_type, '#'> filterS;
	auto comS = KtComposite(fst1, fst2, filterS);
	if (!comS.ok()) // TODO: 更全面的测试
		test_failed(comS.wfst());

	printf("  > passed\n"); fflush(stdout);
}


void composite_test()
{
	printf("composition test...\n");
	fflush(stdout);

	printf("   without epsilon");
	fflush(stdout);
	test_without_eps_();

	printf("   with epsilon");
	fflush(stdout);
	test_with_eps_();
}

