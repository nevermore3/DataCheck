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
            //task名称
            string name;

            //数据路径
            string fileName;

            //数据类型
            string fileType;

            //模型配置文件
            string modelName;
        };

    }
}

#endif //AUTOHDMAP_DATACHECK_TASKDATAMODEL_H
