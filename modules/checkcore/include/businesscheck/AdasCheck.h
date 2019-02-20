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

            void check_adas_node();

        private:
            // 数据加载
            bool load_adas_data();

            bool load_adas_node();

            bool load_adas_node_slope();

            bool load_adas_node_fitting();

            bool load_adas_node_curvature();

            void insert_road_id2_adas_node_maps(const shared_ptr<AdasNode> &ptr_adas_node);

            void release();
        private:
            /**
             * 计算点距离，如果等于adas参数返回1，如果小于返回2， 如果大于返回3
             * @param f_ptr_adas_node
             * @param t_ptr_adas_node
             * @param distance
             * @return
             */
            int check_adas_node_distance(const shared_ptr<AdasNode> &f_ptr_adas_node,
                                          const shared_ptr<AdasNode> &t_ptr_adas_node, double &distance);

        private:
            const string id = "adas_check";
            string base_path;
            shared_ptr<MapDataManager> data_manager;
            shared_ptr<CheckErrorOutput> error_output;
        private:
            vector<shared_ptr<AdasNode>> adas_nodes_vec_;
            // road_id与adas_node_index与adas_node的映射
            unordered_map<long, map<long, shared_ptr<AdasNode>>> road_id2_adas_node_maps_;
            vector<shared_ptr<AdasNodeFitting>> adas_nodes_fitting_vec_;
            vector<shared_ptr<AdasNodeSlope>> adas_nodes_slope_vec_;
            vector<shared_ptr<AdasNodeCurvaTure>> adas_nodes_curvature_vec_;
        };
    }
}


#endif //AUTOHDMAP_DATACHECK_ADASCHECK_H
