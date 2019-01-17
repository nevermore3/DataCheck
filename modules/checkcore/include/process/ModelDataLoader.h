//
// Created by gaoyanhong on 2018/3/29.
//

#ifndef AUTOHDMAP_DATACHECK_MODELDATALOADER_H
#define AUTOHDMAP_DATACHECK_MODELDATALOADER_H

#include <data/DataManager.h>
#include "IModelProcessor.h"

namespace kd {
    namespace dc {

        class ModelDataLoader : public IModelProcessor {
        public:
            ModelDataLoader(const string& basePath);

        public:
            /**
             * 获得对象唯一标识
             * @return 对象标识
             */
            virtual string getId() override ;

            /**
             * 进行任务处理
             * @param modelData 模型数据
             * @param modelDefine 模型定义
             * @return 操作是否成功
             */
            virtual bool execute(shared_ptr<ModelDataManager> modelDataManager, shared_ptr<CheckErrorOutput> errorOutput) override ;

        private:
            //加载任务列表
            bool loadTasks(map<string, shared_ptr<DCTask>>& tasks);

            //加载配置文件及地图数据
            bool loadData(const string& taskName, const map<string, shared_ptr<DCTask>>& tasks,
                          shared_ptr<ModelDataManager> dataManager, shared_ptr<CheckErrorOutput> errorOutput);

        private:
            const string id = "model_data_loader";

            //基础数据路径
            string basePath_;

            //基础配置文件路径
            string configPath_;

            //任务名称
            string taskFile_;

        };
    }
}
#endif //AUTOHDMAP_DATACHECK_MODELDATALOADER_H
