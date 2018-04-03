//
// Created by gaoyanhong on 2018/2/28.
//

#ifndef AUTOHDMAP_FUSIONCORE_DATAMANAGER_H
#define AUTOHDMAP_FUSIONCORE_DATAMANAGER_H

#include "CommonInclude.h"

#include "MapDataModel.h"
#include "DividerGeomModel.h"

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

        };

       /**
        *
        */
       class MapDataManager{

       public:

       public:
           //所有的车道线对象，map的key为车道线的id
           map<string, shared_ptr<DCDivider>> dividers_;

           //记录所有车道线共点信息， map的key为节点id
           //map<string, shared_ptr<DCDividerTopoNode>> topoNodes_;

           //所有车道对象，map的key为车道的id
           map<string, shared_ptr<DCLane>> lanes_;

           //车道组
           map<string, shared_ptr<DCLaneGroup>> laneGroups_;

       };

    }
}

#endif //AUTOHDMAP_FUSIONCORE_DATAMANAGER_H
