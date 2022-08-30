#include <iostream>
#include "../../extend/lattice/KtLattice.h"
#include "../../support/kaldi/KuKaldiLat.h"


int main()
{
    kLattice lat;

    auto b = KuKaldiLat::load("../test/data/lat.1", lat);

    return 0;
}

