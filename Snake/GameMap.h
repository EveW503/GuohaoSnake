#pragma once

#include "DataStructure.h"


class GameMap {
private:
	BlockType game_map[MAP_WIDTH][MAP_HEIGHT];//在1920x1080分辨率下网格化坐标，每个格子20x20，网格化结果宽48，高27
public:
	GameMap();
	~GameMap();
	BlockType getBlock(int x, int y);
	void setBlock(BlockType type, int x, int y);
};