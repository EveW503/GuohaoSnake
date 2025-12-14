#include "RecordManager.h"
#include <fstream>
#include <iostream>

RecordManager::RecordManager() 
{
    file_path = "game_records.txt"; 
    loadRecords();
}

RecordManager::~RecordManager() {}

// 从文件读取
void RecordManager::loadRecords() 
{
    record_list.clear();
    std::ifstream in_file(file_path);

    if (!in_file.is_open()) 
    {
        return; // 文件不存在，可能是第一次运行，直接返回
    }

    // 文件格式为：版本 用户名 分数 (每行一条)
    std::string ver, name;
    int s;
    while (in_file >> ver >> name >> s) 
    {
        record_list.push_back({ ver, name, s });
    }

    in_file.close();

    // 加载后排个序，把高分排前面，方便显示
    std::sort(record_list.begin(), record_list.end());
}

// 写入文件
void RecordManager::saveToFile() 
{
    std::ofstream out_file(file_path);

    if (out_file.is_open())
    {
        for (const auto& rec : record_list)
        {
            out_file << rec.version << " " << rec.user_name << " " << rec.score << "\n";
        }
        out_file.close();
    }
}

// 增
void RecordManager::addRecord(const std::string& version, const std::string& user_name, int score)
{
    record_list.push_back({ version, user_name, score });
    std::sort(record_list.begin(), record_list.end());
    saveToFile(); 
}

// 删 (按索引)
void RecordManager::deleteRecord(int index)
{
    if (index >= 0 && index < record_list.size()) 
    {
        record_list.erase(record_list.begin() + index);
        saveToFile();
    }
}

// 改 (仅限用户名)
void RecordManager::modifyUserName(const std::string& old_name, const std::string& new_name) 
{
    bool changed = false;
    for (auto& rec : record_list) 
    {
        if (rec.user_name == old_name) 
        {
            rec.user_name = new_name;
            changed = true;
        }
    }
    if (changed) 
    {
        saveToFile();
    }
}

// 查 (仅限用户名)
std::vector<Record> RecordManager::searchRecords(const std::string& user_name) 
{
    std::vector<Record> result;
    for (const auto& rec : record_list)
    {
        if (rec.user_name == user_name) 
        {
            result.push_back(rec);
        }
    }
    return result;
}

// 获取最高分
int RecordManager::getHighestScore(const std::string& version)
{
    // 因为我们在 load 和 add 时都做过排序(score > other.score)，
    // 所以找到的第一个匹配版本的记录就是最高分。
    for (const auto& rec : record_list)
    {
        if (rec.version == version) 
        {
            return rec.score;
        }
    }
    return 0; // 如果没记录，最高分是 0
}

const std::vector<Record>& RecordManager::getAllRecords() const 
{
    return record_list;
}