#include "kaldi/KgHmmTopo.h"
#include <sstream>
#include <assert.h>


void hmm_topo_test()
{
	std::string input_str = "<Topology>\n"
		"<TopologyEntry>\n"
		"<ForPhones> 1 2 3 4 5 6 7 8 9 </ForPhones>\n"
		"<State> 0 <PdfClass> 0\n"
		"<Transition> 0 0.5\n"
		"<Transition> 1 0.5\n"
		"</State> \n"
		"<State> 1 <PdfClass> 1 \n"
		"<Transition> 1 0.5\n"
		"<Transition> 2 0.5\n"
		"</State>  \n"
		" <State> 2 <PdfClass> 2\n"
		" <Transition> 2 0.5\n"
		" <Transition> 3 0.5\n"
		" </State>   \n"
		" <State> 3 </State>\n"
		" </TopologyEntry>\n"
		"  <TopologyEntry>\n"
		"  <ForPhones> 10 11 13  </ForPhones>\n"
		"  <State> 0 <PdfClass> 0\n"
		"  <Transition> 0 0.5\n"
		"  <Transition> 1 0.5\n"
		"  </State> \n"
		"  <State> 1 <PdfClass> 1 \n"
		"  <Transition> 1 0.5\n"
		"  <Transition> 2 0.5\n"
		"  </State>  \n"
		" <State> 2 </State>"
		"  </TopologyEntry>\n"
		"  </Topology>\n";

	std::string chain_input_str = "<Topology>\n"
		"<TopologyEntry>\n"
		"<ForPhones> 1 2 3 4 5 6 7 8 9 </ForPhones>\n"
		" <State> 0 <ForwardPdfClass> 0 <SelfLoopPdfClass> 1\n"
		"  <Transition> 0 0.5\n"
		"  <Transition> 1 0.5\n"
		" </State> \n"
		" <State> 1 </State>\n"
		"</TopologyEntry>\n"
		"</Topology>\n";

	printf("test kaldi hmm topo...  ");

	KgHmmTopo topo;
	std::istringstream iss(input_str);
	if (!topo.load(iss, false)) {
		printf("failed: %s\n", topo.errorText().c_str());
		abort();
	}

	if (topo.shortestLength(3) != 3 || topo.shortestLength(11) != 2) {
		printf("failed\n");
		abort();
	}

	std::istringstream chain_iss(chain_input_str);
	if (!topo.load(chain_iss, false)) {
		printf("failed: %s\n", topo.errorText().c_str());
		abort();
	}
	
	if (topo.shortestLength(3) != 1) {
		printf("failed\n");
		abort();
	}

	printf("passed\n");
}
