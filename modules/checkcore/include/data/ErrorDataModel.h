//
// Created by gaoyanhong on 2018/3/28.
//

#ifndef AUTOHDMAP_DATACHECK_ERRORDATAMODEL_H
#define AUTOHDMAP_DATACHECK_ERRORDATAMODEL_H

#include "CommonInclude.h"

#include "DividerGeomModel.h"


namespace kd {
    namespace dc {


        /**
         * 数据字段约束检查
         */
        class DCAttCheckError {
        public:
            //模型名称，或表名
            string modelName_;

            //字段名称
            string fieldName_;

            //错误类型
            string errorType_;

            //错误值
            string errorValue_;

            //错误描述
            string errorDesc_;
        };

        /**
         * 关联关系检查失败记录
         */
        class DCRelationCheckError {
        public:
            //模型名称
            string modelName_;

            //字段名称
            string fieldName_;

            //关联模型名称
            string refModelName_;

            //关联字段名称
            string refFieldName_;

            //原始记录值
            string recordValue_;

            //关联记录值
            string refRecordValue_;

            //错误类型, 相关联的记录不存在；关联的记录冗余
            string errorType_;

        };

        class DCErrorCreator {

        };


        /**
         * 车道线检查错误
         */
        class DCDividerCheckError {
        public:
            DCDividerCheckError(string checkModel) {
                checkModel_ = checkModel;
            }

            static shared_ptr<DCDividerCheckError> createByAtt(string checkModel, shared_ptr<DCDivider> div,
                                                               shared_ptr<DCDividerAttribute> att);

            static shared_ptr<DCDividerCheckError>
            createByNode(string checkModel, shared_ptr<DCDivider> div, shared_ptr<DCDividerNode> node);

        public:
            string toString();

        public:
            string dividerId_;

            string attId_;

            string nodeId_;

            double lng_;

            double lat_;

            double z_;

            string checkModel_;

            string errorType_;

            string errorDesc_;
        };
    }
}

#endif //AUTOHDMAP_DATACHECK_ERRORDATAMODEL_H
