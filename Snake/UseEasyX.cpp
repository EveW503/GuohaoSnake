#include "UseEasyX.h"
#include <string>
#include <vector>
#include <cstdio>  
#include <windows.h> 
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

// 辅助函数：获取鼠标位置（用于悬停效果）
POINT getMousePos()
{
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
    setbkcolor(VA_BG_COLOR);
    cleardevice();

    // 绘制背景网格线
    setlinecolor(RGB(35, 30, 50));
    for (int i = 0; i < SCREEN_WIDTH; i += BLOCK_SIZE) line(i, 0, i, SCREEN_HEIGHT);
    for (int j = 0; j < SCREEN_HEIGHT; j += BLOCK_SIZE) line(0, j, SCREEN_WIDTH, j);

    for (int x = 0; x < MAP_WIDTH; x++) 
    {
        for (int y = 0; y < MAP_HEIGHT; y++) 
        {
            BlockType type = map.getBlock(x, y);

            int sx = GET_X(x);
            int sy = GET_Y(y);

            switch (type) 
            {
            case BlockType::WALL:
                // 墙壁：空心矩形或带边框的深色块
                setfillcolor(VA_WALL_COLOR);
                setlinecolor(VA_WALL_BORDER);
                fillrectangle(sx, sy, sx + BLOCK_SIZE, sy + BLOCK_SIZE);
                line(sx, sy, sx + BLOCK_SIZE, sy + BLOCK_SIZE);
                break;

            case BlockType::FOOD:
            {
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
                // 1. 定义颜色：高亮金
                COLORREF neon_gold = RGB(255, 215, 0);

                // 2. 绘制 "故障" 外框 (Jitter Effect)
                // 边框会随机向外扩散或收缩 1-2 像素，产生视觉上的震动感
                int jitter = rand() % 3;
                setlinecolor(neon_gold);
                setfillcolor(BS_NULL); 

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
                // 文字偶尔会变成乱码颜色或者消失
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

    //画蛇身
    setfillcolor(body_color);
    setlinecolor(VA_BG_COLOR); 
    for (size_t i = 1; i < snake_body.size(); i++) 
    {
        Point p = snake_body[i];
        int sx = GET_X(p.x);
        int sy = GET_Y(p.y);

        fillrectangle(sx + 2, sy + 2, sx + BLOCK_SIZE - 2, sy + BLOCK_SIZE - 2);
    }

    // 画蛇头
    setfillcolor(head_color);
    setlinecolor(WHITE); // 头部高亮边框
    Point head = snake_body.front();
    int hx = GET_X(head.x);
    int hy = GET_Y(head.y);
    fillrectangle(hx + 1, hy + 1, hx + BLOCK_SIZE - 1, hy + BLOCK_SIZE - 1);

    // 画眼睛 
    setfillcolor(VA_BG_COLOR);
    solidrectangle(hx + 8, hy + 8, hx + 12, hy + 12);
    solidrectangle(hx + 28, hy + 8, hx + 32, hy + 12);
}

void UseEasyX::drawUI(int current_score, int high_score, int snake_len, int hp, int game_time_seconds, bool is_paused)
{
    // --- 风格配色 ---
    const COLORREF VA_CYAN = RGB(0, 255, 255);    // 霓虹青 (数据流)
    const COLORREF VA_PINK = RGB(255, 42, 109);   // 故障粉 (警告/高亮)
    const COLORREF VA_WHITE = RGB(220, 220, 220);  // 灰白 (稳定文本)
    const COLORREF VA_GRAY = RGB(100, 100, 110);  // 暗灰 (元数据标签)

    setbkmode(TRANSPARENT);

    // --- 1. 顶部监视器 (HUD) ---
    settextstyle(20, 0, _T("微软雅黑"));

    // 布局坐标
    int y_row1 = 20;
    int y_row2 = 50;
    int y_row3 = 80;

    // 辅助 lambda：绘制形如 "[ 标签 ] 数值" 的监控风格
    auto drawMonitorItem = [&](int x, int y, LPCTSTR label, LPCTSTR value, COLORREF val_color) {
        // 绘制暗色标签 [ LABEL ]
        settextcolor(VA_GRAY);
        outtextxy(x, y, _T("[ "));
        outtextxy(x + textwidth(_T("[ ")) + textwidth(label), y, _T(" ]"));

        // 绘制亮色标签名
        settextcolor(VA_WHITE);
        outtextxy(x + textwidth(_T("[ ")), y, label);

        // 绘制数值 (右侧对齐)
        settextcolor(val_color);
        outtextxy(x + 180, y, value);
        };

    TCHAR buf[128];

    // -> [ 记忆同步值 ] (Score) - 对应“残留记忆”
    _stprintf_s(buf, _T("%06d"), current_score);
    drawMonitorItem(20, y_row1, _T("记忆同步值"), buf, VA_CYAN);

    // -> [ 历史最高同步 ] (High) - 对应“过往记忆数据”
    _stprintf_s(buf, _T("%06d"), high_score);
    // 如果打破纪录，数值变粉色高亮
    drawMonitorItem(320, y_row1, _T("历史最高同步"), buf, (current_score > high_score) ? VA_PINK : VA_WHITE);

    // -> [ 潜入深度 ] (Length) - 对应梦境的深浅，单位用 LAYER (层)
    _stprintf_s(buf, _T("%d LAYER"), snake_len);
    drawMonitorItem(20, y_row2, _T("潜入深度"), buf, VA_WHITE);

    // -> [ 链路运行时长 ] (Time) - 对应“信号传输”
    _stprintf_s(buf, _T("%02d:%02d"), game_time_seconds / 60, game_time_seconds % 60);
    drawMonitorItem(320, y_row2, _T("链路运行时长"), buf, VA_WHITE);

    // -> [ 信号完整度 ] (HP) - 对应“切断信号”
    if (hp > 0)
    {
        // 用信号格 "|||" 表示生命值，模拟无线信号
        TCHAR hp_bar[32] = { 0 };
        for (int i = 0; i < hp; ++i) _tcscat_s(hp_bar, _T("||"));

        drawMonitorItem(20, y_row3, _T("信号完整度"), hp_bar, VA_PINK);
    }

    // --- 2. 功能按钮 ---
    // 按钮文字改为更有操作系统的感觉
    // 暂停 -> 挂起进程 / 恢复
    drawButton(BTN_PAUSE_X, BTN_PAUSE_Y, BTN_W, BTN_H,
        is_paused ? _T("恢复链路") : _T("挂起进程"),
        is_paused ? VA_PINK : VA_CYAN);

    // 菜单 -> 终止连接 (Abort)
    drawButton(BTN_RETURN_X, BTN_RETURN_Y, BTN_W, BTN_H, _T("终止连接"), VA_GRAY);


    // --- 3. 暂停弹窗 (梦境冻结) ---
    if (is_paused)
    {
        // 背景压暗 (隔行扫描线遮罩)
        setlinecolor(RGB(0, 0, 0));
        for (int i = 0; i < SCREEN_HEIGHT; i += 2) {
            line(0, i, SCREEN_WIDTH, i);
        }

        int cx = SCREEN_WIDTH / 2;
        int cy = SCREEN_HEIGHT / 2;
        int box_w = 520;
        int box_h = 220;

        // 弹窗背景
        setfillcolor(RGB(20, 20, 30));
        fillrectangle(cx - box_w / 2, cy - box_h / 2, cx + box_w / 2, cy + box_h / 2);

        // 粉色警告框
        setlinecolor(VA_PINK);
        setlinestyle(PS_SOLID, 3);
        rectangle(cx - box_w / 2, cy - box_h / 2, cx + box_w / 2, cy + box_h / 2);
        setlinestyle(PS_SOLID, 1); // 还原

        // 标题：[ 梦境同步挂起 ]
        settextstyle(40, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
        settextcolor(VA_PINK);
        LPCTSTR p_text = _T("[ 梦境同步挂起 ]");
        outtextxy(cx - textwidth(p_text) / 2, cy - 60, p_text);

        // 副标题：等待交互指令...
        settextstyle(22, 0, _T("微软雅黑"));
        settextcolor(VA_WHITE);
        LPCTSTR sub_text = _T(">> 等待交互指令 (WAITING FOR INPUT)...");
        outtextxy(cx - textwidth(sub_text) / 2, cy + 30, sub_text);
    }

    FlushBatchDraw();
}

void UseEasyX::drawGameOver(int final_score)
{
    // VA-11 风格配色
    const COLORREF VA_BG = RGB(20, 20, 30);     // 深空底色
    const COLORREF VA_RED = RGB(255, 42, 109);   // 故障红
    const COLORREF VA_CYAN = RGB(0, 255, 255);    // 霓虹青
    const COLORREF VA_WHITE = RGB(220, 220, 220);  // 灰白文本

    // 1. 清除背景
    setbkcolor(VA_BG);
    cleardevice();

    // 居中计算
    int center_x = SCREEN_WIDTH / 2;
    int center_y = SCREEN_HEIGHT / 2;
    int box_w = 700; // 稍微再加宽一点，防止大字体换行
    int box_h = 360; // 高度也稍微增加
    int left = center_x - box_w / 2;
    int top = center_y - box_h / 2;
    int right = center_x + box_w / 2;
    int bottom = center_y + box_h / 2;

    // 2. 绘制终端警告框
    setfillcolor(RGB(30, 32, 45));
    fillrectangle(left, top, right, bottom);

    setlinecolor(VA_RED);
    setlinestyle(PS_SOLID, 4); // 边框加粗到 4
    rectangle(left, top, right, bottom);

    // 装饰性扫描线 (间距调大一点，显得疏朗)
    setlinecolor(RGB(45, 40, 55));
    setlinestyle(PS_SOLID, 1);
    for (int y = top + 15; y < bottom; y += 8) {
        line(left + 6, y, right - 6, y);
    }

    // --- 3. 标题 (大幅加大) ---
    // [ 信号传输意外终止 ]
    setbkmode(TRANSPARENT);
    // 字号从 40 改为 60，且使用 FW_BOLD 加粗 (如果编译器不支持加粗参数，只改 60 也可以)
    // 这里的参数依次是：高度, 宽度, 字体, 角度, 角度, 粗细(FW_BOLD=700), 斜体, 下划线, 删除线
    settextstyle(55, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);

    LPCTSTR title = _T("[ 信号传输意外终止 ]");

    // 绘制青色故障阴影 (偏移量也随字号变大调整为 5)
    settextcolor(RGB(0, 100, 100));
    int title_w = textwidth(title);
    outtextxy(center_x - title_w / 2 + 5, center_y - 110 + 5, title); // Y轴位置稍微上提

    // 绘制红色主标题
    settextcolor(VA_RED);
    outtextxy(center_x - title_w / 2, center_y - 110, title);

    // --- 4. 数据 (加大) ---
    // 本次残留记忆...
    TCHAR score_str[128];
    _stprintf_s(score_str, _T("本次残留记忆(DATA): %06d"), final_score);

    // 字号从 28 改为 35，依然用微软雅黑保持风格统一
    settextstyle(35, 0, _T("微软雅黑"), 0, 0, FW_NORMAL, false, false, false);
    settextcolor(VA_CYAN);

    int w = textwidth(score_str);
    outtextxy(center_x - w / 2, center_y - 10, score_str); // 放在正中心附近

    // 分割线 (加长加粗)
    setlinecolor(VA_WHITE);
    setlinestyle(PS_SOLID, 2);
    line(center_x - 180, center_y + 35, center_x + 180, center_y + 35);

    // --- 5. 交互提示 (加大) ---
    // 按任意键...
    LPCTSTR tip = _T(">> 按任意键脱离当前梦境...");

    // 字号从 20 改为 24
    settextstyle(24, 0, _T("微软雅黑"));
    settextcolor(VA_WHITE);
    w = textwidth(tip);
    outtextxy(center_x - w / 2, center_y + 80, tip); // 下移一点避免拥挤

    FlushBatchDraw();
}

std::string UseEasyX::inputPlayerName()
{
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
    // --- 定义配色 ---
    const COLORREF VA_BG = RGB(20, 20, 30);     // 深空底色
    const COLORREF VA_ROW_ALT = RGB(30, 32, 45);     // 偶数行深色背景
    const COLORREF VA_CYAN = RGB(0, 255, 255);    // 霓虹青
    const COLORREF VA_PINK = RGB(255, 42, 109);   // 故障粉
    const COLORREF VA_WHITE = RGB(220, 220, 220);  // 灰白
    const COLORREF VA_GRAY = RGB(100, 100, 110);  // 暗灰

    // 1. 初始化背景
    setbkcolor(VA_BG);
    cleardevice();

    // 绘制背景装饰线 (隔行扫描)
    setlinecolor(RGB(25, 25, 35));
    for (int i = 0; i < SCREEN_HEIGHT; i += 4) {
        line(0, i, SCREEN_WIDTH, i);
    }

    // 2. 标题：[ 记忆归档中心 ]
    setbkmode(TRANSPARENT);
    settextstyle(50, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);

    LPCTSTR title = _T("[ 记忆归档中心 ]");
    int title_w = textwidth(title);
    int cx = SCREEN_WIDTH / 2;

    // 标题阴影 (青色错位)
    settextcolor(RGB(0, 100, 100));
    outtextxy(cx - title_w / 2 + 4, 40 + 4, title);
    // 标题本体 (粉色)
    settextcolor(VA_PINK);
    outtextxy(cx - title_w / 2, 40, title);

    // 副标题 (英文)
    settextstyle(20, 0, _T("Consolas"));
    settextcolor(VA_CYAN);
    LPCTSTR sub_title = _T(">> ACCESSING ARCHIVED MEMORY DATA...");
    outtextxy(cx - textwidth(sub_title) / 2, 100, sub_title);

    // 3. 表头布局
    int table_y = 150;
    int col_ver = 100; // 第一列 x
    int col_nam = 500; // 第二列 x
    int col_scr = 900; // 第三列 x

    // 绘制表头分割线
    setlinecolor(VA_CYAN);
    setlinestyle(PS_SOLID, 2);
    line(50, table_y + 35, SCREEN_WIDTH - 50, table_y + 35);

    settextstyle(24, 0, _T("微软雅黑"));
    settextcolor(VA_GRAY);

    // 辅助 lambda：绘制带方括号的表头
    auto drawHeader = [&](int x, LPCTSTR text) {
        settextcolor(VA_GRAY);
        outtextxy(x, table_y, _T("[ "));
        int w1 = textwidth(_T("[ "));
        settextcolor(VA_WHITE);
        outtextxy(x + w1, table_y, text);
        int w2 = textwidth(text);
        settextcolor(VA_GRAY);
        outtextxy(x + w1 + w2, table_y, _T(" ]"));
        };

    drawHeader(col_ver, _T("梦境类型"));
    drawHeader(col_nam, _T("潜入者代号"));
    drawHeader(col_scr, _T("记忆同步率"));

    // 4. 列表内容
    settextstyle(22, 0, _T("微软雅黑")); // 中文模式名用微软雅黑
    int y = 200;
    int row_h = 40;

    int count = 0;
    for (const auto& rec : records)
    {
        if (count >= 10) break;

        // 绘制斑马纹背景 (偶数行画深色条)
        if (count % 2 == 0) {
            setfillcolor(VA_ROW_ALT);
            solidrectangle(50, y - 5, SCREEN_WIDTH - 50, y + row_h - 5);
        }

        // --- 列 1: 梦境类型 (映射为中文酒名) ---
        // 原始记录可能是 "Intro", "Advanced" 等，这里做个转换显示
        std::string mode_display = rec.version;
        COLORREF mode_color = VA_WHITE;

        if (rec.version == "Intro") { mode_display = "Intro"; mode_color = RGB(0, 255, 255); }
        else if (rec.version == "Advanced") { mode_display = "Advanced"; mode_color = RGB(0, 120, 255); }
        else if (rec.version == "Expert") { mode_display = "Expert"; mode_color = RGB(255, 42, 109); }
        else if (rec.version == "Manual") { mode_display = "Manual"; mode_color = VA_GRAY; }

        TCHAR ver_buf[64];
#ifdef UNICODE
    // 简单转码示例 (如果你项目是 Unicode)
        wchar_t w_mode[64];
        MultiByteToWideChar(CP_ACP, 0, mode_display.c_str(), -1, w_mode, 64);
        _tcscpy_s(ver_buf, w_mode);
#else
        strcpy_s(ver_buf, mode_display.c_str());
#endif

        settextcolor(mode_color);
        settextstyle(22, 0, _T("微软雅黑"));
        outtextxy(col_ver + 20, y, ver_buf);


        // --- 列 2: 名字 (Consolas 显示英文 ID 更有感觉) ---
        settextcolor(VA_WHITE);
        settextstyle(22, 0, _T("Consolas"));

        std::string safe_name = rec.user_name;
        if (safe_name.length() > 15) safe_name = safe_name.substr(0, 15) + "...";

        TCHAR name_buf[64];
#ifdef UNICODE
        wchar_t w_name[64];
        MultiByteToWideChar(CP_ACP, 0, safe_name.c_str(), -1, w_name, 64);
        _tcscpy_s(name_buf, w_name);
#else
        strcpy_s(name_buf, safe_name.c_str());
#endif
        outtextxy(col_nam + 20, y, name_buf);


        // --- 列 3: 分数 (高亮显示) ---
        settextcolor(VA_CYAN);
        TCHAR score_buf[32];
        _stprintf_s(score_buf, _T("%06d"), rec.score); // 补零格式
        outtextxy(col_scr + 20, y, score_buf);

        y += row_h;
        count++;
    }

    // 5. 底部提示
    settextcolor(VA_WHITE);
    settextstyle(20, 0, _T("微软雅黑"));
    LPCTSTR tip = _T(">> 按空格键中断连接 (DISCONNECT)...");
    outtextxy((SCREEN_WIDTH - textwidth(tip)) / 2, SCREEN_HEIGHT - 50, tip);

    FlushBatchDraw();

    // 等待按键
    while (true)
    {
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) break;
        Sleep(10);
    }
}

void UseEasyX::drawButton(int x, int y, int w, int h, LPCTSTR text, COLORREF theme_color)
{
    // 背景半透明黑
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

int UseEasyX::drawMenu()
{
    // --- 菜单布局参数 ---
    int menu_x = 700;
    int start_y = 150;    
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
        { _T("1. SUGAR RUSH [INTRO]"),    _T("“甘甜，清淡，水果风味，不能更加女性化的饮品。”"),          RGB(0, 255, 255) },
        { _T("2. FRINGE WEAVER [ADVANCED]"),  _T("“如同就着一勺糖喝下无水酒精。”"), RGB(0, 120, 255) },
        { _T("3. GUT PUNCH [EXPERT]"), _T("“这个名字的意思是‘由内脏（Gut）制成的混合饮料（Punch）’，但是同时也描述了饮用时的感受（重击腹部）。”"),     RGB(255, 42, 109) },
        { _T("4. BAD TOUCH [DUAL]"),  _T("“我们毕竟不过是群哺乳动物。”"),           RGB(180, 0, 255) },
        { _T("5. DATABASE [HISTORY]"),    _T("检索过往的记忆数据。或许能发现些什么。"),              LIGHTGRAY },
        { _T("6. SHUTDOWN [EXIT]"),       _T("切断信号。本次服务到此结束，祝好梦。"),               DARKGRAY }
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
        for (int i = -40; i < SCREEN_WIDTH; i += 40) 
        {
            line(i + grid_offset, 0, i + grid_offset, SCREEN_HEIGHT);
        }
        // 横线 (向下微动)
        for (int j = -40; j < SCREEN_HEIGHT; j += 40)
        {
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
        if (!is_glitch || (rand() % 2 == 0)) 
        {
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

        for (int i = 0; i < ITEM_COUNT; i++) 
        {
            int x = menu_x;
            int y = start_y + i * (btn_h + gap);
            bool is_hover = (mouse.x >= x && mouse.x <= x + btn_w && mouse.y >= y && mouse.y <= y + btn_h);

            if (is_hover) hover_index = i;

            if (is_hover) 
            {
                // 悬停：背景填充，文字变黑
                setfillcolor(items[i].color);
                solidrectangle(x, y, x + btn_w, y + btn_h);
                settextcolor(BLACK);
            }
            else 
            {
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
            if (is_hover) 
            {
                if ((timer / 10) % 2 == 0) 
                { // 每10帧闪烁一次
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

        if (hover_index != -1) 
        {
            settextcolor(WHITE);
            outtextxy(20, SCREEN_HEIGHT - 40, items[hover_index].desc);
        }
        else 
        {
            // 待机文字呼吸效果 (根据 timer 计算灰度)
            int brightness = 100 + abs((int)(sin(timer * 0.05) * 150));
            if (brightness > 255) brightness = 255;
            settextcolor(RGB(brightness, brightness, brightness));

            // 每隔一段时间加上光标 "_"
            bool show_cursor = (timer / 30) % 2 == 0;
            if (show_cursor)
                outtextxy(20, SCREEN_HEIGHT - 40, _T("等待信号接入..._"));
            else
                outtextxy(20, SCREEN_HEIGHT - 40, _T("等待信号接入..."));
        }

        // --- 6. 全局特效：CRT 扫描线 ---
        setlinecolor(RGB(0, 50, 50));
        line(0, scan_line_y, SCREEN_WIDTH, scan_line_y);
        line(0, scan_line_y + 1, SCREEN_WIDTH, scan_line_y + 1); // 加粗一点

        // 静态旧电视扫描纹理 (每隔2行画一条黑线，产生隔行扫描感)
        setlinecolor(0);
        for (int k = 0; k < SCREEN_HEIGHT; k += 4) 
        {
        }

        FlushBatchDraw();

        // --- 7. 输入处理 ---
        if (MouseHit()) 
        {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN) 
            {
                for (int i = 0; i < ITEM_COUNT; i++) 
                {
                    int x = menu_x;
                    int y = start_y + i * (btn_h + gap);
                    if (isClickIn(msg.x, msg.y, x, y, btn_w, btn_h))
                    {
                        return i + 1;
                    }
                }
            }
        }

        Sleep(16); // 约 60 FPS
    }
}

bool UseEasyX::isClickIn(int mouse_x, int mouse_y, int x, int y, int w, int h) 
{
    return (mouse_x >= x && mouse_x <= x + w && mouse_y >= y && mouse_y <= y + h);
}

int UseEasyX::checkGameButtons(int mouse_x, int mouse_y, int offset_y)
{
    // 将 offset_y 加到 BTN_PAUSE_Y 和 BTN_RETURN_Y
    if (isClickIn(mouse_x, mouse_y, BTN_PAUSE_X, BTN_PAUSE_Y + offset_y, BTN_W, BTN_H)) return 1; // 暂停
    if (isClickIn(mouse_x, mouse_y, BTN_RETURN_X, BTN_RETURN_Y + offset_y, BTN_W, BTN_H)) return 2; // 返回
    return 0;
}

void UseEasyX::drawHistory(RecordManager& mgr)
{
    // --- 定义 VA-11 风格局部配色 ---
    const COLORREF VA_BG = RGB(20, 20, 30);     // 深空底色
    const COLORREF VA_ROW_ALT = RGB(30, 32, 45);     // 偶数行背景
    const COLORREF VA_CYAN = RGB(0, 255, 255);    // 霓虹青 (操作/高亮)
    const COLORREF VA_PINK = RGB(255, 42, 109);   // 故障粉 (警告/删除)
    const COLORREF VA_BLUE = RGB(0, 120, 255);    // 深蓝 (修改)
    const COLORREF VA_YELLOW = RGB(255, 200, 50);   // 黄色 (搜索)
    const COLORREF VA_WHITE = RGB(220, 220, 220);  // 灰白
    const COLORREF VA_GRAY = RGB(100, 100, 110);  // 暗灰

    // 按钮布局参数
    int btn_w = 140; // 按钮加宽以容纳中文和英文
    int btn_h = 40;
    int btn_y = SCREEN_HEIGHT - 80;
    int gap = 15;

    // 计算按钮位置 (居中排列)
    int total_w = 4 * btn_w + 3 * gap;
    int start_x = (SCREEN_WIDTH - total_w) / 2; // 功能按钮居中

    int x_add = start_x;
    int x_del = x_add + btn_w + gap;
    int x_mod = x_del + btn_w + gap;
    int x_search = x_mod + btn_w + gap;

    // 返回按钮独立放在右下角
    int x_back = SCREEN_WIDTH - 160;

    // 搜索状态
    bool is_searching = false;
    std::vector<Record> search_result;

    // 鼠标位置追踪
    int mx = 0, my = 0;

    // 辅助 lambda: 绘制 VA 风格功能按钮
    auto drawFuncButton = [&](int x, int y, int w, int h, LPCTSTR text, LPCTSTR sub, COLORREF color, bool hover) {
        // 背景
        setfillcolor(hover ? color : VA_BG);
        setlinecolor(color);
        fillrectangle(x, y, x + w, y + h);

        // 装饰框 (悬停时)
        if (hover) {
            setlinecolor(WHITE);
            rectangle(x - 3, y - 3, x + w + 3, y + h + 3);
            settextcolor(WHITE);
        }
        else {
            settextcolor(color);
        }

        // 文字
        setbkmode(TRANSPARENT);
        settextstyle(20, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
        outtextxy(x + 10, y + 5, text);

        // 英文副标 (小字)
        settextstyle(12, 0, _T("Consolas"));
        outtextxy(x + 10, y + 25, sub);
        };

    while (true)
    {
        // 1. 处理输入 (追踪鼠标 + 点击)
        bool is_click = false;
        while (MouseHit()) {
            MOUSEMSG msg = GetMouseMsg();
            mx = msg.x;
            my = msg.y;
            if (msg.uMsg == WM_LBUTTONDOWN) is_click = true;
        }

        // 2. 绘制背景
        setbkcolor(VA_BG);
        cleardevice();

        // 绘制背景网格
        setlinecolor(RGB(25, 25, 35));
        for (int i = 0; i < SCREEN_WIDTH; i += 40) line(i, 0, i, SCREEN_HEIGHT);
        for (int i = 0; i < SCREEN_HEIGHT; i += 40) line(0, i, SCREEN_WIDTH, i);

        // 3. 标题：[ 记忆数据管理 ]
        settextstyle(40, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
        LPCTSTR title = is_searching ? _T("[ 检索结果片段 ]") : _T("[ 记忆数据管理 ]");

        // 标题故障阴影
        int title_w = textwidth(title);
        int cx = SCREEN_WIDTH / 2;
        settextcolor(RGB(100, 0, 100)); // 紫色阴影
        outtextxy(cx - title_w / 2 + 4, 30 + 4, title);

        settextcolor(is_searching ? VA_YELLOW : VA_CYAN);
        outtextxy(cx - title_w / 2, 30, title);

        // 装饰横线
        setlinecolor(VA_WHITE);
        line(50, 90, SCREEN_WIDTH - 50, 90);

        // 4. 列表表头
        int y_head = 110;
        settextstyle(20, 0, _T("Consolas"));
        settextcolor(VA_GRAY);

        // 列坐标
        int col_id = 100;
        int col_ver = 250;
        int col_name = 550;
        int col_score = 900;

        outtextxy(col_id, y_head, _T("INDEX_ID"));
        outtextxy(col_ver, y_head, _T("DREAM_TYPE"));
        outtextxy(col_name, y_head, _T("INFILTRATOR"));
        outtextxy(col_score, y_head, _T("SYNC_RATE"));

        // 5. 列表内容
        const std::vector<Record>& list_to_show = is_searching ? search_result : mgr.getAllRecords();
        int y = 140;
        int row_h = 35;

        settextstyle(20, 0, _T("微软雅黑")); // 内容用微软雅黑

        int count = 0;
        for (size_t i = 0; i < list_to_show.size(); ++i)
        {
            if (count >= 12) break;

            const auto& rec = list_to_show[i];

            // 斑马纹背景
            if (count % 2 == 0) {
                setfillcolor(VA_ROW_ALT);
                solidrectangle(50, y - 2, SCREEN_WIDTH - 50, y + row_h - 2);
            }

            // ID
            settextcolor(VA_GRAY);
            TCHAR buf[128];
            if (!is_searching) {
                _stprintf_s(buf, _T("#%03d"), (int)i); // ID 补零
                outtextxy(col_id, y, buf);
            }
            else {
                outtextxy(col_id, y, _T("---"));
            }

            // 版本 (中文映射)
            settextcolor(VA_WHITE);
            std::string ver_str = rec.version;
            COLORREF v_color = VA_WHITE;
            if (ver_str == "Intro") { ver_str = "Intro"; v_color = VA_CYAN; }
            else if (ver_str == "Advanced") { ver_str = "Advanced"; v_color = VA_BLUE; }
            else if (ver_str == "Expert") { ver_str = "Expert"; v_color = VA_PINK; }
            else if (ver_str == "Manual") { ver_str = "Manual"; v_color = VA_YELLOW; }

            // 名字
            settextcolor(VA_WHITE);
            std::string safe_name = rec.user_name;
            if (safe_name.length() > 15) safe_name = safe_name.substr(0, 15) + "...";

            TCHAR ver_buf[64];
#ifdef UNICODE
            wchar_t w_ver[64];
            MultiByteToWideChar(CP_ACP, 0, ver_str.c_str(), -1, w_ver, 64);
            _tcscpy_s(ver_buf, w_ver);
#else
            strcpy_s(ver_buf, ver_str.c_str());
#endif
            settextcolor(v_color);
            outtextxy(col_ver, y, ver_buf);


#ifdef UNICODE
            wchar_t w_name[64];
            MultiByteToWideChar(CP_ACP, 0, safe_name.c_str(), -1, w_name, 64);
            outtextxy(col_name, y, w_name);
#else
            outtextxy(col_name, y, safe_name.c_str());
#endif

            // 分数
            settextcolor(VA_CYAN);
            _stprintf_s(buf, _T("%06d"), rec.score);
            settextstyle(20, 0, _T("Consolas")); // 数字用 Consolas
            outtextxy(col_score, y, buf);
            settextstyle(20, 0, _T("微软雅黑")); // 还原字体

            y += row_h;
            count++;
        }

        // 6. 绘制功能按钮
        // 只有非搜索模式才显示操作按钮
        if (!is_searching)
        {
            // 增 - 注入
            bool h_add = isClickIn(mx, my, x_add, btn_y, btn_w, btn_h);
            drawFuncButton(x_add, btn_y, btn_w, btn_h, _T("注入数据"), _T("INJECT"), VA_CYAN, h_add);
            if (is_click && h_add) {
                TCHAR n[32] = { 0 }, s[32] = { 0 };
                InputBox(n, 32, _T("INFILTRATOR NAME:"), _T("INJECT"), _T("Ghost"), 0, 0, false);
                InputBox(s, 32, _T("SYNC RATE:"), _T("INJECT"), _T("1000"), 0, 0, false);
                // ...转码并 addRecord...
#ifdef UNICODE
                int len = WideCharToMultiByte(CP_ACP, 0, n, -1, NULL, 0, NULL, NULL);
                char* p = new char[len];
                WideCharToMultiByte(CP_ACP, 0, n, -1, p, len, NULL, NULL);
                std::string ns(p); delete[] p;
#else
                std::string ns(n);
#endif
                mgr.addRecord("Manual", ns, _ttoi(s));
            }

            // 删 - 抹除
            bool h_del = isClickIn(mx, my, x_del, btn_y, btn_w, btn_h);
            drawFuncButton(x_del, btn_y, btn_w, btn_h, _T("抹除痕迹"), _T("PURGE"), VA_PINK, h_del);
            if (is_click && h_del) {
                TCHAR i[32] = { 0 };
                InputBox(i, 32, _T("TARGET ID:"), _T("PURGE"), _T("0"), 0, 0, false);
                mgr.deleteRecord(_ttoi(i));
            }

            // 改 - 篡改
            bool h_mod = isClickIn(mx, my, x_mod, btn_y, btn_w, btn_h);
            drawFuncButton(x_mod, btn_y, btn_w, btn_h, _T("篡改记忆"), _T("REWRITE"), VA_BLUE, h_mod);
            if (is_click && h_mod) {
                // ... InputBox 逻辑 ...
                TCHAR o[32], n[32];
                InputBox(o, 32, _T("ORIGINAL NAME:"), _T("REWRITE"), _T(""), 0, 0, false);
                InputBox(n, 32, _T("NEW ALIAS:"), _T("REWRITE"), _T(""), 0, 0, false);
                // ... 转码并 modify ...
#ifdef UNICODE
                // 转码旧名字 (TCHAR -> std::string)
                int len_o = WideCharToMultiByte(CP_ACP, 0, o, -1, NULL, 0, NULL, NULL);
                char* p_o = new char[len_o];
                WideCharToMultiByte(CP_ACP, 0, o, -1, p_o, len_o, NULL, NULL);
                std::string str_old(p_o);
                delete[] p_o;

                // 转码新名字
                int len_n = WideCharToMultiByte(CP_ACP, 0, n, -1, NULL, 0, NULL, NULL);
                char* p_n = new char[len_n];
                WideCharToMultiByte(CP_ACP, 0, n, -1, p_n, len_n, NULL, NULL);
                std::string str_new(p_n);
                delete[] p_n;

                mgr.modifyUserName(str_old, str_new);
#else
                mgr.modifyUserName(o, n);
#endif
            }

            // 查 - 检索
            bool h_src = isClickIn(mx, my, x_search, btn_y, btn_w, btn_h);
            drawFuncButton(x_search, btn_y, btn_w, btn_h, _T("检索片段"), _T("SEARCH"), VA_YELLOW, h_src);
            if (is_click && h_src) {
                TCHAR t[32];
                InputBox(t, 32, _T("KEYWORD:"), _T("SEARCH"), _T(""), 0, 0, false);
#ifdef UNICODE
                // 转码搜索关键词 (TCHAR -> std::string)
                int len = WideCharToMultiByte(CP_ACP, 0, t, -1, NULL, 0, NULL, NULL);
                char* p = new char[len];
                WideCharToMultiByte(CP_ACP, 0, t, -1, p, len, NULL, NULL);
                std::string str_key(p);
                delete[] p;

                search_result = mgr.searchRecords(str_key);
#else
                search_result = mgr.searchRecords(t);
#endif
                is_searching = true;
            }
        }

        // 返回 / 重置
        bool h_back = isClickIn(mx, my, x_back, btn_y, btn_w, btn_h);
        drawFuncButton(x_back, btn_y, btn_w, btn_h,
            is_searching ? _T("重置视图") : _T("断开连接"),
            is_searching ? _T("RESET") : _T("DISCONNECT"),
            VA_WHITE, h_back);

        if (is_click && h_back) {
            if (is_searching) is_searching = false;
            else break;
        }

        FlushBatchDraw();
        Sleep(10);
    }
}

void UseEasyX::drawDualGameOver(int winner)
{
    // --- 局部配色 ---
    const COLORREF VA_BG = RGB(20, 20, 30);
    const COLORREF VA_GREEN = RGB(0, 255, 128);    // P1 主色 (荧光绿)
    const COLORREF VA_BLUE = RGB(0, 120, 255);    // P2 主色 (深空蓝)
    const COLORREF VA_YELLOW = RGB(255, 200, 50);   // 平局/警告
    const COLORREF VA_WHITE = RGB(220, 220, 220);

    // 1. 清除背景
    setbkcolor(VA_BG);
    cleardevice();

    // 2. 确定主色调和文案
    COLORREF main_color = VA_YELLOW;
    LPCTSTR title_text = _T("");
    LPCTSTR sub_text = _T("");

    if (winner == 1) {
        main_color = VA_GREEN;
        title_text = _T("[ 链路 A 覆盖成功 ]");
        sub_text = _T("VICTOR: NEURAL LINK A (P1)");
    }
    else if (winner == 2) {
        main_color = VA_BLUE;
        title_text = _T("[ 链路 B 覆盖成功 ]");
        sub_text = _T("VICTOR: NEURAL LINK B (P2)");
    }
    else {
        main_color = VA_YELLOW;
        title_text = _T("[ 信号严重冲突 ]"); // 平局
        sub_text = _T("ERROR: SIGNAL DEADLOCK (DRAW)");
    }

    // 3. 绘制警告框 (居中)
    int cx = SCREEN_WIDTH / 2;
    int cy = SCREEN_HEIGHT / 2;
    int box_w = 680;
    int box_h = 320;

    // 半透明底
    setfillcolor(RGB(30, 32, 45));
    fillrectangle(cx - box_w / 2, cy - box_h / 2, cx + box_w / 2, cy + box_h / 2);

    // 粗边框 (颜色跟随胜者)
    setlinecolor(main_color);
    setlinestyle(PS_SOLID, 4);
    rectangle(cx - box_w / 2, cy - box_h / 2, cx + box_w / 2, cy + box_h / 2);

    // 内部扫描线
    setlinecolor(RGB(40, 45, 55));
    setlinestyle(PS_SOLID, 1);
    for (int y = cy - box_h / 2 + 10; y < cy + box_h / 2; y += 6) {
        line(cx - box_w / 2 + 5, y, cx + box_w / 2 - 5, y);
    }

    // 4. 绘制标题 (带故障阴影)
    setbkmode(TRANSPARENT);
    settextstyle(50, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);

    // 阴影
    settextcolor(RGB(50, 50, 50));
    outtextxy(cx - textwidth(title_text) / 2 + 4, cy - 80 + 4, title_text);

    // 本体
    settextcolor(main_color);
    outtextxy(cx - textwidth(title_text) / 2, cy - 80, title_text);

    // 5. 绘制副标题 (英文结果)
    settextstyle(24, 0, _T("Consolas"));
    settextcolor(VA_WHITE);
    outtextxy(cx - textwidth(sub_text) / 2, cy + 10, sub_text);

    // 分割线
    setlinecolor(VA_WHITE);
    line(cx - 200, cy + 50, cx + 200, cy + 50);

    // 6. 退出提示
    LPCTSTR tip = _T(">> 按空格键重置对抗链路 (RESET LINK)...");
    settextstyle(20, 0, _T("微软雅黑"));
    outtextxy(cx - textwidth(tip) / 2, cy + 80, tip);

    FlushBatchDraw();

    // 等待按键
    while (true) {
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) break;
        Sleep(10);
    }
}

void UseEasyX::drawDualUI(int score1, int score2, int game_time_seconds, bool is_paused)
{
    // --- 配色定义 ---
    const COLORREF VA_P1_COLOR = RGB(0, 255, 128);   // P1 绿色
    const COLORREF VA_P2_COLOR = RGB(0, 150, 255);   // P2 蓝色
    const COLORREF VA_WHITE = RGB(220, 220, 220);
    const COLORREF VA_GRAY = RGB(100, 100, 110);
    const COLORREF VA_PINK = RGB(255, 42, 109);  // 警告色

    setbkmode(TRANSPARENT);
    settextstyle(20, 0, _T("微软雅黑")); // 标签字体
    TCHAR buf[128];

    // 辅助 Lambda：绘制监控数据块
    auto drawDualItem = [&](int x, int y, LPCTSTR label, int score, COLORREF color) {
        // [ 标签 ]
        settextcolor(VA_GRAY);
        outtextxy(x, y, _T("[ "));
        int w1 = textwidth(_T("[ "));

        settextcolor(color); // 标签本身带颜色
        outtextxy(x + w1, y, label);
        int w2 = textwidth(label);

        settextcolor(VA_GRAY);
        outtextxy(x + w1 + w2, y, _T(" ]"));

        // 数值 (下移一行或者右侧) -> 这里选择下方显示大数字，更有仪表盘感
        _stprintf_s(buf, _T("%05d"), score); // 5位补零
        settextstyle(28, 0, _T("Consolas"));
        settextcolor(VA_WHITE);
        outtextxy(x + 10, y + 25, buf);
        settextstyle(20, 0, _T("微软雅黑")); // 还原
        };

    // --- 1. 顶部数据栏 ---

    // P1 (左上) -> [ 链路 A (P1) ]
    drawDualItem(30, 20, _T("链路 A (P1)"), score1, VA_P1_COLOR);

    // P2 (右上) -> [ 链路 B (P2) ]
    // 需要计算右侧位置
    int p2_x = SCREEN_WIDTH - 200;
    drawDualItem(p2_x, 20, _T("链路 B (P2)"), score2, VA_P2_COLOR);

    // 时间 (正中) -> [ 对抗时长 ]
    settextcolor(VA_GRAY);
    LPCTSTR time_lbl = _T("- 对抗时长 -");
    int tx = (SCREEN_WIDTH - textwidth(time_lbl)) / 2;
    outtextxy(tx, 20, time_lbl);

    _stprintf_s(buf, _T("%02d:%02d"), game_time_seconds / 60, game_time_seconds % 60);
    settextstyle(28, 0, _T("Consolas"));
    settextcolor(VA_WHITE);
    outtextxy((SCREEN_WIDTH - textwidth(buf)) / 2, 45, buf);


    // --- 2. 功能按钮 (复用位置，改文案) ---
    // 为了不遮挡分数，按钮位置可能需要微调，这里保持原位但风格化
    // 暂停 -> 挂起 / 恢复
    drawButton(BTN_PAUSE_X, BTN_PAUSE_Y + 40, BTN_W, BTN_H,
        is_paused ? _T("恢复链路") : _T("挂起对抗"),
        is_paused ? VA_PINK : RGB(0, 200, 200));

    // 菜单 -> 强制终止
    drawButton(BTN_RETURN_X, BTN_RETURN_Y + 40, BTN_W, BTN_H, _T("强制终止"), VA_GRAY);


    // --- 3. 暂停遮罩 (双人版) ---
    if (is_paused)
    {
        // 1. 暗色遮罩
        setlinecolor(RGB(0, 0, 0));
        for (int i = 0; i < SCREEN_HEIGHT; i += 2) line(0, i, SCREEN_WIDTH, i);

        // 2. 弹窗
        int cx = SCREEN_WIDTH / 2;
        int cy = SCREEN_HEIGHT / 2;
        int bw = 500, bh = 200;

        setfillcolor(RGB(20, 20, 30));
        fillrectangle(cx - bw / 2, cy - bh / 2, cx + bw / 2, cy + bh / 2);

        // 边框 (粉色警告)
        setlinecolor(VA_PINK);
        setlinestyle(PS_SOLID, 3);
        rectangle(cx - bw / 2, cy - bh / 2, cx + bw / 2, cy + bh / 2);
        setlinestyle(PS_SOLID, 1);

        // 文字
        settextstyle(40, 0, _T("微软雅黑"), 0, 0, FW_BOLD, false, false, false);
        settextcolor(VA_PINK);
        LPCTSTR t = _T("[ 对抗进程挂起 ]");
        outtextxy(cx - textwidth(t) / 2, cy - 40, t);

        settextstyle(20, 0, _T("Consolas"));
        settextcolor(VA_WHITE);
        LPCTSTR st = _T(">> AWAITING SIGNAL INPUT...");
        outtextxy(cx - textwidth(st) / 2, cy + 30, st);
    }

    FlushBatchDraw();
}