//
// Created by zhangxingang on 19-2-18.
//

#include <util/GeosObjUtil.h>
#include <mvg/Coordinates.hpp>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/LineString.h>
#include "geos/geom/Point.h"
#include <geom/geo_util.h>
#include <geos/geom/Point.h>
#include <util/CommonUtil.h>


namespace kd {
    namespace dc {
        shared_ptr<geos::geom::Coordinate> GeosObjUtil::create_coordinate(const shared_ptr<DCDividerNode> &ptr_node,
                                                                         char *zone0, bool is_z) {
            shared_ptr<geos::geom::Coordinate> ret = nullptr;
            double X0, Y0;
            double x = ptr_node->coord_->x_;
            double y = ptr_node->coord_->y_;
            double z = ptr_node->coord_->z_;
            kd::automap::Coordinates::ll2utm(y, x, X0, Y0, zone0);
            if (is_z) {
                ret = make_shared<geos::geom::Coordinate>(X0, Y0);
            } else {
                ret = make_shared<geos::geom::Coordinate>(X0, Y0, z);
            }
            return ret;
        }

        shared_ptr<geos::geom::Coordinate> GeosObjUtil::create_coordinate(const shared_ptr<DCCoord> &ptr_node,
                                                                         char *zone0, bool is_z) {
            shared_ptr<geos::geom::Coordinate> ret = nullptr;
            double X0, Y0;
            double x = ptr_node->x_;
            double y = ptr_node->y_;
            double z = ptr_node->z_;
            kd::automap::Coordinates::ll2utm(y, x, X0, Y0, zone0);
            if (is_z) {
                ret = make_shared<geos::geom::Coordinate>(X0, Y0);
            } else {
                ret = make_shared<geos::geom::Coordinate>(X0, Y0, z);
            }
            return ret;
        }

        shared_ptr<geos::geom::LineString> GeosObjUtil::create_line_string(const vector<shared_ptr<DCCoord>> &nodes) {
            shared_ptr<geos::geom::LineString> ret_road_line_string = nullptr;

            //创建linestring
            CoordinateSequence *cl = new CoordinateArraySequence();
            for (const auto &node : nodes) {
                double X0, Y0;
                char zone0[8] = {0};

                kd::automap::Coordinates::ll2utm(node->y_, node->x_, X0, Y0, zone0);

                cl->add(geos::geom::Coordinate(X0, Y0, node->z_));
            }

            if (cl->size() >= 2) {
                const GeometryFactory *gf = GeometryFactory::getDefaultInstance();
                geos::geom::LineString *lineString = gf->createLineString(cl);
                if (lineString) {
                    ret_road_line_string.reset(lineString);
                }
            }

            return ret_road_line_string;
        }

        shared_ptr<DCCoord> GeosObjUtil::get_coord(const shared_ptr<geos::geom::Coordinate> &ptr_coord,
                                                    char *zone0, bool is_z) {
            shared_ptr<DCCoord> ptr_node = make_shared<DCCoord>();
            double lat = 0;
            double lon = 0;
            double y = ptr_coord->y;
            double x = ptr_coord->x;
            kd::automap::Coordinates::utm2ll(y, x, zone0, lat, lon);
            ptr_node->x_ = lon;
            ptr_node->y_ = lat;
            if (is_z) {
                ptr_node->z_ = ptr_coord->z;
            }

            return ptr_node;
        }

        double GeosObjUtil::get_length_of_coords(const vector<shared_ptr<DCCoord>> &ptr_coords) {
            geos::geom::CoordinateSequence *cl = new geos::geom::CoordinateArraySequence();
            for(const auto &ptr_coord : ptr_coords){
                double X0, Y0;
                char zone0[8] = {0};
                double x = ptr_coord->x_;
                double y = ptr_coord->y_;
                double z = ptr_coord->z_;
                kd::automap::Coordinates::ll2utm(y, x, X0, Y0, zone0);
                cl->add(geos::geom::Coordinate(X0, Y0, z));
            }
            if (cl->size() < 2) {
                return -1;
            }
            const geos::geom::GeometryFactory *gf = geos::geom::GeometryFactory::getDefaultInstance();
            geos::geom::LineString *linesString = gf->createLineString(cl);
            if (linesString) {
                double len = linesString->getLength();
                return len;
            }
            return -1;
        }

        double GeosObjUtil::get_length_of_coords(const vector<shared_ptr<geos::geom::Coordinate>> &ptr_coords) {
            geos::geom::CoordinateSequence *cl = new geos::geom::CoordinateArraySequence();
            for(const auto &ptr_coord : ptr_coords){
                cl->add(geos::geom::Coordinate(ptr_coord->x, ptr_coord->y, ptr_coord->z));
            }
            if (cl->size() < 2) {
                return -1;
            }
            const geos::geom::GeometryFactory *gf = geos::geom::GeometryFactory::getDefaultInstance();
            geos::geom::LineString *linesString = gf->createLineString(cl);
            if (linesString) {
                double len = linesString->getLength();
                return len;
            }
            return -1;
        }

        bool GeosObjUtil::is_same_coord(Coordinate coord1, Coordinate coord2, double precise) {
            if (fabs(coord1.x - coord2.x) < precise && fabs(coord1.y - coord2.y) < precise &&
                fabs(coord1.z - coord2.z) < precise) {
                return true;
            } else {
                return false;
            }
        }

        bool GeosObjUtil::is_same_coord(const shared_ptr<DCCoord> &coord1, const shared_ptr<DCCoord> &coord2,
                                        double precise) {
            if (fabs(coord1->x_ - coord2->x_) < precise && fabs(coord1->y_ - coord2->y_) < precise &&
                fabs(coord1->z_ - coord2->z_) < precise) {
                return true;
            } else {
                return false;
            }
        }

        double GeosObjUtil::get_length_of_node(shared_ptr<DCCoord> node1, shared_ptr<DCCoord> node2) {
            if (node1 && node2) {
                vector<shared_ptr<DCCoord>> vector_nodes;
                vector_nodes.emplace_back(node1);
                vector_nodes.emplace_back(node2);
                return get_length_of_coords(vector_nodes);
            }
            return -1;
        }

        //确定点集合的参考方向
        double GeosObjUtil::GetAngle(vector<shared_ptr<DCCoord>> &lane_nodes, bool start, double ref_dist, char * zone) {
            //个数太少，则不计算
            if (lane_nodes.size() < 2) {
                return 0.0;
            }

            //个数等于2，直接计算出角度
            if (lane_nodes.size() == 2) {
                shared_ptr<DCCoord> node1 = lane_nodes[0];
                shared_ptr<DCCoord> node2 = lane_nodes[1];
                double angle = geo::geo_util::calcAngle(node1->x_, node1->y_, node2->x_, node2->y_);
                return angle;
            }

            bool firstseg_invalid = CheckFirstSegInvalid(lane_nodes, start, M_PI/32.0, ref_dist);

            //点数较多时，尽量取较长的距离点
            //.1获得参考的节点索引和坐标
            shared_ptr<geos::geom::LineString> line = create_line_string(lane_nodes);

            //看第一段的距离是不是足够长，能够代表数据的方向，如果能够代表，则直接计算
            const geos::geom::CoordinateSequence *cs = line->getCoordinatesRO();
            if(start){
                const geos::geom::Coordinate &coord1 = cs->getAt(0);
                const geos::geom::Coordinate &coord2 = cs->getAt(1);
                if(kd::automap::KDGeoUtil::distance(coord1.x, coord1.y, coord2.x, coord2.y) > ref_dist/2.0){
                    shared_ptr<DCCoord> node1 = lane_nodes[0];
                    shared_ptr<DCCoord> node2 = lane_nodes[1];
                    double angle = geo::geo_util::calcAngle(node1->x_, node1->y_, node2->x_, node2->y_);
                    return angle;
                }
            }else{
                const geos::geom::Coordinate &coord1 = cs->getAt(cs->size()-2);
                const geos::geom::Coordinate &coord2 = cs->getAt(cs->size()-1);
                if(kd::automap::KDGeoUtil::distance(coord1.x, coord1.y, coord2.x, coord2.y) > ref_dist/2.0){
                    shared_ptr<DCCoord> node1 = lane_nodes[lane_nodes.size()-2];
                    shared_ptr<DCCoord> node2 = lane_nodes[lane_nodes.size()-1];
                    double angle = geo::geo_util::calcAngle(node1->x_, node1->y_, node2->x_, node2->y_);
                    return angle;
                }
            }

            //如果第一段的距离不能代表，则需要取足够距离长度的数据
            geos::geom::Coordinate diff_coord;
            int node_index;
            double node_dist;
            int base_index = start ? 0 : (lane_nodes.size() - 1);
            GetDiffPoint(line, base_index, ref_dist, start, diff_coord, node_index, node_dist);

            //.2夹角检查
            bool use_diff_coord = true;
            double angle_limit = M_PI / 64.0;
            if (start) {
                int end_node_index = node_index + 1;
                if (end_node_index >= lane_nodes.size()) {
                    end_node_index = lane_nodes.size() - 1;
                }

                int base_index_temp = firstseg_invalid ?  1 : 0;
                for (int i = base_index_temp; i < end_node_index - 1; i++) {
                    shared_ptr<DCCoord> node1 = lane_nodes[i];
                    shared_ptr<DCCoord> node2 = lane_nodes[i + 1];
                    shared_ptr<DCCoord> node3 = lane_nodes[i + 2];
                    double angle_diff = kd::automap::KDGeoUtil::getAngleDiff(node1->x_, node1->y_, node2->x_, node2->y_,
                                                                             node2->x_, node2->y_, node3->x_,
                                                                             node3->y_);
                    if (angle_diff > angle_limit) {
                        use_diff_coord = false;
                        break;
                    }
                }
            } else {
                int end_node_index = node_index - 1;
                if (end_node_index < 0) {
                    end_node_index = 0;
                }

                int base_index_temp = firstseg_invalid ?  lane_nodes.size() - 2 : lane_nodes.size() - 1;

                for (int i = base_index_temp; i > end_node_index + 1; i--) {
                    shared_ptr<DCCoord> node1 = lane_nodes[i];
                    shared_ptr<DCCoord> node2 = lane_nodes[i - 1];
                    shared_ptr<DCCoord> node3 = lane_nodes[i - 2];
                    double angle_diff = kd::automap::KDGeoUtil::getAngleDiff(node1->x_, node1->y_, node2->x_, node2->y_,
                                                                             node2->x_, node2->y_, node3->x_,
                                                                             node3->y_);
                    if (angle_diff > angle_limit) {
                        use_diff_coord = false;
                        break;
                    }
                }
            }

            //求角度
            if (use_diff_coord) { //使用差分点计算角度
                shared_ptr<DCCoord> base_coord;
                if(!firstseg_invalid){
                    base_coord = start ? lane_nodes.front() : lane_nodes.back();
                }else{
                    base_coord = start ? lane_nodes[1] : lane_nodes[lane_nodes.size()-2];
                }

                double diff_lng, diff_lat;
                kd::automap::Coordinates::utm2ll(diff_coord.y, diff_coord.x, zone, diff_lat, diff_lng);

                double angle = 0.0;
                if (start) {
                    angle = geo::geo_util::calcAngle(base_coord->x_, base_coord->y_, diff_lng, diff_lat);
                } else {
                    angle = geo::geo_util::calcAngle(diff_lng, diff_lat, base_coord->x_, base_coord->y_);
                }

                return angle;
            } else { //直接使用首段作为方向参考
                shared_ptr<DCCoord> node1, node2;
                if (start) {
                    if(!firstseg_invalid){
                        node1 = lane_nodes[0];
                        node2 = lane_nodes[1];
                    }else{
                        node1 = lane_nodes[1];
                        node2 = lane_nodes[2];
                    }

                    return geo::geo_util::calcAngle(node1->x_, node1->y_, node2->x_, node2->y_);
                } else {
                    if(!firstseg_invalid){
                        node1 = lane_nodes[lane_nodes.size() - 2];
                        node2 = lane_nodes[lane_nodes.size() - 1];
                    }else{
                        node1 = lane_nodes[lane_nodes.size() - 3];
                        node2 = lane_nodes[lane_nodes.size() - 2];
                    }

                    return geo::geo_util::calcAngle(node1->x_, node1->y_, node2->x_, node2->y_);
                }
            }
        }


        bool GeosObjUtil::GetDiffPoint(const shared_ptr<geos::geom::LineString> line,
                                       double distance, bool start, geos::geom::Coordinate &coord) {

            int node_index;
            double node_dist;
            if (start) {
                return GetDiffPoint(line, 0, distance, start, coord, node_index, node_dist);
            } else {
                const geos::geom::CoordinateSequence *cs = line->getCoordinatesRO();

                return GetDiffPoint(line, cs->size() - 1, distance, start, coord, node_index, node_dist);
            }
        }

        bool GeosObjUtil::GetDiffPoint(const shared_ptr<geos::geom::LineString> line, int base_index,
                                       double distance, bool start, geos::geom::Coordinate &coord,
                                       int &node_index, double &node_dist) {
            const geos::geom::CoordinateSequence *cs = line->getCoordinatesRO();

            if (start) {
                //从起点开始累计长度
                double dist_total = 0.0;
                for (int i = base_index; i < cs->size() - 1; i++) {
                    const geos::geom::Coordinate &coord1 = cs->getAt(i);
                    const geos::geom::Coordinate &coord2 = cs->getAt(i + 1);
                    double dist = kd::automap::KDGeoUtil::getDistance(&coord1, &coord2);

                    double dist_temp = dist_total + dist;
                    if (dist_temp < distance) {
                        dist_total = dist_temp;
                    } else {
                        double diff_dist = distance - dist_total;
                        node_index = i;
                        node_dist = diff_dist;
                        return GetStringPoint(line, i, diff_dist, coord);
                    }
                }

                const geos::geom::Coordinate &endcoord = cs->back();
                coord = endcoord;
                node_index = cs->size() - 1;
                node_dist = 0.0;
            } else {
                //从终点开始累计长度
                double dist_total = 0.0;
                for (int i = base_index; i >= 1; i--) {
                    const geos::geom::Coordinate &coord1 = cs->getAt(i);
                    const geos::geom::Coordinate &coord2 = cs->getAt(i - 1);
                    double dist = kd::automap::KDGeoUtil::getDistance(&coord1, &coord2);

                    double dist_temp = dist_total + dist;
                    if (dist_temp < distance) {
                        dist_total = dist_temp;
                    } else {
                        double diff_dist = dist - (distance - dist_total);
                        node_index = i - 1;
                        node_dist = diff_dist;
                        return GetStringPoint(line, i - 1, diff_dist, coord);
                    }
                }

                const geos::geom::Coordinate &startcoord = cs->front();
                coord = startcoord;
                node_index = 0;
                node_dist = 0.0;
            }

            return true;
        }

        bool GeosObjUtil::GetDiffPoint(const DCDivider * div, double distance, bool start, DCDividerNode * kdsnode) {
            if(div == nullptr || kdsnode == nullptr){
                return false;
            }

            shared_ptr<geos::geom::LineString> line = div->line_;
            geos::geom::Coordinate diff_coord;
            if(GetDiffPoint(line, distance, start, diff_coord)){
                char zone[4] = {0};
                shared_ptr<DCDividerNode> node = start ? div->nodes_.front() : div->nodes_.back();
                kd::automap::Coordinates::utmZone(node->coord_->y_,node->coord_->x_,zone);

                double lng, lat;
                kd::automap::Coordinates::utm2ll(diff_coord.y, diff_coord.x, zone, lat, lng);
                kdsnode->coord_ = make_shared<DCCoord>();
                kdsnode->coord_->x_ = lng;
                kdsnode->coord_->y_ = lat;
                kdsnode->coord_->z_ = diff_coord.z;
                return true;
            }

            return false;
        }


        bool GeosObjUtil::GetStringPoint(const shared_ptr<geos::geom::LineString> line,
                                         int node_index, double node_dist, geos::geom::Coordinate &coord) {

            if (node_index < 0 || node_index >= (line->getNumPoints() - 1)) {
                return false;
            }

            const geos::geom::CoordinateSequence *cs = line->getCoordinatesRO();
            const geos::geom::Coordinate &coord1 = cs->getAt(node_index);
            const geos::geom::Coordinate &coord2 = cs->getAt(node_index + 1);

            GetDifferenceCoord(coord1, coord2, node_dist, coord);

            return true;
        }

        void GeosObjUtil::GetDifferenceCoord(const geos::geom::Coordinate &coord1,
                                             const geos::geom::Coordinate &coord2,
                                             double distance, geos::geom::Coordinate &coord) {
            double total_dist = kd::automap::KDGeoUtil::distance(coord1.x, coord1.y, coord2.x, coord2.y);

            double ratio = distance / total_dist;
            coord.x = ratio * (coord2.x - coord1.x) + coord1.x;
            coord.y = ratio * (coord2.y - coord1.y) + coord1.y;
            coord.z = ratio * (coord2.z - coord1.z) + coord1.z;
        }

        bool GeosObjUtil::CheckFirstSegInvalid(vector<shared_ptr<DCCoord>> &lane_nodes, bool start, double angle_limit,
                                               double dist_limit) {
            if(lane_nodes.size() < 3){
                return false;
            }

            shared_ptr<DCCoord> node1, node2, node3;
            double angle_diff = 0.0;
            if (start) {
                node1 = lane_nodes[0];
                node2 = lane_nodes[1];
                node3 = lane_nodes[2];

                if(kd::automap::KDGeoUtil::distanceLL(node1->x_, node1->y_, node2->x_, node2->y_) > dist_limit){
                    return false;
                }

                angle_diff = kd::automap::KDGeoUtil::getAngleDiff(node1->x_, node1->y_, node2->x_, node2->y_,
                                                        node2->x_, node2->y_, node3->x_, node3->y_);

            } else {
                node1 = lane_nodes[lane_nodes.size()-1];
                node2 = lane_nodes[lane_nodes.size()-2];
                node3 = lane_nodes[lane_nodes.size()-3];

                if(kd::automap::KDGeoUtil::distanceLL(node1->x_, node1->y_, node2->x_, node2->y_) > dist_limit){
                    return false;
                }

                angle_diff = kd::automap::KDGeoUtil::getAngleDiff(node1->x_, node1->y_, node2->x_, node2->y_,
                                                                  node2->x_, node2->y_, node3->x_, node3->y_);
            }


            return (angle_diff > angle_limit);
        }
    }
}
