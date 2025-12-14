#include "GameMap.h"

GameMap::GameMap()
{
    // 1. 初始化全图为空气
    for (int i = 0; i < MAP_WIDTH; i++)
    {
        for (int j = 0; j < MAP_HEIGHT; j++)
        {
            game_map[i][j] = BlockType::AIR;
        }
    }

    // 2. 绘制上下墙壁
    for (int i = 0; i < MAP_WIDTH; i++)
    {
        game_map[i][0] = BlockType::WALL;
        game_map[i][MAP_HEIGHT - 1] = BlockType::WALL;
    }

    // 3. 绘制左右墙壁
    for (int j = 0; j < MAP_HEIGHT; j++)
    {
        game_map[0][j] = BlockType::WALL;
        game_map[MAP_WIDTH - 1][j] = BlockType::WALL;
    }
}

GameMap::~GameMap() {};

BlockType GameMap::getBlock(int x, int y)
{
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
    {
        return BlockType::WALL;
    }
    return game_map[x][y];
}

void GameMap::setBlock(BlockType type, int x, int y)
{
    if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT)
    {
        game_map[x][y] = type;
    }
}