#include "GameBase.h"
#include <conio.h>   // 用于 _kbhit
#include <windows.h>
#include <stdio.h>
#include <graphics.h>

GameBase::GameBase()
{
	is_game_over = false;
	current_score = 0;
	highest_score = 0;
	hp = 1;
	start_time = 0;
}

void GameBase::run() {
    renderer.initGraph(SCREEN_WIDTH, SCREEN_HEIGHT);
    record_mgr.loadRecords();
    highest_score = record_mgr.getHighestScore(getVersionName());

    start_time = std::time(nullptr);
    food.generateFood(map);

    // 初始化蛇位置
    const std::deque<Point>& initial_body = snake.getBody();
    for (const auto& p : initial_body) map.setBlock(BlockType::SNAKE_BODY, p.x, p.y);
    if (!initial_body.empty()) map.setBlock(BlockType::SNAKE_HEAD, initial_body.front().x, initial_body.front().y);

    bool is_paused = false;
    long long pause_duration = 0;
    long long pause_start = 0;

    // 主循环
    while (!is_game_over) {

        // --- 1. 处理鼠标点击 (按钮) ---
        // 使用 while 循环清空消息队列，解决点击不灵敏问题
        while (MouseHit()) {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN) {
                int action = renderer.checkGameButtons(msg.x, msg.y);

                if (action == 1) { // 暂停/继续
                    is_paused = !is_paused;
                    if (is_paused) {
                        pause_start = std::time(nullptr);
                    }
                    else {
                        pause_duration += (std::time(nullptr) - pause_start);
                    }
                    // 立即重绘一次UI
                    int dt = static_cast<int>(std::time(nullptr) - start_time - pause_duration);
                    renderer.drawUI(current_score, highest_score, snake.getLength(), hp, dt, is_paused);
                }
                else if (action == 2) { // 立即退出
                    renderer.close();
                    return; // 直接结束 run 函数
                }
            }
        }

        // --- 2. 暂停状态 ---
        if (is_paused) {
            Sleep(100);
            continue;
        }

        // --- 3. 正常游戏逻辑 ---
        int frame_wait_time = 150 - (current_score / 5);
        if (frame_wait_time < 50) frame_wait_time = 50;

        Direction current_dir = snake.getDirection();
        DWORD start_tick = GetTickCount();

        // 等待期间检测输入 (非阻塞延时)
        while (GetTickCount() - start_tick < (DWORD)frame_wait_time) {
            // 键盘控制
            if ((GetAsyncKeyState('W') & 0x8000) && current_dir != Direction::DOWN) snake.setDirection(Direction::UP);
            else if ((GetAsyncKeyState('S') & 0x8000) && current_dir != Direction::UP) snake.setDirection(Direction::DOWN);
            else if ((GetAsyncKeyState('A') & 0x8000) && current_dir != Direction::RIGHT) snake.setDirection(Direction::LEFT);
            else if ((GetAsyncKeyState('D') & 0x8000) && current_dir != Direction::LEFT) snake.setDirection(Direction::RIGHT);

            // 【关键修复】等待期间也要检测鼠标
            bool should_break_wait = false; // 标记是否需要跳出等待循环

            while (MouseHit()) {
                MOUSEMSG msg = GetMouseMsg();
                if (msg.uMsg == WM_LBUTTONDOWN) {
                    int btn = renderer.checkGameButtons(msg.x, msg.y);
                    if (btn == 1) { // 暂停
                        is_paused = true;
                        pause_start = std::time(nullptr);
                        should_break_wait = true; // 标记需要跳出外层
                        break; // 跳出 MouseHit 循环
                    }
                    if (btn == 2) { // 退出
                        renderer.close();
                        return; // 直接返回
                    }
                }
            }

            // 如果点击了暂停，跳出等待循环，让主循环处理暂停逻辑
            if (should_break_wait) {
                break;
            }

            Sleep(10);
        }

        // 如果触发了暂停，直接跳过本次 update/render，进入下一次主循环
        if (is_paused) continue;

        update();

        // 渲染
        int display_time = static_cast<int>(std::time(nullptr) - start_time - pause_duration);
        renderer.drawMap(map);
        renderer.drawSnake(snake.getBody());
        renderer.drawUI(current_score, highest_score, snake.getLength(), hp, display_time, false);
    }

    // 游戏结束结算
    renderer.drawGameOver(current_score);

    Sleep(500);

    std::string player_name = renderer.inputPlayerName();
    if (player_name.empty()) player_name = "Anonymous";
    record_mgr.addRecord(getVersionName(), player_name, current_score);

    renderer.drawRankings(record_mgr.getAllRecords());
    renderer.close();
}

void GameBase::update() 
{
    // 1. 预判下一步位置
    Point next_pos = snake.getNextPosition();

    // 2. 碰撞检测：直接查地图
    BlockType type = map.getBlock(next_pos.x, next_pos.y);

    // 3. 分支逻辑
    if (type == BlockType::WALL || type == BlockType::SNAKE_BODY || type == BlockType::SNAKE_HEAD)
    {
        // 撞墙或撞自身 -> 触发死亡逻辑 (多态)
        onSnakeDie();
    }
    else if (type == BlockType::FOOD)
    {
        // --- 吃食物逻辑 ---

        // 1. 蛇变长 (只增不删)
        snake.addSnake();

        // 2. 维护地图数据: 新头的位置设为 SNAKE_HEAD，旧头设为 BODY
        // (注意：这里需要处理旧头变身体的显示逻辑，addSnake内部实现应由Snake类保证，
        // 但为了地图同步，我们需要手动设置新头)
        map.setBlock(BlockType::SNAKE_HEAD, next_pos.x, next_pos.y);
        // 获取旧头位置并设为 BODY (略，由渲染层或下一次循环覆盖)

        // 3. 维护食物状态
        food.eatFood(next_pos); // 从数组移除
        current_score += 10;    // 加分
        if (current_score > highest_score) highest_score = current_score;

        // 4. 补货
        if (food.getCount() == 0)
        {
            food.generateFood(map);
        }
    }
    else 
    {
        // --- 普通移动逻辑 (AIR) ---

        // 1. 获取移动前的尾巴位置 (为了清除地图上的痕迹)
        // 注意：需要 Snake 类提供 getBody()
        Point old_tail = snake.getBody().back();

        // 2. 蛇内部移动 (头增尾删)
        snake.moveToNextPosition();

        // 3. 同步地图: 
        // 旧尾巴变成空气
        map.setBlock(BlockType::AIR, old_tail.x, old_tail.y);
        // 新头变成蛇头
        Point new_head = snake.getBody().front();
        map.setBlock(BlockType::SNAKE_HEAD, new_head.x, new_head.y);

        // (可选) 稍微修正一下旧头变成身体，虽然渲染器通常能处理
        if (snake.getBody().size() > 1) 
        {
            Point old_head = snake.getBody()[1];
            map.setBlock(BlockType::SNAKE_BODY, old_head.x, old_head.y);
        }
    }
}

void GameBase::render() 
{
    // 计算游戏时长 (秒)
    int duration = static_cast<int>(std::time(nullptr) - start_time);

    // 获取蛇长度
    // 注意：需要 Snake 类提供 getLength()
    int len = snake.getLength();

    // 渲染管线
    renderer.drawMap(map);
    // 注意：drawSnake 需要 const deque<Point>& 参数
    renderer.drawSnake(snake.getBody());
    renderer.drawUI(current_score, highest_score, len, hp, duration, false);
}

// --- IntroGame (入门版) 实现 ---

void IntroGame::onSnakeDie() 
{
    // 入门版规则：撞到即死
    is_game_over = true;
}

// --- AdvancedGame (进阶版) 实现 ---

void AdvancedGame::onSnakeDie()
{
    // 进阶版规则：撞到后，尸身变墙，重生

    // 1. 尸身变墙
    const std::deque<Point>& body = snake.getBody();
    for (const auto& p : body)
    {
        map.setBlock(BlockType::WALL, p.x, p.y);
    }

    // 2. 扣血 (如果有生命值设计)
    // hp--; 

    // 3. 尝试重生 (需要 Snake 提供 reset 接口)
    // 简单逻辑：在地图中心重生
    // 如果中心不是 AIR，则无法重生，游戏结束
    static std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
    std::uniform_int_distribution<int> distX(3, MAP_WIDTH - 2);
    std::uniform_int_distribution<int> distY(3, MAP_HEIGHT - 2);

    int tryCount = 0;
    bool success = false;
    int x, y;
    while (tryCount < 200)
    {
        x = distX(rng);
        y = distY(rng);
        if (map.getBlock(x, y) == BlockType::AIR &&
            map.getBlock(x - 1, y) == BlockType::AIR &&
            map.getBlock(x - 2, y) == BlockType::AIR)
        {
            success = true;
            break;
        }

        tryCount++;
    }

    if (!success)
    {
        for (int i = 1; i < MAP_WIDTH - 1 && !success; i++)
        {
            for (int j = 1; j < MAP_HEIGHT - 1; j++) 
            {
                if (map.getBlock(i , j) == BlockType::AIR && 
                    map.getBlock(i - 1, j) == BlockType::AIR && 
                    map.getBlock(i - 2, j) == BlockType::AIR) 
                {
                    x = i; y = j;
                    success = true;
                    break;
                }
            }
        }
    }

    if (success) 
    {
        snake.reset(x, y);
        food.generateFood(map);
    }
    else 
    {
        is_game_over = true;
    }

}

// --- ExpertGame (高级版) ---
ExpertGame::ExpertGame() {
    death_count = 0;
    hp = 5; // 初始显示5条命
}

void ExpertGame::onSnakeDie()
{
    // 1. 尸身变食物
    const std::deque<Point>& body = snake.getBody();
    for (const auto& p : body)
    {
        // 将身体变成食物，注意：这些食物属于额外奖励，不计入FoodManager的数量管理
        map.setBlock(BlockType::FOOD, p.x, p.y);
    }


    death_count++;
    hp--; // 更新UI显示的血量


    // 超过5次死亡，游戏彻底结束
    if (death_count >= 5) 
    {
        is_game_over = true;
        return;
    }


    // 2. 尝试重生
    static std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
    std::uniform_int_distribution<int> distX(3, MAP_WIDTH - 2);
    std::uniform_int_distribution<int> distY(3, MAP_HEIGHT - 2);

    int tryCount = 0;
    bool success = false;
    int x, y;
    while (tryCount < 200)
    {
        x = distX(rng);
        y = distY(rng);
        if (map.getBlock(x, y) == BlockType::AIR &&
            map.getBlock(x - 1, y) == BlockType::AIR &&
            map.getBlock(x - 2, y) == BlockType::AIR)
        {
            success = true;
            break;
        }
        tryCount++;
    }

    if (!success)
    {
        for (int i = 1; i < MAP_WIDTH - 1 && !success; i++)
        {
            for (int j = 1; j < MAP_HEIGHT - 1; j++)
            {
                if (map.getBlock(i, j) == BlockType::AIR &&
                    map.getBlock(i - 1, j) == BlockType::AIR &&
                    map.getBlock(i - 2, j) == BlockType::AIR)
                {
                    x = i; y = j;
                    success = true;
                    break;
                }
            }
        }
    }
}