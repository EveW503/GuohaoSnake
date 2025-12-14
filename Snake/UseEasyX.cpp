#include "UseEasyX.h"
#include <string>
#include <vector>
#include <cstdio>  // 用于 swprintf_s
#include <windows.h> // 用于 Sleep, GetAsyncKeyState
#include <stdio.h>
#include <graphics.h>
#include <ctime>
#include <cstdlib>



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

bool UseEasyX::is_graph_initialized = false;

// 辅助函数：获取鼠标位置（用于悬停效果，不消耗点击事件）
POINT getMousePos() {
    POINT p;
    GetCursorPos(&p);
    ScreenToClient(GetHWnd(), &p);
    return p;
}

void UseEasyX::initGraph(int width, int height)
{
    if (!is_graph_initialized)
    {
        initgraph(width, height);
        BeginBatchDraw();
        is_graph_initialized = true;
    }

    // 设置 VA-11 HALL-A 风格背景
    setbkcolor(VA_BG_COLOR);
    cleardevice(); // 立即应用背景色

    setbkmode(TRANSPARENT);
}

void UseEasyX::close()
{
    cleardevice();
    FlushBatchDraw(); // 刷新一下清空的屏幕
}

void UseEasyX::drawMap(GameMap& map)
{
    // 每次清除屏幕使用深色背景
    setbkcolor(VA_BG_COLOR);
    cleardevice();

    // 可选：绘制淡淡的背景网格线，增加赛博感
    setlinecolor(RGB(35, 30, 50));
    for (int i = 0; i < SCREEN_WIDTH; i += BLOCK_SIZE) line(i, 0, i, SCREEN_HEIGHT);
    for (int j = 0; j < SCREEN_HEIGHT; j += BLOCK_SIZE) line(0, j, SCREEN_WIDTH, j);

    for (int x = 0; x < MAP_WIDTH; x++) {
        for (int y = 0; y < MAP_HEIGHT; y++) {
            BlockType type = map.getBlock(x, y);

            int sx = GET_X(x);
            int sy = GET_Y(y);

            switch (type) {
            case BlockType::WALL:
                // 墙壁：空心矩形或带边框的深色块
                setfillcolor(VA_WALL_COLOR);
                setlinecolor(VA_WALL_BORDER);
                fillrectangle(sx, sy, sx + BLOCK_SIZE, sy + BLOCK_SIZE);
                // 画个叉或者内部装饰，看起来像原本的电子元件
                line(sx, sy, sx + BLOCK_SIZE, sy + BLOCK_SIZE);
                break;

            case BlockType::FOOD:
            {
                // --- 赛博朋克风格：全息能量核心 ---

                // 1. 计算呼吸动画参数 (基于系统时间)
                // 周期约 1秒，范围 0.0 ~ 1.0
                long long t = GetTickCount();
                double pulse = (sin(t * 0.005) + 1.0) / 2.0;

                // 核心大小在 40% 到 60% 之间波动
                int core_size = (int)(BLOCK_SIZE * (0.4 + 0.2 * pulse));
                int offset = (BLOCK_SIZE - core_size) / 2;

                // 2. 绘制发光核心 (霓虹粉)
                setfillcolor(VA_NEON_PINK);
                setlinecolor(VA_NEON_PINK);

                // 画两个交错的图形形成复杂的“核心”感
                // 图层A: 实心矩形
                solidrectangle(sx + offset, sy + offset,
                    sx + BLOCK_SIZE - offset, sy + BLOCK_SIZE - offset);

                // 图层B: 旋转 45 度的线框 (菱形效果) - 简单模拟：画一个略大的空心圆或十字
                setlinecolor(WHITE);
                line(sx + BLOCK_SIZE / 2, sy + offset, sx + BLOCK_SIZE / 2, sy + BLOCK_SIZE - offset);
                line(sx + offset, sy + BLOCK_SIZE / 2, sx + BLOCK_SIZE - offset, sy + BLOCK_SIZE / 2);

                // 3. 绘制全息锁定框 (青色) - 像是一个 HUD 界面锁定了这个食物
                COLORREF holo_cyan = RGB(0, 240, 255);
                setlinecolor(holo_cyan);

                int border_len = 8; // 边角线长度
                int margin = 4;     // 距离格子的边距

                // 左上角
                line(sx + margin, sy + margin, sx + margin + border_len, sy + margin);
                line(sx + margin, sy + margin, sx + margin, sy + margin + border_len);

                // 右上角
                line(sx + BLOCK_SIZE - margin, sy + margin, sx + BLOCK_SIZE - margin - border_len, sy + margin);
                line(sx + BLOCK_SIZE - margin, sy + margin, sx + BLOCK_SIZE - margin, sy + margin + border_len);

                // 左下角
                line(sx + margin, sy + BLOCK_SIZE - margin, sx + margin + border_len, sy + BLOCK_SIZE - margin);
                line(sx + margin, sy + BLOCK_SIZE - margin, sx + margin, sy + BLOCK_SIZE - margin - border_len);

                // 右下角
                line(sx + BLOCK_SIZE - margin, sy + BLOCK_SIZE - margin, sx + BLOCK_SIZE - margin - border_len, sy + BLOCK_SIZE - margin);
                line(sx + BLOCK_SIZE - margin, sy + BLOCK_SIZE - margin, sx + BLOCK_SIZE - margin, sy + BLOCK_SIZE - margin - border_len);
            }
            break;

            case BlockType::DATA_FRAG:
            {
                // --- 赛博故障风格设计 ---

                // 1. 定义颜色：高亮金
                COLORREF neon_gold = RGB(255, 215, 0);

                // 2. 绘制 "故障" 外框 (Jitter Effect)
                // 边框会随机向外扩散或收缩 1-2 像素，产生视觉上的震动感
                int jitter = rand() % 3;
                setlinecolor(neon_gold);
                setfillcolor(BS_NULL); // 设为空心，防止覆盖背景网格太多

                // 画主框
                rectangle(sx + 5 - jitter, sy + 5 - jitter,
                    sx + BLOCK_SIZE - 5 + jitter, sy + BLOCK_SIZE - 5 + jitter);

                // 3. 绘制内部 "数据流" (Data Stream)
                // 随机画几条白色的横线，模拟信号干扰
                setlinecolor(WHITE);
                for (int i = 8; i < BLOCK_SIZE - 8; i += 4) {
                    // 50% 概率画线，让内部纹理每帧都在变
                    if (rand() % 2 == 0) {
                        line(sx + 8, sy + i, sx + BLOCK_SIZE - 8, sy + i);
                    }
                }

                // 4. 绘制文字标识
                // 文字偶尔会变成乱码颜色或者消失，增加 Glitch 感
                if (rand() % 10 != 0) { // 90% 时间正常显示
                    settextcolor(neon_gold);
                    settextstyle(14, 0, _T("Consolas")); // 使用极小的终端字体

                    // 居中显示 "0x" 或 "MEM" 暗示内存地址
                    // 稍微错位一点点
                    outtextxy(sx + 8, sy + 12, _T("0xFF"));
                }
            }
            break;

            case BlockType::AIR:
            default:
                break;
            }
        }
    }
}

void UseEasyX::drawSnake(const std::deque<Point>& snake_body, COLORREF body_color, COLORREF head_color)
{
    if (snake_body.empty()) return;

    // 1. 画蛇身
    setfillcolor(body_color);
    setlinecolor(VA_BG_COLOR); // 用背景色做分割线，产生像素块的断裂感
    for (size_t i = 1; i < snake_body.size(); i++) {
        Point p = snake_body[i];
        int sx = GET_X(p.x);
        int sy = GET_Y(p.y);
        // 稍微缩小一点，留出间隙
        fillrectangle(sx + 2, sy + 2, sx + BLOCK_SIZE - 2, sy + BLOCK_SIZE - 2);
    }

    // 2. 画蛇头 (带一点高光)
    setfillcolor(head_color);
    setlinecolor(WHITE); // 头部高亮边框
    Point head = snake_body.front();
    int hx = GET_X(head.x);
    int hy = GET_Y(head.y);
    fillrectangle(hx + 1, hy + 1, hx + BLOCK_SIZE - 1, hy + BLOCK_SIZE - 1);

    // 画眼睛 (像素风)
    setfillcolor(VA_BG_COLOR);
    solidrectangle(hx + 8, hy + 8, hx + 12, hy + 12);
    solidrectangle(hx + 28, hy + 8, hx + 32, hy + 12);
}

void UseEasyX::drawUI(int current_score, int high_score, int snake_len, int hp, int game_time_seconds, bool is_paused) {

    settextcolor(VA_TEXT_COLOR);
    settextstyle(28, 0, _T("Consolas")); // 或者 "Courier New"
    setbkmode(TRANSPARENT);

    // 1. 绘制基本文字信息
    settextcolor(WHITE);
    settextstyle(24, 0, _T("Consolas"));

    TCHAR str_buf[128];
    _stprintf_s(str_buf, _T("[SCORE]: %06d  [HIGH]: %06d"), current_score, high_score); // 补零格式化更有科技感
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
        is_paused ? VA_NEON_PINK : VA_ACCENT_COLOR); // 使用新变量

    drawButton(BTN_RETURN_X, BTN_RETURN_Y, BTN_W, BTN_H, _T("MENU"), VA_WALL_BORDER);

    // 3. 【新增】绘制屏幕中央的暂停提示
    if (is_paused) {
        // 设置大号字体
        settextstyle(80, 0, _T("Impact")); // 更有冲击力的字体
        settextcolor(VA_NEON_PINK);

        LPCTSTR p_text = _T("GAME PAUSED");

        // 计算居中位置
        int text_w = textwidth(p_text);
        int text_h = textheight(p_text);

        outtextxy((SCREEN_WIDTH - text_w) / 2, (SCREEN_HEIGHT - text_h) / 2, p_text);

        setlinecolor(RGB(0, 0, 0));
        for (int i = 0; i < SCREEN_HEIGHT; i += 4) {
            // 这里很难做半透明，可以用深灰色细线代替
            line(0, i, SCREEN_WIDTH, i); 
        }
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

std::string UseEasyX::inputPlayerName()
{
    // 这里的 InputBox 是 EasyX 封装的 Windows API
    // 参数：接收字符串的缓冲区, 缓冲区大小, 提示文本, 标题, 默认值, ...
    TCHAR str_buf[32] = { 0 };
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
        TCHAR name_buf[128];

        // 1. 截断保护：无论名字多长，只取前 20 个字符
        std::string safe_name = rec.user_name;
        if (safe_name.length() > 20) {
            safe_name = safe_name.substr(0, 20) + "..."; // 超长截断并加省略号
        }

        // 2. 使用截断后的 safe_name 进行绘制
        _stprintf_s(name_buf, _T("%S"), safe_name.c_str());
        outtextxy(500, y, name_buf);

        // 分数
        TCHAR score_buf[128];
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


void UseEasyX::drawButton(int x, int y, int w, int h, LPCTSTR text, COLORREF theme_color) {
    // 背景半透明黑 (EasyX不支持直接alpha，这里用实心黑覆盖地图)
    setfillcolor(VA_BG_COLOR);
    solidrectangle(x, y, x + w, y + h);

    // 亮色边框
    setlinecolor(theme_color);
    rectangle(x, y, x + w, y + h);

    // 内部再画一圈细线，增加细节
    rectangle(x + 3, y + 3, x + w - 3, y + h - 3);

    settextcolor(theme_color);
    settextstyle(20, 0, _T("Consolas"));

    // 文字居中
    int tx = x + (w - textwidth(text)) / 2;
    int ty = y + (h - textheight(text)) / 2;
    outtextxy(tx, ty, text);
}

int UseEasyX::drawMenu() {
    // --- 菜单布局参数 ---
    int menu_x = 700;
    int start_y = 150;       // 保持上次调整的高度
    int btn_w = 400;
    int btn_h = 60;
    int gap = 15;

    // --- 动画状态变量 ---
    int timer = 0;           // 帧计数器
    int scan_line_y = 0;     // 扫描线位置
    int grid_offset = 0;     // 网格滚动偏移

    // 菜单项定义
    const int ITEM_COUNT = 6;
    struct MenuItem {
        LPCTSTR text;
        LPCTSTR desc;
        COLORREF color;
    };

    MenuItem items[ITEM_COUNT] = {
        { _T("1. SUGAR RUSH [INTRO]"),    _T("Casual mode. Don't die."),          RGB(0, 255, 255) },
        { _T("2. MARSBLAST [ADVANCED]"),  _T("Walls are deadly. Respawn enabled."), RGB(0, 120, 255) },
        { _T("3. FLAMING MOAI [EXPERT]"), _T("Bodies become food. 5 Lives."),     RGB(255, 42, 109) },
        { _T("4. COBALT VELVET [DUAL]"),  _T("1v1 Local Multiplayer."),           RGB(180, 0, 255) },
        { _T("5. DATABASE [HISTORY]"),    _T("Check past records."),              LIGHTGRAY },
        { _T("6. SHUTDOWN [EXIT]"),       _T("Terminate program."),               DARKGRAY }
    };

    while (true) {
        // --- 1. 更新动画状态 ---
        timer++;

        // 扫描线移动速度 (每帧 8 像素)
        scan_line_y = (scan_line_y + 8) % SCREEN_HEIGHT;

        // 网格缓慢滚动 (每 4 帧动 1 像素)
        if (timer % 4 == 0) grid_offset = (grid_offset + 1) % 40;

        // 随机故障参数生成 (3% 的概率发生故障)
        bool is_glitch = (rand() % 30 == 0);
        int glitch_x = is_glitch ? (rand() % 10 - 5) : 0;
        int glitch_y = is_glitch ? (rand() % 6 - 3) : 0;
        COLORREF title_shadow_color = is_glitch ? WHITE : VA_NEON_PINK; // 故障时阴影变白

        // --- 2. 绘制背景层 ---
        setbkcolor(VA_BG_COLOR);
        cleardevice();

        // 绘制滚动的网格 (增加动感)
        setlinecolor(RGB(35, 30, 50));
        // 竖线 (向右微动)
        for (int i = -40; i < SCREEN_WIDTH; i += 40) {
            line(i + grid_offset, 0, i + grid_offset, SCREEN_HEIGHT);
        }
        // 横线 (向下微动)
        for (int j = -40; j < SCREEN_HEIGHT; j += 40) {
            line(0, j + grid_offset, SCREEN_WIDTH, j + grid_offset);
        }

        // --- 3. 左侧：动态标题区 ---
        settextstyle(100, 0, _T("Impact"));

        // 阴影层 (带故障位移)
        settextcolor(title_shadow_color);
        outtextxy(105 + glitch_x, 155 + glitch_y, _T("SNAKE"));
        outtextxy(105 - glitch_x, 255 - glitch_y, _T("PROTOCOL")); // 反向抖动

        // 主体层 (正常位置)
        // 如果正在故障，主体层偶尔消失一下 (模拟闪烁)
        if (!is_glitch || (rand() % 2 == 0)) {
            settextcolor(VA_ACCENT_COLOR);
            outtextxy(100, 150, _T("SNAKE"));
            outtextxy(100, 250, _T("PROTOCOL"));
        }

        // 装饰性小字
        settextstyle(20, 0, _T("Consolas"));
        settextcolor(WHITE);

        // 动态数据
        TCHAR time_buf[64];
        _stprintf_s(time_buf, _T("SYS_TICK: %08d"), timer); // 显示不断跳动的数字
        outtextxy(100, 380, _T("SYSTEM: ONLINE"));
        outtextxy(100, 400, _T("MEM: 64KB OK"));
        outtextxy(100, 420, time_buf);

        // 分割线
        setlinecolor(VA_ACCENT_COLOR);
        line(600, 100, 600, 620);

        // --- 4. 右侧：交互式菜单 ---
        POINT mouse = getMousePos();
        int hover_index = -1;

        for (int i = 0; i < ITEM_COUNT; i++) {
            int x = menu_x;
            int y = start_y + i * (btn_h + gap);
            bool is_hover = (mouse.x >= x && mouse.x <= x + btn_w && mouse.y >= y && mouse.y <= y + btn_h);

            if (is_hover) hover_index = i;

            if (is_hover) {
                // 悬停：背景填充，文字变黑
                setfillcolor(items[i].color);
                solidrectangle(x, y, x + btn_w, y + btn_h);
                settextcolor(BLACK);
            }
            else {
                // 默认：黑背景，亮边框
                setfillcolor(VA_BG_COLOR);
                solidrectangle(x, y, x + btn_w, y + btn_h);
                setlinecolor(items[i].color);
                rectangle(x, y, x + btn_w, y + btn_h);
                settextcolor(items[i].color);
            }

            settextstyle(30, 0, _T("Consolas"));
            int tx = x + 20;
            int ty = y + (btn_h - 30) / 2;
            outtextxy(tx, ty, items[i].text);

            // 悬停光标闪烁动画
            if (is_hover) {
                if ((timer / 10) % 2 == 0) { // 每10帧闪烁一次
                    outtextxy(x - 30, ty, _T(">"));
                }
            }
        }

        // --- 5. 底部：动态描述栏 ---
        // 绘制底部背景条
        setfillcolor(RGB(20, 20, 20));
        solidrectangle(0, SCREEN_HEIGHT - 60, SCREEN_WIDTH, SCREEN_HEIGHT);
        setlinecolor(VA_ACCENT_COLOR);
        line(0, SCREEN_HEIGHT - 60, SCREEN_WIDTH, SCREEN_HEIGHT - 60);

        settextstyle(24, 0, _T("Consolas"));

        if (hover_index != -1) {
            settextcolor(WHITE);
            outtextxy(20, SCREEN_HEIGHT - 40, items[hover_index].desc);
        }
        else {
            // 待机文字呼吸效果 (根据 timer 计算灰度)
            int brightness = 100 + abs((int)(sin(timer * 0.05) * 150));
            if (brightness > 255) brightness = 255;
            settextcolor(RGB(brightness, brightness, brightness));

            // 每隔一段时间加上光标 "_"
            bool show_cursor = (timer / 30) % 2 == 0;
            if (show_cursor)
                outtextxy(20, SCREEN_HEIGHT - 40, _T("WAITING FOR INPUT..._"));
            else
                outtextxy(20, SCREEN_HEIGHT - 40, _T("WAITING FOR INPUT..."));
        }

        // --- 6. 全局特效：CRT 扫描线 ---
        // 画一条贯穿全屏的半透明亮线 (模拟扫描)
        // EasyX 没有直接 Alpha，我们用“疏密线”模拟或者直接画一条暗青色线
        setlinecolor(RGB(0, 50, 50));
        line(0, scan_line_y, SCREEN_WIDTH, scan_line_y);
        line(0, scan_line_y + 1, SCREEN_WIDTH, scan_line_y + 1); // 加粗一点

        // 静态旧电视扫描纹理 (每隔2行画一条黑线，产生隔行扫描感)
        setlinecolor(0);
        for (int k = 0; k < SCREEN_HEIGHT; k += 4) {
            // 慎用：如果在高分辨率下这可能会让屏幕太暗
            // 这里可以不做处理，或者只在标题区域画
        }

        FlushBatchDraw();

        // --- 7. 输入处理 ---
        if (MouseHit()) {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN) {
                for (int i = 0; i < ITEM_COUNT; i++) {
                    int x = menu_x;
                    int y = start_y + i * (btn_h + gap);
                    if (isClickIn(msg.x, msg.y, x, y, btn_w, btn_h)) {
                        return i + 1;
                    }
                }
            }
        }

        Sleep(16); // 约 60 FPS
    }
}

bool UseEasyX::isClickIn(int mouse_x, int mouse_y, int x, int y, int w, int h) {
    return (mouse_x >= x && mouse_x <= x + w && mouse_y >= y && mouse_y <= y + h);
}

int UseEasyX::checkGameButtons(int mouse_x, int mouse_y, int offset_y) {
    // 将 offset_y 加到 BTN_PAUSE_Y 和 BTN_RETURN_Y
    if (isClickIn(mouse_x, mouse_y, BTN_PAUSE_X, BTN_PAUSE_Y + offset_y, BTN_W, BTN_H)) return 1; // 暂停
    if (isClickIn(mouse_x, mouse_y, BTN_RETURN_X, BTN_RETURN_Y + offset_y, BTN_W, BTN_H)) return 2; // 返回
    return 0;
}

// 绘制历史记录 (带返回按钮)
void UseEasyX::drawHistory(RecordManager& mgr) {
    // 定义按钮参数
    int btn_w = 120;
    int btn_h = 40;
    int start_x = 50;
    int btn_y = SCREEN_HEIGHT - 80;
    int gap = 20;

    // 按钮 X 坐标
    int x_add = start_x;
    int x_del = x_add + btn_w + gap;
    int x_mod = x_del + btn_w + gap;
    int x_search = x_mod + btn_w + gap;
    int x_back = SCREEN_WIDTH - btn_w - 50;

    // 搜索状态
    bool is_searching = false;
    std::vector<Record> search_result;

    while (true) {
        cleardevice();

        // 1. 标题
        settextstyle(40, 0, _T("Arial"));
        settextcolor(YELLOW);
        LPCTSTR title = is_searching ? _T("SEARCH RESULTS") : _T("RECORD MANAGEMENT");
        outtextxy((SCREEN_WIDTH - textwidth(title)) / 2, 30, title);

        // 2. 表头
        settextstyle(24, 0, _T("Consolas"));
        settextcolor(LIGHTCYAN);
        // 显示 INDEX 方便删除
        outtextxy(100, 100, _T("ID"));
        outtextxy(200, 100, _T("VERSION"));
        outtextxy(500, 100, _T("NAME"));
        outtextxy(900, 100, _T("SCORE"));

        // 3. 列表内容
        settextcolor(WHITE);
        int y = 140;

        // 根据状态决定显示全表还是搜索结果
        const std::vector<Record>& list_to_show = is_searching ? search_result : mgr.getAllRecords();

        int count = 0;
        for (size_t i = 0; i < list_to_show.size(); ++i) {
            if (count >= 12) break; // 每页最多显示12条

            const auto& rec = list_to_show[i];

            TCHAR buf[128];

            // 显示 ID (如果是搜索结果，显示它在原列表的索引可能比较复杂，这里简化显示当前视图的序号)
            // 为了删除功能的准确性，建议在非搜索模式下才显示真实 ID
            if (!is_searching) {
                _stprintf_s(buf, _T("%d"), (int)i);
                outtextxy(100, y, buf);
            }
            else {
                outtextxy(100, y, _T("-"));
            }

            // 版本
            _stprintf_s(buf, _T("%-15S"), rec.version.c_str());
            outtextxy(200, y, buf);

            // 名字 (带截断保护)
            std::string safe_name = rec.user_name;
            if (safe_name.length() > 20) safe_name = safe_name.substr(0, 20) + "...";
            _stprintf_s(buf, _T("%-20S"), safe_name.c_str());
            outtextxy(500, y, buf);

            // 分数
            _stprintf_s(buf, _T("%d"), rec.score);
            outtextxy(900, y, buf);

            y += 35;
            count++;
        }

        // 4. 绘制功能按钮
        if (!is_searching) {
            drawButton(x_add, btn_y, btn_w, btn_h, _T("ADD"), GREEN);
            drawButton(x_del, btn_y, btn_w, btn_h, _T("DELETE"), RED);
            drawButton(x_mod, btn_y, btn_w, btn_h, _T("MODIFY"), BLUE);
            drawButton(x_search, btn_y, btn_w, btn_h, _T("SEARCH"), BROWN);
        }

        // 返回按钮
        drawButton(x_back, btn_y, btn_w, btn_h, is_searching ? _T("RESET") : _T("BACK"), DARKGRAY);

        FlushBatchDraw();

        // 5. 交互逻辑
        if (MouseHit()) {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN) {

                // --- 返回 / 重置 ---
                if (isClickIn(msg.x, msg.y, x_back, btn_y, btn_w, btn_h)) {
                    if (is_searching) {
                        is_searching = false; // 退出搜索模式
                    }
                    else {
                        break; // 退出历史界面
                    }
                }

                // (仅在非搜索模式下可用)
                if (!is_searching) {

                    // --- 增 (ADD) ---
                    if (isClickIn(msg.x, msg.y, x_add, btn_y, btn_w, btn_h)) {
                        TCHAR name_buf[32] = { 0 };
                        InputBox(name_buf, 32, _T("Enter Name:"), _T("Add Record"), _T("TestUser"), 0, 0, false);

                        TCHAR score_buf[32] = { 0 };
                        InputBox(score_buf, 32, _T("Enter Score:"), _T("Add Record"), _T("100"), 0, 0, false);

                        // 转换并添加
#ifdef UNICODE
// 简单转换，实际项目建议封装辅助函数
                        int len = WideCharToMultiByte(CP_ACP, 0, name_buf, -1, NULL, 0, NULL, NULL);
                        char* n_ptr = new char[len];
                        WideCharToMultiByte(CP_ACP, 0, name_buf, -1, n_ptr, len, NULL, NULL);
                        std::string name_str(n_ptr);
                        delete[] n_ptr;
#else
                        std::string name_str(name_buf);
#endif

                        int score_val = _ttoi(score_buf);
                        mgr.addRecord("Manual", name_str, score_val);
                    }

                    // --- 删 (DELETE) ---
                    if (isClickIn(msg.x, msg.y, x_del, btn_y, btn_w, btn_h)) {
                        TCHAR idx_buf[32] = { 0 };
                        InputBox(idx_buf, 32, _T("Enter ID (Index) to Delete:"), _T("Delete Record"), _T("0"), 0, 0, false);
                        int idx = _ttoi(idx_buf);
                        mgr.deleteRecord(idx);
                    }

                    // --- 改 (MODIFY) ---
                    if (isClickIn(msg.x, msg.y, x_mod, btn_y, btn_w, btn_h)) {
                        TCHAR old_buf[32] = { 0 };
                        InputBox(old_buf, 32, _T("Enter Exact Old Username:"), _T("Modify Name"), _T(""), 0, 0, false);

                        TCHAR new_buf[32] = { 0 };
                        InputBox(new_buf, 32, _T("Enter New Username:"), _T("Modify Name"), _T(""), 0, 0, false);

                        // 转换逻辑
                        std::string old_s, new_s;
#ifdef UNICODE
                        // 这里省略重复的转换代码，原理同上，将 TCHAR 转为 std::string
                        // 建议: 在 UseEasyX 类里增加一个 private 的 TcharToString 函数
                        {
                            int l = WideCharToMultiByte(CP_ACP, 0, old_buf, -1, NULL, 0, NULL, NULL);
                            char* p = new char[l];
                            WideCharToMultiByte(CP_ACP, 0, old_buf, -1, p, l, NULL, NULL);
                            old_s = p; delete[] p;

                            l = WideCharToMultiByte(CP_ACP, 0, new_buf, -1, NULL, 0, NULL, NULL);
                            p = new char[l];
                            WideCharToMultiByte(CP_ACP, 0, new_buf, -1, p, l, NULL, NULL);
                            new_s = p; delete[] p;
                        }
#else
                        old_s = old_buf; new_s = new_buf;
#endif

                        mgr.modifyUserName(old_s, new_s);
                    }

                    // --- 查 (SEARCH) ---
                    if (isClickIn(msg.x, msg.y, x_search, btn_y, btn_w, btn_h)) {
                        TCHAR search_buf[32] = { 0 };
                        InputBox(search_buf, 32, _T("Enter Username to Search:"), _T("Search"), _T(""), 0, 0, false);

                        std::string target_name;
#ifdef UNICODE
                        int len = WideCharToMultiByte(CP_ACP, 0, search_buf, -1, NULL, 0, NULL, NULL);
                        char* p = new char[len];
                        WideCharToMultiByte(CP_ACP, 0, search_buf, -1, p, len, NULL, NULL);
                        target_name = p; delete[] p;
#else
                        target_name = search_buf;
#endif

                        search_result = mgr.searchRecords(target_name);
                        is_searching = true; // 进入搜索结果显示模式
                    }
                }
            }
        }
        Sleep(10);
    }
}

void UseEasyX::drawDualGameOver(int winner)
{
    FlushBatchDraw();

    int center_x = SCREEN_WIDTH / 2;
    int center_y = SCREEN_HEIGHT / 2;

    // 1. 标题
    settextstyle(60, 0, _T("Arial"));

    LPCTSTR text = _T("GAME OVER");
    if (winner == 1) {
        settextcolor(GREEN);
        text = _T("PLAYER 1 WINS!");
    }
    else if (winner == 2) {
        settextcolor(LIGHTBLUE);
        text = _T("PLAYER 2 WINS!");
    }
    else {
        settextcolor(YELLOW);
        text = _T("DRAW GAME!");
    }

    int w = textwidth(text);
    outtextxy(center_x - w / 2, center_y - 50, text);

    // 2. 退出提示
    LPCTSTR tip = _T("Press SPACE to return to menu...");
    settextstyle(24, 0, _T("Consolas"));
    settextcolor(WHITE);
    w = textwidth(tip);
    outtextxy(center_x - w / 2, center_y + 50, tip);

    FlushBatchDraw();

    // 等待按空格退出
    while (true) {
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) break;
        Sleep(10);
    }
}

void UseEasyX::drawDualUI(int score1, int score2, int game_time_seconds, bool is_paused)
{
    // 1. 绘制基本文字信息
    settextcolor(WHITE);
    settextstyle(24, 0, _T("Consolas"));

    TCHAR str_buf[128];

    // 显示时间
    _stprintf_s(str_buf, _T("Time: %02d:%02d"), game_time_seconds / 60, game_time_seconds % 60);
    outtextxy(SCREEN_WIDTH / 2 - 60, 20, str_buf); // 时间居中

    // 显示 P1 分数 (左上角，绿色)
    settextcolor(GREEN);
    _stprintf_s(str_buf, _T("P1 Score: %d"), score1);
    outtextxy(20, 20, str_buf);

    // 显示 P2 分数 (右上角，蓝色 - 需要根据 SCREEN_WIDTH 倒推位置)
    settextcolor(LIGHTBLUE);
    _stprintf_s(str_buf, _T("P2 Score: %d"), score2);
    int p2_w = textwidth(str_buf);
    outtextxy(SCREEN_WIDTH - p2_w - 20, 20, str_buf); // 靠右显示

    // 2. 绘制功能按钮 (暂停/菜单) - 复用原来的位置
    drawButton(BTN_PAUSE_X, BTN_PAUSE_Y + 40, BTN_W, BTN_H, // 稍微下移一点避开分数
        is_paused ? _T("RESUME") : _T("PAUSE"),
        is_paused ? GREEN : BROWN);

    drawButton(BTN_RETURN_X, BTN_RETURN_Y + 40, BTN_W, BTN_H, _T("MENU"), RED);

    // 3. 绘制屏幕中央的暂停提示
    if (is_paused) {
        settextstyle(80, 0, _T("Arial"));
        settextcolor(YELLOW);
        LPCTSTR p_text = _T("GAME PAUSED");
        int text_w = textwidth(p_text);
        int text_h = textheight(p_text);
        outtextxy((SCREEN_WIDTH - text_w) / 2, (SCREEN_HEIGHT - text_h) / 2, p_text);
    }

    FlushBatchDraw();
};