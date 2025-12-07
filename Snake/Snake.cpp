#include "Snake.h"

Snake::Snake()
{
    // 1. 初始方向向右
    direction = Direction::RIGHT;

    // 2. 动态计算中心位置 (适配 1280x720 或其他分辨率)
    int center_x = MAP_WIDTH / 2;
    int center_y = MAP_HEIGHT / 2;

    // 3. 初始化 3 节身体
    snake.push_back({ center_x, center_y });      // 头
    snake.push_back({ center_x - 1, center_y });  // 身
    snake.push_back({ center_x - 2, center_y });  // 尾
}

Snake::~Snake() {}

// 【关键修复】获取下个位置（只预测，不移动）
Point Snake::getNextPosition()
{
    // 获取当前蛇头
    Point next_head = snake.front();

    // 根据方向计算偏移
    // 之前可能这里漏了 UP/DOWN 或者写反了
    switch (direction)
    {
    case Direction::UP:
        next_head.y -= 1; // 向上是减 Y
        break;
    case Direction::DOWN:
        next_head.y += 1; // 向下是加 Y
        break;
    case Direction::LEFT:
        next_head.x -= 1;
        break;
    case Direction::RIGHT:
        next_head.x += 1;
        break;
    }

    return next_head;
}

// 实际移动（头增尾删）
void Snake::moveToNextPosition()
{
    // 复用 getNextPosition 的逻辑，防止两套逻辑不一致
    Point new_head = getNextPosition();

    // 加头
    snake.push_front(new_head);
    // 去尾
    snake.pop_back();
}

// 吃食物变长（只加头不去尾）
void Snake::addSnake()
{
    Point new_head = getNextPosition();
    snake.push_front(new_head);
}

// 设置方向（含防自杀逻辑的底层支持，虽然 GameBase 也有检查）
void Snake::setDirection(Direction d)
{
    direction = d;
}

// --- 必须补充的接口 (为了支持进阶版和 GameBase) ---

const std::deque<Point>& Snake::getBody() const
{
    return snake;
}

int Snake::getLength() const
{
    return (int)snake.size();
}

Direction Snake::getDirection() const
{
    return direction;
}

// 进阶版重生逻辑
void Snake::reset(int x, int y)
{
    snake.clear();
    direction = Direction::RIGHT;

    snake.push_back({ x, y });
    snake.push_back({ x - 1, y });
    snake.push_back({ x - 2, y });
}