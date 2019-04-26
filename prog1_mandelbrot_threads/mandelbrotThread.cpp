#include <stdio.h>
#include <pthread.h>

#include "CycleTimer.h"

typedef struct {
    float x0, x1;
    float y0, y1;
    unsigned int width;
    unsigned int height;
    int maxIterations;
    int* output;
    int threadId;
    int numThreads;
    int startRow;
    int stripHeight;
} WorkerArgs;


extern void mandelbrotSerial(
    float x0, float y0, float x1, float y1,
    int width, int height,
    int startRow, int numRows,
    int maxIterations,
    int output[]);


//
// workerThreadStart --
//
// Thread entrypoint.
void* workerThreadStart(void* threadArgs) {
    double startTime = CycleTimer::currentSeconds();
    WorkerArgs* args = static_cast<WorkerArgs*>(threadArgs);

    // TODO: Implement worker thread here.

    // printf("Hello world from thread %d\n", args->threadId);

    // int height = args->height;
    // printf ("%d %d\n", args->numThreads, args->threadId);
    // int startRow = height / (args->numThreads) * (args->threadId); 
    // int endRow = height / (args->numThreads) * (args->threadId + 1);
    // endRow = endRow < height ? endRow : height;
    // int totalRows = height / (args->numThreads);

    // if (args->threadId == args->numThreads - 1)
        // totalRows = height - startRow;
    mandelbrotSerial (args->x0, args->y0, args->x1, args->y1, 
        args->width, args->height, args->startRow, args->stripHeight,
        args->maxIterations, args->output);

    double endTime = CycleTimer::currentSeconds();
    printf ("Thread %d cost %lf to complete.\n", args->threadId, endTime - startTime);
    return NULL;
}

//
// MandelbrotThread --
//
// Multi-threaded implementation of mandelbrot set image generation.
// Multi-threading performed via pthreads.
void mandelbrotThread(
    int numThreads,
    float x0, float y0, float x1, float y1,
    int width, int height,
    int maxIterations, int output[])
{
    const static int MAX_THREADS = 32;

    if (numThreads > MAX_THREADS)
    {
        fprintf(stderr, "Error: Max allowed threads is %d\n", MAX_THREADS);
        exit(1);
    }

    pthread_t workers[MAX_THREADS];
    WorkerArgs args[MAX_THREADS];

    int stripHeight = height / numThreads;

    // Map
    for (int i=0; i<numThreads; i++) {
        // TODO: Set thread arguments here.
        args[i].threadId = i;

        // Adding arguments..
        args[i].x0 = x0;
        args[i].y0 = y0;
        args[i].x1 = x1;
        args[i].y1 = y1;
        args[i].width = width;
        args[i].height = height;
        args[i].maxIterations = maxIterations;
        args[i].output = new int[width * height];
        args[i].threadId = i;
        args[i].numThreads = numThreads;
        args[i].startRow = i * stripHeight;
        args[i].stripHeight = stripHeight;
        if (i == numThreads)
            args[i].stripHeight = height - i * stripHeight;

    }

    // Fire up the worker threads.  Note that numThreads-1 pthreads
    // are created and the main app thread is used as a worker as
    // well.

    for (int i=1; i<numThreads; i++)
        pthread_create(&workers[i], NULL, workerThreadStart, &args[i]);

    workerThreadStart(&args[0]);

    // wait for worker threads to complete
    for (int i=1; i<numThreads; i++)
        pthread_join(workers[i], NULL);

    // Reduce
    for(int i = 0; i < numThreads; i++){
        for(int j = i * width * stripHeight; j < (i + 1) * width * stripHeight; j++){
            output[j] = args[i].output[j];
        }
    }
}


/*
Intel(R) Xeon(R) CPU E3-1230 V2 @ 3.30GHz
1.98x speedup from 2 threads
2.44x speedup from 4 threads
3.84x speedup from 8 threads
5.94x speedup from 16 threads
*/
