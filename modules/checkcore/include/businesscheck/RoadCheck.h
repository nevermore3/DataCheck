//
// Created by zhangxingang on 19-1-25.
//

#ifndef AUTOHDMAP_DATACHECK_ROADCHECK_H
#define AUTOHDMAP_DATACHECK_ROADCHECK_H

#include <IMapProcessor.h>
#include "SCHCheck.h"
namespace kd {
    namespace dc {
        class RoadCheck  : public IMapProcessor, public SCHCheck {
        public:
            RoadCheck(string fileName);
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
            void SetMapDataManager(shared_ptr<MapDataManager> &mapDataManager);
            /**
             * 结点重复
             * @param errorOutput
             * @param ptr_road
             */
            void check_road_node_repeat(shared_ptr<CheckErrorOutput> errorOutput, shared_ptr<DCRoad> ptr_road);

            /**
             * 结点间角度过大
             * @param errorOutput
             * @param ptr_road
             */
            void check_road_node_angle(shared_ptr<CheckErrorOutput> errorOutput, shared_ptr<DCRoad> ptr_road);

            /**
             * 结点间距过小
             * @param errorOutput
             * @param ptr_road
             */
            void check_road_node_distance(shared_ptr<CheckErrorOutput> errorOutput, shared_ptr<DCRoad> ptr_road);

            shared_ptr<geos::geom::LineString> get_road_line_string(const shared_ptr<MapDataManager> &mapDataManager,
                                                                    const shared_ptr<DCRoad> &ptr_road,
                                                                    const string &lane_group_id);

            bool road_divider_intersect(const shared_ptr<MapDataManager> &mapDataManager,
                                        const shared_ptr<DCRoad> &ptr_road,
                                        const string &lane_group_id,
                                        const shared_ptr<DCDivider> &ptr_divider);

            bool LoadTrafficRule();

            bool LoadRoadNode();

            bool LoadCNode();

            bool LoadCNodeConn();

            bool LoadNodeConn();


            void BuildInfo();

            void BuildNodeID2Road();

        private:
            const string id = "road_check";

            shared_ptr<MapDataManager> map_data_manager_;


            map<long, shared_ptr<DCTrafficRule>> map_traffic_rule_;

            // roadnode
            map<long, shared_ptr<DCRoadNode>> map_road_nodes_;

            //cnode
            map<long, shared_ptr<DCCNode>> map_cnodes_;

            // nodeconn
            map<long, shared_ptr<DCNodeConn>> map_node_conn_;

            // cnodeconn
            map<long, shared_ptr<DCCNodeConn>> map_cnode_conn_;

            // key : nodeID, value: {roads}
            map<long, vector<shared_ptr<DCRoad>>> node_id_to_froad_;

            map<long, vector<shared_ptr<DCRoad>>> node_id_to_troad_;

        };
    }
}
#endif //AUTOHDMAP_DATACHECK_ROADCHECK_H
