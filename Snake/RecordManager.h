#pragma once
#include <vector>
#include <string>
#include <algorithm> // 用于排序和查找

// 定义单条记录结构体
struct Record {
    std::string version;   // 游戏版本 
    std::string user_name; // 用户名
    int score;             // 分数

    // 重载 < 运算符，方便排序 (分数高的在前)
    bool operator<(const Record& other) const
    {
        return score > other.score;
    }
};

class RecordManager {
private:
    std::string file_path;           // 文件路径
    std::vector<Record> record_list; // 内存中的记录缓存

    // 内部辅助函数：将内存数据写入文件
    void saveToFile();

public:
    RecordManager();
    ~RecordManager();

    // 从文件加载所有记录
    void loadRecords();

    // 增：添加一条新记录
    void addRecord(const std::string& version, const std::string& user_name, int score);

    //删：根据索引删除一条记录
    void deleteRecord(int index);

    // 改：修改用户名
    // 将所有叫 old_name 的记录改为 new_name
    void modifyUserName(const std::string& old_name, const std::string& new_name);

    // 查：查找特定用户名的所有记录
    std::vector<Record> searchRecords(const std::string& user_name);

    // 获取某版本的历史最高分
    int getHighestScore(const std::string& version);

    // 获取所有记录 (用于 UI 显示排行榜)
    const std::vector<Record>& getAllRecords() const;
};