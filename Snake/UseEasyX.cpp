#include "UseEasyX.h"
#include <string>
#include <vector>
#include <cstdio>  // 用于 swprintf_s
#include <windows.h> // 用于 Sleep, GetAsyncKeyState
#include <stdio.h>
#include <graphics.h>



// 辅助宏：将逻辑坐标转为屏幕像素坐标
#define GET_X(x) ((x) * BLOCK_SIZE)
#define GET_Y(y) ((y) * BLOCK_SIZE)

// 定义游戏内按钮的位置 (右上角)
const int BTN_W = 100;
const int BTN_H = 30;
const int BTN_RETURN_X = SCREEN_WIDTH - 120; // 返回按钮 X
const int BTN_RETURN_Y = 10;
const int BTN_PAUSE_X = SCREEN_WIDTH - 240;  // 暂停按钮 X
const int BTN_PAUSE_Y = 10;

void UseEasyX::initGraph(int width, int height)
{
    // 1. 初始化绘图窗口
    initgraph(width, height);

    // 2. 设置背景模式为透明 (让文字背景不遮挡)
    setbkmode(TRANSPARENT);

    // 3. 开启批量绘图模式 (双缓冲)，防止闪烁
    BeginBatchDraw();
}

void UseEasyX::close()
{
    EndBatchDraw();
    closegraph();
}

void UseEasyX::drawMap(GameMap& map)
{
    cleardevice();

    for (int x = 0; x < MAP_WIDTH; x++) {
        for (int y = 0; y < MAP_HEIGHT; y++) {
            BlockType type = map.getBlock(x, y);

            int screen_x = GET_X(x);
            int screen_y = GET_Y(y);

            switch (type) {
            case BlockType::WALL:
                setfillcolor(LIGHTGRAY);
                solidrectangle(screen_x, screen_y, screen_x + BLOCK_SIZE, screen_y + BLOCK_SIZE);
                break;

            case BlockType::FOOD:
                setfillcolor(RED);
                solidcircle(screen_x + BLOCK_SIZE / 2, screen_y + BLOCK_SIZE / 2, BLOCK_SIZE / 2 - 2);
                break;

            case BlockType::SNAKE_HEAD: // 确保 GameMap 有返回 HEAD 类型，或者在这里根据逻辑绘图
                // 如果 GameMap 只存了 BODY，这里可能不会触发，通常在 drawSnake 处理
                // 但如果你的 map setBlock 设置了 HEAD，这里也可以画
                break;

            case BlockType::AIR:
            default:
                break;
            }
        }
    }
}

void UseEasyX::drawSnake(const std::deque<Point>& snake_body)
{
    if (snake_body.empty()) return;

    // 1. 画蛇身
    setfillcolor(GREEN);
    for (size_t i = 1; i < snake_body.size(); i++) {
        Point p = snake_body[i];
        int sx = GET_X(p.x);
        int sy = GET_Y(p.y);
        solidrectangle(sx + 1, sy + 1, sx + BLOCK_SIZE - 1, sy + BLOCK_SIZE - 1);
    }

    // 2. 画蛇头
    setfillcolor(LIGHTGREEN);
    Point head = snake_body.front();
    int hx = GET_X(head.x);
    int hy = GET_Y(head.y);
    solidrectangle(hx, hy, hx + BLOCK_SIZE, hy + BLOCK_SIZE);

    // 画眼睛
    setfillcolor(BLACK);
    solidcircle(hx + BLOCK_SIZE / 4, hy + BLOCK_SIZE / 4, 3);
    solidcircle(hx + BLOCK_SIZE * 3 / 4, hy + BLOCK_SIZE / 4, 3);
}

void UseEasyX::drawUI(int current_score, int high_score, int snake_len, int hp, int game_time_seconds, bool is_paused) {
    // 1. 绘制基本文字信息
    settextcolor(WHITE);
    settextstyle(24, 0, _T("Consolas"));

    TCHAR str_buf[128];
    _stprintf_s(str_buf, _T("Score: %d  High: %d"), current_score, high_score);
    outtextxy(20, 20, str_buf);

    _stprintf_s(str_buf, _T("Length: %d  Time: %02d:%02d"), snake_len, game_time_seconds / 60, game_time_seconds % 60);
    outtextxy(20, 50, str_buf);

    if (hp > 0) {
        settextcolor(LIGHTRED);
        _stprintf_s(str_buf, _T("HP: %d"), hp);
        outtextxy(20, 80, str_buf);
    }

    // 2. 【修改】绘制功能按钮 (根据暂停状态改变文字和颜色)
    // 如果暂停：显示 "RESUME" (继续)，背景绿色
    // 如果运行：显示 "PAUSE" (暂停)，背景棕色
    drawButton(BTN_PAUSE_X, BTN_PAUSE_Y, BTN_W, BTN_H,
        is_paused ? _T("RESUME") : _T("PAUSE"),
        is_paused ? GREEN : BROWN);

    drawButton(BTN_RETURN_X, BTN_RETURN_Y, BTN_W, BTN_H, _T("MENU"), RED);

    // 3. 【新增】绘制屏幕中央的暂停提示
    if (is_paused) {
        // 设置大号字体
        settextstyle(80, 0, _T("Arial"));
        settextcolor(YELLOW);

        LPCTSTR p_text = _T("GAME PAUSED");

        // 计算居中位置
        int text_w = textwidth(p_text);
        int text_h = textheight(p_text);

        outtextxy((SCREEN_WIDTH - text_w) / 2, (SCREEN_HEIGHT - text_h) / 2, p_text);
    }

    FlushBatchDraw();
}

void UseEasyX::drawGameOver(int final_score)
{
    FlushBatchDraw();

    // 居中计算
    int center_x = SCREEN_WIDTH / 2;
    int center_y = SCREEN_HEIGHT / 2;

    // 1. Game Over 标题
    settextcolor(RED);
    settextstyle(60, 0, _T("Arial")); // 字体稍微改小一点适配 720P
    LPCTSTR text = _T("GAME OVER");
    int w = textwidth(text);
    // Y 轴位置上移，防止挤在一起
    outtextxy(center_x - w / 2, center_y - 80, text);

    // 2. 最终得分
    TCHAR score_str[64];
    _stprintf_s(score_str, _T("Final Score: %d"), final_score);
    settextstyle(30, 0, _T("Arial"));
    w = textwidth(score_str);
    settextcolor(WHITE);
    outtextxy(center_x - w / 2, center_y, score_str);

    // 3. 退出提示
    LPCTSTR tip = _T("Press ANY key to exit...");
    settextstyle(20, 0, _T("Consolas"));
    w = textwidth(tip);
    outtextxy(center_x - w / 2, center_y + 60, tip);

    FlushBatchDraw();
}

int UseEasyX::drawMenu() {
    int btn_w = 300;
    int btn_h = 50;
    int start_y = 200; // 按钮起始高度
    int gap = 70;      // 按钮间距
    int center_x = (SCREEN_WIDTH - btn_w) / 2;

    while (true) {
        cleardevice();

        // 1. 标题
        settextstyle(60, 0, _T("Arial"));
        settextcolor(YELLOW);
        LPCTSTR title = _T("SNAKE GAME C++");
        outtextxy((SCREEN_WIDTH - textwidth(title)) / 2, 80, title);

        // 2. 绘制 5 个按钮
        drawButton(center_x, start_y, btn_w, btn_h, _T("1. Intro Mode"), GREEN);
        drawButton(center_x, start_y + gap, btn_w, btn_h, _T("2. Advanced Mode"), BLUE);
        drawButton(center_x, start_y + gap * 2, btn_w, btn_h, _T("3. Expert Mode"), RED);
        drawButton(center_x, start_y + gap * 3, btn_w, btn_h, _T("History Records"), LIGHTGRAY);
        drawButton(center_x, start_y + gap * 4, btn_w, btn_h, _T("Exit Game"), DARKGRAY);

        FlushBatchDraw();

        // 3. 鼠标交互
        if (MouseHit()) {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN) {
                // 判断点击了哪个按钮
                if (isClickIn(msg.x, msg.y, center_x, start_y, btn_w, btn_h)) return 1;
                if (isClickIn(msg.x, msg.y, center_x, start_y + gap, btn_w, btn_h)) return 2;
                if (isClickIn(msg.x, msg.y, center_x, start_y + gap * 2, btn_w, btn_h)) return 3;
                if (isClickIn(msg.x, msg.y, center_x, start_y + gap * 3, btn_w, btn_h)) return 4; // 历史
                if (isClickIn(msg.x, msg.y, center_x, start_y + gap * 4, btn_w, btn_h)) return 5; // 退出
            }
        }
        Sleep(10);
    }
}

std::string UseEasyX::inputPlayerName()
{
    // 这里的 InputBox 是 EasyX 封装的 Windows API
    // 参数：接收字符串的缓冲区, 缓冲区大小, 提示文本, 标题, 默认值, ...
    TCHAR str_buf[32];
    InputBox(str_buf, 32, _T("Please enter your name:"), _T("New Record"), _T("Player"), 0, 0, false);

    // TCHAR 转 std::string (处理 Unicode/多字节问题)
#ifdef UNICODE
    int len = WideCharToMultiByte(CP_ACP, 0, str_buf, -1, NULL, 0, NULL, NULL);
    char* char_buf = new char[len];
    WideCharToMultiByte(CP_ACP, 0, str_buf, -1, char_buf, len, NULL, NULL);
    std::string name(char_buf);
    delete[] char_buf;
    return name;
#else
    return std::string(str_buf);
#endif
}

void UseEasyX::drawRankings(const std::vector<Record>& records)
{
    cleardevice();

    // 标题
    settextstyle(50, 0, _T("Arial"));
    settextcolor(YELLOW);
    LPCTSTR title = _T("HALL OF FAME");
    outtextxy((SCREEN_WIDTH - textwidth(title)) / 2, 50, title);

    // 表头
    settextstyle(30, 0, _T("Consolas"));
    settextcolor(LIGHTCYAN);
    outtextxy(200, 150, _T("VERSION"));
    outtextxy(500, 150, _T("NAME"));
    outtextxy(900, 150, _T("SCORE"));

    // 列表内容
    settextcolor(WHITE);
    int y = 200;

    // 只显示前 10 名
    int count = 0;
    for (const auto& rec : records) {
        if (count >= 10) break;

        TCHAR buf[128];

        // 转换 string 到 TCHAR 用于显示
        // 注意：如果你项目是 Unicode，需要转换 rec.version 和 rec.user_name
        // 这里假设简单处理，或者直接使用多字节字符集

        // 为了代码通用性，这里用一种简单的格式化方法 (假设项目是多字节字符集)
        // 如果是 Unicode，请参考上面的 inputPlayerName 里的转换逻辑
        _stprintf_s(buf, _T("%-15S %-15S %d"), rec.version.c_str(), rec.user_name.c_str(), rec.score);

        // 手动对齐显示 (简单版)
        // 版本
        TCHAR ver_buf[32];
        _stprintf_s(ver_buf, _T("%S"), rec.version.c_str());
        outtextxy(200, y, ver_buf);

        // 名字
        TCHAR name_buf[32];
        _stprintf_s(name_buf, _T("%S"), rec.user_name.c_str());
        outtextxy(500, y, name_buf);

        // 分数
        TCHAR score_buf[32];
        _stprintf_s(score_buf, _T("%d"), rec.score);
        outtextxy(900, y, score_buf);

        y += 40;
        count++;
    }

    // 底部提示
    settextcolor(LIGHTGRAY);
    LPCTSTR tip = _T("Press SPACE to exit...");
    outtextxy((SCREEN_WIDTH - textwidth(tip)) / 2, 650, tip);

    FlushBatchDraw();

    // 等待按键
    while (true) {
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) break;
        Sleep(10);
    }
}

void UseEasyX::drawButton(int x, int y, int w, int h, LPCTSTR text, COLORREF bg_color) {
    setfillcolor(bg_color);
    setlinecolor(WHITE);
    fillrectangle(x, y, x + w, y + h); // 画带边框的矩形

    setbkmode(TRANSPARENT);
    settextcolor(WHITE);
    settextstyle(20, 0, _T("Consolas"));

    // 文字居中计算
    int tx = x + (w - textwidth(text)) / 2;
    int ty = y + (h - textheight(text)) / 2;
    outtextxy(tx, ty, text);
}

bool UseEasyX::isClickIn(int mouse_x, int mouse_y, int x, int y, int w, int h) {
    return (mouse_x >= x && mouse_x <= x + w && mouse_y >= y && mouse_y <= y + h);
}

int UseEasyX::checkGameButtons(int mouse_x, int mouse_y) {
    if (isClickIn(mouse_x, mouse_y, BTN_PAUSE_X, BTN_PAUSE_Y, BTN_W, BTN_H)) return 1; // 暂停
    if (isClickIn(mouse_x, mouse_y, BTN_RETURN_X, BTN_RETURN_Y, BTN_W, BTN_H)) return 2; // 返回
    return 0;
}

// 绘制历史记录 (带返回按钮)
void UseEasyX::drawHistory(const std::vector<Record>& records) {
    int btn_w = 150;
    int btn_x = (SCREEN_WIDTH - btn_w) / 2;
    int btn_y = SCREEN_HEIGHT - 80;

    while (true) {
        cleardevice();

        // 标题
        settextstyle(50, 0, _T("Arial"));
        settextcolor(YELLOW);
        LPCTSTR title = _T("HALL OF FAME");
        outtextxy((SCREEN_WIDTH - textwidth(title)) / 2, 50, title);

        // 表头
        settextstyle(30, 0, _T("Consolas"));
        settextcolor(LIGHTCYAN);
        outtextxy(200, 150, _T("VERSION"));
        outtextxy(500, 150, _T("NAME"));
        outtextxy(900, 150, _T("SCORE"));

        // 列表
        settextcolor(WHITE);
        int y = 200;
        int count = 0;
        for (const auto& rec : records) {
            if (count >= 10) break;

            TCHAR buf[128];
            _stprintf_s(buf, _T("%-15S"), rec.version.c_str());
            outtextxy(200, y, buf);
            _stprintf_s(buf, _T("%-15S"), rec.user_name.c_str());
            outtextxy(500, y, buf);
            _stprintf_s(buf, _T("%d"), rec.score);
            outtextxy(900, y, buf);

            y += 40;
            count++;
        }

        // 绘制返回按钮
        drawButton(btn_x, btn_y, btn_w, 50, _T("BACK"), DARKGRAY);

        FlushBatchDraw();

        // 鼠标检测
        if (MouseHit()) {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN) {
                // 点击返回按钮退出
                if (isClickIn(msg.x, msg.y, btn_x, btn_y, btn_w, 50)) break;
            }
        }
        Sleep(10);
    }
}