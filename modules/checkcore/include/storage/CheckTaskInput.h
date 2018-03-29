//
// Created by gaoyanhong on 2018/3/28.
//

#ifndef AUTOHDMAP_DATACHECK_CHECKTASKINPUT_H
#define AUTOHDMAP_DATACHECK_CHECKTASKINPUT_H

#include "data/TaskDataModel.h"
#include "data/MapDataModel.h"

namespace kd {
    namespace dc {

        class CheckTaskInput{

        public:
            bool loadTaskInfo(string fileName, vector<shared_ptr<DCTask>> & tasks);

            bool loadTaskModel(string fileName, shared_ptr<DCModelDefine> modelDefine);

        };

    }
}

#endif //AUTOHDMAP_DATACHECK_CHECKTASKINPUT_H
