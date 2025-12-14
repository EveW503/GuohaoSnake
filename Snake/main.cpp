#include "DataStructure.h"
#include "GameBase.h"

int main()
{
    while (true) {
        // 1. 菜单阶段
        UseEasyX menu;
        menu.initGraph(SCREEN_WIDTH, SCREEN_HEIGHT);

        // 获取菜单选择结果
        int choice = menu.drawMenu();

        // 处理特殊选项
        if (choice == 6) { // 退出游戏
            menu.close();
            break;
        }

        if (choice == 5) { // 查看历史记录
            RecordManager temp_mgr; // 临时创建一个管理器读取数据
            temp_mgr.loadRecords();
            menu.drawHistory(temp_mgr);
            continue; // 回到循环开头重新显示菜单
        }

        menu.close(); // 准备进入游戏

        // 2. 游戏实例化
        GameBase* game = nullptr;
        switch (choice) {
        case 1: game = new IntroGame(); break;
        case 2: game = new AdvancedGame(); break;
        case 3: game = new ExpertGame(); break;
            // 【新增】双人模式：P1(左侧,向右) vs P2(右侧,向左)
        case 4: game = new DualGame(5, 9, Direction::RIGHT, MAP_WIDTH - 6, 9, Direction::LEFT); break;
        }

        // 3. 运行
        if (game) {
            game->run();
            delete game;
            game = nullptr;
        }

        // 游戏结束后循环回来，再次显示主菜单
    }

    return 0;
}