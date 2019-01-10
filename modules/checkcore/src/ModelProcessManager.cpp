//
// Created by yuanjinfa on 2018/4/12.
//

#include <util/Timer.hpp>
#include "ModelProcessManager.h"

namespace kd {
    namespace dc {

        ModelProcessManager::ModelProcessManager(string name){
            processName_ = name;
        }

        bool ModelProcessManager::execute(shared_ptr<ModelDataManager> modelDataManager, shared_ptr<CheckErrorOutput> errorOutput) {

            cout << "[Debug] task [" << processName_ << "] start. " << endl;
            for( auto processor : processors){
                Timer compilerTimer;

                if(processor != nullptr){
                    cout << "[Debug] processor [" << processor->getId() << "] start." << endl;

                    bool result = processor->execute(modelDataManager, errorOutput);

                    if(result){
                        cout << "[Debug] processor [" << processor->getId() << "] end." << endl;
                    }else{
                        cout << "[Debug] processor [" << processor->getId() << "] error." << endl;
                    }
                }else{
                    cout << "[Error] find one invalid processor!" << endl;
                    return false;
                }

                string strCompiler = "[Debug] " + processor->getId() + " costs : ";
                compilerTimer.elapsed_message(strCompiler);
            }

            cout << "[Debug] task [" << processName_ << "] end successfully " << endl;

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
