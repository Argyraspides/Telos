#include "model.h"
#include "view.h"
#include "controller.h"
#include <thread>

int main()
{

    Model m;
    std::thread modelThread(&Model::run, &m);
    Controller c(&m);
    View v(&c);
    modelThread.join();
}
