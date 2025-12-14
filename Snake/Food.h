#pragma once
#include <vector> // 【新增】必须包含
#include "DataStructure.h"
#include <random> // 必须引入
#include <ctime>  // 用于获取时间作为种子

class GameMap;




class Food {
private:
	std::vector<Point> food_list;
	int current_food_count;
public:
	Food();
	~Food();
	void generateFood(GameMap& game_map);
	int getCount() const;
	void eatFood(Point p);
};