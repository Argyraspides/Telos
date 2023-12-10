#include "model.h"
#include "view.h"
#include "controller.h"
#include <thread>

int main()
{
    // Instantiate the physics engine
    Model m;
    // Run engine in separate thread
    pthread_t modelThreadId;
    pthread_create(&modelThreadId, nullptr, &Model::threadEntry, &m);

    Controller c(&m);
    View v(&c);

    pthread_join(modelThreadId, nullptr);
}
