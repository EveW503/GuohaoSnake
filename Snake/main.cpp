#include "DataStructure.h"
#include "GameBase.h"

int main()
{
    // 1. 菜单阶段
    UseEasyX menu;
    menu.initGraph(SCREEN_WIDTH, SCREEN_HEIGHT);
    int choice = menu.drawMenu();
    menu.close(); // 关闭菜单窗口，准备进入游戏窗口

    // 2. 游戏实例化 (工厂模式思路)
    GameBase* game = nullptr;

    switch (choice) {
    case 1:
        game = new IntroGame();
        break;
    case 2:
        game = new AdvancedGame();
        break;
    case 3:
        game = new ExpertGame();
        break;
    default:
        game = new IntroGame();
        break;
    }

    // 3. 运行与清理
    if (game) {
        game->run();
        delete game;
        game = nullptr;
    }

    return 0;
}