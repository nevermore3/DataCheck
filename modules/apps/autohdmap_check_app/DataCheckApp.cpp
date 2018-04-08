
//third party
#include <Poco/StringTokenizer.h>
#include <process/DataAttCheck.h>
#include <DataCheckConfig.h>



//module
#include "data/DataManager.h"

#include "ProcessManager.h"

#include "storage/CheckTaskInput.h"
#include "storage/ModelDataInput.h"

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

int dataCheck(string basePath, string taskFileName){

    //交换格式基本属性检查
    shared_ptr<DataAttCheck> attCheck = make_shared<DataAttCheck>(basePath, taskFileName);
    attCheck->execute();


    //交换格式逻辑检查
    //加载配置项
    DataCheckConfig::getInstance().load("config.properties");

    shared_ptr<MapProcessManager> mapProcessManager = make_shared<MapProcessManager>("mapCheck");

    shared_ptr<MapDataLoader> loader = make_shared<MapDataLoader>(basePath + "/data");
    shared_ptr<DividerAttribCheck> divAttCheck = make_shared<DividerAttribCheck>();
    shared_ptr<DividerShapeNormCheck> divShpNormCheck = make_shared<DividerShapeNormCheck>();
    shared_ptr<DividerShapeDefectCheck> divShpDefCheck = make_shared<DividerShapeDefectCheck>();
    shared_ptr<DividerTopoCheck> divTopoCheck = make_shared<DividerTopoCheck>();
    shared_ptr<LaneAttribCheck> laneAttCheck = make_shared<LaneAttribCheck>();
    shared_ptr<LaneShapeNormCheck> laneShpCheck = make_shared<LaneShapeNormCheck>();
    shared_ptr<LaneTopoCheck> laneTopoCheck = make_shared<LaneTopoCheck>();

    mapProcessManager->registerProcessor(loader);
    mapProcessManager->registerProcessor(divAttCheck);
    mapProcessManager->registerProcessor(divShpNormCheck);
    mapProcessManager->registerProcessor(divShpDefCheck);
    mapProcessManager->registerProcessor(divTopoCheck);
    mapProcessManager->registerProcessor(laneAttCheck);
    mapProcessManager->registerProcessor(laneShpCheck);
    mapProcessManager->registerProcessor(laneTopoCheck);

    shared_ptr<MapDataManager> mapDataManager = make_shared<MapDataManager>();
    shared_ptr<CheckErrorOutput> errorOutput = make_shared<CheckErrorOutput>();

    mapProcessManager->execute(mapDataManager, errorOutput);

    return 1;
}


/**
 * 数据下载示例
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char *argv[]) {

    string base_path;
    string task_file_name;

    if (argc >= 3) {
        base_path = argv[1];
        task_file_name = argv[2];

    } else {
        cout << "usage:" << argv[0] << " <base_path> <task_file_name>" << endl;
        return -1;
    }

    //数据质量检查
    return dataCheck(base_path, task_file_name);
}