#include <iostream>
#include "../../extend/lattice/KtLattice.h"
#include "../../support/kaldi/KuKaldiLat.h"


int main()
{

    auto latlist = KuKaldiLat::load<kLatticed>("../test/data/nbest.txt");
    assert(latlist.size() == 10);
    auto acs = latlist.front().second->acousticCostsPerFrame();

    return 0;
}

