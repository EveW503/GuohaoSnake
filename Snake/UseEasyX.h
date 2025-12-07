#pragma once
#include "easyx.h"
#include "DataStructure.h" // 需要用到 Point 和 GameMap 定义
#include "GameMap.h"
#include "RecordManager.h"
#include <deque>


class UseEasyX {
public:
    // 初始化图形界面
    void initGraph(int width, int height);

    // 关闭图形界面
    void close();

    // 【核心】绘制UI - 所有数据都通过参数传入
    // 变量名采取下划线 snake_case
    void drawUI(int current_score, int high_score, int snake_len, int hp, int game_time_seconds);

    // 绘制地图与蛇
    void drawMap(GameMap& map);
    void drawSnake(const std::deque<Point>& snake_body);

    // 绘制游戏结束画面
    void drawGameOver(int final_score);

    // 绘制菜单 (返回用户的选择结果)
    int drawMenu();

    std::string inputPlayerName();

    void drawRankings(const std::vector<Record>& records);
};