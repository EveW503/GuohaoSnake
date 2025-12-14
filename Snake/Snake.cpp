#include "Snake.h"

Snake::Snake()
{
    //初始方向向右
    direction = Direction::RIGHT;

    // 动态计算中心位置
    int center_x = MAP_WIDTH / 2;
    int center_y = MAP_HEIGHT / 2;

    //初始化 3 节身体
    snake.push_back({ center_x, center_y });      // 头
    snake.push_back({ center_x - 1, center_y });  // 身
    snake.push_back({ center_x - 2, center_y });  // 尾
}

Snake::Snake(int x, int y, Direction d)  //为了双人模式修改生成位置
{
    direction = d;

    if (d == Direction::RIGHT)
    {
        snake.push_back({ x, y });      // 头
        snake.push_back({ x - 1, y });  // 身
        snake.push_back({ x - 2, y });  // 尾
    }
    else 
    {
        snake.push_back({ x, y });      // 头
        snake.push_back({ x + 1, y });  // 身
        snake.push_back({ x + 2, y });  // 尾
    }
}

Snake::~Snake() {}

// 获取下个位置（只预测，不移动）
Point Snake::getNextPosition()
{
    // 获取当前蛇头
    Point next_head = snake.front();

    // 根据方向计算偏移
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

void Snake::shrink()
{
    if (snake.size() > 5) 
    {
        // 移除尾部 3 节
        for (int i = 0; i < 3 && snake.size() > 5; i++) 
        {
            snake.pop_back();
        }
    }
}

void Snake::setDirection(Direction d)
{
    direction = d;
}

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