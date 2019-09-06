//
// Created by ubuntu on 2019/9/5.
//

#ifndef AUTOHDMAP_DATACHECK_SLOPECHECK_H
#define AUTOHDMAP_DATACHECK_SLOPECHECK_H

#include "IMapProcessor.h"

namespace kd {
    namespace dc {


        class NodeConn : public DCModel{
        public:
            // 进入road
            long e_road_id_;
            // 关联node
            long node_id_;
            // 退出road
            long q_road_id_;
            // 是否通行
            long flag_;
        };

        class SlopeCheck : public IMapProcessor {
        public:
            SlopeCheck();

            ~SlopeCheck();

            string getId() override;

            virtual bool execute(shared_ptr<MapDataManager> modelDataManager,
                                 shared_ptr<CheckErrorOutput> errorOutput) override;

        private:

            void CheckAdasNode(shared_ptr<MapDataManager> modelDataManager,
                               shared_ptr<CheckErrorOutput> errorOutput);

            void CheckDividerSCH(shared_ptr<MapDataManager> modelDataManager,
                                 shared_ptr<CheckErrorOutput> errorOutput);

            void CheckLaneSCH(shared_ptr<MapDataManager> modelDataManager,
                              shared_ptr<CheckErrorOutput> errorOutput);

            bool LoadNodeConn();

            bool LoadAdasNode();

            bool LoadDividerSCH();

            bool LoadLaneSCH();

            shared_ptr<AdasNode> GetPreRoadAdasNode(long roadID);

            shared_ptr<AdasNode> GetNextRoadAdasNode(long roadID);

            shared_ptr<DCLaneCurvature> GetPreLaneAdasNode(long laneID);

            shared_ptr<DCLaneCurvature> GetNextLaneAdasNode(long laneID);

            bool LoadData();

        private:
            const string id_ = "slope check";

            shared_ptr<MapDataManager> data_manager_;

            string base_path_;

            double slope_threshold_;

            map<string, shared_ptr<NodeConn>> map_node_conn_;

            unordered_map<long, map<long, shared_ptr<DCLaneCurvature>>> map_lane_sch_;

            unordered_map<long, map<long, shared_ptr<AdasNode>>> map_road_adas_node_;

            unordered_map<long, map<long, shared_ptr<DCDivideSCH>>> map_divider_sch_;
        };

    }
}
#endif //AUTOHDMAP_DATACHECK_SLOPECHECK_H
