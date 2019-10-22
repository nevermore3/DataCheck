
#ifndef AUTOHDMAP_DATACHECK_MODELPROCESSMANAGER_H
#define AUTOHDMAP_DATACHECK_MODELPROCESSMANAGER_H

#include <data/DataManager.h>
#include "IModelProcessor.h"

namespace kd {
   namespace dc {

        class ModelProcessManager{
        public:

            ModelProcessManager(string name);

            /**
             * 注册操作项
             * @param name 操作项实例，有时对于同一操作会多个实例
             * @param processor
             * @return 注册是否成功
             */
            bool registerProcessor(string name, shared_ptr<IModelProcessor> processor);

            /**
             * 注册操作项
             * @param processor 操作项实例
             * @return 注册是否成功
             */
            bool registerProcessor(shared_ptr<IModelProcessor> processor);


            /**
             * 注销操作项
             * @param name
             */
            void removeProcessor(string name);

            /**
             * 清楚所有操作项
             */
            void clears();

            /**
             * 进行任务处理
             * @param dataManager 操作数据
             * @param errorOutput 错误输出
             * @return 操作是否成功
             */
            virtual bool execute(shared_ptr<ModelDataManager> dataManager, shared_ptr<CheckErrorOutput> errorOutput);


        private:

            //所有需要进行的操作项
            vector<shared_ptr<IModelProcessor>> processors;

            map<string, shared_ptr<IModelProcessor>> modelProcessors;

            //总体任务名称
            string processName_;

        };

    }
}

#endif //AUTOHDMAP_DATACHECK_MODELPROCESSMANAGER_H
