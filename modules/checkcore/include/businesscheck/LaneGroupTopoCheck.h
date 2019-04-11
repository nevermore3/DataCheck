//
// Created by zhangxingang on 19-1-21.
//

#ifndef AUTOHDMAP_DATACHECK_LANEGROUPTOPOCHECK_H
#define AUTOHDMAP_DATACHECK_LANEGROUPTOPOCHECK_H

#include <IMapProcessor.h>

namespace kd {
    namespace dc {
        class LaneGroupTopoCheck : public IMapProcessor {
        public:

            string getId() override;

            bool execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) override;

        private:
            /**
             * 释放资源
             * @param mapDataManager
             */
            void release(shared_ptr<MapDataManager> mapDataManager);

            /**
             * 检查道路的拓扑连通
             * @param mapDataManager
             * @param errorOutput
             */
            void check_road_topo(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput);

            /**
             * 检查车道的拓扑连通
             * @param mapDataManager
             * @param errorOutput
             */
            void check_lane_topo(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput);

            /**
             * 检查是否存在既是出口又是入口的车道组
             */
            void check_lane_group_depart_merge(shared_ptr<MapDataManager> mapDataManager,
                                               shared_ptr<CheckErrorOutput> errorOutput);

            /**
             * 预处理divider连接关系，lane group连接关系
             * @param mapDataManager
             */
            void pre_divider_topo(shared_ptr<MapDataManager> mapDataManager);


            /**
             * 获取连接组
             * @param mapDataManager
             * @param ptr_lane_group
             */
            void get_conn_lane_groups(shared_ptr<MapDataManager> mapDataManager,
                                      const shared_ptr<DCLaneGroup> &ptr_lane_group);
        private:

            /**
             * 获取所有道路的首尾节点集合，用于判断道路的拓扑连接
             * @param roads
             * @return
             */
            set<string> get_road_nodes(const shared_ptr<MapDataManager> &mapDataManager, const set<string> &roads,
                    bool is_front);

            /**
             * 是否是虚拟组
             * @param mapDataManager
             * @param lane_group_id
             * @return
             */
            bool is_virtual_lane_group(const shared_ptr<MapDataManager> &mapDataManager, const string &lane_group_id);

            bool is_virtual_lane_group(const shared_ptr<DCLaneGroup> &ptr_lane_group);

            bool is_double_lane_group(shared_ptr<MapDataManager> mapDataManager,
                                      const string &lane_group_id);

            /**
             * 获取连接的divider,相同组的DIVIDER
             * @param mapDataManager
             * @param lane_group
             * @param divider
             * @return
             */
            vector<shared_ptr<DCDivider>> get_conn_dividers(const shared_ptr<MapDataManager> &mapDataManager,
                                                            string lane_group, string divider);

            /**
             * 获取连接的divider
             * @param mapDataManager
             * @param divider
             * @return
             */
            vector<shared_ptr<DCDivider>> get_conn_dividers(const shared_ptr<MapDataManager> &mapDataManager,
                                                            string divider);

            /**
             * 连接的divider排序
             * @param mapDataManager
             * @param left_ptr_dividers
             * @param right_ptr_dividers
             * @return
             */
            vector<shared_ptr<DCDivider>> get_conn_ptr_dividers(const shared_ptr<MapDataManager> &mapDataManager,
                                                                const vector<shared_ptr<DCDivider>> &left_ptr_dividers,
                                                                const vector<shared_ptr<DCDivider>> &right_ptr_dividers);

            /**
             * 车道中心线是否连接
             * @param mapDataManager
             * @param pre_ptr_lanes
             * @param lat_ptr_lanes
             * @param tag_f_lane
             * @return
             */
            bool is_lane_conn(const shared_ptr<MapDataManager> &mapDataManager,
                              const vector<shared_ptr<DCLane>> &pre_ptr_lanes,
                              const vector<shared_ptr<DCLane>> &lat_ptr_lanes,
                              set<string> &tag_f_lane);

            /**
             * 车道中心线是否连接
             * @param mapDataManager
             * @param pre_ptr_lanes
             * @param lat_ptr_lanes
             * @return
             */
            bool is_lane_conn(const shared_ptr<MapDataManager> &mapDataManager,
                              const vector<shared_ptr<DCLane>> &pre_ptr_lanes,
                              const vector<shared_ptr<DCLane>> &lat_ptr_lanes);

            /**
             * 车道中心线是否连接
             * @param mapDataManager
             * @param pre_ptr_lanes
             * @param lat_ptr_lanes
             * @return
             */
            bool is_lane_conn_case(const shared_ptr<MapDataManager> &mapDataManager,
                                   const vector<shared_ptr<DCLane>> &pre_ptr_lanes,
                                   const vector<shared_ptr<DCLane>> &lat_ptr_lanes,
                                   set<string> &tag_f_lane);

        private:
            // 插入divider2_conn_dividers_maps_
            void insert_divider2_conn_divider(string divider, string con_divider);

        private:
            // lane group的连接关系
            set<pair<string, string>> lane_group2_conn_lg_;
            // lane连接对
            set<pair<string, string>> pre_lane_conn_pair_;
            // divider的连接
            unordered_map<string, set<string>> divider2_conn_dividers_maps_;
            // 虚拟组
            map<string, shared_ptr<DCLaneGroup>> virtual_lane_groups_maps_;

        private:
            const string id = "lanegroup_topo_check";

        };
    }
}

#endif //AUTOHDMAP_DATACHECK_LANEGROUPTOPOCHECK_H
