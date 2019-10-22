

#ifndef AUTOHDMAP_DATACHECK_TASKLOADER_H
#define AUTOHDMAP_DATACHECK_TASKLOADER_H

#include "IModelProcessor.h"
namespace kd {
    namespace dc {
        class TaskLoader : public IModelProcessor {
        public:
            TaskLoader(const string& basePath);

            ~TaskLoader() override;

            string getId() override;

            bool execute(shared_ptr<ModelDataManager> modelDataManager, shared_ptr<CheckErrorOutput> errorOutput) override;

            //加载任务列表
            bool loadTasks();


        private:
            shared_ptr<ModelDataManager> model_data_manager_;
            shared_ptr<CheckErrorOutput> error_output_;
            string id_ = "task_loader";
            //基础数据路径
            string base_path_;

            //基础配置文件路径
            string config_path_;

            //任务名称
            string task_file_;
        };
    }
}

#endif //AUTOHDMAP_DATACHECK_TASKLOADER_H
