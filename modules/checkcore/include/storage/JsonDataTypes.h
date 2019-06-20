//
// Created by ubuntu on 19-6-19.
//

#ifndef AUTOHDMAP_DATACHECK_JSONDATATYPES_H
#define AUTOHDMAP_DATACHECK_JSONDATATYPES_H
#include <Poco/JSON/Object.h>
#include <mutex>
#include <api/KDSServiceModel.h>

using namespace std;
using namespace kd::api;
const static string JSONLOG_GEOMETRY_POINT = "Point";

struct LogProperty {
    //自动化标识
    string flag;
    //检查项ID
    string check_id;
    //检查项名称
    string check_name;
    //检查阶段
    string check_process;
    //错误类型
    string err_type;
    //报错描述
    string err_desc;
    //场景ID
    string scene_id;
    //标记状态
    string state;
    //编译器版本
    string compile_version;
    //数据分支
    string branch_name;
    //融合任务框号
    string frame_id;
    //数据KEY
    string data_key;
    //创建人员
    string create_by;
    //编辑人员
    string edit_by;
    //核实人员
    string check_by;
    //任务号
    string task_id;
    // project info
    string project_id;

    // 默认值
    LogProperty() {
        check_process = "1";
        err_type = "1";
        scene_id = "0";
        state = "0";
        compile_version = "v3.2.12";
//        auto& cfg_ins = CompilerConfig::getInstance();
//        branch_name = cfg_ins.getProperty(CONFIG_LOG_JSON_BRANCH_NAME);
//        create_by = "AUTO";
//        project_id = cfg_ins.getProperty(CONFIG_LOG_JSON_PROJECT_ID);
    }
};

enum OriginTypeEnum {
    ORIGIN_TYPE_GROUND = 0,
    ORIGIN_TYPE_LANE,
    ORIGIN_TYPE_POLE,
    ORIGIN_TYPE_SIGN
};

class JsonLog {
public:
    static JsonLog &GetInstance();

    void AppendCheckError(const string& check_no,
                          const string& check_no_desc,
                          const string& error_desc,
                          const string& task_id,
                          const string& err_type,
                          const string& flag,
                          shared_ptr<KDSNode> node);

    void AppendCheckError(LogProperty &log_propert, shared_ptr<KDSNode> node);

    void Stringify(ostream& out);

    bool WriteToFile(string path);

    std::string GetKeyByTask(const std::string& task_id, OriginTypeEnum type);
private:
    void SetGeometry(double x, double y, double z);
    void SetGeometry(shared_ptr<KDSNode> node);
    void SetProperties(LogProperty &log_property);
    bool RecursivelyCreateDir(const std::string &path, int mode = 0755);

    std::string GetEnumString(OriginTypeEnum type);
private:
    Poco::JSON::Object geometry_jobj_;
    Poco::JSON::Object properties_jobj_;
    Poco::JSON::Object json_array_element_;
private:
    Poco::JSON::Array log_json_array_;
    std::mutex mutex_;
};


#endif //AUTOHDMAP_DATACHECK_JSONDATATYPES_H
