#include "view.h"
#include "controller.h"
#include "model.h"
int main()
{
    View v;
    Model m;
    Controller c(&m);
    v.RenderUI(&c, &m);
    
    return 0;
}
