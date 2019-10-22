
#include <process/TaskLoader.h>
#include <storage/CheckTaskInput.h>

#include "process/TaskLoader.h"

namespace kd {
    namespace dc {
        TaskLoader::TaskLoader(const string &basePath) : base_path_(basePath) {
            task_file_ = DataCheckConfig::getInstance().getProperty(DataCheckConfig::TASK_FILE);
            config_path_ = DataCheckConfig::getInstance().getProperty(DataCheckConfig::CONFIG_FILE_PATH);
        }

        TaskLoader::~TaskLoader() {

        }

        string TaskLoader::getId() {
            return id_;
        }

        bool TaskLoader::execute(shared_ptr<ModelDataManager> modelDataManager,
                                 shared_ptr<CheckErrorOutput> errorOutput) {
            bool ret = true;
            model_data_manager_ = modelDataManager;
            error_output_ = errorOutput;
            ret = loadTasks();
            return ret;
        }

        bool TaskLoader::loadTasks() {
            CheckTaskInput taskInput;
            string taskPath = config_path_ + task_file_;
            return taskInput.loadTaskInfo(taskPath, model_data_manager_->tasks_);
        }

    }
}
