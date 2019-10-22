#include <utility>
#include <DataCheckConfig.h>
#include "process/ModelDataLoader.h"


#include "storage/CheckTaskInput.h"
#include "storage/ModelDataInput.h"

namespace kd {
    namespace dc {

        ModelDataLoader::ModelDataLoader(const string &basePath) : basePath_(basePath) {
            taskFile_ = DataCheckConfig::getInstance().getProperty(DataCheckConfig::TASK_FILE);
            configPath_ = DataCheckConfig::getInstance().getProperty(DataCheckConfig::CONFIG_FILE_PATH);
        }

        string ModelDataLoader::getId() {
            return id;
        }


        //加载任务列表
        bool ModelDataLoader::loadTasks(map<string, shared_ptr<DCTask>>& tasks){
            CheckTaskInput taskInput;
            string taskPath = configPath_ + taskFile_;
            return taskInput.loadTaskInfo(taskPath, tasks);
        }


        //加载配置文件及地图数据
        bool ModelDataLoader::loadData(const string& taskName, const map<string, shared_ptr<DCTask>>& tasks,
                      shared_ptr<ModelDataManager> dataManager, shared_ptr<CheckErrorOutput> errorOutput){
            bool bLoad = true;
            auto itTask = tasks.find(taskName);
            if (itTask == tasks.end()){
                errorOutput->writeInfo("[ERROR] can't find task: " + taskName);
                return false;
            }

            shared_ptr<DCTask> task = itTask->second;
            string modelPath = task->modelName + ".json";
            string dataPath = basePath_ + "/" + task->fileName;
            shared_ptr<DCModalData> modelData = make_shared<DCModalData>();
            shared_ptr<DCModelDefine> modelDefine = make_shared<DCModelDefine>();

            CheckTaskInput taskInput;
            if (taskInput.loadTaskModel(modelPath, modelDefine)) {
                ModelDataInput dataInput;
                if (task->fileType == "point") {
                    if (!dataInput.loadPointFile(dataPath, modelDefine->vecFieldDefines, modelData, errorOutput)) {
                        bLoad = false;
                        shared_ptr<DCFieldError> error = DCFieldError::createByKXS_01_020(taskName);
                        if (error->check_file(taskName)) {
                            errorOutput->saveError(error);
                        }
                    }
                } else if (task->fileType == "arc") {
                    if (!dataInput.loadArcFile(dataPath, modelDefine->vecFieldDefines, modelData, errorOutput)) {
                        bLoad = false;
                        shared_ptr<DCFieldError> error = DCFieldError::createByKXS_01_020(taskName);
                        if (error->check_file(taskName)) {
                            errorOutput->saveError(error);
                        }
                    }
                } else if (task->fileType == "dbf") {
                    if (!dataInput.loadDBFFile(dataPath, modelDefine->vecFieldDefines, modelData, errorOutput)) {
                        bLoad = false;
                        shared_ptr<DCFieldError> error = DCFieldError::createByKXS_01_020(taskName);
                        if (error->check_file(taskName)) {
                            errorOutput->saveError(error);
                        }
                    }
                } else if (task->fileType == "polygon") {
                    if (!dataInput.loadPolygonFile(dataPath, modelDefine->vecFieldDefines, modelData, errorOutput)) {
                        bLoad = false;
                        shared_ptr<DCFieldError> error = DCFieldError::createByKXS_01_020(taskName);
                        if (error->check_file(taskName)) {
                            errorOutput->saveError(error);
                        }
                    }
                } else {
                    stringstream ss;
                    ss << "[Error] data file type error " << task->fileType;
                    errorOutput->writeInfo(ss.str());
                    return false;
                }
            } else {
                errorOutput->writeInfo("[Error] model config file load fail. " + modelPath);
                return false;
            }
            string shpFile = dataPath + ".shp";
            string dbfFile = dataPath + ".dbf";
            if (access(shpFile.c_str(), F_OK) == 0 || access(dbfFile.c_str(), F_OK) == 0) {
                dataManager->modelDatas_.insert(pair<string, shared_ptr<DCModalData>>(task->name, modelData));
            }
            dataManager->modelDefines_.insert(pair<string, shared_ptr<DCModelDefine>>(task->name, modelDefine));

            for (const auto &rl : modelDefine->vecRelation){
                string modelname = rl->member;
                string rule = rl->rule;

                if (dataManager->modelDatas_.find(modelname) == dataManager->modelDatas_.end()){
                    bLoad &= loadData(modelname, tasks, dataManager, errorOutput);
                }
            }
            return bLoad;
        }


        bool ModelDataLoader::execute(shared_ptr<ModelDataManager> modelDataManager, shared_ptr<CheckErrorOutput> errorOutput) {

            if (!loadTasks(modelDataManager->tasks_))
                return false;

            for (auto taskit : modelDataManager->tasks_) {
                loadData(taskit.first, modelDataManager->tasks_, modelDataManager, errorOutput);
            }

            return true;
        }
    }
}

