//
// Created by gaoyanhong on 2018/3/28.
//

#ifndef AUTOHDMAP_DATACHECK_ERRORDATAMODEL_H
#define AUTOHDMAP_DATACHECK_ERRORDATAMODEL_H

#include <api/KDSServiceModel.h>
#include "CommonInclude.h"

#include "DividerGeomModel.h"
using namespace kd::api;

namespace kd {
    namespace dc {
        static const string LEVEL_WARNING = "warning";
        static const string LEVEL_ERROR = "error";

        static const string DATA_TYPE_NODE = "node";
        static const string DATA_TYPE_WAY = "way";
        static const string DATA_TYPE_RELATION = "relation";
        // 结点错误信息
        struct NodeError {
            // 索引
            long index;
            // 结点坐标
            shared_ptr<DCCoord> ptr_coord;
        };

        struct NodeCompareError {
            // 索引
            long previous;
            long current;
            long next;
            // 结点坐标
            shared_ptr<DCCoord> ptr_previous_coord;
            shared_ptr<DCCoord> ptr_current_coord;
            shared_ptr<DCCoord> ptr_next_coord;
            double distance;
            double angle;
            double height;
            string id;
        };

        // 结点高度错误
        struct NodeCheck {
            // 前一点索引
            int pre_index;
            // 当前索引
            int index;
            // 实际高度差
            double diff_height;
            // 实际距离
            double distance;
        };
        /**
         * 错误点信息
         */
        class ErrNodeInfo : public DCCoord{
        public:
            ErrNodeInfo(){};
            ErrNodeInfo(shared_ptr<DCCoord> coord){
                 x_ = coord->x_;
                 y_ = coord->y_;
                 z_ = coord->z_;
            };
            //数据ID
            string dataId;
            //所在图层
            string dataLayer;
            //数据类型：node,way
            string dataType;

        };
        class DCError {
        public:

            DCError(string checkModel);

            /**
             * 获得错误信息
             * @return 错误信息字符串
             */
            virtual string toString();


        public:
            //检测模型
            string checkId;

            //设置检查模型描述信息
            string checkName;

            //错误详细信息描述
            string detail_;

            // 错误级别
            string checkLevel_;

            //任务号
            string taskId_;

            //融合任务框号
//            string boundId_;
            //数据KEY
            string dataKey_;

            string flag;
            //参考坐标
            shared_ptr<DCCoord> coord;
            //参考点信息
            vector<shared_ptr<ErrNodeInfo>> errNodeInfo;
            //错误所在数据源ID，node、way、relation数据ID
            string sourceId;
        };
        /**
         * 接边检查错误
         */
        class SplitCheckError : public DCError{
        public:
            SplitCheckError(string checkId_);
            virtual string toString() override;

        };
        /**
         * 数据字段约束检查
         */
        class DCAttCheckError : public DCError {

        public:
            DCAttCheckError(string checkModel);

            DCAttCheckError(string checkModel, string modelName, string fieldName, string recordId);

            virtual string toString() override;

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

            static shared_ptr<DCRelationCheckError> createByKXS_01_25(string model_name, string field,
                                                                      string relation_name);


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

            static shared_ptr<DCDividerCheckError>
            createByNode(string checkModel, string nodeId, double lng, double lat, double z);

            static shared_ptr<DCDividerCheckError> createByKXS_01_011(const string &divider_id,
                                                                      const vector<shared_ptr<NodeCompareError>> &ptr_error_nodes);

            static shared_ptr<DCDividerCheckError> createByKXS_01_012(const string &divider_id,
                                                                      const vector<shared_ptr<NodeCompareError>> &ptr_error_nodes);

            static shared_ptr<DCDividerCheckError> createByKXS_01_013(const string &divider_id,
                                                                      const vector<shared_ptr<NodeCompareError>> &ptr_error_nodes);

            static shared_ptr<DCDividerCheckError> createByKXS_01_030(long dividerID, long index,
                                                                      const shared_ptr<DCCoord> &coord, int level);


            static shared_ptr<DCDividerCheckError> createByKXS_01_029(long nodeID, shared_ptr<DCCoord> coord);



        public:
            //车道线id
            string dividerId_;

            //属性变化点id
            string attId_;

            //节点id
            string nodeId_;

        };


        /**
         * 车道检查错误
         */
        class DCLaneCheckError : public DCError {
        public:
            DCLaneCheckError(string checkModel);

            static shared_ptr<DCLaneCheckError> createByAtt(string checkModel, shared_ptr<DCLane> lane,
                                                            shared_ptr<DCLaneAttribute> att);
            static shared_ptr<DCLaneCheckError> createByKXS_05_008(string checkModel, shared_ptr<DCLane> lane, shared_ptr<DCDivider> leftDiv, shared_ptr<DCDivider> rightDiv);
            static shared_ptr<DCLaneCheckError>
            createByNode(string checkModel, shared_ptr<DCLane> lane, shared_ptr<DCDividerNode> node);

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
        };

        /**
         * 数据字段约束检查
         */
        class DCSqlCheckError : public DCError {

        public:
            DCSqlCheckError(string checkModel);

            DCSqlCheckError(string checkModel, string modelName, string fieldName, string recordId);


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
         * 车道组检查错误
         */
        class DCLaneGroupCheckError : public DCError {
        public:
            explicit DCLaneGroupCheckError(const string &checkModel);


        public:
            static shared_ptr<DCLaneGroupCheckError> createByKXS_03_005(string road_id, long s_index, long e_index,
                                                                        bool is_positive = true);

            static shared_ptr<DCLaneGroupCheckError> createByKXS_03_005(string road_id, long index,
                                                                        bool is_positive = true);

            static shared_ptr<DCLaneGroupCheckError> createByKXS_03_006(string road_id, string lg1,
                                                                        long s_index1, long e_index1,
                                                                        string lg2, long s_index2,
                                                                        long e_index2,string taskId, bool is_positive = true);

            static shared_ptr<DCLaneGroupCheckError> createByKXS_03_004(string divider_id, set<string> lane_groups);

            static shared_ptr<DCLaneGroupCheckError> createByKXS_03_003(shared_ptr<DCDivider> div);

            static shared_ptr<DCLaneGroupCheckError> createByKXS_03_002(string lane_group_id);

            static shared_ptr<DCLaneGroupCheckError> createByKXS_03_001(string lane_group_id,
                                                                        const vector<string> &dividers);

            static shared_ptr<DCLaneGroupCheckError> createByKXS_03_027(string lane_group_id);

            static shared_ptr<DCLaneGroupCheckError> createByKXS_03_028(shared_ptr<DCLaneGroup> lanelaneGroup,string daId, shared_ptr<DCCoord> coord);

            static shared_ptr<DCLaneGroupCheckError> createByKXS_03_029(shared_ptr<DCLaneGroup> lanelaneGroup1,shared_ptr<DCLaneGroup> lanelaneGroup2);

        };

        /**
        * 车道组连接检查错误
        */
        class DCLaneGroupTopoCheckError : public DCError {
        public:
            explicit DCLaneGroupTopoCheckError(const string &checkModel);

        public:
            static shared_ptr<DCLaneGroupTopoCheckError> createByKXS_04_001(string lg_id1, string lg_id2);

            static shared_ptr<DCLaneGroupTopoCheckError> createByKXS_05_001(string lg_id1, string lg_id2);

        };

        /**
        * 道路检查错误
        */
        class DCRoadCheckError : public DCError {
        public:
            explicit DCRoadCheckError(const string &checkModel);

        public:
            static shared_ptr<DCRoadCheckError> createByKXS_04_002(const string &road_id,
                                                                   const string &lane_group_id);

            static shared_ptr<DCRoadCheckError> createByKXS_04_003(const string &road_id,
                                                                   vector<NodeCheck> &error_index_pair);

            static shared_ptr<DCRoadCheckError> createByKXS_04_006(const string &road_id,
                                                                   const vector<shared_ptr<NodeError>> &ptr_error_nodes);

            static shared_ptr<DCRoadCheckError> createByKXS_04_007(const string &road_id,
                                                                   const vector<shared_ptr<NodeCompareError>> &ptr_error_nodes);

            static shared_ptr<DCRoadCheckError> createByKXS_04_008(const string &road_id,
                                                                   const vector<shared_ptr<NodeCompareError>> &ptr_error_nodes);

            static shared_ptr<DCRoadCheckError> createByKXS_04_009(int type , const string &from_road_id,const string &to_road_id,shared_ptr<DCCoord> &coord);


            static shared_ptr<DCRoadCheckError> createByKXS_04_010(long nodeID1, long nodeID2, long cNodeID1, long cNodeID2);

            static shared_ptr<DCRoadCheckError> createByKXS_04_011(long roadID);

            static shared_ptr<DCRoadCheckError> createByKXS_04_012(int type , const string &from_road_id,const string &to_road_id,shared_ptr<DCCoord> &coord);
        };

        /**
        * 中心线检查错误
        */
        class DCLaneError : public DCError {
        public:
            explicit DCLaneError(const string &checkModel);

        public:
            static shared_ptr<DCLaneError> createByKXS_05_002(const string &lane_id,
                                                              const string &divider_id);

            static shared_ptr<DCLaneError> createByKXS_05_003(const string taskid,const string dataKey,const string dataType,const string dataLayer,shared_ptr<DCCoord>  coord,const string &lane_id,
                                                              const string &divider_id);

            static shared_ptr<DCLaneError> createByKXS_05_015(const string &lane_id1,const string &lane_id2,const shared_ptr<DCCoord> coord);

            static shared_ptr<DCLaneError> createByKXS_05_016(const string &lane_id,
                                                              const vector<shared_ptr<NodeError>> &ptr_error_nodes);

            static shared_ptr<DCLaneError> createByKXS_05_017(const string &lane_id,
                                                              const vector<shared_ptr<NodeCompareError>> &errorArray);

            static shared_ptr<DCLaneError> createByKXS_05_018(long fromLaneID, long toLaneID, double angle);

            static shared_ptr<DCLaneError> createByKXS_05_020(long laneID, long index,
                                                              const shared_ptr<DCCoord> &coord, int level);


            static shared_ptr<DCLaneError> createByKXS_05_022(long nodeID, shared_ptr<DCCoord> coord);

            static shared_ptr<DCLaneError> createByKXS_05_023(string lane_id,string divider_id,double dis,shared_ptr<DCCoord> coord);

        };

        /**
        * ADAS检查错误
        */
        class DCAdasError : public DCError {
        public:
            explicit DCAdasError(const string &checkModel);

        public:
            static shared_ptr<DCAdasError> createByKXS_07_001(long road_id, string f_adas_node_id,
                                                              string t_adas_node_id, double distance);

            static shared_ptr<DCAdasError> createByKXS_07_002(long road_id, const shared_ptr<DCCoord> &ptr_coord,
                                                              long index, double distance);

            static shared_ptr<DCAdasError> createByKXS_07_003(long road_id, long index,
                                                              const shared_ptr<DCCoord> &ptr_coord, int level);

            static shared_ptr<DCAdasError> createByKXS_07_005(long adas_node_id, shared_ptr<DCCoord> ptr_coord);

            static shared_ptr<DCAdasError> createByKXS_07_007(long adas_node_id, shared_ptr<DCCoord> ptr_coord);

            static shared_ptr<DCAdasError> createByKXS_07_008(long adas_node_id, shared_ptr<DCCoord> ptr_coord);
        };

        /**
        * 表描述检查
        */
        class DCTableDescError : public DCError {
        public:
            explicit DCTableDescError(const string &checkModel) : DCError(checkModel) {}

        public:
            static shared_ptr<DCTableDescError> createByKXS_10_001(const string &detail);
        };


        /**
        * 字段检查错误
        */
        class DCFieldError : public DCError {
        public:
            explicit DCFieldError(const string &checkModel);

        public:
            static shared_ptr<DCFieldError> createByKXS_01_019(const string &detail);

            static shared_ptr<DCFieldError> createByKXS_01_020(const string &file);

            static shared_ptr<DCFieldError> createByKXS_01_024(const string &type, const string &id,
                                                               const set<long> &index);

        public:
            bool check_file(const string &file);
        };



        //长度总和检查
        class DCLengthCheckError : public DCError {
        public:
            explicit DCLengthCheckError(const string &checkModel) : DCError(checkModel) {}

        public:
            static shared_ptr<DCLengthCheckError> createByLength(const double osmLength, const double kxfLength,
                                                                 const string &name);

        };

        //数目总和检查
        class DCCountCheckError : public DCError {
        public:
            explicit DCCountCheckError(const string &checkModel) : DCError(checkModel) {}

            static shared_ptr<DCCountCheckError> createByKXS_09_001(size_t osmCount, size_t kxfCount, const string &name);
        };

        /**
         * 外键检查
         */
        class DCForeignKeyCheckError : public DCError {

        public:

            explicit DCForeignKeyCheckError(const string &checkModel) : DCError(checkModel) {};

            // 外键是否存在检查
            static shared_ptr<DCForeignKeyCheckError> createByKXS_01_026(string &modelName, string &fieldName);

            // 外键完备性检查
            static shared_ptr<DCForeignKeyCheckError> createByKXS_01_027(string &tableName,
                                                                         string &foreignKeyName,
                                                                         string &value,
                                                                         string &relationTableName,
                                                                         string &relationFieldName);
        };

        class DCAttributeCheckError : public DCError {
        public:
            explicit DCAttributeCheckError(const string &checkModel) : DCError(checkModel) {};

            static shared_ptr<DCAttributeCheckError> createByKXS_10_002(const string &detail);
        };

        /*
         * HD_DIVIDER_SCH  HD_LANE_SCH ADAS_NODE 属性点检查错误
         */
        class DCSCHInfoError : public DCError {
        public:
            explicit DCSCHInfoError(const string &checkModel) : DCError(checkModel) {}

            // 曲率值检查
            static shared_ptr<DCSCHInfoError> createByKXS_01_031(string name, string objID, double value,
                                                                 double threshold, shared_ptr<DCCoord> &coord);
            // 属性点间的距离检查
            static shared_ptr<DCSCHInfoError> createByKXS_01_032(long objID, int index1, int index2,
                                                                 double dis, double threshold, string name);

            // 属性点的坡度和关联对象的距离最近的两个形点计算的坡度对比
            static shared_ptr<DCSCHInfoError> createByKXS_01_035(string name, long objID, long index, double value1,
                                                                 double value2, double threshold);
        };

        class PolyLineError : public DCError{
        public:
            explicit PolyLineError(const string &checkModel) : DCError(checkModel) {}
            string toString() override;

            static shared_ptr<PolyLineError> createByKXS_011_02(string line_id,shared_ptr<DCCoord> coord);
        };

    }
}

#endif //AUTOHDMAP_DATACHECK_ERRORDATAMODEL_H
