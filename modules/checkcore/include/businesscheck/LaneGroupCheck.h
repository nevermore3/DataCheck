//
// Created by zhangxingang on 19-1-17.
//

#ifndef AUTOHDMAP_DATACHECK_LANEGROUPCHECK_H
#define AUTOHDMAP_DATACHECK_LANEGROUPCHECK_H

#include "IMapProcessor.h"
namespace kd {
    namespace dc {
        struct LGNodeIndex{
            LGNodeIndex() = default;;
            LGNodeIndex(const string &lg_id, const string &road_id, long f_index, long t_index) {
                this->lanegroup_id = lg_id;
                this->f_idx = f_index;
                this->t_idx = t_index;
                this->road_id = road_id;
            }

            bool operator<(const LGNodeIndex &other) {
                return this->f_idx < other.f_idx;
            }

            bool operator>(const LGNodeIndex &other) {
                return this->f_idx > other.f_idx;
            }

            string lanegroup_id;
            string road_id;
            long f_idx{};
            long t_idx{};
        };

        /**
         * 车道组检查
         */
        class LaneGroupCheck : public IMapProcessor {
        public:

            string getId() override;

            bool execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) override;

        private:
            /**
             * 车道组关联道路范围检查
             * @return
             */
            void check_lanegroup_road(shared_ptr<MapDataManager> mapDataManager,
                                  shared_ptr<CheckErrorOutput> errorOutput);

        private:
            /**
             * 车道组关联道路索引点详细检查逻辑
             * @param errorOutput
             */
            void check_road_node_index(vector<LGNodeIndex> lg_node_index_vec, shared_ptr<DCRoad> ptr_road,
                                       bool is_positive, shared_ptr<CheckErrorOutput> errorOutput);

        private:
            const string id = "lanegroup_check";
        };
    }
}


#endif //AUTOHDMAP_DATACHECK_LANEGROUPCHECK_H
