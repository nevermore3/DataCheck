//
// Created by gaoyh on 19-2-7.
//

#include "util/geos_obj_relation_util.h"

#include <glog/logging.h>

//core
#include "geom/geo_util.h"
#include <mvg/Coordinates.hpp>

#include "util/GeometryUtil.h"

const double kZLimitRate = 0.2;
const double kZLimitMin = 1.5;

bool GeosObjRelationUtil::PointIsConcurrent(const shared_ptr<geos::geom::Point> point1,
                                            const shared_ptr<geos::geom::Point> point2, float buffer_size) {

    if (point1 == nullptr || point2 == nullptr) {
        return false;
    }

    const geos::geom::Coordinate *coord1 = point1->getCoordinate();
    const geos::geom::Coordinate *coord2 = point2->getCoordinate();

    double distance = GeometryUtil::getDistance(coord1->x, coord1->y, coord2->x, coord2->y);
    if (distance < buffer_size) {
        double z_diff = fabs(coord1->z - coord2->z);
        if (z_diff < buffer_size) {
            return true;
        }
    }

    return false;
}

void GeosObjRelationUtil::LineStringIntersect(const shared_ptr<geos::geom::LineString> src_line,
                                              const shared_ptr<geos::geom::LineString> dst_line,
                                              float z_limit, float min_limit,
                                              vector<shared_ptr<IntersectInfo>> &intersect_infos) {
    if (src_line == nullptr || src_line->getNumPoints() < 2 ||
        dst_line == nullptr || dst_line->getNumPoints() < 2) {
        return;
    }

    //二维相交判断
    if (!src_line->intersects(dst_line.get())) {
        return;
    }

    shared_ptr<geos::geom::Geometry> geometry(src_line->intersection(dst_line.get()));
    if (geometry != nullptr) {
        geos::geom::GeometryTypeId typeId = geometry->getGeometryTypeId();
        switch (typeId) {
            case geos::geom::GEOS_POINT: {
                const geos::geom::Coordinate *intersect = geometry->getCoordinate();
                //cout << "point " << intersect->x << "," << intersect->y << "," << intersect->z << endl;
                shared_ptr<IntersectInfo> intersectInfo = std::make_shared<IntersectInfo>();

                int start_index = 0;

                if (GetIntersectInfo(src_line, intersect, start_index, intersectInfo->src_node_index_,
                                     intersectInfo->src_node_dist_, intersectInfo->src_node_z_) &&
                    GetIntersectInfo(dst_line, intersect, start_index, intersectInfo->dst_node_index_,
                                     intersectInfo->dst_node_dist_, intersectInfo->dst_node_z_)) {
                    double zdiff = fabs(intersectInfo->src_node_z_ - intersectInfo->dst_node_z_);

                    double rate_limit = z_limit * intersectInfo->src_node_dist_;
                    if (rate_limit < min_limit) {
                        rate_limit = min_limit;
                    }

                    if (zdiff < rate_limit) {
                        intersectInfo->coord_x_ = intersect->x;
                        intersectInfo->coord_y_ = intersect->y;
                        intersectInfo->coord_z_ = intersect->z;
                        intersect_infos.emplace_back(intersectInfo);
                    }
                }
            }

                break;
            case geos::geom::GEOS_MULTIPOINT: {
                int num = geometry->getNumGeometries();

                for (int i = 0; i < num; i++) {
                    const geos::geom::Geometry *geom = geometry->getGeometryN(i);

                    const geos::geom::Coordinate *intersect = geom->getCoordinate();
                    shared_ptr<IntersectInfo> intersectInfo = std::make_shared<IntersectInfo>();

                    int src_start_index = 0;
                    bool get_match_coord = false;
                    while (GetIntersectInfo(src_line, intersect, src_start_index, intersectInfo->src_node_index_,
                                            intersectInfo->src_node_dist_, intersectInfo->src_node_z_)) {

                        int dst_start_index = 0;
                        while (GetIntersectInfo(dst_line, intersect, dst_start_index, intersectInfo->dst_node_index_,
                                                intersectInfo->dst_node_dist_, intersectInfo->dst_node_z_)) {

                            double zdiff = fabs(intersectInfo->src_node_z_ - intersectInfo->dst_node_z_);

                            double rate_limit = z_limit * intersectInfo->src_node_dist_;
                            if (rate_limit < min_limit) {
                                rate_limit = min_limit;
                            }

                            if (zdiff < rate_limit) {
                                intersectInfo->coord_x_ = intersect->x;
                                intersectInfo->coord_y_ = intersect->y;
                                intersectInfo->coord_z_ = intersect->z;
                                intersect_infos.emplace_back(intersectInfo);

                                get_match_coord = true;
                                break;
                            } else {
                                dst_start_index = intersectInfo->dst_node_index_ + 1;
                            }
                        }

                        if (get_match_coord) {
                            break;
                        } else {
                            src_start_index = intersectInfo->src_node_index_ + 1;
                        }
                    }
                }
            }

                break;
            default:
                break;
        }
    }
}

void GeosObjRelationUtil::LineStringOverlap(const shared_ptr<geos::geom::LineString> src_line,
                                            const shared_ptr<geos::geom::Geometry> buffer,
                                            const shared_ptr<geos::geom::LineString> dst_line, float z_limit,
                                            vector<shared_ptr<OverlapInfo>> &overlap_infos) {

    if (src_line == nullptr || src_line->getNumPoints() < 2 || buffer == nullptr ||
        dst_line == nullptr || dst_line->getNumPoints() < 2) {
        return;
    }

    if (/*src_line->intersects(dst_line.get()) || */!buffer->intersects(dst_line.get())) {
        return;
    }

    shared_ptr<geos::geom::Geometry> geometry(buffer->intersection(dst_line.get()));
    if (geometry != nullptr) {
        geos::geom::GeometryTypeId typeId = geometry->getGeometryTypeId();

        switch (typeId) {
            case geos::geom::GEOS_LINESTRING: {
//                const geos::geom::CoordinateSequence *cs = dst_line->getCoordinatesRO();
//                for (int i = 0; i < cs->size(); i++) {
//                    const geos::geom::Coordinate &coord = cs->getAt(i);
//                    cout << "dst " << i << ", " << coord.x << "," << coord.y << "," << coord.z << endl;
//                }
//
//                size_t geom_count = geometry->getNumGeometries();
//                size_t coord_count = geometry->getNumPoints();
//                cout << "geom_count " << geom_count << ", coord_count " << coord_count << endl;

                geos::geom::CoordinateSequence *cs2 = geometry->getCoordinates();
//                for (int i = 0; i < cs2->size(); i++) {
//                    const geos::geom::Coordinate &coord = cs2->getAt(i);
//                    cout << "intersect " << i << ", " << coord.x << "," << coord.y << "," << coord.z << endl;
//                }

                //获取不考虑Z值情况下的线段叠加关系
                shared_ptr<OverlapInfo> org_overlap = make_shared<OverlapInfo>();
                const geos::geom::Coordinate & start = cs2->front();
                const geos::geom::Coordinate & end = cs2->back();
                GetPointIndex(dst_line, &start, 0, org_overlap->start_node_index_, org_overlap->start_node_dist_);
                GetPointIndex(dst_line, &end, 0, org_overlap->end_node_index_, org_overlap->end_node_dist_);
                overlap_infos.emplace_back(org_overlap);
                delete cs2;

                //考虑Z值的情况下，最终的得加信息
                LineStringOverlap(src_line, dst_line, z_limit, org_overlap, overlap_infos);

//
//                const geos::geom::LineString * line = reinterpret_cast<const geos::geom::LineString*>(geometry->getGeometryN(0));
//
//                const geos::geom::CoordinateSequence *cs2 = line->getCoordinatesRO();
//                for( int i = 0 ; i < cs2->size() ; i ++){
//                    const geos::geom::Coordinate & coord = cs2->getAt(i);
//                    cout << "intersect " << i << ", " << coord.x << "," << coord.y << "," << coord.z << endl;
//                }
                //geometry->getCoord
            }
                break;
            case geos::geom::GEOS_MULTILINESTRING:
                break;
            default:
                break;
        }

//        cout << "type" << typeId << endl;
    }

}

void GeosObjRelationUtil::LineStringOverlap(const shared_ptr<geos::geom::LineString> src_line,
                                            const shared_ptr<geos::geom::LineString> dst_line, float z_limit,
                                            const shared_ptr<OverlapInfo> ref_overlap,
                                            vector<shared_ptr<OverlapInfo>> &overlap_infos) {

    overlap_infos.clear();

    const geos::geom::CoordinateSequence * src_cs = src_line->getCoordinatesRO();
    const geos::geom::CoordinateSequence * dst_cs = dst_line->getCoordinatesRO();

    vector<pair<int,double>> overlap_details;

    //前提：每两个点的的距离并不长（10米之内），高差不大于1m
    for(int i = ref_overlap->start_node_index_ ; i <= ref_overlap->end_node_index_ ; i ++){

        bool startNode = (i == ref_overlap->start_node_index_) ? true : false;

        geos::geom::Coordinate check_coord;
        double check_dist = 0.0;
        if(startNode){
            GetStringPoint(dst_line, i, ref_overlap->start_node_dist_, check_coord);
            check_dist = ref_overlap->start_node_dist_;
        }else{
            check_coord = dst_cs->getAt(i);
        }

        //获得垂足点的坐标信息
        double PtA[3], PtB[2], PtC[4];
        int min_index;
        PtA[0] = check_coord.x;
        PtA[1] = check_coord.y;
        PtA[2] = check_coord.z;
        pt2LineDist(src_cs, PtA, PtB, PtC, kZLimitRate, min_index);
        double dist = GeometryUtil::getDistance(PtB[0], PtB[1], PtC[0], PtC[1]);
        geos::geom::Coordinate diff_coord;
        GetStringPoint(src_line, min_index, dist, diff_coord);

        //确定检查点和垂足点的Z值差异
        double zdiff = fabs(check_coord.z - diff_coord.z);
        if(zdiff < z_limit){
            overlap_details.emplace_back(pair<int,double>(i, check_dist));
        }
    }

    //最后一个点
    {
        geos::geom::Coordinate check_coord;
        GetStringPoint(dst_line, ref_overlap->end_node_index_, ref_overlap->end_node_dist_, check_coord);
        double check_dist = ref_overlap->end_node_dist_;

        //获得垂足点的坐标信息
        double PtA[3], PtB[2], PtC[4];
        int min_index;
        PtA[0] = check_coord.x;
        PtA[1] = check_coord.y;
        PtA[2] = check_coord.z;
        pt2LineDist(src_cs, PtA, PtB, PtC, kZLimitRate, min_index);
        double dist = GeometryUtil::getDistance(PtB[0], PtB[1], PtC[0], PtC[1]);
        geos::geom::Coordinate diff_coord;
        GetStringPoint(src_line, min_index, dist, diff_coord);

        //确定检查点和垂足点的Z值差异
        double zdiff = fabs(check_coord.z - diff_coord.z);
        if(zdiff < z_limit){
            overlap_details.emplace_back(pair<int,double>(ref_overlap->end_node_index_, check_dist));
        }
    }

    //处理匹配的结果
    int detail_num = overlap_details.size();
    if(detail_num < 2){
        return;
    }

    int head_index = 0;
    while(head_index < (detail_num - 1)){
        shared_ptr<OverlapInfo> info = make_shared<OverlapInfo>();
        info->start_node_index_ = overlap_details[head_index].first;
        info->start_node_dist_ = overlap_details[head_index].second;

        int next_index = head_index + 1;
        int last_node_index = info->start_node_index_;
        double last_node_dist = info->start_node_dist_;

        bool find_match = false;
        int match_index = -1;
        while(next_index < detail_num){

            pair<int, double> cur = overlap_details[next_index];
            int index_diff = cur.first - last_node_index;
            if(index_diff == 0 || index_diff == 1){
                find_match = true;
                match_index = next_index;

                next_index ++;
                last_node_index = cur.first;
                last_node_dist = cur.second;
            }
        }

        if(find_match){
            info->end_node_index_ = last_node_index;
            info->end_node_dist_ = last_node_dist;

            overlap_infos.emplace_back(info);

            head_index = next_index;
        }else{
            head_index = next_index;
        }
    }
}

bool GeosObjRelationUtil::LineOverlapAllRange(const shared_ptr<geos::geom::LineString> line,
                                              const vector<shared_ptr<OverlapInfo>> &overlap_infos) {
    if(overlap_infos.size() == 0 || overlap_infos.size() > 1){
        return false;
    }

    shared_ptr<OverlapInfo> overlapInfo = overlap_infos.front();

    //check begin
    if ( overlapInfo->start_node_index_ > 0  ||
            (overlapInfo->start_node_index_ == 0 && overlapInfo->start_node_dist_ > 1)) {
        return false;
    }

    //check end1
    if (overlapInfo->end_node_index_ < line->getNumPoints() - 2) {
        return false;
    }

    //check end2
    if(overlapInfo->end_node_index_ == (line->getNumPoints() - 2))
    {
        const geos::geom::CoordinateSequence * cs = line->getCoordinatesRO();
        const geos::geom::Coordinate coord1 = cs->getAt(cs->size()-2);
        const geos::geom::Coordinate coord2 = cs->getAt(cs->size()-1);
        double length = GeometryUtil::getDistance(coord1.x, coord1.y, coord2.x, coord2.y);

        if(overlapInfo->end_node_dist_ < (length-1)){
            return false;
        }
    }

    return true;
}

bool GeosObjRelationUtil::GeometryIntersectLineString(const shared_ptr<geos::geom::Geometry> geometry, double ref_z,
                                                      const shared_ptr<geos::geom::LineString> dst_line,
                                                      float z_limit) {

    if (geometry == nullptr || dst_line == nullptr) {
        return false;
    }

    if (!geometry->intersects(dst_line.get())) {
        return false;
    }

    bool zdiff_valid = true;
    shared_ptr<geos::geom::Geometry> inter_geometry(geometry->intersection(dst_line.get()));
    if (inter_geometry != nullptr) {
        geos::geom::GeometryTypeId typeId = inter_geometry->getGeometryTypeId();

        switch (typeId) {
            case geos::geom::GEOS_LINESTRING: {

                geos::geom::CoordinateSequence *cs2 = inter_geometry->getCoordinates();

                for (int i = 0; i < cs2->size(); i++) {
                    const geos::geom::Coordinate &coord = cs2->getAt(i);
                    double zdiff = fabs(coord.z - ref_z);
                    if (zdiff > z_limit) {
                        zdiff_valid = false;
                        break;
                    }
                }
                delete cs2;
            }
                break;
            default: {
                LOG(WARNING) << "unhandle PointBufferIntersectLineString status.";
                zdiff_valid = false;
            }
                break;
        }
    }

    return zdiff_valid;
}

bool GeosObjRelationUtil::GeometryIntersectPoint(const shared_ptr<geos::geom::Geometry> geometry, double ref_z,
                                                 const shared_ptr<geos::geom::Point> dst_point, float z_limit) {
    if (geometry == nullptr || dst_point == nullptr) {
        return false;
    }

    if (!geometry->contains(dst_point.get())) {
        return false;
    }

    double zdiff = fabs(dst_point->getCoordinate()->z - ref_z);
    if (zdiff > z_limit) {
        return false;
    }

    return true;
}

bool GeosObjRelationUtil::GetStringPoint(const shared_ptr<geos::geom::LineString> line,
                                         int node_index, double node_dist, geos::geom::Coordinate &coord) {

    if(node_index < 0 || node_index >= (line->getNumPoints()-1)){
        return false;
    }

    const geos::geom::CoordinateSequence * cs = line->getCoordinatesRO();
    const geos::geom::Coordinate &coord1 = cs->getAt(node_index);
    const geos::geom::Coordinate &coord2 = cs->getAt(node_index + 1);

    GetDifferenceCoord(coord1, coord2, node_dist, coord);

    return true;
}

bool GeosObjRelationUtil::GetIntersectInfo(const shared_ptr<geos::geom::LineString> line,
                                           const geos::geom::Coordinate *ref_pt, int begin_index,
                                           int &node_index, float &node_dist, double &node_z) {
    if (GetPointIndex(line, ref_pt, begin_index, node_index, node_dist)) {

        geos::geom::Coordinate diff_coord;
        if (GetDifferenceCoord(line, node_index, node_dist, diff_coord)) {

            node_z = diff_coord.z;
            return true;
        }
    }

    return false;
}

bool GeosObjRelationUtil::
GetPointIndex(const shared_ptr<geos::geom::LineString> line,
              const geos::geom::Coordinate *coord, int start_index, int &node_index, float &node_dist) {
    if (line == nullptr || coord == nullptr) {
        return false;
    }

    const geos::geom::CoordinateSequence *cs = line->getCoordinatesRO();
    int coord_num = cs->size();
    for (int i = start_index; i < coord_num - 1; i++) {
        const geos::geom::Coordinate &coord1 = cs->getAt(i);
        const geos::geom::Coordinate &coord2 = cs->getAt(i + 1);

        if (ValueBetween(coord1.x, coord2.x, coord->x) &&
            ValueBetween(coord1.y, coord2.y, coord->y)) {

            node_index = i;
            node_dist = GeometryUtil::getDistance(coord1.x, coord1.y, coord->x, coord->y);

            return true;
        }
    }
    return false;
}

bool GeosObjRelationUtil::GetDifferenceCoord(const shared_ptr<geos::geom::LineString> line,
                                             int node_index, float &node_dist, geos::geom::Coordinate &coord) {

    if (line == nullptr || node_index < 0 || node_index >= line->getNumPoints() - 1) {
        return false;
    }

    const geos::geom::CoordinateSequence *cs = line->getCoordinatesRO();

    const geos::geom::Coordinate &coord1 = cs->getAt(node_index);
    const geos::geom::Coordinate &coord2 = cs->getAt(node_index + 1);

    GetDifferenceCoord(coord1, coord2, node_dist, coord);

    return true;
}

void GeosObjRelationUtil::GetDifferenceCoord(const geos::geom::Coordinate &coord1,
                                             const geos::geom::Coordinate &coord2,
                                             double distance, geos::geom::Coordinate &coord) {
    double total_dist = GeometryUtil::getDistance(coord1.x, coord1.y, coord2.x, coord2.y);

    double ratio = distance / total_dist;
    coord.x = ratio * (coord2.x - coord1.x) + coord1.x;
    coord.y = ratio * (coord2.y - coord1.y) + coord1.y;
    coord.z = ratio * (coord2.z - coord1.z) + coord1.z;
}


double GeosObjRelationUtil::GetMaxAngle(const shared_ptr<geos::geom::LineString> line1, bool line1_start,
                                        const shared_ptr<geos::geom::LineString> line2, bool line2_start) {

    //s1 --- e1               s1   e1
    //------>------------>--------->

    //获取参考点
    geos::geom::Coordinate s1, e1, s2, e2;
    const geos::geom::CoordinateSequence *cs1 = line1->getCoordinatesRO();
    if(line1_start){
        s1 = cs1->front();
        e1 = cs1->getAt(1);
    }else{
        s1 = cs1->getAt(cs1->size() - 2);
        e1 = cs1->back();
    }

    const geos::geom::CoordinateSequence *cs2 = line2->getCoordinatesRO();
    if(line2_start){
        s2 = cs2->front();
        e2 = cs2->getAt(1);
    }else{
        s2 = cs2->getAt(cs1->size() - 2);
        e2 = cs2->back();
    }

    if(line1_start && !line2_start){
        double angle_diff1 = GeometryUtil::getAngleDiff(s2.x, s2.y, e2.x, e2.y, e2.x, e2.y, s1.x, s1.y);
        double angle_diff2 = GeometryUtil::getAngleDiff(e2.x, e2.y, s1.x, s1.y, s1.x, s1.y, e1.x, e1.y);

        return max(angle_diff1, angle_diff2);
    }else if(!line1_start && line2_start){
        double angle_diff1 = GeometryUtil::getAngleDiff(s1.x, s1.y, e1.x, e1.y, e1.x, e1.y, s2.x, s2.y);
        double angle_diff2 = GeometryUtil::getAngleDiff(e1.x, e1.y, s2.x, s2.y, s2.x, s2.y, e2.x, e2.y);

        return max(angle_diff1, angle_diff2);
    }else {
        return M_PI * 2;
    }
}

double GeosObjRelationUtil::GetDiffZ(const geos::geom::Coordinate &coord1, const geos::geom::Coordinate &coord2,
                                   geos::geom::Coordinate &coord3) {

    double dist1 = GeometryUtil::getDistance(coord1.x, coord1.y, coord2.x, coord2.y);
    double dist2 = GeometryUtil::getDistance(coord1.x, coord1.y, coord3.x, coord3.y);

    if(dist1 <= 0.000001){
        return coord2.z;
    }

    double diff_z = dist2 * (coord2.z-coord1.z) / dist1 + coord1.z;
    return diff_z;
}

bool GeosObjRelationUtil::ValueBetween(double value1, double value2, double value3) {

    if ((value1 <= value3 && value3 <= value2) ||
        (value1 >= value3 && value3 >= value2)) {
        return true;
    }
    return false;
}


//double GeosObjRelationUtil::pt2LineDist(const geos::geom::CoordinateSequence *cs, double z_ref, double PtA[],
//                                        double PtB[], double PtC[], int &min_index) {
//
//    if (cs->size() < 2 || !PtB || !PtC) {
//        return -1;
//    }
//
//    double minDistance = 99999999999;
//    double closestPoint[2];
//    double LineSeg[4];
//    int minIndex = -1;
//    for (int i = 0; i < cs->size() - 1; i++) {
//        const geos::geom::Coordinate &start = cs->getAt(i);
//        const geos::geom::Coordinate &end = cs->getAt(i + 1);
//
//        double z_mean = (start.z + end.z) / 2.0;
//        double zdiff = fabs(z_mean - z_ref);
//        if(zdiff > 2.0){
//            continue;
//        }
//
//        LineSeg[0] = start.x;
//        LineSeg[1] = start.y;
//        LineSeg[2] = end.x;
//        LineSeg[3] = end.y;
//        //
//        double dis = geo::geo_util::pt2LineSegmentDist(LineSeg, PtA, PtB);
//        if (dis < minDistance) {
//            minDistance = dis;
//            closestPoint[0] = PtB[0];
//            closestPoint[1] = PtB[1];
//            PtC[0] = LineSeg[0];
//            PtC[1] = LineSeg[1];
//            PtC[2] = LineSeg[2];
//            PtC[3] = LineSeg[3];
//            //
//            minIndex = i;
//        }
//    }
//    min_index = minIndex;
//    PtB[0] = closestPoint[0];
//    PtB[1] = closestPoint[1];
//    //
//    return sqrt((PtA[0] - closestPoint[0]) * (PtA[0] - closestPoint[0]) +
//                (PtA[1] - closestPoint[1]) * (PtA[1] - closestPoint[1]));
//}

double GeosObjRelationUtil::pt2LineDist(const geos::geom::CoordinateSequence *cs, double PtA[], double PtB[],
                                        double PtC[], double z_limit, int &min_index, int position) {
    if (cs->size() < 2 || !PtB || !PtC) {
        return -1;
    }

    double minDistance = DBL_MAX;
    double closestPoint[2];
    double LineSeg[4];
    int minIndex = -1;
    double z = PtA[2];
    bool flag = false;
    for (int i = 0; i < cs->size() - 1; i++) {
        const geos::geom::Coordinate &start = cs->getAt(i);
        const geos::geom::Coordinate &end = cs->getAt(i + 1);
        double find_z = (start.z + end.z) / 2;

        LineSeg[0] = start.x;
        LineSeg[1] = start.y;
        LineSeg[2] = end.x;
        LineSeg[3] = end.y;

        double sline = GeometryUtil::UT_GetMapDistance(LineSeg[0], LineSeg[1], LineSeg[2], LineSeg[3]);

        //
        double dis = geo::geo_util::pt2LineSegmentDist(LineSeg, PtA, PtB);
        double real_dis = sqrt(dis);
        double comp_z = z_limit * real_dis;
        if (comp_z < kZLimitMin) {
            comp_z = kZLimitMin;
        }
        if (dis < minDistance) {
            double bline = GeometryUtil::UT_GetMapDistance(LineSeg[0], LineSeg[1], PtB[0], PtB[1]);
            if (fabs(sline) > 0.00000001) {
                find_z = bline / sline * (end.z - start.z) + start.z;
            }

            if (position == 0) {
                if(fabs(z - find_z) > comp_z){
                    continue; // 考虑高度差
                }
            } else if (position == 1) {
                if(z - find_z > comp_z || z - find_z < 0){
                    continue; // 考虑高度差
                }
            } else if (position == 2) {
                if(find_z - z> comp_z || find_z - z < 0){
                    continue; // 考虑高度差
                }
            } else {
                if(fabs(z - find_z) > comp_z){
                    continue; // 考虑高度差
                }
            }

            minDistance = dis;
            closestPoint[0] = PtB[0];
            closestPoint[1] = PtB[1];
            PtC[0] = LineSeg[0];
            PtC[1] = LineSeg[1];
            PtC[2] = LineSeg[2];
            PtC[3] = LineSeg[3];
            //
            minIndex = i;
            flag = true;
        }
    }

    double ret = 0;

    if (flag) {
        min_index = minIndex;
        PtB[0] = closestPoint[0];
        PtB[1] = closestPoint[1];
        ret = sqrt((PtA[0] - closestPoint[0]) * (PtA[0] - closestPoint[0]) +
                    (PtA[1] - closestPoint[1]) * (PtA[1] - closestPoint[1]));
    } else {
        const geos::geom::Coordinate &start = cs->front();
        const geos::geom::Coordinate &end = cs->back();
        double closest_point0 = sqrt((start.x - PtA[0]) * (start.x - PtA[0]) +
                                     (start.y - PtA[1]) * (start.y - PtA[1]));
        double closest_point1 = sqrt((end.x - PtA[0]) * (end.x - PtA[0]) +
                                     (end.y - PtA[1]) * (end.y - PtA[1]));

        if (closest_point0 < closest_point1) {
            min_index = 0;
            PtC[0] = start.x;
            PtC[1] = start.y;
            PtC[2] = end.x;
            PtC[3] = end.y;
            PtB[0] = start.x;
            PtB[1] = start.y;
            ret = closest_point0;
        } else {
            min_index = cs->size() - 1;
            PtC[0] = start.x;
            PtC[1] = start.y;
            PtC[2] = end.x;
            PtC[3] = end.y;
            PtB[0] = end.x;
            PtB[1] = end.y;
            ret = closest_point1;
        }
    }
    return ret;
}

double GeosObjRelationUtil::pt2LineDist(const geos::geom::CoordinateSequence *cs,
                          const geos::geom::Coordinate * coord, int & min_index){
    double PtA[3] = {0};
    double PtB[2], PtC[4];
    PtA[0] = coord->x;
    PtA[1] = coord->y;
    PtA[2] = coord->z;

    return pt2LineDist(cs, PtA, PtB, PtC, kZLimitRate, min_index);
}

double GeosObjRelationUtil::pt2LineDist(const geos::geom::CoordinateSequence *cs,
                                      const geos::geom::Coordinate *coord, geos::geom::Coordinate *pedal_coord,
                                      int &node_index, double &node_dist) {
    int min_index;
    double PtA[3] = {0};
    double PtB[2], PtC[4];
    PtA[0] = coord->x;
    PtA[1] = coord->y;
    PtA[2] = coord->z;

    double length = pt2LineDist(cs, PtA, PtB, PtC, kZLimitRate, min_index);

    node_index = min_index;
    node_dist = GeometryUtil::getDistance(PtB[0], PtB[1], PtC[0], PtC[1]);

    const geos::geom::Coordinate &coord1 = cs->getAt(min_index);
    const geos::geom::Coordinate &coord2 = cs->getAt(min_index+1);

    GetDifferenceCoord(coord1, coord2, node_dist, *pedal_coord);
    return length;
}


bool GeosObjRelationUtil::GetDiffPoint(const shared_ptr<geos::geom::LineString> line,
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

bool GeosObjRelationUtil::GetDiffPoint(const shared_ptr<geos::geom::LineString> line, int base_index,
                                       double distance, bool start, geos::geom::Coordinate &coord,
                                       int & node_index, double & node_dist) {
    const geos::geom::CoordinateSequence *cs = line->getCoordinatesRO();

    if(start){
        //从起点开始累计长度
        double dist_total = 0.0;
        for(int i = base_index ; i < cs->size()-1 ; i ++ ){
            const geos::geom::Coordinate &coord1 = cs->getAt(i);
            const geos::geom::Coordinate &coord2 = cs->getAt(i + 1);
            double dist = GeometryUtil::getDistance(&coord1, &coord2);

            double dist_temp = dist_total + dist;
            if(dist_temp < distance){
                dist_total = dist_temp;
            }else{
                double diff_dist = distance - dist_total;
                node_index = i;
                node_dist = diff_dist;
                return GetStringPoint(line, i, diff_dist, coord);
            }
        }

        const geos::geom::Coordinate & endcoord = cs->back();
        coord = endcoord;
        node_index = cs->size()-1;
        node_dist = 0.0;
    }else{
        //从终点开始累计长度
        double dist_total = 0.0;
        for(int i = base_index ; i >= 1 ; i -- ){
            const geos::geom::Coordinate &coord1 = cs->getAt(i);
            const geos::geom::Coordinate &coord2 = cs->getAt(i - 1);
            double dist = GeometryUtil::getDistance(&coord1, &coord2);

            double dist_temp = dist_total + dist;
            if(dist_temp < distance){
                dist_total = dist_temp;
            }else{
                double diff_dist = dist - (distance - dist_total);
                node_index = i-1;
                node_dist = diff_dist;
                return GetStringPoint(line, i-1, diff_dist, coord);
            }
        }

        const geos::geom::Coordinate & startcoord = cs->front();
        coord = startcoord;
        node_index = 0;
        node_dist = 0.0;
    }

    return true;
}

double GeosObjRelationUtil::GetReferDirection(const shared_ptr<geos::geom::LineString> line,
                                            double distance, bool start, int node_index, double node_dist) {
    //获得node距离起点的距离
    double node_distance = GetNodeDistance(line, true, node_index);
    node_distance += node_dist;

    double start_dist = node_distance - distance;
    double end_dist = node_distance + distance;

    //获得插值点
    geos::geom::Coordinate startpt, endpt;
    GetDiffPoint(line, start_dist, true, startpt);
    GetDiffPoint(line, end_dist, true, endpt);

    //计算方向
    if(start){
        return geo::geo_util::calcAngle(startpt.x, startpt.y, endpt.x, endpt.y);
    }else{
        return geo::geo_util::calcAngle( endpt.x, endpt.y, startpt.x, startpt.y);
    }
}

double GeosObjRelationUtil::GetNodeDistance(const shared_ptr<geos::geom::LineString> line, bool start, int node_index) {

    //参数判断
    if(node_index < 0)
        node_index = 0;
    if(node_index >= line->getNumPoints()){
        node_index = line->getNumPoints() - 1;
    }

    int node_start_index = start ?  0 : node_index;
    int node_end_index = start ? node_index : line->getNumPoints() - 1;

    double distance = 0.0;
    const geos::geom::CoordinateSequence *cs = line->getCoordinatesRO();
    for(int i = node_start_index ; i < node_end_index ; i ++){
        const geos::geom::Coordinate & spt = cs->getAt(i);
        const geos::geom::Coordinate & ept = cs->getAt(i+1);

        double dist_temp = GeometryUtil::getDistance(spt.x, spt.y, ept.x, ept.y);
        distance += dist_temp;
    }

    return distance;
}

bool GeosObjRelationUtil::DirectionSame(double dir1, double dir2, double angle_limit) {
    double anglediff = fabs(dir1 - dir2);
    if (anglediff > M_PI) {
        anglediff = 2 * M_PI - anglediff;
    }

    return anglediff < angle_limit;
}


double GeosObjRelationUtil::GetRefAngle(const shared_ptr<geos::geom::LineString> line, double distance, bool start){
    geos::geom::Coordinate endpt;
    if(GetDiffPoint(line, distance, start, endpt)){
        geos::geom::Coordinate startpt = start ? line->getCoordinatesRO()->front() : line->getCoordinatesRO()->back();
        return geo::geo_util::calcAngle(startpt.x, startpt.y, endpt.x, endpt.y);
    }else{
       return 0.0;
    }
}

bool GeosObjRelationUtil::GetLineStringExtendPoint(shared_ptr<geos::geom::LineString> line,
                                                   float distance, bool is_start,
                                                   geos::geom::Coordinate &start, geos::geom::Coordinate &end) {
    if(line == nullptr){
        return false;
    }

    geos::geom::Coordinate diff;
    if (is_start) {
        GetDiffPoint(line, distance, is_start, diff);
        start = line->getCoordinatesRO()->front();
        return GetMirrorPoint(diff, start, end);
    } else {
        GetDiffPoint(line, distance, is_start, diff);
        start = line->getCoordinatesRO()->back();
        return GetMirrorPoint(diff, start, end);
    }

    return true;
}

bool GeosObjRelationUtil::GetMirrorPoint(const geos::geom::Coordinate &coord1, const geos::geom::Coordinate &coord2,
                                         geos::geom::Coordinate &mirror_coord) {

    mirror_coord.x = 2 * coord2.x - coord1.x;
    mirror_coord.y = 2 * coord2.y - coord1.y;
    mirror_coord.z = 2 * coord2.z - coord1.z;

    return true;
}

bool GeosObjRelationUtil::
GetLinesIntersectPoint(const geos::geom::Coordinate &start1, const geos::geom::Coordinate &end1,
                       const geos::geom::Coordinate &start2, const geos::geom::Coordinate &end2,
                       geos::geom::Coordinate &intersect) {

    double x1 = start1.x, x2 = end1.x, x3 = start2.x, x4 = end2.x;
    double y1 = start1.y, y2 = end1.y, y3 = start2.y, y4 = end2.y;

    double A1 = y2 - y1;
    double B1 = x1 - x2;
    double C1 = (A1 * x1) + (B1 * y1);

    double A2 = y4 - y3;
    double B2 = x3 - x4;
    double C2 = A2 * x3 + B2 * y3;

    double det = A1 * B2 - A2 * B1;

    if (fabs(det) < 0.0000001)
    {
        return false;
    }
    else
    {
        // Return the point of intersection
        intersect.x = (B2 * C1 - B1 * C2) / det;
        intersect.y = (A1 * C2 - A2 * C1) / det;

        double diff1 = GetDiffZ(start1, end1, intersect);
        double diff2 = GetDiffZ(start2, end2, intersect);
        intersect.z = (diff1 + diff2) / 2.0;
        return true;
    }
}

void GeosObjRelationUtil::GetDisToStratAndEndByIndex(const shared_ptr<geos::geom::LineString> line, int index,
                                                     float& dis_start, float& dis_end) {
    if (line == nullptr ) {
        return;
    }
    const geos::geom::CoordinateSequence *cs = line->getCoordinatesRO();
    int coord_num = cs->size();

    for (int i = index; i < coord_num - 1; i++) {
        const geos::geom::Coordinate &coord1 = cs->getAt(i);
        const geos::geom::Coordinate &coord2 = cs->getAt(i + 1);
        double dist = GeometryUtil::getDistance(coord1.x, coord1.y, coord2.x, coord2.y);
        dis_end += dist;
    }

    for (int i = 0; i < index; i++) {
        const geos::geom::Coordinate &coord1 = cs->getAt(i);
        const geos::geom::Coordinate &coord2 = cs->getAt(i + 1);
        double dist = GeometryUtil::getDistance(coord1.x, coord1.y, coord2.x, coord2.y);
        dis_start += dist;
    }
}

double GeosObjRelationUtil::pt2LineDist(double Line[], double zLine[], int size, double PtA[],
                             double PtB[], double PtC[], double z_limit, int &min_index) {
    int len = size;
    if (len < 4 || len % 2 != 0 || !PtB
        || !PtC) {
        return -1;
    }
    double minDistance = DBL_MAX;
    double closestPoint[2];
    double LineSeg[4];
    int minIndex = -1;
    double z = PtA[2];
    bool flag = false;
    for (int i = 0; i < len - 2; i += 2) {

        LineSeg[0] = Line[i];
        LineSeg[1] = Line[i + 1];
        LineSeg[2] = Line[i + 2];
        LineSeg[3] = Line[i + 3];

        double sline = GeometryUtil::UT_GetMapDistance(LineSeg[0], LineSeg[1], LineSeg[2], LineSeg[3]);

        int index = i/2;
        double find_z = (zLine[index] + zLine[index + 1]) / 2;
        //
        double dis = geo::geo_util::pt2LineSegmentDist(LineSeg, PtA, PtB);
        if (dis < minDistance) {
            double bline = GeometryUtil::UT_GetMapDistance(LineSeg[0], LineSeg[1], PtB[0], PtB[1]);
            if (fabs(sline) > 0.00000001) {
                find_z = bline / sline * (zLine[index + 1] - zLine[index]) + zLine[index];
            }
            if(fabs(z - find_z) > z_limit){
                continue; // 考虑高度差
            }
            minDistance = dis;
            closestPoint[0] = PtB[0];
            closestPoint[1] = PtB[1];
            PtC[0] = LineSeg[0];
            PtC[1] = LineSeg[1];
            PtC[2] = LineSeg[2];
            PtC[3] = LineSeg[3];
            //
            minIndex = i;

            flag = true;
        }
    }

    double ret = 0;

    if (flag) {
        min_index = minIndex;
        PtB[0] = closestPoint[0];
        PtB[1] = closestPoint[1];
        ret = sqrt((PtA[0] - closestPoint[0]) * (PtA[0] - closestPoint[0]) +
                   (PtA[1] - closestPoint[1]) * (PtA[1] - closestPoint[1]));
    } else {
        double closest_point0 = sqrt((Line[0] - PtA[0]) * (Line[0] - PtA[0]) +
                                     (Line[1] - PtA[1]) * (Line[1] - PtA[1]));
        double closest_point1 = sqrt((Line[len - 2] - PtA[0]) * (Line[len - 2] - PtA[0]) +
                                     (Line[len - 1] - PtA[1]) * (Line[len - 1] - PtA[1]));

        if (closest_point0 < closest_point1) {
            min_index = 0;
            PtC[0] = Line[0];
            PtC[1] = Line[1];
            PtC[2] = Line[2];
            PtC[3] = Line[3];
            PtB[0] = Line[0];
            PtB[1] = Line[1];
            ret = closest_point0;
        } else {
            min_index = (len - 2) / 2;
            PtC[0] = Line[len - 4];
            PtC[1] = Line[len - 3];
            PtC[2] = Line[len - 2];
            PtC[3] = Line[len - 1];
            PtB[0] = Line[len - 2];
            PtB[1] = Line[len - 1];
            ret = closest_point1;
        }
    }
    return ret;
}

double GeosObjRelationUtil::GetDistance(const shared_ptr<geos::geom::LineString> line,
                                        int nodex_index1, int node_index2) {
    double node1 = min(nodex_index1, node_index2);
    double node2 = max(nodex_index1, node_index2);

    double total_len = 0.0;
    const geos::geom::CoordinateSequence *cs = line->getCoordinatesRO();
    for( int i = node1 ; i < node2 ; i ++ ){
        const geos::geom::Coordinate &coord1 = cs->getAt(i);
        const geos::geom::Coordinate &coord2 = cs->getAt(i + 1);
        double dist = GeometryUtil::getDistance(coord1.x, coord1.y, coord2.x, coord2.y);
        total_len += dist;
    }

    return total_len;
}

double GeosObjRelationUtil::GetDistance(const shared_ptr<geos::geom::LineString> line,
                                        int node_index, double node_dist, bool start) {
    if(start){
        double base_dist = GetDistance(line, 0, node_index);
        base_dist += node_dist;
        return base_dist;
    }else{
        long last_node_index = line->getNumPoints()-1;
        double base_dist = GetDistance(line, node_index, last_node_index);
        base_dist -= node_dist;
        return base_dist;
    }
}

double GeosObjRelationUtil::GetDistance(const shared_ptr<geos::geom::LineString> line,
                                        int node_index1, double node_dist1,
                                        int node_index2, double node_dist2) {
    double dist1 = GetDistance(line, node_index1, node_dist1, true);
    double dist2 = GetDistance(line, node_index2, node_dist2, true);

    //TODO: 此处计算浪费，应该只计算两个node之间的距离，有时间优化

    double dist = fabs(dist1 - dist2);
    return dist;
}


double GeosObjRelationUtil::GetDistanceLngLat(double lng1, double lat1, double lng2, double lat2){
    double utmX1, utmY1, utmX2, utmY2;
    char zone[4];
    kd::automap::Coordinates::ll2utm(lat1, lng1, utmX1, utmY1, zone);
    kd::automap::Coordinates::ll2utm(lat2, lng2, utmX2, utmY2, zone);

    double dist = GeometryUtil::getDistance(utmX1, utmY1, utmX2, utmY2);
    return dist;
}

shared_ptr<geos::geom::Coordinate> GeosObjRelationUtil::GetExtendPoint(shared_ptr<geos::geom::Coordinate> start,
                                                  shared_ptr<geos::geom::Coordinate> end,
                                                  float distance, bool is_start) {
    shared_ptr<geos::geom::Coordinate> ret_coord = nullptr;
    if (fabs(end->x - start->x) > 0.0000001) {
        ret_coord = make_shared<geos::geom::Coordinate>();

        double k = (end->y - start->y) / (end->x - start->x);

        if (is_start) {
            if (start->x < end->x) {
                ret_coord->x = -distance * sqrt(1 / (1 + k * k)) + start->x ;
            } else {
                ret_coord->x = distance * sqrt(1 / (1 + k * k)) + start->x ;
            }

            if (start->y < end->y) {
                ret_coord->y = -distance * sqrt(k * k / (1 + k * k)) + start->y;
            } else {
                ret_coord->y = distance * sqrt(k * k / (1 + k * k)) + start->y;
            }
        } else {
            if (end->x > start->x) {
                ret_coord->x = distance * sqrt(1 / (1 + k * k)) + end->x;
            } else {
                ret_coord->x = -distance * sqrt(1 / (1 + k * k)) + end->x;
            }

            if (end->y > start->y) {
                ret_coord->y = distance * sqrt(k * k / (1 + k * k)) + end->y;
            } else {
                ret_coord->y = -distance * sqrt(k * k / (1 + k * k)) + end->y;
            }
        }
    }

    return ret_coord;
}
//确定点集合的参考方向
template <typename T> double GeosObjRelationUtil::GetAngle(vector<T> &lane_nodes, bool start, double ref_dist, char * zone)
{
    //个数太少，则不计算
    if (lane_nodes.size() < 2) {
        return 0.0;
    }

    //个数等于2，直接计算出角度
    if (lane_nodes.size() == 2) {
        T node1 = lane_nodes[0];
        T node2 = lane_nodes[1];
        double angle = geo::geo_util::calcAngle(node1->x, node1->y, node2->x, node2->y);
        return angle;
    }

    bool firstseg_invalid = CheckFirstSegInvalid(lane_nodes, start, M_PI/32.0, ref_dist);

    //点数较多时，尽量取较长的距离点
    //.1获得参考的节点索引和坐标
    shared_ptr<geos::geom::LineString> line = kd::dc::GeosObjUtil::CreateLineString(lane_nodes);

    //看第一段的距离是不是足够长，能够代表数据的方向，如果能够代表，则直接计算
    const geos::geom::CoordinateSequence *cs = line->getCoordinatesRO();
    if(start){
        const geos::geom::Coordinate &coord1 = cs->getAt(0);
        const geos::geom::Coordinate &coord2 = cs->getAt(1);
        if(GeometryUtil::getDistance(coord1.x, coord1.y, coord2.x, coord2.y) > ref_dist/2.0){
            T node1 = lane_nodes[0];
            T node2 = lane_nodes[1];
            double angle = geo::geo_util::calcAngle(node1->x, node1->y, node2->x, node2->y);
            return angle;
        }
    }else{
        const geos::geom::Coordinate &coord1 = cs->getAt(cs->size()-2);
        const geos::geom::Coordinate &coord2 = cs->getAt(cs->size()-1);
        if(GeometryUtil::getDistance(coord1.x, coord1.y, coord2.x, coord2.y) > ref_dist/2.0){
            T node1 = lane_nodes[lane_nodes.size()-2];
            T node2 = lane_nodes[lane_nodes.size()-1];
            double angle = geo::geo_util::calcAngle(node1->x, node1->y, node2->x, node2->y);
            return angle;
        }
    }

    //如果第一段的距离不能代表，则需要取足够距离长度的数据
    geos::geom::Coordinate diff_coord;
    int node_index;
    double node_dist;
    int base_index = start ? 0 : (lane_nodes.size() - 1);
    GeosObjRelationUtil::GetDiffPoint(line, base_index, ref_dist, start, diff_coord, node_index, node_dist);

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
            T node1 = lane_nodes[i];
            T node2 = lane_nodes[i + 1];
            T node3 = lane_nodes[i + 2];
            double angle_diff = GeometryUtil::getAngleDiff(node1->x, node1->y, node2->x, node2->y,
                                                           node2->x, node2->y, node3->x, node3->y);
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
            T node1 = lane_nodes[i];
            T node2 = lane_nodes[i - 1];
            T node3 = lane_nodes[i - 2];
            double angle_diff = GeometryUtil::getAngleDiff(node1->x, node1->y, node2->x, node2->y,
                                                           node2->x, node2->y, node3->x, node3->y);
            if (angle_diff > angle_limit) {
                use_diff_coord = false;
                break;
            }
        }
    }

    //求角度
    if (use_diff_coord) { //使用差分点计算角度
        T base_coord;
        if(!firstseg_invalid){
            base_coord = start ? lane_nodes.front() : lane_nodes.back();
        }else{
            base_coord = start ? lane_nodes[1] : lane_nodes[lane_nodes.size()-2];
        }

        double diff_lng, diff_lat;
        kd::automap::Coordinates::utm2ll(diff_coord.y, diff_coord.x, zone, diff_lat, diff_lng);

        double angle = 0.0;
        if (start) {
            angle = geo::geo_util::calcAngle(base_coord->x, base_coord->y, diff_lng, diff_lat);
        } else {
            angle = geo::geo_util::calcAngle(diff_lng, diff_lat, base_coord->x, base_coord->y);
        }

        return angle;
    } else { //直接使用首段作为方向参考
        T node1, node2;
        if (start) {
            if(!firstseg_invalid){
                node1 = lane_nodes[0];
                node2 = lane_nodes[1];
            }else{
                node1 = lane_nodes[1];
                node2 = lane_nodes[2];
            }

            return geo::geo_util::calcAngle(node1->x, node1->y, node2->x, node2->y);
        } else {
            if(!firstseg_invalid){
                node1 = lane_nodes[lane_nodes.size() - 2];
                node2 = lane_nodes[lane_nodes.size() - 1];
            }else{
                node1 = lane_nodes[lane_nodes.size() - 3];
                node2 = lane_nodes[lane_nodes.size() - 2];
            }

            return geo::geo_util::calcAngle(node1->x, node1->y, node2->x, node2->y);
        }
    }
}
template <typename T> static bool CheckFirstSegInvalid(vector<T> &lane_nodes, bool start,
                                                       double angle_limit, double dist_limit){
    if(lane_nodes.size() < 3){
        return false;
    }

    T node1, node2, node3;
    double angle_diff = 0.0;
    if (start) {
        node1 = lane_nodes[0];
        node2 = lane_nodes[1];
        node3 = lane_nodes[2];

        if(GetDistanceLngLat(node1->x, node1->y, node2->x, node2->y) > dist_limit){
            return false;
        }

        angle_diff = GeometryUtil::getAngleDiff(node1->x, node1->y, node2->x, node2->y,
                                                node2->x, node2->y, node3->x, node3->y);

    } else {
        node1 = lane_nodes[lane_nodes.size()-1];
        node2 = lane_nodes[lane_nodes.size()-2];
        node3 = lane_nodes[lane_nodes.size()-3];

        if(GetDistanceLngLat(node1->x, node1->y, node2->x, node2->y) > dist_limit){
            return false;
        }

        angle_diff = GeometryUtil::getAngleDiff(node1->x, node1->y, node2->x, node2->y,
                                                node2->x, node2->y, node3->x, node3->y);
    }


    return (angle_diff > angle_limit);
}