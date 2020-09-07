#include "CommonDefinitions.h"

int main()
{
    GAME_INIT();

    const std::string& levelName = CONFIG.getStartLevelName();

    CONFIG.loadLevel(levelName, GAME_INSTANCE.scene);

    GAME_START();

    return 0;
}