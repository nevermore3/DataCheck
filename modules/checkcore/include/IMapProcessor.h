//
// Created by gaoyanhong on 2018/3/29.
//

#ifndef AUTOHDMAP_DATACHECK_IMAPPROCESSOR_H
#define AUTOHDMAP_DATACHECK_IMAPPROCESSOR_H

#include "CommonInclude.h"
#include "data/DataManager.h"
#include "storage/CheckErrorOutput.h"

namespace kd {
    namespace dc {

        class IMapProcessor{
        public:
            virtual ~IMapProcessor() = default;

        public:
            /**
             * 获得对象唯一标识
             * @return 对象标识
             */
            virtual string getId() = 0;

            /**
             * 进行任务处理
             * @param mapDataManager 地图数据
             * @return 操作是否成功
             */
            virtual bool execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) = 0;

        };

    }
}
#endif //AUTOHDMAP_DATACHECK_IMAPPROCESSOR_H
