//
// Created by gaoyanhong on 2018/2/28.
//

#ifndef AUTOHDMAP_FUSIONCORE_DATAMANAGER_H
#define AUTOHDMAP_FUSIONCORE_DATAMANAGER_H

#include "CommonInclude.h"

#include "MapDataModel.h"
#include "DividerGeomModel.h"
#include "TaskDataModel.h"
#include "data/data_type.h"

namespace kd {
   namespace dc {

       static const string DATA_TYPE_LANE = "lane_";
       static const string DATA_TYPE_SIGN = "sign_";
       static const string DATA_TYPE_GROUND = "ground_";
       static const string DATA_TYPE_POLE = "pole_";
       static const string DATA_TYPE_LAST_NUM = "_1";

        /**
         * 内存数据管理
         */
        class ModelDataManager{
        public:


            /**
             * 获得指定模型的数据
             * @param modelName
             * @return 模型数据，查询不到则为nullptr
             */
            shared_ptr<DCModalData> getModelData(string modelName);


            /**
             * 获得指定模型的定义
             * @param modelName
             * @return 模型定义，查询不到则为nullptr
             */
            shared_ptr<DCModelDefine> getModelDefine(string modelName);

        public:

            //所有模型定义， map的key是模型名，如"HD_DIVIDER_NODE"
            map<string, shared_ptr<DCModalData>> modelDatas_;

            //所有模型定义， map的key是模型名，如"HD_DIVIDER_NODE"
            map<string, shared_ptr<DCModelDefine>> modelDefines_;

            //检查任务
            map<string, shared_ptr<DCTask>> tasks_;
        };

       /**
        *
        */
       class MapDataManager{

       public:

           // divider2_lane_groups_插入
           void insert_divider2_lane_groups(string divider, string lane_group_id);

           // fnode_id2_dividers_maps_插入
           void insert_fnode_id2_dividers(string fnode_id, shared_ptr<DCDivider> ptr_divider);

           // tnode_id2_dividers_maps_插入
           void insert_tnode_id2_dividers(string tnode_id, shared_ptr<DCDivider> ptr_divider);

           // node_id2_dividers_maps_插入
           void insert_node_id2_dividers(string tnode_id, shared_ptr<DCDivider> ptr_divider);

           // lane_group2_roads_maps_插入
           void insert_lane_group2_roads(string lane_group_id, string road_id);

           map<long,shared_ptr<KxsData>> getKxfData(string modelName);

           void clearData(string modelName);
           /**
            * 关系类型数据初始化到内存
            * @param modelName 模型名称，表名称
            */
           void initRelation(string modelName);
           /**
            * line类型数据初始化到内存
            * @param modelName 模型名称，表名称
            * @param initGeom
            */
           void initPolyline(string modelName,bool initGeom=true);
           /**
            * 面对象初始化到内存
            * @param modelName 模型名称，表名称
            */
           void initPolygon(string modelName);
           /**
            * 点对象初始化到内存
            * @param modelName 模型名称，表名称
            */
           void initKxsNode(string modelName);

       public:
           // 所有的道路
           map<string, shared_ptr<DCRoad>> roads_;

           //所有的车道线对象，map的key为车道线的id
           map<string, shared_ptr<DCDivider>> dividers_;

           //所有车道对象，map的key为车道的id
           map<string, shared_ptr<DCLane>> lanes_;

           //车道组
           map<string, shared_ptr<DCLaneGroup>> laneGroups_;

           //车道组与道路的关联索引集合
           unordered_map<string, unordered_map<string, std::pair<long, long>>> road2LaneGroup2NodeIdxs_;

           //车道关联关系
           map<string, shared_ptr<DCLaneConnectivity>> laneConnectivitys_;

           //所有线对象
           map<string, shared_ptr<DCObjectPL>> objectPLs_;

           // key为divider id,value为关联的lane group id集合
           map<string, set<string>> divider2_lane_groups_;

           // fnode与连接的DIVIDER映射
           map<string, vector<shared_ptr<DCDivider>>> fnode_id2_dividers_maps_;

           // tnode与连接的DIVIDER映射
           map<string, vector<shared_ptr<DCDivider>>> tnode_id2_dividers_maps_;

           // node与连接的DIVIDER映射
           map<string, vector<shared_ptr<DCDivider>>> node_id2_dividers_maps_;

           // key为lane group id，value为road id
           map<string, vector<string>> lane_group2_roads_maps_;

           // lane group拓扑关系
           unordered_map<long, shared_ptr<TopoLaneGroup>> topo_lane_groups_;

       public:
           // 是否道路是自动生成
           bool is_auto_road = false;

       private:
           map<long, shared_ptr<KxsData>> no_data;
           map<string, map<long, shared_ptr<KxsData>>> map_kxf_data;
       };

    }
}

#endif //AUTOHDMAP_FUSIONCORE_DATAMANAGER_H
