//
// Created by zhangxingang on 19-1-25.
//

#ifndef AUTOHDMAP_DATACHECK_ROADCHECK_H
#define AUTOHDMAP_DATACHECK_ROADCHECK_H

#include <IMapProcessor.h>

namespace kd {
    namespace dc {
        class RoadCheck  : public IMapProcessor {
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
             * 道路与lanegroup边缘线无交点
             * @param mapDataManager
             * @param errorOutput
             */
            void check_road_divider_intersect(shared_ptr<MapDataManager> mapDataManager,
                                              shared_ptr<CheckErrorOutput> errorOutput);

            void check_road_node_height(shared_ptr<MapDataManager> mapDataManager,
                                        shared_ptr<CheckErrorOutput> errorOutput);

            void check_road_node(shared_ptr<MapDataManager> mapDataManager,
                                 shared_ptr<CheckErrorOutput> errorOutput);

        private:
            shared_ptr<geos::geom::LineString> get_road_line_string(const shared_ptr<MapDataManager> &mapDataManager,
                                                                    const shared_ptr<DCRoad> &ptr_road,
                                                                    const string &lane_group_id);

            bool road_divider_intersect(const shared_ptr<MapDataManager> &mapDataManager,
                                        const shared_ptr<DCRoad> &ptr_road,
                                        const string &lane_group_id,
                                        const shared_ptr<DCDivider> &ptr_divider);
        private:
            const string id = "road_check";
        };
    }
}
#endif //AUTOHDMAP_DATACHECK_ROADCHECK_H
