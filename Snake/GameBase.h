#pragma once

#include "DataStructure.h"
#include "Snake.h"
#include "GameMap.h"
#include "UseEasyX.h"
#include "RecordManager.h"
#include "Food.h"
#include <ctime> 

// 抽象基类
class GameBase {
protected:
    RecordManager record_mgr;   // 存档管理器

    Snake snake;  //蛇
    GameMap map;  //地图

    UseEasyX renderer;          // 渲染器

    Food food;

    // 【数据源】游戏状态数据 (snake_case)
    bool is_game_over;
    int current_score;      // 当前分数
    int highest_score;      // 历史最高分 (加载进来后存在这里)
    int hp;                 // 生命值
    time_t start_time;      // 游戏开始时间戳

public:
    GameBase();
    GameBase(int x, int y, Direction d);
    virtual ~GameBase() {}

    virtual void run(); // 主循环

protected:
    // 核心虚函数
    virtual void onSnakeDie() = 0;

    // 逻辑更新
    virtual void update();

    // 统一渲染
    virtual std::string getVersionName() const = 0;
};

// 入门版
class IntroGame : public GameBase {
protected:
    void onSnakeDie() override;
    std::string getVersionName() const override { return "Intro"; }  //用于记录版本
};

// 进阶版
class AdvancedGame : public GameBase {
protected:
    void onSnakeDie() override;
    std::string getVersionName() const override { return "Advanced"; }
};

//专家版
class ExpertGame : public GameBase {
private:
    int death_count; // 记录撞墙次数

public:
    ExpertGame(); 

protected:
    void onSnakeDie() override;
    std::string getVersionName() const override { return "Expert"; }

};

//本地双人游戏
class DualGame : public GameBase {
private:
    Snake snake_2;
    int score_2; // 记录 P2 的分数
    int winner; // 0=平局, 1=P1赢, 2=P2赢

public:
    DualGame(int x_1, int y_1, Direction d_1, int x_2, int y_2, Direction d_2);
    void run() override;

protected:
    void onSnakeDie() override;
    void update() override;
    std::string getVersionName() const override { return "Dual"; }
};