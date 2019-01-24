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
             * 预处理divider连接关系，lane group连接关系
             * @param mapDataManager
             */
            void pre_divider_topo(shared_ptr<MapDataManager> mapDataManager);

            void pre_lane_topo(shared_ptr<MapDataManager> mapDataManager);

            void get_conn_lane_groups(shared_ptr<MapDataManager> mapDataManager,
                                      const shared_ptr<DCLaneGroup> &ptr_lane_group);
        private:

            void pre_lane_groups_divider(const shared_ptr<MapDataManager> &mapDataManager,
                                         const pair<const string, shared_ptr<DCDivider>> &pair,
                                         const string &lane_group);

            void pre_lane_group_divider(const shared_ptr<MapDataManager> &mapDataManager,
                                        const pair<const string, shared_ptr<DCDivider>> &pair,
                                        const string &lane_group);
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

            void insert_divider2_conn_divider(string divider, string con_divider);

            vector<shared_ptr<DCDivider>> get_conn_dividers(const shared_ptr<MapDataManager> &mapDataManager,
                                                            string divider);

            vector<shared_ptr<DCDivider>> get_conn_ptr_dividers(const shared_ptr<MapDataManager> &mapDataManager,
                                                                const shared_ptr<DCDivider> &left_ptr_divider,
                                                                const shared_ptr<DCDivider> &right_ptr_divider,
                                                                bool &is_normal);

            vector<shared_ptr<DCDivider>> get_conn_ptr_dividers(const shared_ptr<MapDataManager> &mapDataManager,
                                                                const vector<shared_ptr<DCDivider>> &left_ptr_dividers,
                                                                const vector<shared_ptr<DCDivider>> &right_ptr_dividers);

            bool is_lane_conn(const shared_ptr<MapDataManager> &mapDataManager,
                              const vector<shared_ptr<DCLane>> &pre_ptr_lanes,
                              const vector<shared_ptr<DCLane>> &lat_ptr_lanes,
                              set<string> &tag_f_lane);

            bool is_lane_conn(const shared_ptr<MapDataManager> &mapDataManager,
                              const vector<shared_ptr<DCLane>> &pre_ptr_lanes,
                              const vector<shared_ptr<DCLane>> &lat_ptr_lanes);
        private:
            // lane group的连接关系
            set<pair<string, string>> lane_group2_conn_lg_;
            // divider的连接关系
            set<pair<string, string>> divider2_conn_divider_;
            set<pair<string, string>> pre_lane_conn_pair_;
            unordered_map<string, set<string>> divider2_conn_dividers_maps_;

            map<string, shared_ptr<DCLaneGroup>> virtual_lane_groups_maps_;

        private:
            const string id = "lanegroup_topo_check";

        };
    }
}

#endif //AUTOHDMAP_DATACHECK_LANEGROUPTOPOCHECK_H
