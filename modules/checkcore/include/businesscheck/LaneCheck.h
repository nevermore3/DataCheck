//
// Created by zhangxingang on 19-1-25.
//

#ifndef AUTOHDMAP_DATACHECK_LANECHECK_H
#define AUTOHDMAP_DATACHECK_LANECHECK_H

#include <IMapProcessor.h>

namespace kd {
    namespace dc {

        /**
         * 车道线拓扑关系检查
         * 对应检查项：JH_C_22
         */
        class LaneCheck : public IMapProcessor {

        public:

            /**
             * 获得对象唯一标识
             * @return 对象标识
             */
            string getId() override;

            /**
             * 进行任务处理
             * @param mapDataManager 地图数据
             * @param errorOutput 错误信息输出
             * @return 操作是否成功
             */
            bool execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) override;

        private:

            /**
             * 检查中心线与边缘线是否交叉
             * @param mapDataManager
             * @param errorOutput
             */
            void check_lane_divider_intersect(shared_ptr<MapDataManager> mapDataManager,
                                              shared_ptr<CheckErrorOutput> errorOutput);
        private:
            /**
             * lane与divider是否相交
             * @param mapDataManager
             * @param ptr_lane
             * @param ptr_divider
             * @return
             */
            bool lane_divider_intersect(const shared_ptr<MapDataManager> &mapDataManager,
                                        const shared_ptr<DCLane> &ptr_lane,
                                        const shared_ptr<DCDivider> &ptr_divider);

            /**
             * lane与divider多个交点情况
             * @param mapDataManager
             * @param ptr_lane
             * @param ptr_divider
             * @return 1：2个交点并且小于50米 2：多于2个交点
             */
            bool lane_divider_intersects(const shared_ptr<MapDataManager> &mapDataManager,
                                        const shared_ptr<DCLane> &ptr_lane,
                                        const shared_ptr<DCDivider> &ptr_divider);
        private:

            const string id = "lane_topo_check";

        };
    }
}


#endif //AUTOHDMAP_DATACHECK_LANECHECK_H
