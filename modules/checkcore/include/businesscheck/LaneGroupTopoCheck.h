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
             * 预处理divider连接关系，lane group连接关系
             * @param mapDataManager
             */
            void pre_divider_topo(shared_ptr<MapDataManager> mapDataManager);
        private:
            /**
             * 获取所有道路的首尾节点集合，用于判断道路的拓扑连接
             * @param roads
             * @return
             */
            set<string> get_road_nodes(const shared_ptr<MapDataManager> &mapDataManager, const set<string> &roads,
                    bool is_front);

            bool is_virtual_lane_group(const shared_ptr<MapDataManager> &mapDataManager, const string &lane_group_id);
        private:
            // lane group的连接关系
            set<pair<string, string>> lane_group2_conn_lg_;
            // divider的连接关系
            set<pair<string, string>> divider2_conn_divider_;

            vector<shared_ptr<DCLaneGroup>> virtual_lane_groups_;

        private:
            const string id = "lanegroup_topo_check";
        };
    }
}

#endif //AUTOHDMAP_DATACHECK_LANEGROUPTOPOCHECK_H
