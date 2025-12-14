#pragma once
#include <vector> /
#include "DataStructure.h"
#include <random> 
#include <ctime>  

class GameMap;//先声明GameMap

class Food {
private:
	std::vector<Point> food_list;  //存储每个食物坐标
	int current_food_count;  //地图上剩余食物数

public:
	Food();
	~Food();
	void generateFood(GameMap& game_map);  //随机生成食物
	int getCount() const;  //获取地图上剩余食物数
	void eatFood(Point p);  //吃食物，即从food_list中删除某个坐标点
};