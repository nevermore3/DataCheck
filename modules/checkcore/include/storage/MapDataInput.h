//
// Created by gaoyanhong on 2018/3/29.
//

#ifndef AUTOHDMAP_DATACHECK_MAPDATAINPUT_H
#define AUTOHDMAP_DATACHECK_MAPDATAINPUT_H


#include "data/DataManager.h"

#include "CheckErrorOutput.h"
#include "DataInput.h"
#include "ModelInput.h"

namespace kd {
    namespace dc {

        /**
         * 地图数据加载
         */
        class MapDataInput : public DataInput {
        public:
            MapDataInput(const shared_ptr<MapDataManager> &map_data_manager_,
                         const shared_ptr<CheckErrorOutput> &error_output_, const string &base_path_);

            bool LoadData();

        public:
            /**
             * 加载车道线数据
             * @return 加载是否成功
             */
            bool loadDivider();

            /**
             * 加载车道信息
             * @return 加载是否成功
             */
            bool loadLane();


            /**
             * 加载车道组信息
             * @return 加载是否成功
             */
            bool loadLaneGroup();


            /**
             * 加载车道拓扑关系
             * @return 加载是否成功
             */
            bool loadLaneConnectivity();


            /**
             * 加载所有线对象
             * @return 加载是否成功
             */
            bool loadObjectLine();

            /**
             * 加载道路
             * @return
             */
            bool loadRoad();

            /**
             * 加载lanegroup相关逻辑关系表
             * @return
             */
            bool loadLaneGroupLogicInfo();

        private:
            bool setDividerNode(shared_ptr<DCDivider> div, string nodeId, map<string, shared_ptr<DCDividerNode>> & commonNodeInfos);
        };
    }
}

#endif //AUTOHDMAP_DATACHECK_MAPDATAINPUT_H
