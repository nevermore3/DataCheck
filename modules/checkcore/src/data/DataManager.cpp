//

#include "data/DataManager.h"

namespace kd {
   namespace dc {

       shared_ptr<DCModalData> ModelDataManager::getModelData(string modelName) {
           auto itr = modelDatas_.find(modelName);
           if (itr != modelDatas_.end()) {
               return itr->second;
           }
           return nullptr;
       }

       shared_ptr<DCModelDefine> ModelDataManager::getModelDefine(string modelName) {
           auto itr = modelDefines_.find(modelName);
           if (itr != modelDefines_.end()) {
               return itr->second;
           }
           return nullptr;
       }

    }
}