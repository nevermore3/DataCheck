//
// Created by gaoyanhong on 2018/3/30.
//

#ifndef AUTOHDMAP_DATACHECK_DIVIDERTOPOCHECK_H
#define AUTOHDMAP_DATACHECK_DIVIDERTOPOCHECK_H

#include "IMapProcessor.h"

namespace kd {
    namespace dc {

        /**
         * 车道线属性检查
         * 对应检查项：JH_C_6
         */
        class DividerTopoCheck : public IMapProcessor {

        public:

            /**
             * 获得对象唯一标识
             * @return 对象标识
             */
            virtual string getId() override;

            /**
             * 进行任务处理
             * @param mapDataManager 地图数据
             * @return 操作是否成功
             */
            virtual bool
            execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) override;

        private:
            void check_JH_C_6(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput);

        private:

            const string id = "divider_topo_check";
        };
    }
}

#endif //AUTOHDMAP_DATACHECK_DIVIDERTOPOCHECK_H
