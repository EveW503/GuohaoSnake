#pragma once
#include "easyx.h"
#include "DataStructure.h" 
#include "GameMap.h"
#include "RecordManager.h" 
#include <deque>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime> 
#include <random> 

// --- VA-11 HALL-A 主题配色 ---
// 背景色：深邃的午夜紫
const COLORREF VA_BG_COLOR = RGB(22, 18, 35);
// 墙壁颜色：稍亮的紫色网格
const COLORREF VA_WALL_COLOR = RGB(59, 36, 66);
const COLORREF VA_WALL_BORDER = RGB(100, 80, 120);
// UI 文字颜色：复古终端绿
const COLORREF VA_TEXT_COLOR = RGB(0, 255, 128);
// 装饰线条/按钮边框：青色
const COLORREF VA_ACCENT_COLOR = RGB(0, 240, 255);
// 警告/高亮/食物：霓虹粉
const COLORREF VA_NEON_PINK = RGB(255, 42, 109);
// P1 蛇身：青色 (Jill 风格)
const COLORREF VA_P1_BODY = RGB(0, 120, 140);
const COLORREF VA_P1_HEAD = RGB(0, 255, 255);
// P2 蛇身：红色/粉色 (Dorothy 风格)
const COLORREF VA_P2_BODY = RGB(160, 0, 60);
const COLORREF VA_P2_HEAD = RGB(255, 42, 109);

class UseEasyX {
public:

    static bool is_graph_initialized;  

    void initGraph(int width, int height);

    void close();

    void drawUI(int current_score, int high_score, int snake_len, int hp, int game_time_seconds, bool is_paused);

    void drawDualUI(int score1, int score2, int game_time_seconds, bool is_paused);

    void drawMap(GameMap& map);

    void drawSnake(const std::deque<Point>& snake_body, COLORREF body_color, COLORREF head_color);

    void drawGameOver(int final_score);

    int drawMenu();  //返回值来判断点击哪里

    std::string inputPlayerName();

    void drawRankings(const std::vector<Record>& records);  //游戏结束显示分数前十名

    void drawHistory(RecordManager& mgr);

    int checkGameButtons(int mouse_x, int mouse_y, int offset_y = 0);

    void drawDualGameOver(int winner);

private:
    //内部辅助：画一个按钮
    void drawButton(int x, int y, int w, int h, LPCTSTR text, COLORREF bg_color);
    //内部辅助：判断点击是否在区域内
    bool isClickIn(int mouse_x, int mouse_y, int x, int y, int w, int h);
};