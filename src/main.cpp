#include "app.hpp"
#include "context.hpp"

App gApp;

int main(int, char **) {
    gApp.OnInit();
    while (!gApp.ShouldExit()) {
        gApp.OnUpdate();
    }
    gApp.OnQuit();

    return 0;
}
