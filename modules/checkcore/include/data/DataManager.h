//
// Created by gaoyanhong on 2018/2/28.
//

#ifndef AUTOHDMAP_FUSIONCORE_DATAMANAGER_H
#define AUTOHDMAP_FUSIONCORE_DATAMANAGER_H


//fusioncore
#include "DividerGeomModel.h"

namespace kd {
   namespace dc {

        /**
         * 内存数据管理
         */
        class DataManager{
        public:

        public:

            //所有的车道线对象
            std::map<string, shared_ptr<KDDivider>> dividers;

        };

    }
}

#endif //AUTOHDMAP_FUSIONCORE_DATAMANAGER_H
