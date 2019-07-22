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
#include "businesscheck/DividerAttribCheck.h"
#include "businesscheck/DividerShapeNormCheck.h"
#include "businesscheck/DividerShapeDefectCheck.h"
#include "businesscheck/DividerTopoCheck.h"
#include "businesscheck/LaneGroupCheck.h"
#include "businesscheck/LaneAttribCheck.h"
#include "businesscheck/LaneShapeNormCheck.h"
#include "businesscheck/LaneTopoCheck.h"
#include "businesscheck/LaneGroupRelationCheck.h"
#include "businesscheck/LaneGroupTopoCheck.h"
#include "businesscheck/RoadCheck.h"
#include "businesscheck/LaneCheck.h"
#include "businesscheck/AdasCheck.h"
#include "businesscheck/JsonDataLoader.h"

#include "util/TimerUtil.h"
#include "util/check_list_config.h"

using namespace kd::dc;

const char kCheckListFile[] = "check_list.json";
const char checkresult[] = "checkresult.json";
int dataCheck(string basePath, const shared_ptr<CheckErrorOutput> &errorOutput) {
    int ret = 0;
    //交换格式基本属性检查
    {
        shared_ptr<ModelProcessManager> modelProcessManager = make_shared<ModelProcessManager>("modelCheck");

        //加载数据
        shared_ptr<ModelDataLoader> modelLoader = make_shared<ModelDataLoader>(basePath);
        modelProcessManager->registerProcessor(modelLoader);

        //属性字段检查
        shared_ptr<ModelFieldCheck> modelFiledCheck = make_shared<ModelFieldCheck>();
        modelProcessManager->registerProcessor(modelFiledCheck);

        //属性业务检查
//        shared_ptr<ModelBussCheck> modelBussCheck = make_shared<ModelBussCheck>();
//        modelProcessManager->registerProcessor(modelBussCheck);

        //属性关系检查
//        shared_ptr<ModelRelationCheck> modelRelationCheck = make_shared<ModelRelationCheck>();
//        modelProcessManager->registerProcessor(modelRelationCheck);

        //执行已注册检查项
//        shared_ptr<ModelDataManager> modelDataManager = make_shared<ModelDataManager>();
//        if (!modelProcessManager->execute(modelDataManager, errorOutput)) {
//            LOG(ERROR) << "modelDataManager execute error!";
//            ret = 1;
//        }
    }

    //交换格式逻辑检查
    {
        shared_ptr<MapProcessManager> mapProcessManager = make_shared<MapProcessManager>("mapCheck");

        shared_ptr<JsonDataLoader> json_data_loader = make_shared<JsonDataLoader>();
        mapProcessManager->registerProcessor(json_data_loader);



        //加载数据
//        shared_ptr<MapDataLoader> loader = make_shared<MapDataLoader>(basePath);
//        mapProcessManager->registerProcessor(loader);

        //车道线属性检查
        shared_ptr<DividerAttribCheck> divAttCheck = make_shared<DividerAttribCheck>();
        mapProcessManager->registerProcessor(divAttCheck);

        //车道线几何形态检查
        shared_ptr<DividerShapeNormCheck> divShpNormCheck = make_shared<DividerShapeNormCheck>();
        mapProcessManager->registerProcessor(divShpNormCheck);

        //车道线形状缺陷检查
        shared_ptr<DividerShapeDefectCheck> divShpDefCheck = make_shared<DividerShapeDefectCheck>();
        mapProcessManager->registerProcessor(divShpDefCheck);

        shared_ptr<LaneGroupCheck> lane_group_check = make_shared<LaneGroupCheck>();
        mapProcessManager->registerProcessor(lane_group_check);

//        //车道线拓扑检查
//        shared_ptr<DividerTopoCheck> divTopoCheck = make_shared<DividerTopoCheck>();
//        mapProcessManager->registerProcessor(divTopoCheck);
//
//        //车道属性检查
//        shared_ptr<LaneAttribCheck> laneAttCheck = make_shared<LaneAttribCheck>();
//        mapProcessManager->registerProcessor(laneAttCheck);
//
//        //车道几何形状检查
//        shared_ptr<LaneShapeNormCheck> laneShpCheck = make_shared<LaneShapeNormCheck>();
//        mapProcessManager->registerProcessor(laneShpCheck);
//
//        //车道拓扑检查
//        shared_ptr<LaneTopoCheck> laneTopoCheck = make_shared<LaneTopoCheck>();
//        mapProcessManager->registerProcessor(laneTopoCheck);
//
//        shared_ptr<LaneCheck> lane_check = make_shared<LaneCheck>();
//        mapProcessManager->registerProcessor(lane_check);
//
//        shared_ptr<RoadCheck> road_check = make_shared<RoadCheck>();
//        mapProcessManager->registerProcessor(road_check);
//
//        shared_ptr<LaneGroupRelationCheck> lanegroup_rel_check = make_shared<LaneGroupRelationCheck>();
//        mapProcessManager->registerProcessor(lanegroup_rel_check);
//
//        shared_ptr<LaneGroupTopoCheck> lanegroup_topo_check = make_shared<LaneGroupTopoCheck>();
//        mapProcessManager->registerProcessor(lanegroup_topo_check);
//
//        shared_ptr<AdasCheck> adas_check = make_shared<AdasCheck>(basePath);
//        mapProcessManager->registerProcessor(adas_check);

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
    google::InitGoogleLogging(exe_path.c_str());
    google::LogToStderr();
    string log_path = ur_path + "/data_check";
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

/**
 * 数据下载示例
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char *argv[]) {

    if (argc < 2) {
        LOG(ERROR) << " not has url parameter";
        return -1;
    }

    // app返回值
    int ret = 0;

    TimerUtil compilerTimer;

    string exe_path;
    string base_path;

    KDSDivider::FLAG;
    string errJsonPath ="";
    try {
        exe_path = argv[0];

        InitGlog(exe_path, "./");

        // 加载配置
        ret = DataCheckConfig::getInstance().load("config.properties");
        if (ret != 0) {
            LOG(ERROR) << "读取配置文件config.properties失败,程序退出!";
            return ret;
        }

        // 检查项配置管理初始化 本地调试使用
        if(!CheckListConfig::getInstance().GetCheckList(argv[1])){
            LOG(ERROR) << "download and parse checklist error!";
            return 1;
        }

//        std::string check_file = (std::string)"./" + kCheckListFile;
//        Poco::File in_dir(check_file);
//        if (!in_dir.exists()) {
//            LOG(ERROR) << check_file << " is not exists!";
//            return 1;
//        } else {
//            CheckListConfig::getInstance().Load(check_file);
//        }

        errJsonPath = DataCheckConfig::getInstance().getProperty(DataCheckConfig::OUTPUT_PATH)+checkresult;
        Poco::File error_file(errJsonPath);
        if (error_file.exists()) {
            error_file.remove();
        }

        auto error_output = make_shared<CheckErrorOutput>();

        //数据质量检查
        ret |= dataCheck(base_path, error_output);

//        ret |= error_output->saveJsonError();

        ret |= error_output->saveErrorReport(errJsonPath);

        LOG(INFO) << "total task costs: " << compilerTimer.elapsed_message();

    } catch (std::exception &e) {
        LOG(ERROR) << "An exception occurred: " << e.what();
        ReportJsonLog::GetInstance().WriteToFile(errJsonPath,true);
        ret = 1;
    }

    google::ShutdownGoogleLogging();

    return ret;
}