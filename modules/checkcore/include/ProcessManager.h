#ifndef AUTOHDMAP_ROADFUSION_PROCESSMANAGER_H
#define AUTOHDMAP_ROADFUSION_PROCESSMANAGER_H

#include "IProcessor.h"

namespace kd {
   namespace dc {

        class ProcessManager{
        public:

            ProcessManager(string name);

            /**
             * 注册操作项
             * @param name 操作项实例，有时对于同一操作会多个实例
             * @param processor
             * @return 注册是否成功
             */
            bool registerProcessor(string name, shared_ptr<IProcessor> processor);

            /**
             * 注册操作项
             * @param processor 操作项实例
             * @return 注册是否成功
             */
            bool registerProcessor(shared_ptr<IProcessor> processor);


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
             * @param listener 操作监听项
             * @return 操作是否成功
             */
            virtual bool execute(shared_ptr<CheckErrorOutput> errorOutput);


        private:

            //所有需要进行的操作项
            vector<shared_ptr<IProcessor>> processors;

            map<string, shared_ptr<IProcessor>> mapProcessors;

            //总体任务名称
            string processName_;

        };

    }
}

#endif //AUTOHDMAP_ROADFUSION_PROCESSMANAGER_H
