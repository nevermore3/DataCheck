//
// Created by zhangxingang on 19-2-19.
//

#include <businesscheck/AdasCheck.h>
#include <shp/shapefil.h>
#include <shp/ShpData.hpp>
#include <util/CommonUtil.h>
#include <seg/LineSegmentation.h>
#include <seg/CircleSegmentation.h>
#include <seg/ClothoidSegmentation.h>
#include <util/GeosObjUtil.h>

using namespace kd::seg;

namespace kd {
    namespace dc {

        AdasCheck::AdasCheck(const string &base_path) : base_path(base_path) {}

        AdasCheck::~AdasCheck() {
            release();
        }

        string AdasCheck::getId() {
            return id;
        }

        bool AdasCheck::execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) {
            bool ret = true;

            data_manager = mapDataManager;
            error_output = errorOutput;

            // 加载数据
//            ret &= load_adas_data();

            // 加载点数据
            if (load_adas_node()) {
                check_adas_node();
            }

            // 加载坡度数据
//            if (load_adas_node_slope()) {
//                check_adas_node_slope();
//            }

            // 加载曲率数据
            if (load_adas_node_curvature()) {
                check_adas_node_curvature();
            }

            return false;
        }

        bool AdasCheck::load_adas_data() {
            bool ret = true;
//            ret &= load_adas_node();
            ret &= load_adas_node_slope();
//            ret &= load_adas_node_fitting();
            ret &= load_adas_node_curvature();
            return ret;
        }

        bool AdasCheck::load_adas_node() {
            bool ret = true;
            string adas_node_file = base_path + "/ADAS_NODE";
            ShpData shpNodeData(adas_node_file);
            if (shpNodeData.isInit()) {
                int record_nums = shpNodeData.getRecords();
                for (int i = 0; i < record_nums; i++) {
                    SHPObject *shpObject = shpNodeData.readShpObject(i);
                    if (!shpObject || shpObject->nSHPType != SHPT_POINTZ)
                        continue;

                    shared_ptr<AdasNode> ptr_adas_node = make_shared<AdasNode>();
                    //读取属性信息
                    ptr_adas_node->id_ = std::to_string(shpNodeData.readIntField(i, "ID"));
                    ptr_adas_node->road_id_ = shpNodeData.readIntField(i, "R_ID");
                    ptr_adas_node->road_node_idx_ = shpNodeData.readIntField(i, "R_NodeIdx");
                    ptr_adas_node->adas_node_id_ = shpNodeData.readIntField(i, "A_NodeID");
                    ptr_adas_node->curvature_ = shpNodeData.readDoubleField(i, "Curvature");
                    ptr_adas_node->slope_ = shpNodeData.readDoubleField(i, "Slope");
                    ptr_adas_node->heading_ = shpNodeData.readDoubleField(i, "Heading");

                    int nVertices = shpObject->nVertices;
                    if (nVertices == 1) {
                        shared_ptr<DCCoord> ptr_coord = make_shared<DCCoord>();
                        ptr_coord->lng_ = shpObject->padfX[0];
                        ptr_coord->lat_ = shpObject->padfY[0];
                        ptr_coord->z_ = shpObject->padfZ[0];
                        ptr_adas_node->coord_ = ptr_coord;
                    }
                    insert_road_id2_adas_node_maps(ptr_adas_node);
                }
            } else {
                LOG(ERROR) << "open shp file " << adas_node_file << "failed!";
                ret = false;
            }
            return ret;
        }

        bool AdasCheck::load_adas_node_slope() {
            bool ret = true;
            string adas_node_slope_file = base_path + "/ADAS_NODE_SLOPE_SEG_temp";
            ShpData shpData(adas_node_slope_file);
            if (shpData.isInit()) {
                int record_nums = shpData.getRecords();
                for (int i = 0; i < record_nums; i++) {
                    SHPObject *shpObject = shpData.readShpObject(i);
                    if (!shpObject || shpObject->nSHPType != SHPT_ARCZ)
                        continue;

                    //读取基本属性
                    shared_ptr<AdasNodeSlope> ptr_adas_node_slope = make_shared<AdasNodeSlope>();
                    ptr_adas_node_slope->id_ = to_string(shpData.readIntField(i, "id"));
                    ptr_adas_node_slope->road_id_ = shpData.readIntField(i, "road_id");
                    ptr_adas_node_slope->node_num_ = shpData.readIntField(i, "node_num");
                    ptr_adas_node_slope->seg_index_ = shpData.readIntField(i, "seg_index");
                    ptr_adas_node_slope->from_node_ = shpData.readIntField(i, "from_node");
                    ptr_adas_node_slope->to_node_ = shpData.readIntField(i, "to_node");
                    ptr_adas_node_slope->ratio_ = shpData.readDoubleField(i, "ratio");
                    ptr_adas_node_slope->intercept_ = shpData.readDoubleField(i, "intercept");

                    //读取空间信息
                    int nVertices = shpObject->nVertices;
                    for (int idx = 0; idx < nVertices; idx++) {
                        shared_ptr<DCCoord> ptr_coord = make_shared<DCCoord>();
                        ptr_coord->lng_ = shpObject->padfX[idx];
                        ptr_coord->lat_ = shpObject->padfY[idx];
                        ptr_coord->z_ = shpObject->padfZ[idx];
                        ptr_adas_node_slope->nodes_.emplace_back(ptr_coord);
                    }
                    insert_road_id2_adas_nodes_slope(ptr_adas_node_slope);
                }
            } else {
                LOG(ERROR) << "open shp file " << adas_node_slope_file << "failed!";
                ret = false;
            }
            return ret;
        }

        bool AdasCheck::load_adas_node_fitting() {
            bool ret = true;
            string adas_node_fitting_file = base_path + "/ADAS_NODE_FITTING_temp";
            ShpData shpNodeData(adas_node_fitting_file);
            if (shpNodeData.isInit()) {
                int record_nums = shpNodeData.getRecords();
                for (int i = 0; i < record_nums; i++) {
                    SHPObject *shpObject = shpNodeData.readShpObject(i);
                    if (!shpObject || shpObject->nSHPType != SHPT_POINTZ)
                        continue;

                    shared_ptr<AdasNodeFitting> ptr_adas_node_fitting = make_shared<AdasNodeFitting>();
                    //读取属性信息
                    ptr_adas_node_fitting->id_ = std::to_string(shpNodeData.readIntField(i, "id"));
                    ptr_adas_node_fitting->road_id_ = shpNodeData.readIntField(i, "road_id");
                    ptr_adas_node_fitting->node_index_ = shpNodeData.readIntField(i, "node_index");
                    ptr_adas_node_fitting->curvature_ = shpNodeData.readDoubleField(i, "curvature");
                    ptr_adas_node_fitting->slope_ = shpNodeData.readDoubleField(i, "slope");

                    int nVertices = shpObject->nVertices;
                    if (nVertices == 1) {
                        shared_ptr<DCCoord> ptr_coord = make_shared<DCCoord>();
                        ptr_coord->lng_ = shpObject->padfX[0];
                        ptr_coord->lat_ = shpObject->padfY[0];
                        ptr_coord->z_ = shpObject->padfZ[0];
                        ptr_adas_node_fitting->coord_ = ptr_coord;
                    }

                    adas_nodes_fitting_vec_.emplace_back(ptr_adas_node_fitting);
                }
            } else {
                LOG(ERROR) << "open shp file " << adas_node_fitting_file << "failed!";
                ret = false;
            }
            return ret;
        }

        bool AdasCheck::load_adas_node_curvature() {
            bool ret = true;
            string adas_node_cur_file = base_path + "/ADAS_NODE_CUR_SEG_temp";
            ShpData shpData(adas_node_cur_file);
            if (shpData.isInit()) {
                int record_nums = shpData.getRecords();
                for (int i = 0; i < record_nums; i++) {
                    SHPObject *shpObject = shpData.readShpObject(i);
                    if (!shpObject || shpObject->nSHPType != SHPT_ARCZ)
                        continue;

                    //读取基本属性
                    shared_ptr<AdasNodeCurvature> ptr_adas_node_curvature = make_shared<AdasNodeCurvature>();
                    ptr_adas_node_curvature->id_ = to_string(shpData.readIntField(i, "id"));
                    ptr_adas_node_curvature->road_id_ = shpData.readIntField(i, "road_id");
                    ptr_adas_node_curvature->node_num_ = shpData.readIntField(i, "node_num");
                    ptr_adas_node_curvature->seg_index_ = shpData.readIntField(i, "seg_index");
                    ptr_adas_node_curvature->from_node_ = shpData.readIntField(i, "from_node");
                    ptr_adas_node_curvature->to_node_ = shpData.readIntField(i, "to_node");
                    ptr_adas_node_curvature->type_ = shpData.readIntField(i, "type");
                    if (ptr_adas_node_curvature->type_ == 1) {
                        ptr_adas_node_curvature->curvature_line_.ratio_ = shpData.readDoubleField(i, "ratio");
                        ptr_adas_node_curvature->curvature_line_.intercept_ = shpData.readDoubleField(i, "intercept");
                        ptr_adas_node_curvature->curvature_line_.x_axis_based_ = shpData.readIntField(i, "xAxisBased");
                    } else if (ptr_adas_node_curvature->type_ == 2) {
                        ptr_adas_node_curvature->curvature_circle_.radius_ = shpData.readDoubleField(i, "radius");
                        ptr_adas_node_curvature->curvature_circle_.center_x_ = shpData.readDoubleField(i, "center_x");
                        ptr_adas_node_curvature->curvature_circle_.center_y_ = shpData.readDoubleField(i, "center_y");
                        ptr_adas_node_curvature->curvature_circle_.center_dir_ = shpData.readDoubleField(i,
                                                                                                         "center_dir");
                    } else if (ptr_adas_node_curvature->type_ == 3) {
                        ptr_adas_node_curvature->curvature_curve_.theta0_ = shpData.readDoubleField(i, "theta0");
                        ptr_adas_node_curvature->curvature_curve_.theta1_ = shpData.readDoubleField(i, "theta1");
                        ptr_adas_node_curvature->curvature_curve_.arc_len_ = shpData.readDoubleField(i, "arc_len");
                        ptr_adas_node_curvature->curvature_curve_.curvature0_ = shpData.readDoubleField(i,
                                                                                                        "curvature0");
                        ptr_adas_node_curvature->curvature_curve_.curvature1_ = shpData.readDoubleField(i,
                                                                                                        "curvature1");
                        ptr_adas_node_curvature->curvature_curve_.x0_ = shpData.readDoubleField(i, "x0");
                        ptr_adas_node_curvature->curvature_curve_.y0_ = shpData.readDoubleField(i, "y0");
                        ptr_adas_node_curvature->curvature_curve_.x1_ = shpData.readDoubleField(i, "x1");
                        ptr_adas_node_curvature->curvature_curve_.y1_ = shpData.readDoubleField(i, "y1");
                        ptr_adas_node_curvature->curvature_curve_.x_axis_based_ = shpData.readIntField(i, "xAxisBased");
                    }
                    ptr_adas_node_curvature->offset_x_ = shpData.readDoubleField(i, "offset_x");
                    ptr_adas_node_curvature->offset_y_ = shpData.readDoubleField(i, "offset_y");

                    //读取空间信息
                    int nVertices = shpObject->nVertices;
                    for (int idx = 0; idx < nVertices; idx++) {
                        shared_ptr<DCCoord> ptr_coord = make_shared<DCCoord>();
                        ptr_coord->lng_ = shpObject->padfX[idx];
                        ptr_coord->lat_ = shpObject->padfY[idx];
                        ptr_coord->z_ = shpObject->padfZ[idx];
                        ptr_adas_node_curvature->nodes_.emplace_back(ptr_coord);
                    }
                    insert_road_id2_adas_nodes_cur(ptr_adas_node_curvature);
                }
            } else {
                LOG(ERROR) << "open shp file " << adas_node_cur_file << "failed!";
                ret = false;
            }
            return ret;
        }

        void AdasCheck::release() {
            road_id2_adas_node_maps_.clear();
            adas_nodes_fitting_vec_.clear();
            road_id2_adas_nodes_slope_maps_.clear();
            road_id2_adas_nodes_cur_maps_.clear();
        }

        void AdasCheck::check_adas_node() {
            for (const auto &road_id2_adas_node : road_id2_adas_node_maps_) {
                long road_id = road_id2_adas_node.first;
                auto adas_node_iter = road_id2_adas_node.second.begin();
                auto adas_node_next_iter = ++road_id2_adas_node.second.begin();
                while (adas_node_next_iter != road_id2_adas_node.second.end()) {
                    bool check = false;
                    auto f_adas_node_id = adas_node_iter->second->id_;
                    auto t_adas_node_id = adas_node_next_iter->second->id_;
                    double distance = 0;
                    int distance_ret = check_adas_node_distance(adas_node_iter->second,
                                                                adas_node_next_iter->second, distance);
                    adas_node_iter = adas_node_next_iter;
                    adas_node_next_iter++;
                    // 距离大于参数
                    if (distance_ret == 3) {
                        check = true;
                    }
                    if (distance_ret == 2 && adas_node_next_iter != road_id2_adas_node.second.end()) {
                        // 距离小于参数并且不是最后两个点
                        check = true;
                    }

                    if (check) {
                        shared_ptr<DCError> ptr_error = DCAdasError::createByKXS_07_001(road_id, f_adas_node_id,
                                                                                        t_adas_node_id, distance);
                        if (ptr_error) {
                            error_output->saveError(ptr_error);
                        }
                    }
                }

            }
        }


        void AdasCheck::check_adas_node_slope() {
            for (const auto &road_id2_adas_node_slope : road_id2_adas_nodes_slope_maps_) {
                auto ptr_nodes_slopes_vec = road_id2_adas_node_slope.second;
                for (const auto &node_slope : ptr_nodes_slopes_vec) {
                    auto ptr_road = CommonUtil::get_road(data_manager, to_string(node_slope->road_id_));
                    vector<shared_ptr<DCCoord>> road_nodes_vec;
                    // 获取道路节点集合
                    get_road_nodes(ptr_road, node_slope->from_node_, node_slope->to_node_, road_nodes_vec);
                    vector<shared_ptr<DCCoord>> slope_nodes_vec;
                    // 获取拟合节点集合
                    LineModel lm;
                    lm.setup(node_slope->ratio_, node_slope->intercept_, 0, 0);
                    int count = 0;
                    for (const auto &node : node_slope->nodes_) {
                        double predit_z = lm.predict(node->z_);
                        double fabs_sub_dis = fabs(predit_z - node->z_);


                        count++;
                    }
                }
            }
        }

        void AdasCheck::check_adas_node_curvature() {
            for (const auto &road_id2_adas_node_cur : road_id2_adas_nodes_cur_maps_) {
                auto ptr_nodes_curs = road_id2_adas_node_cur.second;
                for (const auto &nodes_curvature : ptr_nodes_curs) {
                    auto ptr_road = CommonUtil::get_road(data_manager, to_string(nodes_curvature->road_id_));
                    vector<shared_ptr<DCCoord>> road_nodes_vec;
                    // 获取道路节点集合
                    get_road_nodes(ptr_road, nodes_curvature->from_node_, nodes_curvature->to_node_, road_nodes_vec);
                    vector<shared_ptr<geos::geom::Coordinate>> curvature_nodes_vec;

                    char zone[8] = {};
                    if (nodes_curvature->type_ == 1) {
                        LineModel lm;
                        lm.setup(nodes_curvature->curvature_line_.ratio_, nodes_curvature->curvature_line_.intercept_,
                                 nodes_curvature->offset_x_, nodes_curvature->offset_y_);
                        int count = 0;
                        for (const auto &node : nodes_curvature->nodes_) {
                            auto ptr_utm_node = GeosObjUtil::create_coordinate(node, zone);

                            // 拟合值计算
                            if (nodes_curvature->curvature_line_.x_axis_based_) {
                                double predit_y = lm.predict(ptr_utm_node->x);
                                double fabs_sub_dis = fabs(predit_y - ptr_utm_node->y);
                                check_cur_node_distance(nodes_curvature, node, fabs_sub_dis,
                                                        nodes_curvature->from_node_ + count);
                            } else {
                                double predit_x = lm.predict(ptr_utm_node->y);
                                double fabs_sub_dis = fabs(predit_x - ptr_utm_node->x);
                                check_cur_node_distance(nodes_curvature, node, fabs_sub_dis,
                                                        nodes_curvature->from_node_ + count);
                            }

                            count++;
                        }
                    } else if (nodes_curvature->type_ == 2) {
                        CircleModel cm;
                        cm.setup(nodes_curvature->curvature_circle_.radius_,
                                 nodes_curvature->curvature_circle_.center_x_,
                                 nodes_curvature->curvature_circle_.center_y_,
                                 nodes_curvature->offset_x_,
                                 nodes_curvature->offset_y_);
                        int count = 0;

                        for (const auto &node : nodes_curvature->nodes_) {
                            auto ptr_utm_node = GeosObjUtil::create_coordinate(node, zone);

                            // 拟合值计算
                            double predit_y = cm.predictWithGuess(ptr_utm_node->y, ptr_utm_node->x);
                            double fabs_sub_dis = fabs(predit_y - ptr_utm_node->y);
                            check_cur_node_distance(nodes_curvature, node, fabs_sub_dis,
                                                    nodes_curvature->from_node_ + count);

                            count++;
                        }
                    } else if (nodes_curvature->type_ == 3) {
                        ClothoidModel clm;
                        clm.setup(nodes_curvature->curvature_curve_.theta0_,
                                  nodes_curvature->curvature_curve_.theta1_,
                                  nodes_curvature->curvature_curve_.x0_,
                                  nodes_curvature->curvature_curve_.y0_,
                                  nodes_curvature->curvature_curve_.x1_,
                                  nodes_curvature->curvature_curve_.y1_,
                                  nodes_curvature->offset_x_,
                                  nodes_curvature->offset_y_);
                        vector<double> vFit, sFit, vec_x, vec_y;
                        for (const auto &node : nodes_curvature->nodes_) {
                            auto ptr_utm_node = GeosObjUtil::create_coordinate(node, zone);
                            vec_x.emplace_back(ptr_utm_node->x);
                            vec_y.emplace_back(ptr_utm_node->y);
                        }

                        clm.predict(vFit, sFit, vec_x, vec_y, nodes_curvature->curvature_curve_.x_axis_based_);

                        for (size_t idx = 0; idx < vFit.size(); idx++) {
                            double fabs_sub_dis = 0;
                            if (nodes_curvature->curvature_curve_.x_axis_based_) {
                                fabs_sub_dis = fabs(vFit[idx] - vec_y[idx]);
                                check_cur_node_distance(nodes_curvature, nodes_curvature->nodes_[idx], fabs_sub_dis,
                                                        nodes_curvature->from_node_ + idx);
                            } else {
                                fabs_sub_dis = fabs(vFit[idx] - vec_x[idx]);
                                check_cur_node_distance(nodes_curvature, nodes_curvature->nodes_[idx], fabs_sub_dis,
                                                        nodes_curvature->from_node_ + idx);
                            }
                        }
                    } else {
                        LOG(ERROR) << "adas node curavature type is wrong, id : " << nodes_curvature->id_;
                    }
                }
            }
        }

        void AdasCheck::insert_road_id2_adas_node_maps(const shared_ptr<AdasNode> &ptr_adas_node) {
            auto adas_node_iter = road_id2_adas_node_maps_.find(ptr_adas_node->road_id_);
            if (adas_node_iter != road_id2_adas_node_maps_.end()) {
                adas_node_iter->second.insert(make_pair(ptr_adas_node->adas_node_id_, ptr_adas_node));
            } else {
                map<long, shared_ptr<AdasNode>> adas_node_index2_nodes_map;
                adas_node_index2_nodes_map.insert(make_pair(ptr_adas_node->adas_node_id_, ptr_adas_node));
                road_id2_adas_node_maps_.insert(make_pair(ptr_adas_node->road_id_, adas_node_index2_nodes_map));
            }
        }

        void AdasCheck::insert_road_id2_adas_nodes_slope(const shared_ptr<AdasNodeSlope> &ptr_adas_node_slope) {
            auto adas_node_slope_iter = road_id2_adas_nodes_slope_maps_.find(ptr_adas_node_slope->road_id_);
            if (adas_node_slope_iter != road_id2_adas_nodes_slope_maps_.end()) {
                adas_node_slope_iter->second.emplace_back(ptr_adas_node_slope);
            } else {
                vector<shared_ptr<AdasNodeSlope>> adas_node_slope_vector;
                adas_node_slope_vector.emplace_back(ptr_adas_node_slope);
                road_id2_adas_nodes_slope_maps_.insert(
                        make_pair(ptr_adas_node_slope->road_id_, adas_node_slope_vector));
            }
        }

        void AdasCheck::insert_road_id2_adas_nodes_cur(const shared_ptr<AdasNodeCurvature> &ptr_adas_node_cur) {
            auto adas_node_cur_iter = road_id2_adas_nodes_cur_maps_.find(ptr_adas_node_cur->road_id_);
            if (adas_node_cur_iter != road_id2_adas_nodes_cur_maps_.end()) {
                adas_node_cur_iter->second.emplace_back(ptr_adas_node_cur);
            } else {
                vector<shared_ptr<AdasNodeCurvature>> adas_node_cur_vector;
                adas_node_cur_vector.emplace_back(ptr_adas_node_cur);
                road_id2_adas_nodes_cur_maps_.insert(make_pair(ptr_adas_node_cur->road_id_, adas_node_cur_vector));
            }
        }


        int AdasCheck::check_adas_node_distance(const shared_ptr<AdasNode> &f_ptr_adas_node,
                                                const shared_ptr<AdasNode> &t_ptr_adas_node, double &distance) {
            int ret = 1;
            distance = 0;
            if (f_ptr_adas_node->road_node_idx_ == t_ptr_adas_node->road_node_idx_) {
                vector<shared_ptr<DCCoord>> dc_coords_vec;
                dc_coords_vec.emplace_back(f_ptr_adas_node->coord_);
                dc_coords_vec.emplace_back(t_ptr_adas_node->coord_);

                distance = GeosObjUtil::get_length_of_coords(dc_coords_vec);
            } else {
                auto ptr_road = CommonUtil::get_road(data_manager, to_string(t_ptr_adas_node->road_id_));
                auto ptr_road_node = ptr_road->nodes_[t_ptr_adas_node->road_node_idx_];
                vector<shared_ptr<DCCoord>> dc_coords_vec;
                dc_coords_vec.emplace_back(f_ptr_adas_node->coord_);
                dc_coords_vec.emplace_back(ptr_road_node);
                dc_coords_vec.emplace_back(t_ptr_adas_node->coord_);
                distance = GeosObjUtil::get_length_of_coords(dc_coords_vec);
            }

            double adas_node_distance = DataCheckConfig::getInstance().getPropertyD(
                    DataCheckConfig::ADAS_NODE_DISTANCE);
            double adas_node_distance_acc = DataCheckConfig::getInstance().getPropertyD(
                    DataCheckConfig::ADAS_NODE_DISTANCE_ACCURACY);
            if (fabs(distance - adas_node_distance) < adas_node_distance_acc) {
                ret = 1;
            } else if (distance < adas_node_distance) {
                ret = 2;
            } else {
                ret = 3;
            }
            return ret;
        }

        void AdasCheck::get_road_nodes(const shared_ptr<DCRoad> &ptr_road, long from_node, long to_node,
                                       vector<shared_ptr<DCCoord>> &ptr_nodes_vec) {
            for (long idx = from_node; idx <= to_node; idx++) {
                if (idx >= 0 && idx < ptr_road->nodes_.size()) {
                    ptr_nodes_vec.emplace_back(ptr_road->nodes_[idx]);
                }
            }
        }

        void AdasCheck::get_slope_nodes(const shared_ptr<AdasNodeSlope> &ptr_node_slope,
                                        vector<shared_ptr<DCCoord>> &ptr_nodes_vec) {
//            for (const auto &node : ptr_node_slope->nodes_) {
//                LineModel lm;
//                lm.setup(node->lng_, ptr_node_slope->intercept_, 0, 0);
//            }
        }

        void AdasCheck::check_cur_node_distance(const shared_ptr<AdasNodeCurvature> &ptr_adas_node_cur,
                                                const shared_ptr<DCCoord> &ptr_coord,
                                                double fabs_sub_dis, int index) {

            double adas_cur_dis = DataCheckConfig::getInstance().getPropertyD(
                    DataCheckConfig::ADAS_NODE_CURVATURE_DISTANCE);

            if (fabs_sub_dis > adas_cur_dis) {
                shared_ptr<DCError> ptr_error = DCAdasError::createByKXS_07_002(ptr_adas_node_cur->road_id_,
                                                                                ptr_coord, index, fabs_sub_dis);
                if (ptr_error) {
                    error_output->saveError(ptr_error);
                }
            }
        }
    }
}