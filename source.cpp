#include <stdio.h>
#include "OpenCL.h"
#include <windows.h>

int main(int argc, char **argv)
{
    ShowAvailableDevicesCL();
    InitializeCL();

    static unsigned char frame[1000 * 1000 * 3],res[1000 * 1000];
    for (size_t i = 0; i < sizeof(frame); i++)
        frame[i] = rand() % 256;
    for (size_t i = 0; i < sizeof(res); i++)
        res[i] = rand() % 256;

    GetMemoryCL(sizeof(frame),sizeof(res), 1000, 1000);

    LARGE_INTEGER clock, start, end;
    QueryPerformanceFrequency(&clock);

    while (!GetAsyncKeyState(VK_ESCAPE))
    {

        WriteSourceCL(frame);

        QueryPerformanceCounter(&start);

        for (int i = 0; i < 1000; i++)
            ProcessCL();
        //cvtColor(frame, res, COLOR_BGR2GRAY);

        QueryPerformanceCounter(&end);

        ReadResultCL(res);
        printf("%lf[ms]\n", (double)(end.QuadPart - start.QuadPart) * 1000.0 / clock.QuadPart);
    }

    ReleaseMemoryCL();
}