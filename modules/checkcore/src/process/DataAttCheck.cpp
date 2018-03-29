//
// Created by gaoyanhong on 2018/3/28.
//


#include "process/DataAttCheck.h"

//thirdparty
#include <Poco/StringTokenizer.h>

using namespace Poco;

//module
#include "data/DataManager.h"

#include "storage/CheckTaskInput.h"

#include "process/ModelDataLoader.h"
#include "process/ModelFieldCheck.h"
#include "process/ModelBussCheck.h"
#include "process/ModelRelationCheck.h"


namespace kd {
    namespace dc {

        DataAttCheck::DataAttCheck(string basePath, string taskName) {
            basePath_ = basePath;
            taskName_ = taskName;
        }

        string DataAttCheck::getId() {
            return id;
        }

        bool DataAttCheck::execute() {

            CheckTaskInput taskInput;

            vector<shared_ptr<DCTask>> tasks;
            string taskPath = basePath_ + "/" + taskName_;
            taskInput.loadTaskInfo(taskPath, tasks);
            cout << "[Debug] task size is " << tasks.size() << endl;

            shared_ptr<ModelDataManager> dataManager = make_shared<ModelDataManager>();

            shared_ptr<ModelFieldCheck> fieldCheck = make_shared<ModelFieldCheck>();
            shared_ptr<ModelBussCheck> bussCheck = make_shared<ModelBussCheck>();


            for (shared_ptr<DCTask> task : tasks) {

                string modelPath = basePath_ + "/" + task->modelName + ".json";
                string dataFilePath = basePath_ + "/" + task->fileName;

                shared_ptr<ModelDataLoader> dataLoader = make_shared<ModelDataLoader>(modelPath, dataFilePath,
                                                                                      task->fileType);

                shared_ptr<DCModalData> modelData = make_shared<DCModalData>();
                shared_ptr<DCModelDefine> modelDefine = make_shared<DCModelDefine>();

                //加载数据
                if (dataLoader->execute(modelData, modelDefine)) {

                    //加入缓存
                    dataManager->modelDatas_.insert(pair<string, shared_ptr<DCModalData>>(task->modelName, modelData));

                    dataManager->modelDefines_.insert(
                            pair<string, shared_ptr<DCModelDefine>>(task->modelName, modelDefine));

                    //属性检查
                    fieldCheck->execute(modelData, modelDefine);

                    bussCheck->execute(modelData, modelDefine);
                }
            }

            //关联关系检查
            shared_ptr<ModelRelationCheck> relationCheck = make_shared<ModelRelationCheck>();
            relationCheck->execute(dataManager);
            return true;
        }
    }
}
