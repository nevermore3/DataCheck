//
// Created by gaoyanhong on 2018/3/29.
//

#ifndef AUTOHDMAP_DATACHECK_MAPDATAINPUT_H
#define AUTOHDMAP_DATACHECK_MAPDATAINPUT_H


#include "data/DataManager.h"

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
             * @return 加载是否成功
             */
            bool loadDivider(string basePath, map<string, shared_ptr<DCDivider>> & dividers);

        };
    }
}

#endif //AUTOHDMAP_DATACHECK_MAPDATAINPUT_H
