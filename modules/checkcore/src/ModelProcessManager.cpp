//
// Created by yuanjinfa on 2018/4/12.
//

#include <util/TimerUtil.h>
#include "ModelProcessManager.h"

namespace kd {
    namespace dc {

        ModelProcessManager::ModelProcessManager(string name){
            processName_ = name;
        }

        bool ModelProcessManager::execute(shared_ptr<ModelDataManager> modelDataManager, shared_ptr<CheckErrorOutput> errorOutput) {

            LOG(INFO) << "task [" << processName_ << "] start. ";
            for( auto processor : processors){
                TimerUtil compilerTimer;

                if(processor != nullptr){
                    LOG(INFO) << "processor [" << processor->getId() << "] start.";

                    bool result = processor->execute(modelDataManager, errorOutput);

                    if(result){
                        LOG(INFO) << "processor [" << processor->getId() << "] end.";
                    }else{
                        LOG(ERROR) << "processor [" << processor->getId() << "] error.";
                    }
                }else{
                    LOG(ERROR) << "find one invalid processor!";
                    return false;
                }

                LOG(INFO) << processor->getId() <<  " costs : " << compilerTimer.elapsed_message();
            }

            LOG(INFO) << "task [" << processName_ << "] end successfully ";

            return true;
        }

        bool ModelProcessManager::registerProcessor(string name, shared_ptr<IModelProcessor> processor) {
            if(processor == nullptr || name.length() == 0)
                return false;

            //判断是否有同名的操作项存在
            if(modelProcessors.find(name) == modelProcessors.end()){
                //添加操作
                modelProcessors.insert(pair<string, shared_ptr<IModelProcessor>>(name, processor));
                processors.emplace_back(processor);
            }else{
                //替换操作
                shared_ptr<IModelProcessor> oldProcessor = modelProcessors[name];

                bool findAndReplace = false;
                for( int i = 0 ; i < processors.size() ; i ++){
                    if(processors[i] == oldProcessor){
                        processors[i] = processor;
                        findAndReplace = true;
                        break;
                    }
                }
                if(!findAndReplace)
                    return false;

                modelProcessors[name] = processor;
            }
            return true;
        }

        bool ModelProcessManager::registerProcessor(shared_ptr<IModelProcessor> processor){
            if(nullptr == processor)
                return false;

            return registerProcessor(processor->getId(), processor);
        }


        void ModelProcessManager::removeProcessor(string name) {

        }

        void ModelProcessManager::clears(){

        }
    }
}
