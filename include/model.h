#pragma once
#include <pthread.h>
class Model
{
public:
    Model();
    ~Model();
    void run(); // BEGINS THE MODEL
    
    // Emscripten doesn't support std::thread for multithreading, only C-type pthread's. This will essentially be a pointer
    // to the run() function so we can actually pass it into pthread_create() in main.cpp
    static void *threadEntry(void *instance)
    {
        reinterpret_cast<Model *>(instance)->run();
        return nullptr;
    }
};
