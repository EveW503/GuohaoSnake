#pragma once

#include "DataStructure.h"
#include <random> // 必须引入
#include <ctime>  // 用于获取时间作为种子

class GameMap;

// 定义最大食物数量常量
const int MAX_FOOD_COUNT = 5;

class Food {
private:
	Point food_positions[MAX_FOOD_COUNT];
	int current_food_count;
public:
	Food();
	~Food();
	void generateFood(GameMap& game_map);
	int getCount() const;
	void eatFood(Point p);
};