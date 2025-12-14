#pragma once

#include "DataStructure.h"
#include "Snake.h"
#include "GameMap.h"
#include "UseEasyX.h"
#include "RecordManager.h"
#include "Food.h"
#include <ctime> // 用于计时

// 抽象基类
class GameBase {
protected:
    // 核心对象
    Snake snake;
    GameMap map;
    Food food;

    // 工具对象
    UseEasyX renderer;          // 渲染器
    RecordManager record_mgr;   // 存档管理器

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
    // 核心虚函数 (smallCamel)
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
    std::string getVersionName() const override { return "Intro"; }
};

// 进阶版
class AdvancedGame : public GameBase {
protected:
    void onSnakeDie() override;
    std::string getVersionName() const override { return "Advanced"; }
};

class ExpertGame : public GameBase {
private:
    int death_count; // 记录撞墙次数
public:
    ExpertGame(); // 构造函数初始化计数器
protected:
    void onSnakeDie() override;
    std::string getVersionName() const override { return "Expert"; }
};

class DualGame : public GameBase {
private:
    Snake snake_2;
    int winner; // 【新增】记录胜者: 0=平局, 1=P1赢, 2=P2赢
public:
    DualGame(int x_1, int y_1, Direction d_1, int x_2, int y_2, Direction d_2);
    ~DualGame() {};
    void run() override;
protected:
    void onSnakeDie() override;
    void update() override;
    std::string getVersionName() const override { return "Dual"; }
};