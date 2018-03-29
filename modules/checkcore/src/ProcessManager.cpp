//
// Created by gaoyanhong on 2018/3/14.
//

#include "ProcessManager.h"

namespace kd {
    namespace dc {

        ProcessManager::ProcessManager(string name){
            processName_ = name;
        }

        bool ProcessManager::execute() {

            cout << "[Debug] task [" << processName_ << "] start. " << endl;
            for( auto processor : processors){
                if(processor != nullptr){
                    cout << "[Debug] processor [" << processor->getId() << "] start." << endl;

                    bool result = processor->execute();

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
