//
// Created by gaoyanhong on 2018/3/28.
//

#ifndef AUTOHDMAP_DATACHECK_ERRORDATAMODEL_H
#define AUTOHDMAP_DATACHECK_ERRORDATAMODEL_H

#include "CommonInclude.h"

namespace kd {
    namespace dc {


        /**
         * 数据字段约束检查
         */
        class DCAttCheckError{
        public:
            //模型名称，或表名
            string modelName;

            //字段名称
            string fieldName;

            //错误类型
            string errorType;

            //错误值
            string errorValue;

            //错误描述
            string errorDesc;
        };

        class DCRelationCheckError{

        };
    }
}

#endif //AUTOHDMAP_DATACHECK_ERRORDATAMODEL_H
