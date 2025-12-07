#include "Food.h"
#include "GameMap.h"
#include <ctime>

Food::Food() 
{
    current_food_count = 0;
    for (int i = 0; i < MAX_FOOD_COUNT; i++) 
    {
        food_positions[i] = { -1, -1 };
    }
}

Food::~Food() {};

void Food::eatFood(Point p)
{
    for (int i = 0; i < current_food_count; i++)
    {
        if (food_positions[i] == p)
        {
            food_positions[i] = food_positions[current_food_count - 1];
            current_food_count--;
            return;
        }
    }
}

void Food::generateFood(GameMap& map) 
{
    static std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
    std::uniform_int_distribution<int> distX(1, MAP_WIDTH - 2);
    std::uniform_int_distribution<int> distY(1, MAP_HEIGHT - 2);
    std::uniform_int_distribution<int> distCount(1, MAX_FOOD_COUNT);

    // 【修改点 1：清理旧食物】----------------------------------------
    // 在生成新食物前，必须先把地图上现有的食物清除掉
    // 否则地图上会残留上一轮的食物，导致数量越积越多
    for (int i = 0; i < current_food_count; i++)
    {
        Point p = food_positions[i];
        // 确保该位置确实是食物（防止覆盖了已经被蛇身或其他物体占据的格子）
        if (map.getBlock(p.x, p.y) == BlockType::FOOD) {
            map.setBlock(BlockType::AIR, p.x, p.y);
        }
    }
    // -----------------------------------------------------------------

    // 重置计数
    current_food_count = 0;

    int spawnCount = distCount(rng);

    for (int k = 0; k < spawnCount; k++)
    {
        int x, y;
        bool success = false;
        int tryCount = 0;

        while (tryCount < 200)
        {
            x = distX(rng);
            y = distY(rng);
            if (map.getBlock(x, y) == BlockType::AIR) 
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
                    if (map.getBlock(i, j) == BlockType::AIR)
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
            food_positions[current_food_count] = { x, y };
            current_food_count++;
            map.setBlock(BlockType::FOOD, x, y);
        }
    }
}

int Food::getCount() const
{
    return current_food_count;
}