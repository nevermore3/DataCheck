//module
#include "data/DataManager.h"
#include "storage/CheckTaskInput.h"
#include "storage/ModelDataInput.h"
#include "ProcessManager.h"

#include "DataCheckConfig.h"
#include "ModelProcessManager.h"
#include "process/ModelDataLoader.h"
#include "process/ModelFieldCheck.h"
#include "process/ModelBussCheck.h"
#include "process/ModelRelationCheck.h"
#include "process/TaskLoader.h"

#include "MapProcessManager.h"
#include "businesscheck/MapDataLoader.h"
#include "businesscheck/DividerSplitCheck.h"

#include "businesscheck/JsonDataLoader.h"

#include "util/TimerUtil.h"
#include "util/check_list_config.h"
#include "util/FileUtil.h"
#include "util/task_info.h"
using namespace kd::dc;

const char checkresult[] = "checkresult.json";
const char checkresultforjson[] = "error.json";
//加载任务框信息
bool LoadTaskBound(const AdjustTaskInfo task_info,map<string, shared_ptr<TaskBound>>& task_bounds) {
    auto boundinfo = task_info.param_results.find("range");
    if(boundinfo!=task_info.param_results.end()){
        string content =boundinfo->second;
        CTaskBoundService taskBoundService("");
        return taskBoundService.ParseTaskBound(content, task_bounds);
    }else{
        string content;
        FileUtil::LoadFile("/home/ubuntu/orgdata/task/110000.json",content);
        CTaskBoundService taskBoundService("");
        return taskBoundService.ParseTaskBound(content, task_bounds);
    }
    LOG(ERROR) << "not find task bound info!";
    return false;
}
int dataCheck(const AdjustTaskInfo task_info, const shared_ptr<CheckErrorOutput> &errorOutput) {
    int ret = 0;
    //加载任务框数据
    map<string, shared_ptr<kd::api::TaskBound>> task_bounds;
    if (!LoadTaskBound(task_info,task_bounds)) {
        LOG(ERROR) << "LoadTaskBound error.";
        return 16;
    }

    //交换格式逻辑检查
    {
        shared_ptr<MapProcessManager> mapProcessManager = make_shared<MapProcessManager>("mapCheck");

        shared_ptr<JsonDataLoader> json_data_loader = make_shared<JsonDataLoader>();
        json_data_loader->setTaskBound(task_bounds);
        auto taskFrameId = task_info.param_results.find("taskFrameId");
        if(taskFrameId != task_info.param_results.end()){
            json_data_loader->setMainFrameId(taskFrameId->second);
        }
        json_data_loader->setMainTaskId(task_info.task_id_);
        json_data_loader->setSplitCheck(true);
        mapProcessManager->registerProcessor(json_data_loader);

        //车道线属性检查
        shared_ptr<DividerSplitCheck> divSplitCheck = make_shared<DividerSplitCheck>();
        divSplitCheck->set_task_id(stol(task_info.task_id_));
        mapProcessManager->registerProcessor(divSplitCheck);

        //执行已注册检查项
        shared_ptr<MapDataManager> mapDataManager = make_shared<MapDataManager>();
        if (!mapProcessManager->execute(mapDataManager, errorOutput)){
            LOG(ERROR) << "mapProcessManager execute error!";
            ret = 1;
        }
    }

    return ret;
}

void InitGlog(const string &exe_path, const string &ur_path) {
    Poco::File outDir(ur_path);
    if (!outDir.exists()) {
        outDir.createDirectories();
    }
    google::InitGoogleLogging(exe_path.c_str());
    google::LogToStderr();
    string log_path = ur_path + "data_check";
    google::SetLogDestination(0, log_path.c_str());
    google::SetLogFilenameExtension(".log");
}

/**
 * UR6位截取
 * @param ur_path
 * @return
 */
string getUpdateRegion(string ur_path) {
    if (ur_path.length() > 4) {
        return ur_path.substr(0, 4);
    }
    return ur_path;
}

std::string GetConfigProperty(const std::string& key) {
    return DataCheckConfig::getInstance().getProperty(key);
}

int forAllCheck(int argc, const char *argv[]){

    // app返回值
    int ret = 0;

    TimerUtil compilerTimer;

    string exe_path;
    string base_path;

    KDSDivider::FLAG;
    string errJsonPath ="";
    try {
        exe_path = argv[0];

        //加载任务信息
        AdjustTaskInfo task_info;
        {
            std::string config_file(argv[1]);
            Poco::File file(config_file);
            if(!file.exists()){
                return 2;
            }

            string file_content;
            if(FileUtil::LoadFile(config_file, file_content)){
                AdjustTaskInfoLoader loader;
                if(!loader.Load(file_content, task_info)) {
                    return 2;
                }
            }
        }
        InitGlog(exe_path, task_info.logs_path_);

        // 加载配置
        ret = DataCheckConfig::getInstance().load("config.properties");
        if (ret != 0) {
            LOG(ERROR) << "读取配置文件config.properties失败,程序退出!";
            return ret;
        }
        DataCheckConfig::getInstance().setProperty(DataCheckConfig::OUTPUT_PATH,task_info.output_path_);
        DataCheckConfig::getInstance().setProperty(DataCheckConfig::JSON_DATA_INPUT,task_info.input_path_);
        DataCheckConfig::getInstance().setProperty(DataCheckConfig::ERR_JSON_PATH,task_info.input_path_+checkresultforjson);

        //检查项
        auto checkItems = task_info.param_results.find("checkItemConfig");
        if(checkItems==task_info.param_results.end()){
            LOG(ERROR) << "read checkItemConfig param error!!";
            return 3;
        }
        CheckListConfig::getInstance().ParsseItemDesc(checkItems->second);

        errJsonPath = task_info.input_path_+checkresultforjson;
        Poco::File error_file(errJsonPath);
        if (error_file.exists()) {
            error_file.remove();
        }

        int check_state = DataCheckConfig::getInstance().getPropertyI(DataCheckConfig::CHECK_STATE);

        auto error_output = make_shared<CheckErrorOutput>(check_state);

        //数据质量检查
        ret |= dataCheck(task_info, error_output);

        ret |= error_output->saveJsonError(checkresultforjson);

//        ret |= error_output->saveErrorReport(checkresult);

        LOG(INFO) << "total task costs: " << compilerTimer.elapsed_message();

    } catch (std::exception &e) {
        LOG(ERROR) << "An exception occurred: " << e.what();
        ReportJsonLog::GetInstance().WriteToFile(errJsonPath,true);
        ret = 1;
    }

    google::ShutdownGoogleLogging();

    return ret;
}
/**
 * 数据下载示例
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char *argv[]) {
    if(argc>1){
        //全库检查
        return forAllCheck(argc,argv);
    }else{
        //OSM检查
        return 1;
    }
}