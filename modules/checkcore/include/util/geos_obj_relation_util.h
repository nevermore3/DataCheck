//
// Created by gaoyh on 19-2-7.
//

#ifndef AUTOHDMAP_COMPILE_GEOS_OBJ_RELATION_UTIL_H
#define AUTOHDMAP_COMPILE_GEOS_OBJ_RELATION_UTIL_H

#include <math.h>

#include "geos/geom/LineString.h"
#include "geos/geom/Polygon.h"
#include "geos/geom/Point.h"
#include "geos/geom/CoordinateSequence.h"

//core
#include "geom/geo_util.h"
#include <mvg/Coordinates.hpp>
#include "util/GeosObjUtil.h"
#include "util/GeometryUtil.h"

using namespace std;

class IntersectInfo {
public:
    int src_node_index_;

    float src_node_dist_;

    double src_node_z_;

    int dst_node_index_;

    float dst_node_dist_;

    double dst_node_z_;

    double coord_x_;

    double coord_y_;

    double coord_z_;
};

class OverlapInfo{
public:
    int start_node_index_;

    float start_node_dist_;

    int end_node_index_;

    float end_node_dist_;
};


class GeosObjRelationUtil {
public:

    /**
     * 判断两个点是否共点，要求水平距离和垂直距离都要小于 buffer_size
     * @param point1 第一个点
     * @param point2 第二个点
     * @param buffer_size 两个点的间距，单位为米
     * @return true代表为共点, false代表非共点
     */
    static bool PointIsConcurrent(const shared_ptr<geos::geom::Point> point1,
                                  const shared_ptr<geos::geom::Point> point2, float buffer_size);

    /**
     * 判断两条线是否相交，除判断水平方向相交外，还需要判断垂直方向上是否满足高度差限制。
     * @param src_line 线1
     * @param dst_line 线2
     * @param z_limit 相交时两个点Z值间距的判断
     * @param intersect_infos 交点信息
     */
    static void LineStringIntersect(const shared_ptr<geos::geom::LineString> src_line,
                                    const shared_ptr<geos::geom::LineString> dst_line,
                                    float z_limit, float min_limit,
                                    vector<shared_ptr<IntersectInfo>> &intersect_infos);

    static void LineStringOverlap(const shared_ptr<geos::geom::LineString> src_line,
                                    const shared_ptr<geos::geom::Geometry> buffer,
                                    const shared_ptr<geos::geom::LineString> dst_line, float z_limit,
                                    vector<shared_ptr<OverlapInfo>> &overlap_infos);

    static void LineStringOverlap(const shared_ptr<geos::geom::LineString> src_line,
                                  const shared_ptr<geos::geom::LineString> dst_line, float z_limit,
                                  const shared_ptr<OverlapInfo> ref_overlap,
                                  vector<shared_ptr<OverlapInfo>> &overlap_infos);

    static bool LineOverlapAllRange(const shared_ptr<geos::geom::LineString> line,
                                    const vector<shared_ptr<OverlapInfo>> &overlap_infos);

    /**
     * 判断几何对象是否同线对象相交，（本几何对象范围较小，其Z值可以由ref_z来代表）
     * @param geometry 几何对象
     * @param ref_z 几何对象相对的Z值
     * @param dst_line 判断的相交线
     * @param z_limit 高度相交时的Z差值限差
     * @return true代表相交， false代表不相交
     */
    static bool GeometryIntersectLineString(const shared_ptr<geos::geom::Geometry> geometry, double ref_z,
                                            const shared_ptr<geos::geom::LineString> dst_line, float z_limit);


    /**
     * 判断集合对象同点对象是否相交（本几何对象范围较小，其Z值可以由ref_z来代表）
     * @param geometry 几何对象
     * @param ref_z 几何对象相对的Z值
     * @param dst_point 判断的点
     * @param z_limit 高度相交时的Z差值限差
     * @return true代表相交， false代表不相交
     */
    static bool GeometryIntersectPoint(const shared_ptr<geos::geom::Geometry> geometry, double ref_z,
                                            const shared_ptr<geos::geom::Point> dst_point, float z_limit);


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


    //获得线的参考方向
    static double GetRefAngle(const shared_ptr<geos::geom::LineString> line, double distance, bool start);


    /**
     * 获得车道线的参考方向
     * @param line 参考线
     * @param distance 前后插值距离
     * @param start 起点到终点方向还是终点到起点方向
     * @param node_index 取值点的节点索引
     * @param node_dist 取值点距离node_index的距离
     * @return
     */
    static double GetReferDirection(const shared_ptr<geos::geom::LineString> line,
                                  double distance, bool start, int node_index, double node_dist);
    //获得一个节点距离起点或终点的距离
    static double GetNodeDistance(const shared_ptr<geos::geom::LineString> line, bool start, int node_index);

    static bool DirectionSame(double dir1, double dir2, double angle_limit = M_PI/4);


    /**
     * 获得线对象端点的延长线
     * @param line 要延长的线
     * @param distance 延长距离
     * @param is_start 起点延长还是终点延长
     * @param start 延长线的起点
     * @param end 延长线的终点
     * @return 操作是否成功
     */
    static bool GetLineStringExtendPoint(shared_ptr<geos::geom::LineString> line,
                                         float distance, bool is_start,
                                         geos::geom::Coordinate &start, geos::geom::Coordinate &end);

    static shared_ptr<geos::geom::Coordinate> GetExtendPoint(shared_ptr<geos::geom::Coordinate> start,
                                                             shared_ptr<geos::geom::Coordinate> end,
                                                             float distance, bool is_start);



    /**
     * 计算一个点相对于另外一个点的镜像点
     * @param coord1
     * @param coord2
     * @param mirror_coord
     * @return
     */
    static bool GetMirrorPoint(const geos::geom::Coordinate &coord1, const geos::geom::Coordinate &coord2,
            geos::geom::Coordinate &mirror_coord);


    /**
     * 获得线段的交点
     * @param start1 线段1的起点
     * @param end1 线段1的终点
     * @param start2 线段2的起点
     * @param end2 线段2的终点
     * @param intersect 线段交点
     * @return true代表有交点
     */
    static bool GetLinesIntersectPoint(const geos::geom::Coordinate & start1, const geos::geom::Coordinate & end1,
                                       const geos::geom::Coordinate & start2, const geos::geom::Coordinate & end2,
                                       geos::geom::Coordinate & intersect);



    /**
     * 计算点到点集的最近距离
     * @param cs 点集
     * @param PtA 待检测点
     * @param PtB 垂足点，或者线段的起点和终点
     * @param PtC 线段的两个坐标点，存储格式为x1,y1,x2,y2
     * @param min_index 最近线段节点的索引
     * @return 点到"垂足"的距离
     */
//    static double pt2LineDist(const geos::geom::CoordinateSequence *cs, double z_ref,
//                              double PtA[], double PtB[], double PtC[], int &min_index);

    /**
     * 计算点到点集的最近距离
     * @param cs 点集
     * @param PtA 待检测点
     * @param PtB 垂足点，或者线段的起点和终点
     * @param PtC 线段的两个坐标点，存储格式为x1,y1,x2,y2
     * @param z_limit 高度限制
     * @param min_index
     * @param position 1:PtA高于点集合 2:PtA低于点集合 0:默认值
     * @return
     */
    static double pt2LineDist(const geos::geom::CoordinateSequence *cs,
                              double PtA[], double PtB[], double PtC[], double z_limit, int &min_index, int position = 0);

    /**
     * 计算点到点集的最近距离
     * @param Line 点集合
     * @param zLine 高度集合
     * @param size 点数量
     * @param PtA 待检测点
     * @param PtB 垂足点，或者线段的起点和终点
     * @param PtC 线段的两个坐标点，存储格式为x1,y1,x2,y2
     * @param z_limit 高度限制
     * @param min_index
     * @return
     */
    static double pt2LineDist(double Line[], double zLine[], int size, double PtA[],
                                 double PtB[], double PtC[], double z_limit, int &min_index);
    /**
     * 计算点到点集的最近距离
     * @param cs 点集
     * @param coord 计算点
     * @param min_index 最近线段节点的索引
     * @return 点到"垂足"的距离
     */
    static double pt2LineDist(const geos::geom::CoordinateSequence *cs,
                const geos::geom::Coordinate * coord, int & min_index);

    /**
     * 计算一个点到一条线的垂足点信息
     * @param cs 点集
     * @param coord 要计算垂足点的坐标点
     * @param pedal_coord 垂足信息
     * @param node_index 垂足所在线的起点节点索引
     * @param node_dist 垂足距离所在线段起点的距离
     * @return 点到垂足到距离
     */
    static double pt2LineDist(const geos::geom::CoordinateSequence *cs,
                            const geos::geom::Coordinate * coord, geos::geom::Coordinate * pedal_coord,
                            int & node_index, double & node_dist);


    /**
     * 根据已知点，获得两点之间的插值点
     * @param coord1 参考点1
     * @param coord2 参考点2
     * @param distance 插值点距离参考点1的距离
     * @param coord 差指点坐标
     */
    static void GetDifferenceCoord(const geos::geom::Coordinate &coord1, const geos::geom::Coordinate &coord2,
                                   double distance, geos::geom::Coordinate &coord);



    static double GetMaxAngle(const shared_ptr<geos::geom::LineString> line1, bool line1_start,
                const shared_ptr<geos::geom::LineString> line2, bool line2_start);

    /**
     * 根据某个节点索引分别计算该点距离线首尾点的距离
     * @param line   原始线
     * @param index  节点索引
     * @param dis_start  返回距离首点距离
     * @param dis_end    返回距离尾点距离
     */
    static void GetDisToStratAndEndByIndex(const shared_ptr<geos::geom::LineString> line, int index,
                                           float& dis_start, float& dis_end);

    //获得两个节点之间的距离
    static double GetDistance(const shared_ptr<geos::geom::LineString> line, int nodex_index1, int node_index2);

    //获得一个参考点(由node_index和node_dist确认)到线的起点或终点的距离
    //start取true，则代表计算参考点到线起点的距离，false代表到线尾点的距离
    static double GetDistance(const shared_ptr<geos::geom::LineString> line, int node_index, double node_dist, bool start);

    //获得两个控制点之间的距离
    static double GetDistance(const shared_ptr<geos::geom::LineString> line, int node_index1,
                              double node_dist1, int node_index2, double node_dist2);

    static double GetDistanceLngLat(double lng1, double lat1, double lng2, double lat2);


    //检查点集合中的第一段是否是异常段，角度过大
    template <typename T> static bool CheckFirstSegInvalid(vector<T> &lane_nodes, bool start,
            double angle_limit, double dist_limit);

    //确定点集合的参考方向
    template <typename T> static double GetAngle(vector<T> &lane_nodes, bool start, double ref_dist, char * zone);


private:
    static bool GetStringPoint(const shared_ptr<geos::geom::LineString> line,
            int node_index, double node_dist, geos::geom::Coordinate & coord);

    static bool GetIntersectInfo(const shared_ptr<geos::geom::LineString> line,
                                 const geos::geom::Coordinate *ref_pt, int begin_index,
                                 int &node_index, float &node_dist, double &node_z);

    static bool GetPointIndex(const shared_ptr<geos::geom::LineString> line,
                              const geos::geom::Coordinate *coord, int start_index, int &node_index, float &node_dist);

    static bool GetDifferenceCoord(const shared_ptr<geos::geom::LineString> line,
                                   int node_index, float &node_dist, geos::geom::Coordinate &coord);



    /**
     * 根据坐标之间的关系，插值出第三个点的Z值
     * @param coord1
     * @param coord2
     * @param coord3
     * @return  Z值
     */
    static double GetDiffZ(const geos::geom::Coordinate &coord1, const geos::geom::Coordinate &coord2,
            geos::geom::Coordinate &coord3);

    static bool ValueBetween(double value1, double value2, double value3);
};

#endif //AUTOHDMAP_COMPILE_GEOS_OBJ_RELATION_UTIL_H
