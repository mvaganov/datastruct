#pragma once

#include "license.txt"

int randomInt();

void randomSeed(int a_seed);

/**
 * @return relatively random bit sequence (on a fast CPU). Use sparingly!
 * Expected runtime is about (a_numBits*2)+1 milliseconds (a little less),
 * during which 100% of CPU is used
 */
int randomIntTRNG(int a_numBits);

float randomFloat();

float randomFloat(float min, float max);
