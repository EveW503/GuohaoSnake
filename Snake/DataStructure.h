#pragma once

#include "easyx.h"

#include <deque>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime> 
#include <random> 

// 渲染参数（给 EasyX 用的）
const int BLOCK_SIZE = 40;     // 每个格子 40x40 像素
const int SCREEN_WIDTH = 1280; // 屏幕宽
const int SCREEN_HEIGHT = 720;// 屏幕高

// 逻辑参数（给数组用的）
// 宽 32 格，高 18 格
const int MAP_WIDTH = SCREEN_WIDTH / BLOCK_SIZE;
const int MAP_HEIGHT = SCREEN_HEIGHT / BLOCK_SIZE;


struct Point {
	int x;
	int y;
	bool operator==(const Point& other) const
	{ 
		return x == other.x && y == other.y;
	};

	bool operator!=(const Point& other) const 
	{
		return !(*this == other);
	}
};

enum class BlockType{
	AIR,
	SNAKE_HEAD,
	SNAKE_BODY,
	WALL,
	FOOD,
	DATA_FRAG
};

enum class Direction{ 
	UP,
	DOWN,
	LEFT, 
	RIGHT
};
