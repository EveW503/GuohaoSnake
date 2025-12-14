#include "Food.h"
#include "GameMap.h"
#include <algorithm> // 用于 std::remove_if (可选，这里用手动循环也行)

Food::Food()
{
    // vector 自动初始化，无需手动循环赋值 -1
    food_list.clear();
}

Food::~Food() {};

void Food::eatFood(Point p)
{
    // 使用迭代器遍历并删除
    for (auto it = food_list.begin(); it != food_list.end(); ++it)
    {
        if (it->x == p.x && it->y == p.y)
        {
            food_list.erase(it); // 安全删除，vector 自动处理后续元素前移
            return; // 吃一个就返回
        }
    }
}

void Food::generateFood(GameMap& map)
{
    static std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
    // 限制生成区域
    std::uniform_int_distribution<int> distX(1, MAP_WIDTH - 2);
    std::uniform_int_distribution<int> distY(1, MAP_HEIGHT - 2);

    // 假设最大同时也只有 5 个食物
    const int TARGET_MAX_FOOD = 5;
    std::uniform_int_distribution<int> distCount(1, TARGET_MAX_FOOD);

    // 1. 清理地图上旧的食物
    for (const auto& p : food_list)
    {
        if (map.getBlock(p.x, p.y) == BlockType::FOOD) {
            map.setBlock(BlockType::AIR, p.x, p.y);
        }
    }

    // 2. 清空列表
    food_list.clear();

    // 3. 生成新食物
    int spawnCount = distCount(rng);

    for (int k = 0; k < spawnCount; k++)
    {
        // vector 动态增长，理论上不会溢出。
        // 但为了游戏平衡，如果已经存了太多，可以停止生成
        if (food_list.size() >= TARGET_MAX_FOOD) {
            break;
        }

        int x, y;
        bool success = false;
        int tryCount = 0;

        // 尝试随机位置
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

        // 兜底策略：遍历地图找空位
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
            // 【关键】使用 push_back，永远不会越界写到堆元数据上
            food_list.push_back({ x, y });
            map.setBlock(BlockType::FOOD, x, y);
        }
    }
}

int Food::getCount() const
{
    return (int)food_list.size();
}