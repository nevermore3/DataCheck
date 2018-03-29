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

        /**
         * 关联关系检查失败记录
         */
        class DCRelationCheckError{
        public:
            //模型名称
            string modelName;

            //字段名称
            string fieldName;

            //关联模型名称
            string refModelName;

            //关联字段名称
            string refFieldName;

            //原始记录值
            string recordValue;

            //关联记录值
            string refRecordValue;

            //错误类型, 相关联的记录不存在；关联的记录冗余
            string errorType;

        };
    }
}

#endif //AUTOHDMAP_DATACHECK_ERRORDATAMODEL_H
