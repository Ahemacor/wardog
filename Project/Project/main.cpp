#include "CommonDefinitions.h"

int main()
{
    GAME_INIT();

    BUILD_ACTIONS_FROM_CONFIG();

    const std::string& levelName = CONFIG.getStartLevelName();

    CONFIG.loadLevel(levelName);

    GAME_START();

    return 0;
}