//
// Created by zhangxingang on 19-2-18.
//

#ifndef AUTOHDMAP_DATACHECK_GEOSOBJUTIL_H
#define AUTOHDMAP_DATACHECK_GEOSOBJUTIL_H
#include <geom/geo_util.h>
#include <mvg/Coordinates.hpp>
#include "data/DividerGeomModel.h"
#include "KDGeoUtil.hpp"

using namespace geos::geom;

namespace kd {
    namespace dc {
        class GeosObjUtil {
        public :
            static shared_ptr<geos::geom::Coordinate> create_coordinate(const shared_ptr<DCDividerNode> &ptr_node,
                                                                       char *zone0, bool is_z = false);

            static shared_ptr<geos::geom::Coordinate> create_coordinate(const shared_ptr<DCCoord> &ptr_node,
                                                                       char *zone0, bool is_z = false);

            static shared_ptr<geos::geom::LineString> create_line_string(const vector<shared_ptr<DCCoord>> &nodes);

            static shared_ptr<DCCoord> get_coord(const shared_ptr<geos::geom::Coordinate> &ptr_coord,
                                                                 char *zone0, bool is_z = false);

            static shared_ptr<geos::geom::Point> CreatePoint(const shared_ptr<DCCoord> &node);

            static shared_ptr<geos::geom::Point> CreatePointUTM(double utmX, double utmY, double z);

            static shared_ptr<geos::geom::LineString> CreateLineString(const vector<shared_ptr<DCDividerNode>> &coords);
            /**
             * 获取点集合长度
             * @param ptr_coords
             * @return
             */
            static double get_length_of_coords(const vector<shared_ptr<DCCoord>> &ptr_coords);

            /**
             * 获取点集合长度
             * @param ptr_coords
             * @return
             */
            static double get_length_of_coords(const vector<shared_ptr<geos::geom::Coordinate>> &ptr_coords);

            /**
             * 获取结点长度
             */
            static double get_length_of_node(shared_ptr<DCCoord> node1, shared_ptr<DCCoord> node2);

            /**
             * 判断是否是相同点
             * @param coord1
             * @param coord2
             * @return
             */
            static bool is_same_coord(Coordinate coord1, Coordinate coord2, double precise = 0.0001);

            /**
             * 判断是否是相同点
             * @param coord1
             * @param coord2
             * @return
             */
            static bool is_same_coord(const shared_ptr<DCCoord> &coord1, const shared_ptr<DCCoord> &coord2,
                                      double precise = 0.0000001);
            static bool has_same_coord(CoordinateSequence *line,Coordinate coord);
            /**
             * 在线上找到指定距离的插值点
             * @param line 参考线对象
             * @param distance 插值距离
             * @param start 从起点插值，还是终点插值
             * @return 计算出来的对象
             */
            static bool GetDiffPoint(const shared_ptr<geos::geom::LineString> line,
                                     double distance, bool start, geos::geom::Coordinate &coord);

            static bool GetDiffPoint(const shared_ptr<geos::geom::LineString> line, int base_index,
                                     double distance, bool start, geos::geom::Coordinate &coord,
                                     int & node_index, double & node_dist);

            static bool GetDiffPoint(const DCDivider * div, double distance, bool start, DCDividerNode * kdsnode);

            //确定点集合的参考方向
            static double GetAngle(vector<shared_ptr<DCCoord>> &lane_nodes, bool start, double ref_dist, char *zone);

            static double GetVerticleDistance(shared_ptr<geos::geom::LineString> line,
                                              shared_ptr<geos::geom::Point> point);
/**
             * 根据 start, end所确定的方向，创建三角形对象
             * @param startx 开始点坐标X
             * @param starty 开始点坐标Y
             * @param endx  结束点坐标X
             * @param endy  结束点坐标Y
             * @param trace_Angle 三角形的张角的1/2
             * @param trace_Length 三角形张角的边距离开始点的距离
             * @return  三角形对象
             */
            static shared_ptr<geos::geom::Polygon>
            CreateTriGeometry(double startx, double starty, double endx, double endy,
                              double ref_z, double trace_Angle, double trace_Length);
            static void rotate(double centx, double centy, double x, double y, double radius, double *newx, double *newy);
        private:
            static bool CheckFirstSegInvalid(vector<shared_ptr<DCCoord>> &lane_nodes, bool start, double angle_limit,
                                             double dist_limit);

            static bool GetStringPoint(const shared_ptr<geos::geom::LineString> line,
                                       int node_index, double node_dist, geos::geom::Coordinate &coord);

            /**
            * 根据已知点，获得两点之间的插值点
            * @param coord1 参考点1
            * @param coord2 参考点2
            * @param distance 插值点距离参考点1的距离
            * @param coord 差指点坐标
            */
            static void GetDifferenceCoord(const geos::geom::Coordinate &coord1, const geos::geom::Coordinate &coord2,
                                           double distance, geos::geom::Coordinate &coord);
        };
    }
}


#endif //AUTOHDMAP_DATACHECK_GEOSOBJUTIL_H
