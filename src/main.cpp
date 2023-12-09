#include "model.h"
#include "view.h"
#include "controller.h"

int main()
{
    Model m;
    Controller c(&m);
    View v(&c);
}
