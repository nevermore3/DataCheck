
#ifndef AUTOHDMAP_DATACHECK_LANETOPOCHECK_H
#define AUTOHDMAP_DATACHECK_LANETOPOCHECK_H

#include "IMapProcessor.h"

namespace kd {
    namespace dc {

        /**
         * 车道线拓扑关系检查
         * 对应检查项：JH_C_22
         */
        class LaneTopoCheck : public IMapProcessor {

        public:

            /**
             * 获得对象唯一标识
             * @return 对象标识
             */
            virtual string getId() override;

            /**
             * 进行任务处理
             * @param mapDataManager 地图数据
             * @param errorOutput 错误信息输出
             * @return 操作是否成功
             */
            virtual bool execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) override;

        private:

            //存在没有有进入车道和退出车道的车道
            void check_JH_C_22(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput);

            // HD_TOPO_LANEGROUP中的外键检查
            void check_topo_lanegroup_foreignkey(shared_ptr<MapDataManager> mapDataManager,
                                                 shared_ptr<CheckErrorOutput> errorOutput);

            const string id = "lane_topo_check";

        };
    }
}

#endif //AUTOHDMAP_DATACHECK_LANETOPOCHECK_H
