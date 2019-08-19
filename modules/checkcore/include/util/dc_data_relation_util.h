//
// Created by zhangxingang on 19-8-15.
//

#ifndef AUTOHDMAP_DATACHECK_DC_DATA_RELATION_UTIL_H
#define AUTOHDMAP_DATACHECK_DC_DATA_RELATION_UTIL_H

#include <data/DataManager.h>
#include <data/data_type.h>

const static double ANGLE_REF_DIST_ = 10.0;

namespace kd {
    namespace dc {
        class DCDataRelationUtil {
        public:
            static bool BuildTopoLaneGroup(shared_ptr<MapDataManager> data_manager,
                                           unordered_map<long, shared_ptr<TopoLaneGroup>> &topo_lane_groups);

            static shared_ptr<TopoLaneGroup>
            GetTopoLaneGroup(unordered_map<long, shared_ptr<TopoLaneGroup>> &topo_lane_groups, long lane_group);
        private:
            static void buildDivider2NodeTopo(shared_ptr<DCDivider> divider,
                                              shared_ptr<DCDividerNode> node, bool from_or_to,
                                              unordered_map<long, shared_ptr<TopoDividerNodeExt>> &topo_divider_nodes_);

            static void getLaneGroupRef(int left_div_index, int right_div_limit,
                                        shared_ptr<TopoLaneGroup> topo_lg,
                                        unordered_map<long, shared_ptr<TopoLaneGroup>> &topo_lanegroups,
                                        set<long> &from_groups, set<long> &to_groups,
                                        unordered_map<long, shared_ptr<TopoDividerNodeExt>> &topo_divider_nodes_,
                                        unordered_map<long, set<long>> &divider_lanegroup_maps_);

            static void getLanegroupRefAccordFromNode(shared_ptr<TopoLaneGroup> topo_lg,
                                                      unordered_map<long, shared_ptr<TopoLaneGroup>> &topo_lanegroups,
                                                      long nodeId1, long nodeId2, set<long> &groups,
                                                      unordered_map<long, shared_ptr<TopoDividerNodeExt>> &topo_divider_nodes_,
                                                      unordered_map<long, set<long>> &divider_lanegroup_maps_,
                                                      const set<long> & expect_div_ids);

            static void getLanegroupRefAccordToNode(shared_ptr<TopoLaneGroup> topo_lg,
                                                    unordered_map<long, shared_ptr<TopoLaneGroup>> &topo_lanegroups,
                                                    long nodeId1, long nodeId2, set<long> &groups,
                                                    unordered_map<long, shared_ptr<TopoDividerNodeExt>> &topo_divider_nodes_,
                                                    unordered_map<long, set<long>> &divider_lanegroup_maps_,
                                                    const set<long> & expect_div_ids);

            static void findGroupAccrodToDividers(shared_ptr<TopoLaneGroup> topo_lg,
                                                  unordered_map<long, shared_ptr<TopoLaneGroup>> &topo_lanegroups,
                                                  shared_ptr<TopoDividerNodeExt> node1,
                                                  shared_ptr<TopoDividerNodeExt> node2,
                                                  set<long> &groups,
                                                  unordered_map<long, set<long>> &divider_lanegroup_maps_,
                                                  const set<long> &expect_div_ids, bool is_from);

            static void findGroupAccordDivider(shared_ptr<TopoLaneGroup> topo_lg,
                                               unordered_map<long, shared_ptr<TopoLaneGroup>> &topo_lanegroups,
                                               map<long, shared_ptr<DCDivider>> &dividers,
                                               map<long, shared_ptr<DCDivider>> &conn_dividers, set<long> &groups,
                                               unordered_map<long, set<long>> &divider_lanegroup_maps_,
                                               const set<long> &expect_div_ids);

            static bool checkLaneGroupIntersect(const vector<shared_ptr<TopoLaneGroup>> &topo_lane_groups);

            static bool orderLaneGroups(shared_ptr<TopoLaneGroup> cur_topolg, bool start,
                                        unordered_map<long, shared_ptr<TopoLaneGroup>> &topo_lane_groups);

            static double getLaneGroupDirection(shared_ptr<TopoLaneGroup> topo_lg, bool is_start);

            static long getLaneGroupPosition(double start_angle, double end_angle);

        };
    }
}


#endif //AUTOHDMAP_DATACHECK_DC_DATA_RELATION_UTIL_H
