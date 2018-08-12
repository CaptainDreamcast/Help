#include <prism/wrapper.h>

void loadCountries();

extern Screen WorldScreen;

void resetDays();
void addCurrentPopulation(int tVal);
void addCurrentGrowth(int tVal);
void addWarValues(int tPop1, int tPop2, int tGrowth1, int tGrowth2);
char* getShortFirstWarCountry();
char* getShortSecondWarCountry();