//
// Created by zhangxingang on 19-2-19.
//

#ifndef AUTOHDMAP_DATACHECK_ADASCHECK_H
#define AUTOHDMAP_DATACHECK_ADASCHECK_H

#include "IMapProcessor.h"
#include "geos/index/quadtree/Quadtree.h"

namespace kd {
    namespace dc {
        class AdasCheck : public IMapProcessor {
        public:
            explicit AdasCheck(const string &base_path);

            ~AdasCheck() override;

            string getId() override;

            bool execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) override;

            /**
             * 相邻属性点间的距离检查
             */
            void Check_KXS_07_001();

            /**
             * ADAS_NODE点完备性检查
             */
            void Check_KXS_07_003();

            void Check_KXS_07_004();

            /*
             * ADAS_NODE曲率值域检查
             * @param ptr_adas_node
             */
            void Check_KXS_07_005(shared_ptr<AdasNode> ptr_adas_node);

            /**
             * ADAS_NODE坡度值域检查
             * @param ptr_adas_node
             */
            void Check_KXS_07_007(shared_ptr<AdasNode> ptr_adas_node);

            /**
             * ADAS_NODE与关联ROAD距离检查
             * @param ptr_adas_node
             */
            void Check_KXS_07_008(shared_ptr<AdasNode> ptr_adas_node);

            /**
             * 坡度检查
             */
            void check_adas_node_slope();

            /**
             * 曲率检查
             */
            void check_adas_node_curvature();


        private:
            // 数据加载
            bool load_adas_data();

            bool load_adas_node();

            bool load_adas_node_slope();

            bool load_adas_node_fitting();

            bool load_adas_node_curvature();

            void insert_road_id2_adas_node_maps(const shared_ptr<AdasNode> &ptr_adas_node);

            void insert_road_id2_adas_nodes_slope(const shared_ptr<AdasNodeSlope> &ptr_adas_node_slope);

            void insert_road_id2_adas_nodes_cur(const shared_ptr<AdasNodeCurvature> &ptr_adas_node_cur);

            void PrepareAdasNode();

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

            void get_road_nodes(const shared_ptr<DCRoad> &ptr_road, long from_node, long to_node,
                                vector<shared_ptr<DCCoord>> &ptr_nodes_vec);

            void get_slope_nodes(const shared_ptr<AdasNodeSlope> &ptr_node_slope,
                                 vector<shared_ptr<DCCoord>> &ptr_nodes_vec);

            void check_cur_node_distance(const shared_ptr<AdasNodeCurvature> &ptr_adas_node_cur,
                                         const shared_ptr<DCCoord> &ptr_coord,
                                         double fabs_sub_dis, int index);


        private:
            const string id = "adas_check";
            string base_path;
            shared_ptr<MapDataManager> data_manager;
            shared_ptr<CheckErrorOutput> error_output;
        private:
            // road_id与adas_node_index与adas_node的映射
            unordered_map<long, map<long, shared_ptr<AdasNode>>> road_id2_adas_node_maps_;
            unordered_map<long, vector<shared_ptr<AdasNodeSlope>>> road_id2_adas_nodes_slope_maps_;
            unordered_map<long, vector<shared_ptr<AdasNodeCurvature>>> road_id2_adas_nodes_cur_maps_;
            vector<shared_ptr<AdasNodeFitting>> adas_nodes_fitting_vec_;
//            vector<shared_ptr<AdasNodeSlope>> adas_nodes_slope_vec_;

            shared_ptr<geos::index::quadtree::Quadtree> adas_node_quadtree_;
        };
    }
}


#endif //AUTOHDMAP_DATACHECK_ADASCHECK_H
