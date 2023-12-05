#include "model.h"
#include "view.h"
#include "controller.h"

int main()
{
    Model m;
    View v;
    Controller c(&m);
    
    v.RenderUI(&c, &m);
}
