
//third party
#include <Poco/StringTokenizer.h>
#include <glog/logging.h>
#include <glog/log_severity.h>
#include <util/TimerUtil.h>
#include <Poco/File.h>

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

#include "MapProcessManager.h"
#include "businesscheck/MapDataLoader.h"
#include "businesscheck/DividerAttribCheck.h"
#include "businesscheck/DividerShapeNormCheck.h"
#include "businesscheck/DividerShapeDefectCheck.h"
#include "businesscheck/DividerTopoCheck.h"
#include "businesscheck/LaneAttribCheck.h"
#include "businesscheck/LaneShapeNormCheck.h"
#include "businesscheck/LaneTopoCheck.h"

using namespace kd::dc;

void loadTaskInfo(string fileName, string & taskName, string & baseUrl,
                  vector<string> & taskIds, vector<string> & batchs, vector<string> & trackIds){
    ifstream in(fileName);
    if (in.is_open()) {
        string tempvalue;

        if(!in.eof()) {
            getline(in, tempvalue);

            taskName = tempvalue;

            getline(in, tempvalue);
            baseUrl = tempvalue;

            getline(in, tempvalue);
            int taskCount = stoi(tempvalue);
            for( int i = 0 ; i < taskCount ; i ++ ){
                getline(in, tempvalue);

                Poco::StringTokenizer st(tempvalue, ",");
                if(st.count() == 3){
                    taskIds.emplace_back(st[0]);
                    batchs.emplace_back(st[1]);
                    trackIds.emplace_back(st[2]);
                }
            }
        }
    }
}

int dataCheck(string basePath, string taskFileName, string ur_path){

    //输出错误文件
    string outputFile = ur_path + "/check_result.csv";
    shared_ptr<CheckErrorOutput> errorOutput = make_shared<CheckErrorOutput>(outputFile);

    //交换格式基本属性检查
    {
        shared_ptr<ModelProcessManager> modelProcessManager = make_shared<ModelProcessManager>("modelCheck");

        //加载数据
        shared_ptr<ModelDataLoader> modelLoader = make_shared<ModelDataLoader>(basePath, taskFileName);
        modelProcessManager->registerProcessor(modelLoader);

        //属性字段检查
        shared_ptr<ModelFieldCheck> modelFiledCheck = make_shared<ModelFieldCheck>();
        modelProcessManager->registerProcessor(modelFiledCheck);

        //属性业务检查
        shared_ptr<ModelBussCheck> modelBussCheck = make_shared<ModelBussCheck>();
        modelProcessManager->registerProcessor(modelBussCheck);

        //属性关系检查
        shared_ptr<ModelRelationCheck> modelRelationCheck = make_shared<ModelRelationCheck>();
        modelProcessManager->registerProcessor(modelRelationCheck);

        //执行已注册检查项
        shared_ptr<ModelDataManager> modelDataManager = make_shared<ModelDataManager>();
        modelProcessManager->execute(modelDataManager, errorOutput);
    }

    //交换格式逻辑检查
    {
        DataCheckConfig::getInstance().load("config.properties");
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

        //执行已注册检查项
        shared_ptr<MapDataManager> mapDataManager = make_shared<MapDataManager>();
        mapProcessManager->execute(mapDataManager, errorOutput);
    }

    return 0;
}

void InitGlog(string ur_path)
{
    google::InitGoogleLogging("./");
    google::LogToStderr();
    string log_path = ur_path + "/data_check";
    google::SetLogDestination(0, log_path.c_str());
    google::SetLogFilenameExtension(".log");
}

/**
 * 数据下载示例
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char *argv[]) {

    TimerUtil compilerTimer;

    string base_path;
    string task_file_name;
    string ur_path;

    if (argc >= 4) {
        base_path = argv[1];
        task_file_name = argv[2];
        ur_path = argv[3];
    } else {
        LOG(ERROR) << "usage:" << argv[0] << " <base_path> <task_file_name> <ur>";
        return 1;
    }
    InitGlog(ur_path);

    Poco::File outDir(ur_path);
    outDir.createDirectories();

    //数据质量检查
    int ret = dataCheck(base_path, task_file_name, ur_path);

    LOG(INFO) << "total task costs: " << compilerTimer.elapsed_message();

    return ret;
}