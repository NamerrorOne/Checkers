#include "Game/Game.h"
#include <nlohmann/json.hpp>
int WinMain(int argc, char* argv[])
{
    Game g;
    g.play();

    return 0;
}
