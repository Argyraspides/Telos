#include "view.h"
#include "controller.h"
#include "model.h"
// Main code
int main(int, char**)
{ 
    View v;
    Controller *c;
    v.RenderUI(c);
    return 0;
}
