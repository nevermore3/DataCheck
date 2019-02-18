//
// Created by gaoyanhong on 2018/3/28.
//

#ifndef AUTOHDMAP_DATACHECK_ERRORDATAMODEL_H
#define AUTOHDMAP_DATACHECK_ERRORDATAMODEL_H

#include "CommonInclude.h"

#include "DividerGeomModel.h"


namespace kd {
    namespace dc {
        static const string LEVEL_WARNING = "warning";
        static const string LEVEL_ERROR = "error";

        class DCError{
        public:

            DCError(string checkModel);

            /**
             * 获得错误字段信息
             * @return 字段信息描述字符串
             */
            virtual string getHeader() = 0;

            /**
             * 获得错误信息
             * @return 错误信息字符串
             */
            virtual string toString() = 0;


        public:
            //检测模型
            string checkModel_;

        public:
            //设置检查模型描述信息
            string checkDesc_;

        public:
            // 错误级别
            string checkLevel_;
        };

        /**
         * 数据字段约束检查
         */
        class DCAttCheckError : public DCError {

        public:
            DCAttCheckError(string checkModel);

            DCAttCheckError(string checkModel, string modelName, string fieldName, string recordId);

            virtual string getHeader() override ;

            virtual string toString() override ;

        public:
            //模型名称，或表名
            string modelName_;

            //字段名称
            string fieldName_;

            //记录id
            string recordId_;

            //错误描述
            string errorDesc_;
        };

        /**
         * 关联关系检查失败记录
         */
        class DCRelationCheckError : public DCError {

        public:
            DCRelationCheckError(string checkModel);

            DCRelationCheckError(string checkModel, string modelName, string fieldName, string refModelName, string refFieldName);

            virtual string getHeader() override ;

            virtual string toString() override ;

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

            //错误描述
            string errorDesc_;
        };


        /**
         * 车道线检查错误
         */
        class DCDividerCheckError : public DCError {
        public:
            DCDividerCheckError(string checkModel);

            static shared_ptr<DCDividerCheckError> createByAtt(string checkModel, shared_ptr<DCDivider> div,
                                                               shared_ptr<DCDividerAttribute> att);

            static shared_ptr<DCDividerCheckError>
            createByNode(string checkModel, shared_ptr<DCDivider> div, shared_ptr<DCDividerNode> node);

            static shared_ptr<DCDividerCheckError> createByNode(string checkModel, string nodeId, double lng, double lat, double z);

        public:
            virtual string getHeader() override ;

            virtual string toString() override ;

        public:
            //车道线id
            string dividerId_;

            //属性变化点id
            string attId_;

            //节点id
            string nodeId_;

            //节点、属性变化点对应的经度
            double lng_;

            //节点、属性变化点对应的纬度
            double lat_;

            //节点、属性变化点对应的高度
            double z_;

            //错误描述
            string errorDesc_;
        };


        /**
         * 车道检查错误
         */
        class DCLaneCheckError : public DCError {
        public:
            DCLaneCheckError(string checkModel);

            static shared_ptr<DCLaneCheckError> createByAtt(string checkModel, shared_ptr<DCLane> lane,
                                                               shared_ptr<DCLaneAttribute> att);

            static shared_ptr<DCLaneCheckError>
            createByNode(string checkModel, shared_ptr<DCLane> lane, shared_ptr<DCDividerNode> node);

        public:
            virtual string getHeader() override ;

            virtual string toString() override ;

        public:
            //车道id
            string laneId_;

            //左车道线id
            string leftDividerId_;

            //右车道线id
            string rightDividerId_;

            //属性变化点id
            string attId_;

            //节点id
            string nodeId_;

            //节点、属性变化点对应的经度
            double lng_;

            //节点、属性变化点对应的纬度
            double lat_;

            //节点、属性变化点对应的高度
            double z_;

            //错误描述
            string errorDesc_;
        };

        /**
         * 数据字段约束检查
         */
        class DCSqlCheckError : public DCError {

        public:
            DCSqlCheckError(string checkModel);

            DCSqlCheckError(string checkModel, string modelName, string fieldName, string recordId);

            virtual string getHeader() override ;

            virtual string toString() override ;

        public:
            //模型名称，或表名
            string modelName_;

            //字段名称
            string fieldName_;

            //记录id
            string recordId_;

            //错误描述
            string errorDesc_;

            //错误详细信息描述
            string detail;
        };

        /**
         * 车道组检查错误
         */
        class DCLaneGroupCheckError : public DCError {
        public:
            explicit DCLaneGroupCheckError(const string &checkModel);

        public:
            string getHeader() override;

            string toString() override;
        public:
            static shared_ptr<DCLaneGroupCheckError> createByKXS_03_005(string road_id, long s_index, long e_index,
                                                                        bool is_positive = true);

            static shared_ptr<DCLaneGroupCheckError> createByKXS_03_005(string road_id, long index,
                                                                        bool is_positive = true);

            static shared_ptr<DCLaneGroupCheckError> createByKXS_03_006(string road_id, string lg1,
                                                                        long s_index1, long e_index1,
                                                                        string lg2, long s_index2,
                                                                        long e_index2, bool is_positive = true);

            static shared_ptr<DCLaneGroupCheckError> createByKXS_03_004(string divider_id, set<string> lane_groups);

            static shared_ptr<DCLaneGroupCheckError> createByKXS_03_002(string lane_group_id);

            static shared_ptr<DCLaneGroupCheckError> createByKXS_03_001(string lane_group_id,
                                                                        const vector<string> &dividers);

            static shared_ptr<DCLaneGroupCheckError> createByKXS_03_027(string lane_group_id);

        public:

            //错误详细信息描述
            string detail;
        };

        /**
        * 车道组连接检查错误
        */
        class DCLaneGroupTopoCheckError : public DCError {
        public:
            explicit DCLaneGroupTopoCheckError(const string &checkModel);

        public:
            string getHeader() override;

            string toString() override;
        public:
            static shared_ptr<DCLaneGroupTopoCheckError> createByKXS_04_001(string lg_id1, string lg_id2);

            static shared_ptr<DCLaneGroupTopoCheckError> createByKXS_05_001(string lg_id1, string lg_id2);
        public:

            //错误详细信息描述
            string detail;
        };

        /**
        * 道路检查错误
        */
        class DCRoadCheckError : public DCError {
        public:
            explicit DCRoadCheckError(const string &checkModel);

        public:
            string getHeader() override;

            string toString() override;
        public:
            static shared_ptr<DCRoadCheckError> createByKXS_04_002(const string &road_id,
                                                                            const string &lane_group_id);
        public:

            //错误详细信息描述
            string detail;
        };

        /**
        * 道路检查错误
        */
        class DCLaneError : public DCError {
        public:
            explicit DCLaneError(const string &checkModel);

        public:
            string getHeader() override;

            string toString() override;
        public:
            static shared_ptr<DCLaneError> createByKXS_05_002(const string &lane_id,
                                                              const string &divider_id);

            static shared_ptr<DCLaneError> createByKXS_05_003(const string &lane_id,
                                                              const string &divider_id);
        public:

            //错误详细信息描述
            string detail;
        };
    }
}

#endif //AUTOHDMAP_DATACHECK_ERRORDATAMODEL_H
