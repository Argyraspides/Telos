#include "view.h"
#include "controller.h"
#include "model.h"
// Main code
int main()
{ 
    // Instantiate the view
    View v;

    // Provide controller to the view for
    Controller *c;
    v.RenderUI(c);

    return 0;
}
