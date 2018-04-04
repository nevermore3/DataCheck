//
// Created by gaoyanhong on 2018/3/29.
//

#ifndef AUTOHDMAP_DATACHECK_MAPDATAINPUT_H
#define AUTOHDMAP_DATACHECK_MAPDATAINPUT_H


#include "data/DataManager.h"

#include "CheckErrorOutput.h"

namespace kd {
    namespace dc {

        /**
         * 地图数据加载
         */
        class MapDataInput{

        public:
            /**
             * 加载车道线数据
             * @param basePath 车道线基础数据路径
             * @param dividers 车道线对象
             * @param errorOutput 错误信息输出
             * @return 加载是否成功
             */
            bool loadDivider(string basePath, map<string, shared_ptr<DCDivider>> & dividers, shared_ptr<CheckErrorOutput> errorOutput);

            /**
             * 加载车道信息
             * @param basePath 数据路径
             * @param dividers 道路分割线信息
             * @param lanes 车道信息
             * @param errorOutput 错误信息输出
             * @return 加载是否成功
             */
            bool loadLane(string basePath, const map<string, shared_ptr<DCDivider>> &dividers, map<string, shared_ptr<DCLane>> & lanes, shared_ptr<CheckErrorOutput> errorOutput);


            /**
             * 加载车道组信息
             * @param basePath 数据路径
             * @param lanes 参考的车道信息
             * @param laneGroups_ 车道组信息
             * @return 加载是否成功
             */
            bool loadLaneGroup(string basePath, const map<string, shared_ptr<DCLane>> & lanes, map<string, shared_ptr<DCLaneGroup>> & laneGroups, shared_ptr<CheckErrorOutput> errorOutput);


        private:
            bool setDividerNode(shared_ptr<DCDivider> div, string nodeId, map<string, shared_ptr<DCDividerNode>> & commonNodeInfos);

        };
    }
}

#endif //AUTOHDMAP_DATACHECK_MAPDATAINPUT_H
