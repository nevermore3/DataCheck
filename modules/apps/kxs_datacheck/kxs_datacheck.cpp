
//third party

#include "util/TimerUtil.h"

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
#include "process/ModelSqlCheck.h"

#include "MapProcessManager.h"
#include "businesscheck/MapDataLoader.h"
#include "businesscheck/DividerAttribCheck.h"
#include "businesscheck/DividerShapeNormCheck.h"
#include "businesscheck/DividerShapeDefectCheck.h"
#include "businesscheck/DividerTopoCheck.h"
#include "businesscheck/LaneAttribCheck.h"
#include "businesscheck/LaneShapeNormCheck.h"
#include "businesscheck/LaneTopoCheck.h"
#include "businesscheck/LaneGroupCheck.h"
#include "businesscheck/LaneGroupTopoCheck.h"
#include "businesscheck/RoadCheck.h"
#include "businesscheck/LaneCheck.h"
#include "businesscheck/AdasCheck.h"

using namespace kd::dc;

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
        shared_ptr<ModelBussCheck> modelBussCheck = make_shared<ModelBussCheck>();
        modelProcessManager->registerProcessor(modelBussCheck);

        //属性关系检查
//        shared_ptr<ModelRelationCheck> modelRelationCheck = make_shared<ModelRelationCheck>();
//        modelProcessManager->registerProcessor(modelRelationCheck);

        //执行已注册检查项
        shared_ptr<ModelDataManager> modelDataManager = make_shared<ModelDataManager>();
        if (!modelProcessManager->execute(modelDataManager, errorOutput)) {
            LOG(ERROR) << "modelDataManager execute error!";
            ret = 1;
        }
    }

    //交换格式逻辑检查
    {
        shared_ptr<MapProcessManager> mapProcessManager = make_shared<MapProcessManager>("mapCheck");

        //加载数据
        shared_ptr<MapDataLoader> loader = make_shared<MapDataLoader>(basePath);
        mapProcessManager->registerProcessor(loader);

        //车道线属性检查
        shared_ptr<DividerAttribCheck> divAttCheck = make_shared<DividerAttribCheck>();
        mapProcessManager->registerProcessor(divAttCheck);

        //车道线几何形态检查
        shared_ptr<DividerShapeNormCheck> divShpNormCheck = make_shared<DividerShapeNormCheck>();
        mapProcessManager->registerProcessor(divShpNormCheck);

        //车道线形状缺陷检查
        shared_ptr<DividerShapeDefectCheck> divShpDefCheck = make_shared<DividerShapeDefectCheck>();
        mapProcessManager->registerProcessor(divShpDefCheck);

        //车道线拓扑检查
        shared_ptr<DividerTopoCheck> divTopoCheck = make_shared<DividerTopoCheck>();
        mapProcessManager->registerProcessor(divTopoCheck);

        //车道属性检查
        shared_ptr<LaneAttribCheck> laneAttCheck = make_shared<LaneAttribCheck>();
        mapProcessManager->registerProcessor(laneAttCheck);

        //车道几何形状检查
        shared_ptr<LaneShapeNormCheck> laneShpCheck = make_shared<LaneShapeNormCheck>();
        mapProcessManager->registerProcessor(laneShpCheck);

        //车道拓扑检查
        shared_ptr<LaneTopoCheck> laneTopoCheck = make_shared<LaneTopoCheck>();
        mapProcessManager->registerProcessor(laneTopoCheck);

        shared_ptr<LaneCheck> lane_check = make_shared<LaneCheck>();
        mapProcessManager->registerProcessor(lane_check);

        shared_ptr<RoadCheck> road_check = make_shared<RoadCheck>();
        mapProcessManager->registerProcessor(road_check);

        shared_ptr<LaneGroupCheck> lanegroup_check = make_shared<LaneGroupCheck>();
        mapProcessManager->registerProcessor(lanegroup_check);

        shared_ptr<LaneGroupTopoCheck> lanegroup_topo_check = make_shared<LaneGroupTopoCheck>();
        mapProcessManager->registerProcessor(lanegroup_topo_check);

        shared_ptr<AdasCheck> adas_check = make_shared<AdasCheck>(basePath);
        mapProcessManager->registerProcessor(adas_check);

        //执行已注册检查项
        shared_ptr<MapDataManager> mapDataManager = make_shared<MapDataManager>();
        if (!mapProcessManager->execute(mapDataManager, errorOutput)){
            LOG(ERROR) << "mapProcessManager execute error!";
            ret = 1;
        }
    }

    return ret;
}

int sql_data_check(CppSQLite3::Database *p_db, const shared_ptr<CheckErrorOutput> &errorOutput) {
    int ret = 0;
    shared_ptr<ProcessManager> process_manager = make_shared<ProcessManager>("sql_data_check");
    //加载数据
    shared_ptr<ModelSqlCheck> model_sql_check = make_shared<ModelSqlCheck>(p_db);
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
    string base_path;
    string ur_path;
    string db_file_name;
    string output_path;

    CppSQLite3::Database *p_db = nullptr;
    CppSQLite3::Database *p_db_out = nullptr;

    try {
        exe_path = argv[0];
        if (argc >= 4) {
            ur_path = argv[1];
            base_path = argv[2];
            db_file_name = argv[3];
            output_path = argc >= 5 ? argv[4] : ".";
            output_path = output_path + "/" + ur_path;
            base_path = base_path + "/" + ur_path;
        } else {
            LOG(ERROR) << "usage:" << argv[0] << " <ur> <base_path> <dump_db_file> [<output_path>]";
            return 1;
        }

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

        InitGlog(exe_path, output_path);

        // 加载配置
        ret = DataCheckConfig::getInstance().load("config.properties");
        if (ret != 0) {
            LOG(ERROR) << "读取配置文件config.properties失败,程序退出!";
            return ret;
        }

        // 添加UR
        DataCheckConfig::getInstance().addProperty(DataCheckConfig::UPDATE_REGION, getUpdateRegion(ur_path));

        // 创建数据库
        p_db = new CppSQLite3::Database();
        p_db_out = new CppSQLite3::Database();

        p_db->open(db_file_name);
        p_db_out->open(output_file);

        shared_ptr<CheckErrorOutput> errorOutput = make_shared<CheckErrorOutput>(p_db_out);

        //数据质量检查
        ret |= sql_data_check(p_db, errorOutput);
        ret |= dataCheck(base_path, errorOutput);

        ret |= errorOutput->saveError();

        LOG(INFO) << "total task costs: " << compilerTimer.elapsed_message();
    } catch (CppSQLite3::Exception &e) {
        LOG(ERROR) << "An exception occurred: " << e.errorMessage().c_str();
        ret = 1;
    } catch (std::exception &e) {
        LOG(ERROR) << "An exception occurred: " << e.what();
        ret = 1;
    }

    if (p_db) {
        p_db->close();
        delete p_db;
        p_db = nullptr;
    }

    if (p_db_out) {
        p_db_out->close();
        delete p_db_out;
        p_db_out = nullptr;
    }

    google::ShutdownGoogleLogging();

    return ret;
}