#include "license.txt"
#include <time.h>

#ifdef _WIN32
#define CLOCK clock
#else
#include <sys/time.h>	// for wall-clock timer (as opposed to clock cycle timer)
/** Linux keeps track of time this way. clock() returns CPU cycles, not time. */
long long clock_NIX()
{
	static timeval g_startTime = {0,0};
	if(!g_startTime.tv_sec)
		gettimeofday(&g_startTime, NULL);	// start the timer
	timeval now;
	__time_t seconds, useconds, ms;
	gettimeofday(&now, NULL);
	seconds  = now.tv_sec  - g_startTime.tv_sec;
	useconds = now.tv_usec - g_startTime.tv_usec;
	ms = seconds*1000 + useconds/1000;
	return ms;
}
#define CLOCK	clock_NIX
#endif
/**
 * @return relatively random bit sequence (on a fast CPU). Use sparingly!
 * Expected runtime is about (a_numBits*2)+1 milliseconds (a little less),
 * during which 100% of CPU is used
 */
int randomIntTRNG(int a_numBits)
{
    long instructionsTillOneMS, iter;
    int index = 0, result = 0;
    time_t now = CLOCK();
    while(CLOCK() == now); // start timing at the turn of the  millisecond
    for(int i = 0; i < a_numBits; ++i)
    {
        for(now = CLOCK(), instructionsTillOneMS = 0; CLOCK() == now;
            ++instructionsTillOneMS);
        for(now = CLOCK(), iter = 0; CLOCK() == now; ++iter);
        result |= (int)(iter > instructionsTillOneMS) << index++;
    }
    return result;
}

static unsigned int nSeed = 5223;

void randomSeed(int a_seed)
{
	nSeed = a_seed;
}

static int random()
{
	// Take the current seed and generate a new value 
	// from it. Due to our use of large constants and 
	// overflow, it would be very hard for someone to
	// predict what the next number is going to be from 
	// the previous one.
	nSeed = (8253729 * nSeed + 2396403);

	// return a value between 0 and 2.14 billion
	return nSeed  & 0x7fffffff;
}

int randomInt()
{
	return random();
}

float randomFloat()
{
	int i = random() & 0xffffff;
	return i / (float)(0x1000000);
}

float randomFloat(float min, float max)
{
	float delta = max-min;
	float number = randomFloat()*delta;
	number += min;
	return number;
}
