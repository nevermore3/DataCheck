//
// Created by zhangxingang on 19-6-11.
//

#include <util/CommonCheck.h>
#include <util/GeosObjUtil.h>
#include <util/CommonUtil.h>
#include <mvg/Coordinates.hpp>

namespace kd {
    namespace dc {
        vector<shared_ptr<NodeCompareError>> CommonCheck::DistanceCheck(const vector<shared_ptr<DCCoord>> &dc_coord_vec,
                                                                        double dis_threshold) {
            vector<shared_ptr<NodeCompareError>> ret_error_nodes;
            auto previous_node = dc_coord_vec.front();
            long previous = 0;
            for (int i = 1; i < dc_coord_vec.size(); i++) {
                double node_dis = GeosObjUtil::get_length_of_node(previous_node, dc_coord_vec.at(i));
                if (node_dis >= 0 && node_dis < dis_threshold) {
                    shared_ptr<NodeCompareError> ptr_cur_e_node = make_shared<NodeCompareError>();
                    ptr_cur_e_node->previous = previous;
                    ptr_cur_e_node->ptr_previous_coord = previous_node;
                    ptr_cur_e_node->current = i;
                    ptr_cur_e_node->ptr_current_coord = dc_coord_vec[i];
                    ptr_cur_e_node->distance = node_dis;
                    ret_error_nodes.emplace_back(ptr_cur_e_node);

                    previous_node = dc_coord_vec[i];
                    previous = i;
                }
            }

            return ret_error_nodes;
        }

        vector<shared_ptr<NodeCompareError>>
        CommonCheck::AngleCheck(const vector<shared_ptr<DCCoord>> &dc_coord_vec, double angle_threshold) {
            vector<shared_ptr<NodeCompareError>> ret_error_nodes;
            if (dc_coord_vec.size() > 2) {
                auto previous_node = dc_coord_vec.front();
                long previous = 0;
                auto current_node = dc_coord_vec[1];
                long current = 1;

                for (int i = 2; i < dc_coord_vec.size(); i++) {
                    auto next_node = dc_coord_vec[i];
                    long next = i;
                    double angle = 0;
                    if (!CommonUtil::CheckCoordAngle(previous_node, current_node, next_node, angle_threshold, angle)) {
                        shared_ptr<NodeCompareError> ptr_error_node = make_shared<NodeCompareError>();
                        ptr_error_node->ptr_previous_coord = previous_node;
                        ptr_error_node->previous = previous;
                        ptr_error_node->ptr_current_coord = current_node;
                        ptr_error_node->current = current;
                        ptr_error_node->ptr_next_coord = next_node;
                        ptr_error_node->next = next;
                        ptr_error_node->angle = angle * 180 / kd::automap::PI;
                        ret_error_nodes.emplace_back(ptr_error_node);
                    }

                    previous_node = current_node;
                    previous = current;
                    current_node = next_node;
                    current = next;
                }
            }

            return ret_error_nodes;
        }
    }
}