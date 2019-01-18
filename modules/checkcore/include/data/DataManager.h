//
// Created by gaoyanhong on 2018/2/28.
//

#ifndef AUTOHDMAP_FUSIONCORE_DATAMANAGER_H
#define AUTOHDMAP_FUSIONCORE_DATAMANAGER_H

#include "CommonInclude.h"

#include "MapDataModel.h"
#include "DividerGeomModel.h"
#include "TaskDataModel.h"

namespace kd {
   namespace dc {

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

       public:
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

       };

    }
}

#endif //AUTOHDMAP_FUSIONCORE_DATAMANAGER_H
