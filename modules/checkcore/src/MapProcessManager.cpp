//
// Created by gaoyanhong on 2018/4/3.
//

#include <util/TimerUtil.h>
#include "MapProcessManager.h"

namespace kd {
    namespace dc {

        MapProcessManager::MapProcessManager(string name){
            processName_ = name;
        }

        bool MapProcessManager::execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) {

            LOG(INFO) << "task [" << processName_ << "] start. ";
            for( auto processor : processors){
                TimerUtil compilerTimer;

                if(processor != nullptr){
                    LOG(INFO) << "processor [" << processor->getId() << "] start.";

                    bool result = processor->execute(mapDataManager, errorOutput);

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

        bool MapProcessManager::registerProcessor(string name, shared_ptr<IMapProcessor> processor) {
            if(processor == nullptr || name.length() == 0)
                return false;

            //判断是否有同名的操作项存在
            if(mapProcessors.find(name) == mapProcessors.end()){
                //添加操作
                mapProcessors.insert(pair<string, shared_ptr<IMapProcessor>>(name, processor));
                processors.emplace_back(processor);
            }else{
                //替换操作
                shared_ptr<IMapProcessor> oldProcessor = mapProcessors[name];

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

        bool MapProcessManager::registerProcessor(shared_ptr<IMapProcessor> processor){
            if(nullptr == processor)
                return false;

            return registerProcessor(processor->getId(), processor);
        }


        void MapProcessManager::removeProcessor(string name) {

        }

        void MapProcessManager::clears(){

        }
    }
}
