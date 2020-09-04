#include "CommonDefinitions.h"

int main()
{
    GAME_INIT();

    BUILD_ACTIONS_FROM_CONFIG();

    GAME_LOAD_MAP();

    GAME_START();

    return 0;
}