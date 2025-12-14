#pragma once

#include "DataStructure.h"

class GameMap {
private:
	BlockType game_map[MAP_WIDTH][MAP_HEIGHT];//在1280x720分辨率下网格化坐标，每个格子20x20，网格化结果宽32，高18

public:
	GameMap();
	~GameMap();
	//获取指定网格的类型
	BlockType getBlock(int x, int y);
	//设置指定网格为指定类型
	void setBlock(BlockType type, int x, int y);
};