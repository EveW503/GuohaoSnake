#include "GameBase.h"
#include <conio.h>   
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

GameBase::GameBase(int x, int y, Direction d):snake(x ,y ,d)
{
    is_game_over = false;
    current_score = 0;
    highest_score = 0;
    hp = 1;
    start_time = 0;
}	

void GameBase::run() 
{
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
    while (!is_game_over) 
    {

        // 1. 计算帧等待时间,分数越高蛇运动得越快
        int frame_wait_time = 150 - (current_score / 5);
        if (frame_wait_time < 50) frame_wait_time = 50;

        Direction current_dir = snake.getDirection();
        DWORD start_tick = GetTickCount();

        //渲染循环
        while (GetTickCount() - start_tick < (DWORD)frame_wait_time)
        {

            // A. 处理输入 (保持原有逻辑)
            if ((GetAsyncKeyState('W') & 0x8000) && current_dir != Direction::DOWN) snake.setDirection(Direction::UP);
            else if ((GetAsyncKeyState('S') & 0x8000) && current_dir != Direction::UP) snake.setDirection(Direction::DOWN);
            else if ((GetAsyncKeyState('A') & 0x8000) && current_dir != Direction::RIGHT) snake.setDirection(Direction::LEFT);
            else if ((GetAsyncKeyState('D') & 0x8000) && current_dir != Direction::LEFT) snake.setDirection(Direction::RIGHT);

            // B. 处理鼠标 (暂停/退出)
            bool should_break_wait = false;
            while (MouseHit()) 
            {
                MOUSEMSG msg = GetMouseMsg();
                if (msg.uMsg == WM_LBUTTONDOWN) 
                {
                    int btn = renderer.checkGameButtons(msg.x, msg.y);
                    if (btn == 1) { // 暂停
                        is_paused = !is_paused; // 切换状态
                        if (is_paused) pause_start = std::time(nullptr);
                        else pause_duration += (std::time(nullptr) - pause_start);
                        should_break_wait = true; // 跳出等待去处理暂停
                    }
                    if (btn == 2) { renderer.close(); return; }
                }
            }
            if (should_break_wait) break; // 如果点击了暂停，立即跳出等待

            // C. 暂停逻辑
            if (is_paused)
            {
                // 暂停时也需要渲染，否则画面会卡死
                int display_time = static_cast<int>(std::time(nullptr) - start_time - pause_duration);
                renderer.drawMap(map);
                renderer.drawSnake(snake.getBody(), RGB(0, 120, 140), RGB(0, 255, 255));
                renderer.drawUI(current_score, highest_score, snake.getLength(), hp, display_time, true);
                Sleep(100); // 暂停时不需要太高帧率

                // 重置 start_tick 以免暂停结束后蛇瞬间移动
                start_tick = GetTickCount();
                continue;
            }

            // D. 高频渲染
            int display_time = static_cast<int>(std::time(nullptr) - start_time - pause_duration);

            renderer.drawMap(map); 
            renderer.drawSnake(snake.getBody(), RGB(0, 120, 140), RGB(0, 255, 255));
            renderer.drawUI(current_score, highest_score, snake.getLength(), hp, display_time, false);

            Sleep(16); // 锁定约 60 FPS
        }

        //逻辑更新
        if (!is_paused) 
        {
            update();
        }
    }

    // 游戏结束结算
    renderer.drawGameOver(current_score);

    Sleep(500);

    std::string player_name = renderer.inputPlayerName();
    if (player_name.empty()) player_name = "Anonymous";  //默认名Anonymous
    record_mgr.addRecord(getVersionName(), player_name, current_score);

    renderer.drawRankings(record_mgr.getAllRecords());
    renderer.close();
}

void GameBase::update() 
{
    // 预判下一步位置
    Point next_pos = snake.getNextPosition();

    // 查地图碰撞检测
    BlockType type = map.getBlock(next_pos.x, next_pos.y);

    if (type == BlockType::WALL || type == BlockType::SNAKE_BODY || type == BlockType::SNAKE_HEAD)
    {
        onSnakeDie();
    }
    else if (type == BlockType::FOOD || type == BlockType::DATA_FRAG)
    {
        if (type == BlockType::DATA_FRAG)
        {
            const std::deque<Point>& old_body = snake.getBody();
            // 倒序清除最后 3 个
            int cut_count = 0;
            for (auto it = old_body.rbegin(); it != old_body.rend(); ++it)
            {
                if (cut_count >= 3) break;
                map.setBlock(BlockType::AIR, it->x, it->y);
                cut_count++;
            }

            snake.shrink(); // 物理变短
            current_score += 50; // 特殊道具加50分
        }
        else 
        {
            current_score += 10;
        }

        snake.addSnake();

        //维护地图数据
        map.setBlock(BlockType::SNAKE_HEAD, next_pos.x, next_pos.y);

        //维护食物状态
        food.eatFood(next_pos);
        if (current_score > highest_score) highest_score = current_score;

        // 5. 补货
        if (food.getCount() == 0)
        {
            food.generateFood(map);
        }
    }
    else 
    {
        // 获取移动前的尾巴位置
        Point old_tail = snake.getBody().back();

        // 蛇内部移动 (头增尾删)
        snake.moveToNextPosition();

        // 同步地图
        // 旧尾巴变成空气
        map.setBlock(BlockType::AIR, old_tail.x, old_tail.y);
        // 新头变成蛇头
        Point new_head = snake.getBody().front();
        map.setBlock(BlockType::SNAKE_HEAD, new_head.x, new_head.y);
        if (snake.getBody().size() > 1) 
        {
            Point old_head = snake.getBody()[1];
            map.setBlock(BlockType::SNAKE_BODY, old_head.x, old_head.y);
        }
    }
}


//IntroGame实现
void IntroGame::onSnakeDie() 
{
    is_game_over = true;
}

//AdvancedGame实现
void AdvancedGame::onSnakeDie()
{
    // 进阶版规则：撞到后，尸身变墙，重生
    // 尸身变墙
    const std::deque<Point>& body = snake.getBody();
    for (const auto& p : body)
    {
        map.setBlock(BlockType::WALL, p.x, p.y);
    }
    // 3. 尝试重生
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

//ExpertGame(高级版）
ExpertGame::ExpertGame() 
{
    death_count = 0;
    hp = 5; // 初始5条命
}

void ExpertGame::onSnakeDie()
{
    // 尸身变食物
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


    // 尝试重生
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

//DualGame (本地双人对战版)
DualGame::DualGame(int x_1, int y_1, Direction d_1, int x_2, int y_2, Direction d_2):
    GameBase(x_1,y_1,d_1),snake_2(x_2, y_2, d_2)
{
    score_2 = 0; // 记录 P2 的分数
    winner = 0; // 初始化为平局
    hp = 0;     
}

void DualGame::run()
{
    renderer.initGraph(SCREEN_WIDTH, SCREEN_HEIGHT);
    start_time = std::time(nullptr);
    food.generateFood(map);

    // 初始化蛇位置 (保持原样)
    const std::deque<Point>& initial_body_1 = snake.getBody();
    for (const auto& p : initial_body_1) map.setBlock(BlockType::SNAKE_BODY, p.x, p.y);
    if (!initial_body_1.empty()) map.setBlock(BlockType::SNAKE_HEAD, initial_body_1.front().x, initial_body_1.front().y);

    const std::deque<Point>& initial_body_2 = snake_2.getBody();
    for (const auto& p : initial_body_2) map.setBlock(BlockType::SNAKE_BODY, p.x, p.y);
    if (!initial_body_2.empty()) map.setBlock(BlockType::SNAKE_HEAD, initial_body_2.front().x, initial_body_2.front().y);

    bool is_paused = false;
    long long pause_duration = 0;
    long long pause_start = 0;

    while (!is_game_over) 
    {
        int frame_wait_time = 150 - (current_score / 5);
        if (frame_wait_time < 50) frame_wait_time = 50;

        Direction current_dir_1 = snake.getDirection();
        Direction current_dir_2 = snake_2.getDirection();
        DWORD start_tick = GetTickCount();

        while (GetTickCount() - start_tick < (DWORD)frame_wait_time)
        {

            // 1. P1 控制
            if ((GetAsyncKeyState('W') & 0x8000) && current_dir_1 != Direction::DOWN) snake.setDirection(Direction::UP);
            else if ((GetAsyncKeyState('S') & 0x8000) && current_dir_1 != Direction::UP) snake.setDirection(Direction::DOWN);
            else if ((GetAsyncKeyState('A') & 0x8000) && current_dir_1 != Direction::RIGHT) snake.setDirection(Direction::LEFT);
            else if ((GetAsyncKeyState('D') & 0x8000) && current_dir_1 != Direction::LEFT) snake.setDirection(Direction::RIGHT);

            // 2. P2 控制
            if ((GetAsyncKeyState(VK_UP) & 0x8000) && current_dir_2 != Direction::DOWN) snake_2.setDirection(Direction::UP);
            else if ((GetAsyncKeyState(VK_DOWN) & 0x8000) && current_dir_2 != Direction::UP) snake_2.setDirection(Direction::DOWN);
            else if ((GetAsyncKeyState(VK_LEFT) & 0x8000) && current_dir_2 != Direction::RIGHT) snake_2.setDirection(Direction::LEFT);
            else if ((GetAsyncKeyState(VK_RIGHT) & 0x8000) && current_dir_2 != Direction::LEFT) snake_2.setDirection(Direction::RIGHT);

            // 3. 鼠标检测
            bool should_break_wait = false;
            while (MouseHit())
            {
                MOUSEMSG msg = GetMouseMsg();
                if (msg.uMsg == WM_LBUTTONDOWN)
                {
                    int btn = renderer.checkGameButtons(msg.x, msg.y, 40);
                    if (btn == 1) 
                    {
                        is_paused = !is_paused;
                        if (is_paused) pause_start = std::time(nullptr);
                        else pause_duration += (std::time(nullptr) - pause_start);
                        should_break_wait = true;
                    }
                    if (btn == 2) { renderer.close(); return; }
                }
            }
            if (should_break_wait) break;

            // 4. 暂停渲染
            if (is_paused) 
            {
                int display_time = static_cast<int>(std::time(nullptr) - start_time - pause_duration);
                renderer.drawMap(map);
                renderer.drawSnake(snake.getBody(), RGB(0, 120, 140), RGB(0, 255, 255)); 
                renderer.drawSnake(snake_2.getBody(), RGB(160, 0, 60), RGB(255, 42, 109)); 
                renderer.drawDualUI(current_score, score_2, display_time, true);
                Sleep(100);
                start_tick = GetTickCount();
                continue;
            }

            // 5. 高频渲染
            int display_time = static_cast<int>(std::time(nullptr) - start_time - pause_duration);
            renderer.drawMap(map);

            renderer.drawSnake(snake.getBody(), RGB(0, 120, 140), RGB(0, 255, 255));
            renderer.drawSnake(snake_2.getBody(), RGB(160, 0, 60), RGB(255, 42, 109));

            renderer.drawDualUI(current_score, score_2, display_time, false);

            Sleep(16); 
        }

        if (!is_paused)
        {
            update();
            if (is_game_over) break;
        }
    }

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

    // 2. 平局判定，头对头相撞
    if (p1_next == p2_next)
    {
        winner = 0;
        is_game_over = true;
        return;
    }

    // 3. 准备碰撞检测
    BlockType t1 = map.getBlock(p1_next.x, p1_next.y);
    BlockType t2 = map.getBlock(p2_next.x, p2_next.y);

    bool p1_die = false;
    bool p2_die = false;

    // 4. P1 死亡判定 (撞墙、撞身体、撞另一条蛇)
    if (t1 == BlockType::WALL || t1 == BlockType::SNAKE_BODY || t1 == BlockType::SNAKE_HEAD)
    {
        p1_die = true;
    }
    // 5. P2 死亡判定
    if (t2 == BlockType::WALL || t2 == BlockType::SNAKE_BODY || t2 == BlockType::SNAKE_HEAD) 
    {
        p2_die = true;
    }

    // 6. 结算死亡
    if (p1_die && p2_die) { winner = 0; is_game_over = true; return; }
    if (p1_die) { winner = 2; is_game_over = true; return; } // P1死，P2赢
    if (p2_die) { winner = 1; is_game_over = true; return; } // P2死，P1赢

    // 7. 处理 P1 移动/吃食
    if (t1 == BlockType::FOOD) 
    {
        snake.addSnake(); // 变长
        food.eatFood(p1_next); // 清除食物
        current_score += 10;   // 借用 current_score 存 P1 分数
    }
    else 
    {
        Point tail = snake.getBody().back();
        map.setBlock(BlockType::AIR, tail.x, tail.y); // 清除尾巴
        snake.moveToNextPosition();
    }
    // 更新 P1 新头位置
    map.setBlock(BlockType::SNAKE_HEAD, p1_next.x, p1_next.y);
    // 将 P1 旧头标记为身体
    if (snake.getLength() > 1) 
    {
        Point old_head = snake.getBody()[1];
        map.setBlock(BlockType::SNAKE_BODY, old_head.x, old_head.y);
    }

    // 8. 处理 P2 移动/吃食
    if (t2 == BlockType::FOOD)
    {
        snake_2.addSnake();
        food.eatFood(p2_next);
        score_2 += 10; // P2 得分 +10
    }
    else 
    {
        Point tail = snake_2.getBody().back();
        map.setBlock(BlockType::AIR, tail.x, tail.y);
        snake_2.moveToNextPosition();
    }
    map.setBlock(BlockType::SNAKE_HEAD, p2_next.x, p2_next.y);
    if (snake_2.getLength() > 1)
    {
        Point old_head = snake_2.getBody()[1];
        map.setBlock(BlockType::SNAKE_BODY, old_head.x, old_head.y);
    }

    // 9. 补充食物
    if (food.getCount() == 0)
    {
        food.generateFood(map);
    }
}

void DualGame::onSnakeDie()
{
    is_game_over = true;
}
