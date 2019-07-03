//
// Created by ubuntu on 19-7-2.
//

#ifndef AUTOHDMAP_DATACHECK_ERRORREPORTJSONOUTPUT_H
#define AUTOHDMAP_DATACHECK_ERRORREPORTJSONOUTPUT_H
#include <Poco/JSON/Object.h>
#include <mutex>
#include <api/KDSServiceModel.h>
#include "data/ErrorDataModel.h"
using namespace std;
using namespace kd::api;
using namespace kd::dc;

struct ReportLogItem {
    //报错描述
    string err_desc;///////////
    //标记状态
    string state;///////////
    //任务号
    string task_id;///
    // project info
    string project_id;
    shared_ptr<DCCoord> node;////
    vector<shared_ptr<ErrNodeInfo>> errNodeInfo;/////
    // 默认值
    ReportLogItem() {
        state = "0";
//        auto& cfg_ins = CompilerConfig::getInstance();
//        branch_name = cfg_ins.getProperty(CONFIG_LOG_JSON_BRANCH_NAME);
//        create_by = "AUTO";
//        project_id = cfg_ins.getProperty(CONFIG_LOG_JSON_PROJECT_ID);
    }
};
struct ReportLogTotal{
    string batchId;
    ///检查项ID
    string checkItemCode;
    long endAt;
    ///问题数量
    int failNum;
    int startAt;
    ///没问题的数据数量
    int successNum;
    ///总数
    int totalNum;

};


class ReportJsonLog {
public:
    static ReportJsonLog &GetInstance();


    void AppendCheckItemTotal(ReportLogTotal &checkItem);

    void AppendErrorCase(ReportLogItem &reportLogItem);

    void Stringify(ostream& out);

    bool WriteToFile(string path);

private:

    bool RecursivelyCreateDir(const std::string &path, int mode = 0755);

private:
    Poco::JSON::Array check_item;
private:
    Poco::JSON::Array log_json_array_;
    std::mutex mutex_;
};


#endif //AUTOHDMAP_DATACHECK_ERRORREPORTJSONOUTPUT_H
