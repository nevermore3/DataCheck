//
// Created by zhangxingang on 19-1-25.
//

#ifndef AUTOHDMAP_DATACHECK_LANECHECK_H
#define AUTOHDMAP_DATACHECK_LANECHECK_H

#include <IMapProcessor.h>
#include "geos/index/quadtree/Quadtree.h"
#include "SCHCheck.h"
namespace kd {
    namespace dc {

        /**
         * 车道线拓扑关系检查
         * 对应检查项：JH_C_22
         */
        class LaneCheck : public IMapProcessor, public SCHCheck {

        public:
            LaneCheck(string fileName);
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

            /**
             * 车道中心线交叉检查,检查同组内中心线与中心线是否存在交叉问题
             * @param mapDataManager
             * @param errorOutput
             */
            void check_lane_lane_intersect(shared_ptr<MapDataManager> mapDataManager,
                                           shared_ptr<CheckErrorOutput> errorOutput);

            /**
             * 车道中心线结点重复，前后点xy坐标相同（z值可能相同）
             * @param mapDataManager
             * @param errorOutput
             */
            void check_lane_node(shared_ptr<MapDataManager> mapDataManager,
                                 shared_ptr<CheckErrorOutput> errorOutput,
                                 shared_ptr<DCLaneGroup> ptr_lane_group);

            /**
             * 车道中心线折线检查
             * 同一条车道中心线上连续三个节点构成的夹角（绝对值）不能小于165度 (可配置)
             * @param mapDataManager
             * @param errorOutput
             */
            void check_lane_nodes_angle(shared_ptr<MapDataManager> mapDataManager,
                                         shared_ptr<CheckErrorOutput> errorOutput);

            /**
            * 车道中心线折线检查
            * 两条相交且有车道拓扑关系的车道中心线最近的形状点构成的夹角（绝对值）不能小于170度，(可配置)
            * @param mapDataManager
            * @param errorOutput
            */
            void check_lane_angle(shared_ptr<MapDataManager> mapDataManager,
                                  shared_ptr<CheckErrorOutput> errorOutput);



        private:
            void SetMapDataManager(shared_ptr<MapDataManager> &mapDataManager);

            /**
             * lane与divider是否相交
             * @param mapDataManager
             * @param ptr_lane
             * @param ptr_divider
             * @param [out]intersections
             * @return
             */
            bool lane_divider_intersect(const shared_ptr<MapDataManager> &mapDataManager,
                                        const shared_ptr<DCLane> &ptr_lane,
                                        const shared_ptr<DCDivider> &ptr_divider,CoordinateSequence *intersections);

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

            /**
             * lane之间是否存在交点
             * @param mapDataManager
             * @param errorOutput
             * @param ptr_lanes 结点相同的车道集合
             * @param is_front 是否是首点相同
             */
            void lane_intersects(const shared_ptr<MapDataManager> &mapDataManager,
                                 const shared_ptr<CheckErrorOutput> &errorOutput,
                                 const vector<shared_ptr<DCLane>> &ptr_lanes);

            /**
             * 每一LANE的形状点周围1.5米内必有一个关联该LANE的HD_LANE_SCH
             * @param mapDataManager
             * @param errorOutput
             */
            void LaneRelevantLaneSCH(shared_ptr<CheckErrorOutput> &errorOutput);

            /**
             * HD_LANE_SCH点离LANE的垂直距离不超过10cm
             * @param mapDataManager
             * @param errorOutput
             */
             void LaneSCHVerticalDistance(shared_ptr<CheckErrorOutput> &errorOutput);
        private:

            const string id = "lane_check";

            shared_ptr<MapDataManager> map_data_manager_;

        };
    }
}


#endif //AUTOHDMAP_DATACHECK_LANECHECK_H
