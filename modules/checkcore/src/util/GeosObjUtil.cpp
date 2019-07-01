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


namespace kd {
    namespace dc {
        shared_ptr<geos::geom::Coordinate> GeosObjUtil::create_coordinate(const shared_ptr<DCDividerNode> &ptr_node,
                                                                         char *zone0, bool is_z) {
            shared_ptr<geos::geom::Coordinate> ret = nullptr;
            double X0, Y0;
            double x = ptr_node->coord_.lng_;
            double y = ptr_node->coord_.lat_;
            double z = ptr_node->coord_.z_;
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
            double x = ptr_node->lng_;
            double y = ptr_node->lat_;
            double z = ptr_node->z_;
            kd::automap::Coordinates::ll2utm(y, x, X0, Y0, zone0);
            if (is_z) {
                ret = make_shared<geos::geom::Coordinate>(X0, Y0);
            } else {
                ret = make_shared<geos::geom::Coordinate>(X0, Y0, z);
            }
            return ret;
        }

        shared_ptr<DCCoord> GeosObjUtil::get_coord(const shared_ptr<geos::geom::Coordinate> &ptr_coord,
                                                    char *zone0, bool is_z) {
            shared_ptr<DCCoord> ptr_node = make_shared<DCCoord>();
            double lat = 0;
            double lon = 0;
            double y = ptr_coord->y;
            double x = ptr_coord->x;
            kd::automap::Coordinates::utm2ll(y, x, zone0, lat, lon);
            ptr_node->lng_ = lon;
            ptr_node->lat_ = lat;
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
                double x = ptr_coord->lng_;
                double y = ptr_coord->lat_;
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
            if (fabs(coord1->lng_ - coord2->lng_) < precise && fabs(coord1->lat_ - coord2->lat_) < precise &&
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
    }
}
