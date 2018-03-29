
//third party
#include <Poco/StringTokenizer.h>
#include <process/DataAttCheck.h>



//module
#include "data/DataManager.h"

#include "ProcessManager.h"

#include "storage/CheckTaskInput.h"
#include "storage/ModelDataInput.h"

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

    shared_ptr<DataAttCheck> attCheck = make_shared<DataAttCheck>(basePath, taskFileName);
    attCheck->execute();



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