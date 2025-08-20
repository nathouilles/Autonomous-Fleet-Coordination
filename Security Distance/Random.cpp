#include "Random.h"

#include <random>
#include <chrono>
using namespace std;



//
// alea
// renvoit une valeur entre 0 et 1, loi uniforme,
//

double Random::alea()
{
    static mt19937_64 rng;
    static uint64_t timeSeed = chrono::high_resolution_clock::now().time_since_epoch().count();
    static seed_seq ss{ uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed >> 32) };

    static bool didit = false;
    if (!didit)
    {
        rng.seed(ss);
        didit = true;
    }

    // initialize a uniform distribution between 0 and 1
    static std::uniform_real_distribution<double> unif(0, 1);

    return unif(rng);
}

Random::~Random()
{
}

