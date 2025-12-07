#include "UseEasyX.h"
#include <string>
#include <vector>
#include <cstdio>  // 用于 swprintf_s
#include <windows.h> // 用于 Sleep, GetAsyncKeyState

// 辅助宏：将逻辑坐标转为屏幕像素坐标
#define GET_X(x) ((x) * BLOCK_SIZE)
#define GET_Y(y) ((y) * BLOCK_SIZE)

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

void UseEasyX::drawUI(int current_score, int high_score, int snake_len, int hp, int game_time_seconds)
{
    settextcolor(WHITE);
    settextstyle(24, 0, _T("Consolas"));

    TCHAR str_buf[128];

    // 分数
    _stprintf_s(str_buf, _T("Score: %d  High: %d"), current_score, high_score);
    outtextxy(20, 20, str_buf);

    // 长度与时间
    _stprintf_s(str_buf, _T("Length: %d  Time: %02d:%02d"), snake_len, game_time_seconds / 60, game_time_seconds % 60);
    outtextxy(20, 50, str_buf);

    // 生命值
    if (hp > 0) {
        settextcolor(LIGHTRED);
        _stprintf_s(str_buf, _T("HP: %d"), hp);
        outtextxy(20, 80, str_buf);
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

int UseEasyX::drawMenu()
{
    cleardevice();

    // 计算屏幕中心 X
    int center_x = SCREEN_WIDTH / 2;

    // --- 1. 标题 ---
    settextstyle(60, 0, _T("Arial"));
    settextcolor(YELLOW);
    LPCTSTR title = _T("SNAKE GAME C++");
    // 动态居中：(屏幕宽 - 文字宽) / 2
    // Y 轴设为 80 (适配 720P)
    outtextxy(center_x - textwidth(title) / 2, 80, title);

    // --- 2. 选项 ---
    settextstyle(30, 0, _T("Consolas")); // 字体稍微改小
    settextcolor(WHITE);

    // 定义选项内容
    LPCTSTR opt1 = _T("1. Intro Mode (Basic)");
    LPCTSTR opt2 = _T("2. Advanced Mode (Body -> Wall)");
    LPCTSTR opt3 = _T("3. Expert Mode (Body -> Food)");

    // 动态计算每个选项的居中位置，并且 Y 轴更紧凑
    outtextxy(center_x - textwidth(opt1) / 2, 240, opt1);
    outtextxy(center_x - textwidth(opt2) / 2, 300, opt2);
    outtextxy(center_x - textwidth(opt3) / 2, 360, opt3);

    // --- 3. 提示 ---
    settextcolor(LIGHTGRAY);
    LPCTSTR prompt = _T("Please press 1, 2, or 3 to start...");
    outtextxy(center_x - textwidth(prompt) / 2, 500, prompt);

    FlushBatchDraw();

    // 输入循环 (使用 GetAsyncKeyState 避免焦点问题)
    while (true) {
        if (GetAsyncKeyState('1') & 0x8000) return 1;
        if (GetAsyncKeyState('2') & 0x8000) return 2;
        if (GetAsyncKeyState('3') & 0x8000) return 3;
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