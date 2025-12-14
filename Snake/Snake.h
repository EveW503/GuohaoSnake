#pragma once
#include "DataStructure.h"
#include <deque>

class Snake {
private:
    std::deque<Point> snake;
    Direction direction;

public:
    Snake();
    Snake(int x, int y, Direction d);
    ~Snake();

    // 核心移动
    void moveToNextPosition();
    Point getNextPosition();
    void addSnake();  //增长身体
    void shrink(); // 缩短身体

    // 设置/获取方向
    void setDirection(Direction d);
    Direction getDirection() const;

    // 数据获取
    const std::deque<Point>& getBody() const;
    int getLength() const;

    // 游戏重置 (进阶版用)
    void reset(int x,int y);
};