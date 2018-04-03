//
// Created by gaoyanhong on 2018/4/3.
//

#include "MapProcessManager.h"

namespace kd {
    namespace dc {

        MapProcessManager::MapProcessManager(string name){
            processName_ = name;
        }

        bool MapProcessManager::execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) {

            cout << "[Debug] task [" << processName_ << "] start. " << endl;
            for( auto processor : processors){
                if(processor != nullptr){
                    cout << "[Debug] processor [" << processor->getId() << "] start." << endl;

                    bool result = processor->execute(mapDataManager, errorOutput);

                    if(result){
                        cout << "[Debug] processor [" << processor->getId() << "] end." << endl;
                    }else{
                        cout << "[Debug] processor [" << processor->getId() << "] error." << endl;
                        return false;
                    }
                }else{
                    cout << "[Error] find one invalid processor!" << endl;
                    return false;
                }
            }

            cout << "[Debug] task [" << processName_ << "] end successfully " << endl;

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
