#ifndef KXS_DATACHECK_MODULES_CHECKCORE_UTIL_CHECK_LIST_CONFIG_H
#define KXS_DATACHECK_MODULES_CHECKCORE_UTIL_CHECK_LIST_CONFIG_H

#include <string>
#include <cstring>
#include <memory>
#include <map>
#include <set>
#include <fstream>

class CheckListConfig {
public:
    static CheckListConfig &getInstance() {
        static CheckListConfig instance;
        return instance;
    }

    /**
     * 加载配置文件
     * @param fileName 配置文件
     */
    void Load(std::string fileName);

    //获取检查项值
    bool IsNeedCheck(std::string key);
    /**
     * 从平台获取检查项列表
     * @param url
     * @return
     */
    bool GetCheckList(std::string url);

private:
    bool ParseCheckList(const std::string &json_result);

    //所有可用检查项列表
    std::set<std::string> check_list_;
};

#endif  // KXS_DATACHECK_MODULES_CHECKCORE_UTIL_CHECK_LIST_CONFIG_H
