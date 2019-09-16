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
#include "process/ModelSqlCheck.h"

#include "MapProcessManager.h"
#include "businesscheck/MapDataLoader.h"
#include "businesscheck/DividerAttribCheck.h"
#include "businesscheck/DividerShapeNormCheck.h"
#include "businesscheck/DividerShapeDefectCheck.h"
#include "businesscheck/DividerTopoCheck.h"
#include "businesscheck/LaneGroupCheck.h"
#include "businesscheck/LaneAttribCheck.h"
#include "businesscheck/LaneShapeNormCheck.h"
#include "businesscheck/DividerCheck.h"
#include "businesscheck/LaneTopoCheck.h"
#include "businesscheck/LaneGroupRelationCheck.h"
#include "businesscheck/LaneGroupTopoCheck.h"
#include "businesscheck/RoadCheck.h"
#include "businesscheck/LaneCheck.h"
#include "businesscheck/AdasCheck.h"
#include "businesscheck/JsonDataLoader.h"
#include "datacheck/LengthCheck.h"
#include "datacheck/CountCheck.h"
#include "util/TimerUtil.h"
#include "util/check_list_config.h"
#include "data/ResourceDataManager.h"
#include "datacheck/TableDescCheck.h"
#include "datacheck/ForeignKeyCheck.h"
#include "datacheck/SlopeCheck.h"
using namespace kd::dc;

const char kCheckListFile[] = "check_list.json";
const char checkresult[] = "checkresult.json";
shared_ptr<ResourceDataManager> ResourceDataManager::instance_ = nullptr;

int TopoAutoCheck(const shared_ptr<CheckErrorOutput> &errorOutput, int check_state) {
    int ret = 0;

    // 拓扑自动化检查项
    shared_ptr<MapProcessManager> mapProcessManager = make_shared<MapProcessManager>("topo_auto_check");

    // 加载json数据
    shared_ptr<JsonDataLoader> json_data_loader = make_shared<JsonDataLoader>();
    mapProcessManager->registerProcessor(json_data_loader);

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

    //车道属性检查
    shared_ptr<LaneAttribCheck> laneAttCheck = make_shared<LaneAttribCheck>();
    mapProcessManager->registerProcessor(laneAttCheck);

    shared_ptr<LaneTopoCheck> laneTopoCheck = make_shared<LaneTopoCheck>();
    mapProcessManager->registerProcessor(laneTopoCheck);

    shared_ptr<RoadCheck> road_check = make_shared<RoadCheck>();
    mapProcessManager->registerProcessor(road_check);

    shared_ptr<LaneGroupRelationCheck> lanegroup_rel_check = make_shared<LaneGroupRelationCheck>();
    mapProcessManager->registerProcessor(lanegroup_rel_check);

    shared_ptr<LaneGroupTopoCheck> lanegroup_topo_check = make_shared<LaneGroupTopoCheck>();
    mapProcessManager->registerProcessor(lanegroup_topo_check);

    //执行已注册检查项
    shared_ptr<MapDataManager> mapDataManager = make_shared<MapDataManager>();
    if (!mapProcessManager->execute(mapDataManager, errorOutput)) {
        LOG(ERROR) << "mapProcessManager execute error!";
        ret = 1;
    }
    return ret;
}

int ConsistencyCheck(const shared_ptr<CheckErrorOutput> &errorOutput)
{
    int ret = 0;
    shared_ptr<MapProcessManager> dataCheckManager = make_shared<MapProcessManager>("dataCheck");

    shared_ptr<ResourceDataManager> resourceDataManager = ResourceDataManager::GetInstance();
    dataCheckManager->registerProcessor(resourceDataManager);

    shared_ptr<LengthCheck> lengthCheck = make_shared<LengthCheck>();
    dataCheckManager->registerProcessor(lengthCheck);

    shared_ptr<CountCheck> countCheck = make_shared<CountCheck>();
    dataCheckManager->registerProcessor(countCheck);

    shared_ptr<MapDataManager> mapDataManager = make_shared<MapDataManager>();
    if (!dataCheckManager->execute(mapDataManager, errorOutput)){
        LOG(ERROR) << "dataCheckManager execute error!";
        ret = 1;
    }
    return ret;
}

int AllAutoCheck(const shared_ptr<CheckErrorOutput> &errorOutput, const string& base_path) {
    int ret = 0;
    // KXF全要素检查
    shared_ptr<ModelProcessManager> model_process_manager = make_shared<ModelProcessManager>("all_auto_field_check");

    //加载数据
    shared_ptr<ModelDataLoader> modelLoader = make_shared<ModelDataLoader>(base_path);
    model_process_manager->registerProcessor(modelLoader);

    //属性字段检查
    shared_ptr<ModelFieldCheck> modelFiledCheck = make_shared<ModelFieldCheck>();
    model_process_manager->registerProcessor(modelFiledCheck);

    //kxf規格检查-表描述检查
    shared_ptr<TableDescCheck> tableDescCheck = make_shared<TableDescCheck>();
    model_process_manager->registerProcessor(tableDescCheck);

    //外键及其外键完备性检查
    shared_ptr<ForeignKeyCheck> foreignKeyCheck = make_shared<ForeignKeyCheck>();
    model_process_manager->registerProcessor(foreignKeyCheck);

    //执行已注册检查项
    shared_ptr<ModelDataManager> modelDataManager = make_shared<ModelDataManager>();
    if (!model_process_manager->execute(modelDataManager, errorOutput)) {
        LOG(ERROR) << "ModelProcessManager execute error!";
        ret = 1;
    }

    shared_ptr<MapProcessManager> map_process_manager = make_shared<MapProcessManager>("all_auto_check");


    //加载数据
    shared_ptr<MapDataLoader> loader = make_shared<MapDataLoader>(base_path);
    map_process_manager->registerProcessor(loader);

    //车道线属性检查
    shared_ptr<DividerAttribCheck> divAttCheck = make_shared<DividerAttribCheck>();
    map_process_manager->registerProcessor(divAttCheck);

    //车道线几何形态检查
    shared_ptr<DividerShapeNormCheck> divShpNormCheck = make_shared<DividerShapeNormCheck>();
    map_process_manager->registerProcessor(divShpNormCheck);

    //车道线形状缺陷检查
    shared_ptr<DividerShapeDefectCheck> divShpDefCheck = make_shared<DividerShapeDefectCheck>();
    map_process_manager->registerProcessor(divShpDefCheck);

//        //车道线拓扑检查
        shared_ptr<DividerTopoCheck> divTopoCheck = make_shared<DividerTopoCheck>();
        map_process_manager->registerProcessor(divTopoCheck);

    //车道属性检查
    shared_ptr<LaneAttribCheck> laneAttCheck = make_shared<LaneAttribCheck>();
    map_process_manager->registerProcessor(laneAttCheck);

//        //车道几何形状检查
//        shared_ptr<LaneShapeNormCheck> laneShpCheck = make_shared<LaneShapeNormCheck>();
//        map_process_manager->registerProcessor(laneShpCheck);

    //车道拓扑检查
    shared_ptr<LaneTopoCheck> laneTopoCheck = make_shared<LaneTopoCheck>();
    map_process_manager->registerProcessor(laneTopoCheck);

    shared_ptr<LaneCheck> lane_check = make_shared<LaneCheck>();
    map_process_manager->registerProcessor(lane_check);

    shared_ptr<RoadCheck> road_check = make_shared<RoadCheck>();
    map_process_manager->registerProcessor(road_check);

    shared_ptr<LaneGroupCheck> lanegroup_check = make_shared<LaneGroupCheck>();
    map_process_manager->registerProcessor(lanegroup_check);

    shared_ptr<LaneGroupTopoCheck> lanegroup_topo_check = make_shared<LaneGroupTopoCheck>();
    map_process_manager->registerProcessor(lanegroup_topo_check);

    shared_ptr<AdasCheck> adas_check = make_shared<AdasCheck>(base_path);
    map_process_manager->registerProcessor(adas_check);

    // 坡度检查
    shared_ptr<SlopeCheck> slopeCheck = make_shared<SlopeCheck>();
    map_process_manager->registerProcessor(slopeCheck);

    // divider检查
    shared_ptr<DividerCheck> dividerCheck = make_shared<DividerCheck>();
    map_process_manager->registerProcessor(dividerCheck);

    //执行已注册检查项
    shared_ptr<MapDataManager> mapDataManager = make_shared<MapDataManager>();
    if (!map_process_manager->execute(mapDataManager, errorOutput)) {
        LOG(ERROR) << "MapProcessManager execute error!";
        ret = 1;
    }

    return ret;
}

int SqlAutoCheck(const shared_ptr<CheckErrorOutput> &errorOutput) {
    int ret = 0;
    shared_ptr<ProcessManager> process_manager = make_shared<ProcessManager>("sql_auto_check");
    //加载数据
    shared_ptr<ModelSqlCheck> model_sql_check = make_shared<ModelSqlCheck>();
    process_manager->registerProcessor(model_sql_check);



    if (!process_manager->execute(errorOutput)){
        LOG(ERROR) << "ProcessManager execute error!";
        ret = 1;
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
    // app返回值
    int ret = 0;

    TimerUtil compilerTimer;

    string exe_path;
    string ur_path;
    string output_path;

    KDSDivider::FLAG;
    string errJsonPath = "";
    try {
        exe_path = argv[0];

        InitGlog(exe_path, "./");

        // 加载配置
        ret = DataCheckConfig::getInstance().load("config.properties");
        if (ret != 0) {
            LOG(ERROR) << "读取配置文件config.properties失败,程序退出!";
            return ret;
        }

        string output_path = DataCheckConfig::getInstance().getProperty(DataCheckConfig::OUTPUT_PATH);
        string base_path = DataCheckConfig::getInstance().getProperty(DataCheckConfig::SHP_FILE_PATH);
        string db_file_name = DataCheckConfig::getInstance().getProperty(DataCheckConfig::DB_INPUT_FILE);

        if (argc >= 4) {
            ur_path = argv[1];
            base_path = argv[2];
            db_file_name = argv[3];
            output_path = argc >= 5 ? argv[4] : ".";
            output_path = output_path + "/" + ur_path;
            base_path = base_path + "/" + ur_path;

            DataCheckConfig::getInstance().setProperty(DataCheckConfig::DB_INPUT_FILE, db_file_name);
            DataCheckConfig::getInstance().setProperty(DataCheckConfig::CHECK_STATE, to_string(DataCheckConfig::ALL_AUTO_CHECK));
            DataCheckConfig::getInstance().addProperty(DataCheckConfig::UPDATE_REGION, getUpdateRegion(ur_path));
        }

        // 检查项配置管理初始化 本地调试使用
//        if(!CheckListConfig::getInstance().GetCheckList(argv[1],argv[2])){
//            LOG(ERROR) << "download and parse checklist error!";
//            return 1;
//        }


        errJsonPath = DataCheckConfig::getInstance().getProperty(DataCheckConfig::OUTPUT_PATH) + checkresult;
        Poco::File error_file(errJsonPath);
        if (error_file.exists()) {
            error_file.remove();
        }

        int check_state = DataCheckConfig::getInstance().getPropertyI(DataCheckConfig::CHECK_STATE);

        auto error_output = make_shared<CheckErrorOutput>(check_state);

        if (check_state == DataCheckConfig::TOPO_AUTO_CHECK) {
            //拓扑自动化检查
            string checkFilePath = DataCheckConfig::getInstance().getProperty(DataCheckConfig::CHECK_FILE_PATH);
            Poco::File in_dir(checkFilePath);
            if (!in_dir.exists()) {
                LOG(ERROR) << checkFilePath << " is not exists!";
                return 1;
            } else {
                CheckListConfig::getInstance().Load(checkFilePath);
            }
            ret |= TopoAutoCheck(error_output, check_state);
            ret |= error_output->saveErrorReport(checkresult);
            ret |= error_output->saveJsonError();
        } else if (check_state == DataCheckConfig::ALL_AUTO_CHECK) {
            // 创建UR路径
            Poco::File outDir(output_path);
            if (!outDir.exists()) {
                if (!outDir.createDirectory()) {
                    LOG(ERROR) << "create ur directory failed!";
                    return 1;
                }
            }

            string output_file = output_path + "/data_check.db";
            Poco::File output(output_file);
            if (output.exists()) {
                output.remove();
            }

            // KXF全要素检查
            ret |= SqlAutoCheck(error_output);
            ret |= AllAutoCheck(error_output, base_path);
            ret |= ConsistencyCheck(error_output);
            ret |= error_output->saveErrorToDb(output_file);
        }

        LOG(INFO) << "total task costs: " << compilerTimer.elapsed_message();

    } catch (std::exception &e) {
        LOG(ERROR) << "An exception occurred: " << e.what();
        ReportJsonLog::GetInstance().WriteToFile(errJsonPath, true);
        ret = 1;
    }

    google::ShutdownGoogleLogging();

    return ret;
}