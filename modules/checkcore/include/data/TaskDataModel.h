//
// Created by gaoyanhong on 2018/3/28.
//

#ifndef AUTOHDMAP_DATACHECK_TASKDATAMODEL_H
#define AUTOHDMAP_DATACHECK_TASKDATAMODEL_H

#include "CommonInclude.h"

namespace kd {
    namespace dc {

        /**
         * 检查任务信息
         */
        class DCTask{
        public:
            string fileName;

            string fileType;

            string modelName;
        };

    }
}

#endif //AUTOHDMAP_DATACHECK_TASKDATAMODEL_H
