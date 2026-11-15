#include "TestSupport.h"

void runAVLTests();
void runOrderStatisticTests();
void runMaxHeapTests();
void runHeapSortTests();
void runBinaryTreeTests();

int main()
{
    runAVLTests();
    runOrderStatisticTests();
    runMaxHeapTests();
    runHeapSortTests();
    runBinaryTreeTests();

    std::printf("Model unit checks: %d assertions, %d failures.\n",
                vhTestAssertions, vhTestFailures);
    return vhTestFailures == 0 ? 0 : 1;
}
