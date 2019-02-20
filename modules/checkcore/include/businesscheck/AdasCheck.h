//
// Created by zhangxingang on 19-2-19.
//

#ifndef AUTOHDMAP_DATACHECK_ADASCHECK_H
#define AUTOHDMAP_DATACHECK_ADASCHECK_H

#include "IMapProcessor.h"
namespace kd {
    namespace dc {
        class AdasCheck : public IMapProcessor {
        public:
            explicit AdasCheck(const string &base_path);

            ~AdasCheck() override;

            string getId() override;

            bool execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) override;
        private:
            // 数据加载
            bool load_adas_data();

            bool load_adas_node();

            bool load_adas_node_slope();

            bool load_adas_node_fitting();

            bool load_adas_node_curvature();

            void release();

        private:
            const string id = "adas_check";
            string base_path;
            shared_ptr<MapDataManager> data_manager;
            shared_ptr<CheckErrorOutput> error_output;
        private:
            vector<shared_ptr<AdasNode>> adas_nodes_vec_;
            vector<shared_ptr<AdasNodeFitting>> adas_nodes_fitting_vec_;
            vector<shared_ptr<AdasNodeSlope>> adas_nodes_slope_vec_;
            vector<shared_ptr<AdasNodeCurvaTure>> adas_nodes_curvature_vec_;

            void check_adas_node();
        };
    }
}


#endif //AUTOHDMAP_DATACHECK_ADASCHECK_H
