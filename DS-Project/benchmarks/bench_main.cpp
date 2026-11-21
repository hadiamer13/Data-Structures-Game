#include <cstdio>
#include <cstring>

bool g_recordBenchEvents = false;

void runHeightSearchBench();
void runRotationsKthBench();
void runHeapBench();
void runSortingBench();

int main(int argc, char** argv)
{
    for (int i = 1; i < argc; i++)
    {
        if (std::strcmp(argv[i], "--events") == 0)
        {
            g_recordBenchEvents = true;
        }
        else if (std::strcmp(argv[i], "--no-events") == 0)
        {
            g_recordBenchEvents = false;
        }
        else if (std::strcmp(argv[i], "--help") == 0)
        {
            std::printf("Usage: benchmarks [--events|--no-events]\n");
            return 0;
        }
        else
        {
            std::fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return 2;
        }
    }
    std::printf("Vault Heist benchmarks (event recording %s)\n", g_recordBenchEvents ? "enabled" : "disabled");
    runHeightSearchBench();
    runRotationsKthBench();
    runHeapBench();
    runSortingBench();
    return 0;
}
