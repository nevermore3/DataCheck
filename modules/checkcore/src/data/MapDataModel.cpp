//
// Created by gaoyanhong on 2018/3/28.
//

#include "data/MapDataModel.h"

namespace kd {
    namespace dc {
        shared_ptr<DCFieldDefine> DCModelDefine::getFieldDefine(string fieldName) {
            for(shared_ptr<DCFieldDefine> fieldDefine : vecFieldDefines ){

                if(fieldDefine != nullptr && fieldDefine->name == fieldName){
                    return fieldDefine;
                }
            }
            return nullptr;
        }
    }
}
