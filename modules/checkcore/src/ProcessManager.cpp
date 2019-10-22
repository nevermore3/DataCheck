
#include "ProcessManager.h"
#include <util/TimerUtil.h>

namespace kd {
    namespace dc {

        ProcessManager::ProcessManager(string name){
            processName_ = name;
        }

        bool ProcessManager::execute(shared_ptr<CheckErrorOutput> errorOutput) {

            LOG(INFO) << "task [" << processName_ << "] start. ";
            for( auto processor : processors){
                TimerUtil compilerTimer;
                if(processor != nullptr){
                    LOG(INFO) << "processor [" << processor->getId() << "] start.";

                    bool result = processor->execute(errorOutput);

                    if(result){
                        LOG(INFO) << "processor [" << processor->getId() << "] end.";
                    }else{
                        LOG(ERROR) << "processor [" << processor->getId() << "] error.";
                        return false;
                    }
                }else{
                    LOG(ERROR) << "find one invalid processor!";
                    return false;
                }
                LOG(INFO) << processor->getId() <<  " costs : " << compilerTimer.elapsed_message();
            }

            LOG(INFO) << "task [" << processName_ << "] end successfully " << endl;

            return true;
        }

        bool ProcessManager::registerProcessor(string name, shared_ptr<IProcessor> processor) {
            if(processor == nullptr || name.length() == 0)
                return false;

            //判断是否有同名的操作项存在
            if(mapProcessors.find(name) == mapProcessors.end()){
                //添加操作
                mapProcessors.insert(pair<string, shared_ptr<IProcessor>>(name, processor));
                processors.emplace_back(processor);
            }else{
                //替换操作
                shared_ptr<IProcessor> oldProcessor = mapProcessors[name];

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

                mapProcessors[name] = processor;
            }
            return true;
        }

        bool ProcessManager::registerProcessor(shared_ptr<IProcessor> processor){
            if(nullptr == processor)
                return false;

            return registerProcessor(processor->getId(), processor);
        }


        void ProcessManager::removeProcessor(string name) {

        }

        void ProcessManager::clears(){

        }
    }
}
