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

GameBase::GameBase(int x, int y, Direction d):snake(x ,y ,d) {
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

                        int dt = static_cast<int>(std::time(nullptr) - start_time - pause_duration);
                        renderer.drawUI(current_score, highest_score, snake.getLength(), hp, dt, is_paused);

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
        renderer.drawSnake(snake.getBody(), GREEN, LIGHTGREEN);
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

// --- DualGame (本地双人对战版) 实现 ---
DualGame::DualGame(int x_1, int y_1, Direction d_1, int x_2, int y_2, Direction d_2):
    GameBase(x_1,y_1,d_1),snake_2(x_2, y_2, d_2)
{
    score_2 = 0; // 【新增】记录 P2 的分数
    winner = 0; // 初始化为平局
    hp = 0;     // 双人模式不需要 HP
}

void DualGame::run()
{
    renderer.initGraph(SCREEN_WIDTH, SCREEN_HEIGHT);

    start_time = std::time(nullptr);
    food.generateFood(map);

    // 初始化蛇位置
    const std::deque<Point>& initial_body_1 = snake.getBody();
    for (const auto& p : initial_body_1) map.setBlock(BlockType::SNAKE_BODY, p.x, p.y);
    if (!initial_body_1.empty()) map.setBlock(BlockType::SNAKE_HEAD, initial_body_1.front().x, initial_body_1.front().y);

    const std::deque<Point>& initial_body_2 = snake_2.getBody();
    for (const auto& p : initial_body_2) map.setBlock(BlockType::SNAKE_BODY, p.x, p.y);
    if (!initial_body_2.empty()) map.setBlock(BlockType::SNAKE_HEAD, initial_body_2.front().x, initial_body_2.front().y);

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
                    renderer.drawDualUI(current_score, score_2, dt, is_paused);
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

        Direction current_dir_1 = snake.getDirection();   // 蛇一方向
        Direction current_dir_2 = snake_2.getDirection(); // 【新增】蛇二方向

        DWORD start_tick = GetTickCount();

        while (GetTickCount() - start_tick < (DWORD)frame_wait_time) {
            // 1. 蛇一控制 (使用 current_dir_1)
            if ((GetAsyncKeyState('W') & 0x8000) && current_dir_1 != Direction::DOWN)
                snake.setDirection(Direction::UP);
            else if ((GetAsyncKeyState('S') & 0x8000) && current_dir_1 != Direction::UP)
                snake.setDirection(Direction::DOWN);
            else if ((GetAsyncKeyState('A') & 0x8000) && current_dir_1 != Direction::RIGHT)
                snake.setDirection(Direction::LEFT);
            else if ((GetAsyncKeyState('D') & 0x8000) && current_dir_1 != Direction::LEFT)
                snake.setDirection(Direction::RIGHT);

            // 2. 蛇二控制 (使用 current_dir_2) 【修复点】
            if ((GetAsyncKeyState(VK_UP) & 0x8000) && current_dir_2 != Direction::DOWN)
                snake_2.setDirection(Direction::UP);
            else if ((GetAsyncKeyState(VK_DOWN) & 0x8000) && current_dir_2 != Direction::UP)
                snake_2.setDirection(Direction::DOWN);
            else if ((GetAsyncKeyState(VK_LEFT) & 0x8000) && current_dir_2 != Direction::RIGHT) // 现在逻辑正确了
                snake_2.setDirection(Direction::LEFT);
            else if ((GetAsyncKeyState(VK_RIGHT) & 0x8000) && current_dir_2 != Direction::LEFT)
                snake_2.setDirection(Direction::RIGHT);

            // 【关键修复】等待期间也要检测鼠标
            bool should_break_wait = false; // 标记是否需要跳出等待循环

            while (MouseHit()) {
                MOUSEMSG msg = GetMouseMsg();
                if (msg.uMsg == WM_LBUTTONDOWN) {
                    int btn = renderer.checkGameButtons(msg.x, msg.y);
                    if (btn == 1) { // 暂停
                        is_paused = true;
                        pause_start = std::time(nullptr);

                        int dt = static_cast<int>(std::time(nullptr) - start_time - pause_duration);
                        renderer.drawDualUI(current_score, score_2, dt, is_paused);

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

        if (is_game_over) break;

        // 渲染
        int display_time = static_cast<int>(std::time(nullptr) - start_time - pause_duration);
        renderer.drawMap(map);
        renderer.drawSnake(snake.getBody(), GREEN, LIGHTGREEN);
        renderer.drawSnake(snake_2.getBody(), BLUE, LIGHTBLUE);
        renderer.drawDualUI(current_score, score_2, display_time, false);
    }

    // 游戏结束结算
    renderer.drawDualGameOver(winner);
    renderer.close();
}

void DualGame::update()
{
    // 1. 获取两蛇的下一步位置
    Point p1_next = snake.getNextPosition();
    Point p2_next = snake_2.getNextPosition();

    bool p1_out = (p1_next.x < 0 || p1_next.x >= MAP_WIDTH || p1_next.y < 0 || p1_next.y >= MAP_HEIGHT);
    bool p2_out = (p2_next.x < 0 || p2_next.x >= MAP_WIDTH || p2_next.y < 0 || p2_next.y >= MAP_HEIGHT);

    if (p1_out && p2_out) { winner = 0; is_game_over = true; return; }
    if (p1_out) { winner = 2; is_game_over = true; return; }
    if (p2_out) { winner = 1; is_game_over = true; return; }

    // 2. 【平局判定 1】头对头相撞
    if (p1_next == p2_next) {
        winner = 0;
        is_game_over = true;
        return;
    }

    // 3. 准备碰撞检测
    // 为了允许“追尾”（比如 P1 的头走到 P2 刚离开的尾巴位置），
    // 我们需要先检查是否吃食。如果不吃食，尾巴稍后会移走，所以尾巴是安全的。
    // 但为了逻辑简化和稳定性，这里我们先判断该位置当前的状态。

    BlockType t1 = map.getBlock(p1_next.x, p1_next.y);
    BlockType t2 = map.getBlock(p2_next.x, p2_next.y);

    bool p1_die = false;
    bool p2_die = false;

    // 4. P1 死亡判定 (撞墙、撞身体、撞另一条蛇)
    // 注意：如果是 FOOD，不算死
    if (t1 == BlockType::WALL || t1 == BlockType::SNAKE_BODY || t1 == BlockType::SNAKE_HEAD) {
        // 特殊处理：如果是撞到自己的尾巴且没吃食，或者撞到对方的尾巴且对方没吃食，其实是安全的
        // 但为了代码简洁，这里采用严格判定：撞到任何非空气非食物即死
        p1_die = true;
    }

    // 5. P2 死亡判定
    if (t2 == BlockType::WALL || t2 == BlockType::SNAKE_BODY || t2 == BlockType::SNAKE_HEAD) {
        p2_die = true;
    }

    // 6. 结算死亡
    if (p1_die && p2_die) { winner = 0; is_game_over = true; return; }
    if (p1_die) { winner = 2; is_game_over = true; return; } // P1死，P2赢
    if (p2_die) { winner = 1; is_game_over = true; return; } // P2死，P1赢

    // 7. 处理 P1 移动/吃食
    if (t1 == BlockType::FOOD) {
        snake.addSnake(); // 变长
        food.eatFood(p1_next); // 清除食物
        current_score += 10;   // 借用 current_score 存 P1 分数
    }
    else {
        Point tail = snake.getBody().back();
        map.setBlock(BlockType::AIR, tail.x, tail.y); // 清除尾巴
        snake.moveToNextPosition();
    }
    // 更新 P1 新头位置
    map.setBlock(BlockType::SNAKE_HEAD, p1_next.x, p1_next.y);
    // 将 P1 旧头标记为身体
    if (snake.getLength() > 1) {
        Point old_head = snake.getBody()[1];
        map.setBlock(BlockType::SNAKE_BODY, old_head.x, old_head.y);
    }

    // 8. 处理 P2 移动/吃食
    if (t2 == BlockType::FOOD) {
        snake_2.addSnake();
        food.eatFood(p2_next);
        score_2 += 10; // 【新增】P2 得分 +10
    }
    else {
        Point tail = snake_2.getBody().back();
        map.setBlock(BlockType::AIR, tail.x, tail.y);
        snake_2.moveToNextPosition();
    }
    map.setBlock(BlockType::SNAKE_HEAD, p2_next.x, p2_next.y);
    if (snake_2.getLength() > 1) {
        Point old_head = snake_2.getBody()[1];
        map.setBlock(BlockType::SNAKE_BODY, old_head.x, old_head.y);
    }

    // 9. 补充食物
    if (food.getCount() < 2) {
        food.generateFood(map);
    }
}

void DualGame::onSnakeDie()
{
    // 这个函数可能不会被调用，因为我们在 update 里直接处理了死亡
    // 但为了接口完整性：
    is_game_over = true;
}
