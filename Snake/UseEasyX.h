#pragma once
#include "easyx.h"
#include "DataStructure.h" 
#include "GameMap.h"
#include "RecordManager.h" // 确保引用了 Record 定义
#include <deque>
#include <string>
#include <vector>

class UseEasyX {
public:
    void initGraph(int width, int height);
    void close();

    // 【修改】增加 is_paused 参数，用于显示“暂停/继续”按钮的不同文字
    void drawUI(int current_score, int high_score, int snake_len, int hp, int game_time_seconds, bool is_paused);

    void drawMap(GameMap& map);
    void drawSnake(const std::deque<Point>& snake_body, COLORREF body_color, COLORREF head_color);
    void drawGameOver(int final_score);

    // 【修改】菜单现在返回 1-5 (包含历史和退出)
    int drawMenu();

    std::string inputPlayerName();

    void drawRankings(const std::vector<Record>& records);

    // 【新增】绘制历史记录界面
    void drawHistory(RecordManager& mgr);

    // 【新增】检查游戏内按钮点击 (返回 0:无, 1:暂停, 2:返回菜单)
    int checkGameButtons(int mouse_x, int mouse_y);

private:
    // 【新增】内部辅助：画一个按钮
    void drawButton(int x, int y, int w, int h, LPCTSTR text, COLORREF bg_color);
    // 【新增】内部辅助：判断点击是否在区域内
    bool isClickIn(int mouse_x, int mouse_y, int x, int y, int w, int h);
};