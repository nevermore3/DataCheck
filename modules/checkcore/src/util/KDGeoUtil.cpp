
#include "../../include/util/KDGeoUtil.hpp"

//system
#include <math.h>
#include <memory.h>

//third party
#include <geos/geom/GeometryFactory.h>
#include "geos/geom/CoordinateSequence.h"
#include "geos/geom/CoordinateArraySequence.h"

#include <mvg/Coordinates.hpp>


//core
#include "geom/geo_util.h"

using namespace geo;

const double kZLimitRate = 0.2;

namespace kd {
    namespace automap {

        double KDGeoUtil::get3DDistance(const geos::geom::Coordinate *start,
                                        const geos::geom::Coordinate *end) {
            double dx = start->x - end->x;
            double dy = start->y - end->y;
            double dz = start->z - end->z;

            return sqrt(dx * dx + dy * dy + dz * dz);
        }

        double KDGeoUtil::get3DDistance(const geos::geom::Point *start,
                                        const geos::geom::Point *end) {
            const geos::geom::Coordinate *startpt = start->getCoordinate();
            const geos::geom::Coordinate *endpt = end->getCoordinate();
            double dx = startpt->x - endpt->x;
            double dy = startpt->y - endpt->y;
            double dz = startpt->z - endpt->z;

            return sqrt(dx * dx + dy * dy + dz * dz);
        }

        double KDGeoUtil::getDistance(const geos::geom::Point *start,
                                      const geos::geom::Point *end) {
            const geos::geom::Coordinate *startpt = start->getCoordinate();
            const geos::geom::Coordinate *endpt = end->getCoordinate();
            double dx = startpt->x - endpt->x;
            double dy = startpt->y - endpt->y;

            return sqrt(dx * dx + dy * dy);
        }

        double KDGeoUtil::getDistance(const geos::geom::Coordinate *start,
                                      const geos::geom::Coordinate *end) {
            double dx = start->x - end->x;
            double dy = start->y - end->y;

            return sqrt(dx * dx + dy * dy);
        }

        double KDGeoUtil::getZDistance(const geos::geom::Point *start,
                                       const geos::geom::Point *end) {
            const geos::geom::Coordinate *startpt = start->getCoordinate();
            const geos::geom::Coordinate *endpt = end->getCoordinate();
            double dz = startpt->z - endpt->z;

            return dz;
        }

        bool KDGeoUtil::calZValue(const geos::geom::Coordinate *start, const geos::geom::Coordinate *end,
                                  geos::geom::Coordinate *middle) {

            if (start == nullptr || end == nullptr || middle == nullptr)
                return false;

            double len1 = getDistance(start, end);
            double len2 = getDistance(middle, end);
            if (len1 == 0.0 || len2 == 0.0) {
                middle->z = end->z;
                return true;
            }

            middle->z = len2 * (start->z - end->z) / len1 + end->z;

            return true;
        }

        bool KDGeoUtil::isLineSegmentCross(const geos::geom::Coordinate *start1, const geos::geom::Coordinate *end1,
                                           const geos::geom::Coordinate *start2, const geos::geom::Coordinate *end2,
                                           double sameLimit, CoordinateSequence **coor_seq) {

            //判断两个线段在平面上是否相交
            double cpt[2];
            double LineA[] = {start1->x, start1->y, end1->x, end1->y};
            double LineB[] = {start2->x, start2->y, end2->x, end2->y};

            bool xycross = geo::geo_util::getCrossPoint(LineA, LineB, cpt);
            if (xycross) {

                geos::geom::Coordinate crosspt1;
                crosspt1.x = cpt[0];
                crosspt1.y = cpt[1];

                geos::geom::Coordinate crosspt2;
                crosspt2.x = cpt[0];
                crosspt2.y = cpt[1];

                if (calZValue(start1, end1, &crosspt1) && calZValue(start2, end2, &crosspt2)) {
                    double dz = fabs(crosspt1.z - crosspt2.z);
                    if (dz < sameLimit) {
                        if (coor_seq != nullptr) {
                            if (*coor_seq == nullptr) {
                                *coor_seq = new CoordinateArraySequence();
                            }
                            double cpt_z = (crosspt1.z + crosspt2.z) / 2;
                            geos::geom::Coordinate cross_pt(cpt[0], cpt[1], cpt_z);
                            (*coor_seq)->add(cross_pt);
                        }
                        return true;
                    }
                }
            }

            return false;
        }


        bool KDGeoUtil::isLineCross(geos::geom::LineString *line1, geos::geom::LineString *line2,
                                    CoordinateSequence **coor_seq, double sameLimit) {
            if (line1 == nullptr || line2 == nullptr || sameLimit < 0)
                return false;

            if (sameLimit == 0) {
                //TODO 如何处理此种情况
            }

            geos::geom::Geometry *geom_buffer = line1->buffer(sameLimit);

            bool ret = isLineCross(line1, geom_buffer, line2, sameLimit, coor_seq);

            delete geom_buffer;

            return ret;
        }

        bool KDGeoUtil::isLineCross(geos::geom::LineString *line1, geos::geom::Geometry *geom_buffer,
                                    geos::geom::LineString *line2, double sameLimit, CoordinateSequence **coor_seq) {

            //参数检验
            if (line1 == nullptr || line2 == nullptr || geom_buffer == nullptr || sameLimit < 0)
                return false;

            //水平方向缓冲区检验
            if (!geom_buffer->intersects(line2)) {
                return false;
            }

            //循环判断每段是否相交
            geos::geom::LineString *shortLine = line1->getNumPoints() < line2->getNumPoints() ? line1 : line2;
            geos::geom::LineString *longLine = line1->getNumPoints() < line2->getNumPoints() ? line2 : line1;

            int shortLoop = shortLine->getNumPoints() - 1;
            int longLoop = longLine->getNumPoints() - 1;

            for (int i = 0; i < shortLoop; i++) {
                const Coordinate &start1 = shortLine->getCoordinateN(i);
                const Coordinate &end1 = shortLine->getCoordinateN(i + 1);

                for (int j = 0; j < longLoop; j++) {

                    const Coordinate &start2 = longLine->getCoordinateN(j);
                    const Coordinate &end2 = longLine->getCoordinateN(j + 1);

                    if (isLineSegmentCross(&start1, &end1, &start2, &end2, sameLimit, coor_seq)) {
                        return true;
                    }
                }
            }

            return false;

        }

        bool KDGeoUtil::getCorssPart(geos::geom::LineString * line1, geos::geom::LineString * line2,
                                     vector<int> & vecLine1, vector<int> & vecLine2, double buffer){


            return true;
        }

        bool KDGeoUtil::getPedalInfo(geos::geom::LineString * line, geos::geom::Coordinate * coord, double buffer, int & index, double & len){
            if(line == nullptr || coord == nullptr){
                return false;
            }

            double PtA[] = { coord->x, coord->y };
            double PtB[2];  //垂足
            double PtC[4];  //线段端点
            int min_index;

            double distance = pt2LineDist(line->getCoordinates(), PtA, PtB, PtC, min_index);
            if(distance > buffer)
                return false;




            return true;
        }

        bool KDGeoUtil::isLineInBuffer(geos::geom::LineString *line1, geos::geom::Geometry *geom_buffer,
                                       geos::geom::LineString *line2, double sameLimit) {
            //参数检验
            if (line1 == nullptr || line2 == nullptr || geom_buffer == nullptr || sameLimit < 0)
                return false;

            //水平方向缓冲区检验
            if (!geom_buffer->contains(line2)) {
                return false;
            }

            //判断垂直方向是否在缓冲范围内


            return true;
        }

        bool KDGeoUtil::isLineAdjoin(const geos::geom::LineString *line1, const geos::geom::LineString *line2,
                                     int & startIndex, int & endIndex, double buffer){

            if(line1 == nullptr || line2 == nullptr)
                return false;

            //实线原则：找出line2中，投影点在line1上的所有点的距离，求平均值。
            CoordinateSequence * cs1  = line1->getCoordinates();
            int cs1Count = cs1->size();

            CoordinateSequence * cs2  = line2->getCoordinates();

            vector<double> vecDistance;

            for( int i = 0 ; i < cs2->size() ; i ++ ){

                const geos::geom::Coordinate & coord = cs2->getAt(i);

                double PtA[] = { coord.x, coord.y };
                double PtB[2];  //垂足
                double PtC[4];  //线段端点
                int min_index;

                double distance = pt2LineDist(cs1, PtA, PtB, PtC, min_index);

                if(min_index > 0 && min_index < (cs1Count -2) ){
                    vecDistance.emplace_back(distance);
                }else if(min_index == 0){
                    if(PtC[0] == PtB[0] && PtC[1] == PtB[1] && distance < buffer){ //
                        vecDistance.emplace_back(distance);
                    }else if(PtC[0] != PtB[0] && PtC[1] != PtB[1]){
                        vecDistance.emplace_back(distance);
                    }
                }else if(min_index == (cs1Count-2)){
                    if(PtC[2] == PtB[0] && PtC[3] == PtB[1] && distance < buffer){ //
                        vecDistance.emplace_back(distance);
                    }else if(PtC[2] != PtB[0] && PtC[3] != PtB[1]){
                        vecDistance.emplace_back(distance);
                    }
                }
            }

            if(vecDistance.size() == 0)
                return false;

            double totalDist = 0.0;
            for( double value : vecDistance ){
                totalDist += value;
            }

            double distMean = totalDist / vecDistance.size();

            return distMean < buffer;
        }


        int KDGeoUtil::calLineRelation(geos::geom::LineString *line1, geos::geom::LineString *line2) {
            if (line1 == nullptr || line2 == nullptr ||
                line1->getNumPoints() < 2 || line2->getNumPoints() < 2)
                return -1;

            //查找出线的对应的临近点
            const geos::geom::Coordinate *first1 = nullptr;
            const geos::geom::Coordinate *first2 = nullptr;
            const geos::geom::Coordinate *second1 = nullptr;

            geos::geom::CoordinateSequence *csRef = line1->getCoordinates();

            geos::geom::CoordinateSequence *csCheck = line2->getCoordinates();
            int coordNum = csCheck->getSize();
            for (int i = 0; i < coordNum; i++) {
                const geos::geom::Coordinate &coord = csCheck->getAt(i);
                double PtA[] = {coord.x, coord.y};
                double PtB[4];
                double PtC[2];
                int min_index;

                double len = pt2LineDist(csRef, PtA, PtB, PtC, min_index);

                //索引是第一个线段的情形
                if (min_index == 0) {

                    const geos::geom::Coordinate &startCoord = csRef->getAt(0);

                    if (PtC[0] != startCoord.x && PtC[1] != startCoord.y) {
                        //查找索引是第一个线段，但垂足在该线段中间的点
                        first1 = &csRef->getAt(0);
                        first2 = &csRef->getAt(1);
                        second1 = &csCheck->getAt(i);
                        break;
                    }
                }

                if (min_index > 0) {
                    first1 = &csRef->getAt(min_index);
                    first2 = &csRef->getAt(min_index + 1);
                    second1 = &csCheck->getAt(i);
                    break;
                }
            }

            if (first1 == nullptr || first2 == nullptr || second1 == nullptr)
                return -1;

            //根据对应的临近点判断两条线的关系
            double linepRel = first1->x * (second1->y - first2->y) + \
            first2->x * (first1->y - second1->y) + second1->x * (first2->y - first1->y);

            if (linepRel < 0)
                return 1;
            else
                return 2;
        }

        int KDGeoUtil::calLineStartEndRelation(geos::geom::LineString *line1, geos::geom::LineString *line2,
                                               double sameLimit) {
            //判断起点和终点融合情况
            double startDist = get3DDistance(line1->getStartPoint(), line2->getStartPoint());
            double endDist = get3DDistance(line1->getEndPoint(), line2->getEndPoint());
            bool startSame = startDist < sameLimit ? true : false;
            bool endSame = endDist < sameLimit ? true : false;

            if (startSame) {
                if (endSame)
                    return 3; //起点、终点都相连
                else
                    return 1; //起点相连
            }

            if (endSame) {
                return 2; //终点相连
            }

            double startEndDist = get3DDistance(line1->getStartPoint(), line2->getEndPoint());
            double endStartDist = get3DDistance(line1->getEndPoint(), line2->getStartPoint());
            bool startEndSame = startEndDist < sameLimit ? true : false;
            bool endStartSame = endStartDist < sameLimit ? true : false;

            if (startEndSame) {
                if (endStartSame)
                    return 6; //两条线两个端点均首尾相连
                else
                    return 4; //第一条线的起点和第二条线的终点相连
            }

            if (endStartSame) {
                return 5; //第一条线的终点和第二条线的起点相连
            }

            return -1; //两条线的起终点不相连

        }

        bool KDGeoUtil::isLoopLine(geos::geom::LineString *line, double sameLimit) {
            if (line == nullptr)
                return false;

            double dist = get3DDistance(line->getStartPoint(), line->getEndPoint());
            bool startEndSame = dist < sameLimit ? true : false;
            return startEndSame;
        }


        bool KDGeoUtil::checkLineDirection(geos::geom::LineString *line1, geos::geom::LineString *line2) {

            return true;
        }


        bool KDGeoUtil::calIntersectPartsExt(const geos::geom::LineString *line1,
                                             const geos::geom::LineString *line2,
                                             std::vector<shared_ptr<LineRelation>> &lineRelations, double bufferLen) {
            if (line1 == nullptr || line2 == nullptr || bufferLen < 0)
                return false;

            lineRelations.clear();

            const GeometryFactory *gf = geos::geom::GeometryFactory::getDefaultInstance();
            shared_ptr<Geometry> geom_buffer(line1->buffer(bufferLen));
            CoordinateSequence *src_cs = line1->getCoordinates();

            bool lastRelation = false; //上一次存储的状况
            bool firstRelation = true; //

            std::vector<bool> vecRelation;
            CoordinateSequence *dst_cs = line2->getCoordinates();
            for (int i = 0; i < dst_cs->size(); i++) {
                const Coordinate &coord = dst_cs->getAt(i);

                shared_ptr<geos::geom::Point> pointPtr(gf->createPoint(coord));
                bool curRelation = false;
                //水平相交判断
                if (geom_buffer->intersects(pointPtr.get())) {
                    if (coordInZBuffer(src_cs, coord, bufferLen)) {
                        curRelation = true;
                    }
                }

                if (curRelation != lastRelation) {
                    shared_ptr<LineRelation> relation = make_shared<LineRelation>();
                    relation->index = i;
                    relation->relation = curRelation;
                    lineRelations.emplace_back(relation);
                    lastRelation = curRelation;
                }

                if (i > 0 && !curRelation && !firstRelation) {
                    //本次和上次的点位都不在缓冲区内，有个可能是线段同缓冲区相交，需要特殊处理
                    CoordinateSequence *cl = new CoordinateArraySequence();
                    cl->add(dst_cs->getAt(i - 1));
                    cl->add(dst_cs->getAt(i));

                    shared_ptr<geos::geom::LineString> tempLine(gf->createLineString(cl));
                    if (geom_buffer->intersects(tempLine.get())) {
                        //检查z值
                        if (coordInZBuffer(src_cs, dst_cs->getAt(i - 1), bufferLen) &&
                            coordInZBuffer(src_cs, dst_cs->getAt(i), bufferLen)) {
                            shared_ptr<LineRelation> relation1 = make_shared<LineRelation>();
                            relation1->index = i - 1;
                            relation1->relation = true;
                            lineRelations.emplace_back(relation1);

                            shared_ptr<LineRelation> relation2 = make_shared<LineRelation>();
                            relation2->index = i;
                            relation2->relation = false;
                            lineRelations.emplace_back(relation2);
                        }
                    }
                }

                //记录一下当前的状态，以便下一次判断使用
                firstRelation = curRelation;
            }

            //delete geom_buffer;

            //check relation
//            cout << "check relations -----------------------" << endl;
//            for(shared_ptr<LineRelation> relation : lineRelations){
//                cout << "index:" << relation->index << ", relation: " << relation->relation << endl;
//            }

            return true;
        }

        bool KDGeoUtil::coordInZBuffer(geos::geom::CoordinateSequence *cs, const Coordinate &coord, double bufferLen) {
            if (cs == nullptr || bufferLen < 0)
                return false;

            double PtA[2];
            PtA[0] = coord.x;
            PtA[1] = coord.y;
            double PtB[2] = {0.0, 0.0};
            double PtC[4] = {0.0, 0.0, 0.0, 0.0};
            int min_index;

            pt2LineDist(cs, PtA, PtB, PtC, min_index);

            if (min_index < 0 || min_index >= cs->size()) {
                return false;
            }

            const Coordinate &compCoord = cs->getAt(min_index);
            bool inBuffer = false;
            if (fabs(coord.z - compCoord.z) < bufferLen)
                inBuffer = true;
            return inBuffer;
        }


        double KDGeoUtil::pt2LineDist(geos::geom::CoordinateSequence *cs, double PtA[],
                                      double PtB[], double PtC[], int &min_index) {
            if (cs->size() < 2 || !PtB || !PtC) {
                return -1;
            }

            double minDistance = 99999999999;
            double closestPoint[2];
            double LineSeg[4];
            int minIndex = -1;
            for (int i = 0; i < cs->size() - 1; i++) {
                const geos::geom::Coordinate &start = cs->getAt(i);
                const geos::geom::Coordinate &end = cs->getAt(i + 1);

                LineSeg[0] = start.x;
                LineSeg[1] = start.y;
                LineSeg[2] = end.x;
                LineSeg[3] = end.y;
                //
                double dis = geo::geo_util::pt2LineSegmentDist(LineSeg, PtA, PtB);
                if (dis < minDistance) {
                    minDistance = dis;
                    closestPoint[0] = PtB[0];
                    closestPoint[1] = PtB[1];
                    PtC[0] = LineSeg[0];
                    PtC[1] = LineSeg[1];
                    PtC[2] = LineSeg[2];
                    PtC[3] = LineSeg[3];
                    //
                    minIndex = i;
                }
            }
            min_index = minIndex;
            PtB[0] = closestPoint[0];
            PtB[1] = closestPoint[1];
            //
            return sqrt((PtA[0] - closestPoint[0]) * (PtA[0] - closestPoint[0]) +
                        (PtA[1] - closestPoint[1]) * (PtA[1] - closestPoint[1]));
        }


        double KDGeoUtil::getLength(geos::geom::LineString *line, int beginIndex, int endIndex) {
            if (line == nullptr ||
                (beginIndex != -1 && beginIndex >= line->getNumPoints()) || //检查开始索引
                (endIndex != -1 && endIndex >= line->getNumPoints()))
                return -1.0;

            int realBeginIndex = beginIndex == -1 ? 0 : beginIndex;
            int realEndIndex = endIndex == -1 ? (line->getNumPoints() - 1) : endIndex;

            CoordinateSequence *cs = line->getCoordinates();
            double totalLen = 0.0;
            for (int i = realBeginIndex; i < realEndIndex; i++) {
                const Coordinate &start = cs->getAt(i);
                const Coordinate &end = cs->getAt(i + 1);

                totalLen += getDistance(&start, &end);
            }

            return totalLen;
        }


        bool KDGeoUtil::isPointLineCorss(const geos::geom::Point *point, const geos::geom::Geometry *buffer,
                                         const geos::geom::LineString *line, double bufferLen) {

            //参数判断
            if (point == nullptr || buffer == nullptr || line == nullptr)
                return false;

            //判断缓冲区和线是否相交
            if (!buffer->intersects(line))
                return false;

            //查找最近的坐标点，判断其高度值
            const Coordinate *ptCoord = point->getCoordinate();
            double PtA[] = {ptCoord->x, ptCoord->y};
            double PtB[2];
            double PtC[4];
            int min_index;

            double len = pt2LineDist(line->getCoordinates(), PtA, PtB, PtC, min_index);
            if (len < bufferLen) {
                const Coordinate &lineCoord = line->getCoordinateN(min_index);
                if (abs(lineCoord.z - ptCoord->z) < bufferLen)
                    return true;
            }

            return false;
        }


        bool KDGeoUtil::isLineParallel(const geos::geom::Coordinate *start1, const geos::geom::Coordinate *end1,
                                       const geos::geom::Coordinate *start2, const geos::geom::Coordinate *end2,
                                       double angleLimit) {
            if (start1 == nullptr || end1 == nullptr || start2 == nullptr || end2 == nullptr)
                return false;

            double angle1 = geo_util::calcAngle(start1->x, start1->y, end1->x, end1->y);
            double angle2 = geo_util::calcAngle(start2->x, start2->y, end2->x, end2->y);

            if (fabs(angle1 - angle2) < angleLimit || (360 - fabs(angle1 - angle2)) < angleLimit) {
                return true;
            }

            return false;
        }


        bool KDGeoUtil::get2DCrossPoint(const geos::geom::Coordinate *start1, const geos::geom::Coordinate *end1,
                                        const geos::geom::Coordinate *start2, const geos::geom::Coordinate *end2,
                                        geos::geom::Coordinate *crossPt) {
            if (start1 == nullptr || end1 == nullptr || start2 == nullptr || end2 == nullptr || crossPt == nullptr)
                return false;

            double tempLeft, tempRight;
            // 求x坐标
            tempLeft = (end2->x - start2->x) * (start1->y - end1->y) - (end1->x - start1->x)
                                                                       * (start2->y - end2->y);
            tempRight = (start1->y - start2->y) * (end1->x - start1->x) * (end2->x - start2->x) + start2->x
                                                                                                  * (end2->y -
                                                                                                     start2->y) *
                                                                                                  (end1->x -
                                                                                                   start1->x) -
                        start1->x * (end1->y - start1->y)
                        * (end2->x - start2->x);
            crossPt->x = tempRight / tempLeft;
            // 求y坐标
            tempLeft = (start1->x - end1->x) * (end2->y - start2->y) - (end1->y - start1->y)
                                                                       * (start2->x - end2->x);
            tempRight = end1->y * (start1->x - end1->x) * (end2->y - start2->y) + (end2->x - end1->x)
                                                                                  * (end2->y - start2->y) *
                                                                                  (start1->y - end1->y) -
                        end2->y * (start2->x - end2->x)
                        * (end1->y - start1->y);
            crossPt->y = tempRight / tempLeft;

            return true;
        }

        int KDGeoUtil::calPTOrentationOfLine(double x0, double y0, double x1, double y1, double ptx, double pty) {
            double tempValue =(y0 - y1)*ptx + (x1 - x0)* pty +( x0 * y1) - (x1 * y0);
            //tempValue>0，说明点在线的左边，小于在右边，等于则在线上
            if (tempValue > 0){
                return 1;
            } else if(tempValue == 0){
                return 0;
            } else{
                return -1;
            }
        }

        //计算两条线段质检的角度差，返回值为角度差的弧度值
        double KDGeoUtil::getAngleDiff(double cx1, double cy1, double cx2, double cy2,
                                       double cx3, double cy3, double cx4, double cy4) {
            double thetaFirst = geo::geo_util::calcAngle(cx1, cy1, cx2, cy2);
            double thetaSencond = geo::geo_util::calcAngle(cx3, cy3, cx4, cy4);

            double angleDiff = fabs(thetaFirst - thetaSencond);
            while (angleDiff > 2 * PI) {
                angleDiff = angleDiff - 2 * PI;
            }
            return angleDiff;
        }

        double KDGeoUtil::getAngleDiff(shared_ptr<geos::geom::Coordinate> ptr_coord1,
                                       shared_ptr<geos::geom::Coordinate> ptr_coord2,
                                       shared_ptr<geos::geom::Coordinate> ptr_coord3,
                                       shared_ptr<geos::geom::Coordinate> ptr_coord4) {
            double cx1 = ptr_coord2->x - ptr_coord1->x;
            double cx2 = ptr_coord4->x - ptr_coord3->x;
            double cy1 = ptr_coord2->y - ptr_coord1->y;
            double cy2 = ptr_coord4->y - ptr_coord3->y;
            double theta = sqrt((cx1 * cx1 + cy1 * cy1) * (cx2 * cx2 + cy2 * cy2));

            if (theta > 0) {
                double angle_cos = (cx1 * cx2 + cy1 * cy2) / theta;
                double angleDiff = acos(angle_cos);
                return angleDiff;
            }

            return 0;
        }
        
        /**
         * 创建三角形追踪区域
         *
         * @param startx
         * @param starty
         * @param endx
         * @param endy
         * @return
         */
        geos::geom::Polygon *
        KDGeoUtil::getTraceTriGeometry(double startx, double starty, double endx, double endy, int type) {
            //虚线跟踪得更远一些，角度更小一些
            double trace_Angle = 2.0; //2度
            double trace_Length = 35.0; //30米
            if (type == 1) {
                //虚线跟踪角度大一些，距离更近一些
                trace_Angle = 4.0;
                trace_Length = 15;
            }
            return getTraceTriGeometry(startx, starty, endx, endy, trace_Angle, trace_Length);
        }

        geos::geom::Polygon *
        KDGeoUtil::getTraceTriGeometry(double startx, double starty, double endx, double endy,
                                       double trace_Angle, double trace_Length) {

            double angle = geo::geo_util::calcAngle(startx, starty, endx, endy);
            double angle_radian = angle; // * M_PI / 180.0;

            double trace_angle_radian = trace_Angle * M_PI / 180.0;

            double newx = startx + trace_Length;
            double newy = starty;
            double dy = atan(trace_angle_radian) * trace_Length;

            const geos::geom::GeometryFactory *gf = geos::geom::GeometryFactory::getDefaultInstance();

            geos::geom::CoordinateSequence *cl = new geos::geom::CoordinateArraySequence();
            cl->add(geos::geom::Coordinate(startx, starty, 0.0));

            double newrotatex, newrotatey;
            rotate(startx, starty, newx, newy + dy, angle_radian, &newrotatex, &newrotatey);
            cl->add(geos::geom::Coordinate(newrotatex, newrotatey, 0.0));

            rotate(startx, starty, newx, newy - dy, angle_radian, &newrotatex, &newrotatey);
            cl->add(geos::geom::Coordinate(newrotatex, newrotatey, 0.0));

            cl->add(geos::geom::Coordinate(startx, starty, 0.0));
            //创建三角形追踪区域
            geos::geom::LinearRing *linearRing = gf->createLinearRing(cl);
            return gf->createPolygon(linearRing, NULL);
        }

        void KDGeoUtil::rotate(double centx, double centy, double x, double y, double radius, double *newx,
                               double *newy) {

            Eigen::AngleAxisd rotation_vector(radius, Eigen::Vector3d(0, 0, 1));

            Eigen::Vector3d org(x, y, 0.0);
            Eigen::Vector3d cent(centx, centy, 0.0);
            Eigen::Vector3d disp_org = org - cent;
            Eigen::Vector3d v_rotated = rotation_vector * disp_org;
            Eigen::Vector3d v_final = v_rotated + cent;

            *newx = v_final[0];
            *newy = v_final[1];
        }

        double KDGeoUtil::distance(double x1, double y1, double x2, double y2) {
            double v1 = (x2 - x1);
            double v2 = (y2 - y1);
            return sqrt(v1 * v1 + v2 * v2);
        }

        double KDGeoUtil::distanceLL(double lng1, double lat1, double lng2, double lat2){
            double X1, Y1, X2, Y2;
            char zone0[8] = {0};

            Coordinates::ll2utm(lat1, lng1, X1, Y1, zone0);
            Coordinates::ll2utm(lat2, lng2, X2, Y2, zone0);

            return  distance(X1, Y1, X2, Y2);
        }


        //计算点到曲线段的最近的索引，并返回投影点
        double KDGeoUtil::pt2LineDist(vector<shared_ptr<Eigen::Vector3d>> &points, double PtA[],
                                      double PtB[], double PtC[], int &min_index) {

            if (points.size() < 2 || !PtB || !PtC) {
                return -1;
            }

            double minDistance = 1111111111111111111;
            double closestPoint[2];
            double LineSeg[4];
            int minIndex = -1;
            for (int i = 0; i < points.size() - 1; i++) {
                shared_ptr<Eigen::Vector3d> start = points[i];
                shared_ptr<Eigen::Vector3d> end = points[i + 1];

                LineSeg[0] = (*start)[0];
                LineSeg[1] = (*start)[1];
                LineSeg[2] = (*end)[0];
                LineSeg[3] = (*end)[1];
                //
                double dis = geo::geo_util::pt2LineSegmentDist(LineSeg, PtA, PtB);
                if (dis < minDistance) {
                    minDistance = dis;
                    closestPoint[0] = PtB[0];
                    closestPoint[1] = PtB[1];
                    PtC[0] = LineSeg[0];
                    PtC[1] = LineSeg[1];
                    PtC[2] = LineSeg[2];
                    PtC[3] = LineSeg[3];
                    //
                    minIndex = i;
                }
            }
            min_index = minIndex;
            PtB[0] = closestPoint[0];
            PtB[1] = closestPoint[1];
            //
            return sqrt((PtA[0] - closestPoint[0]) * (PtA[0] - closestPoint[0]) +
                        (PtA[1] - closestPoint[1]) * (PtA[1] - closestPoint[1]));
        }

//        double GeoUtil::pt2LineDist(geos::geom::CoordinateSequence * cs, double PtA[],
//                                      double PtB[], double PtC[], int &min_index)
//        {
//            if ( cs->size() < 2 || !PtB || !PtC) {
//                return -1;
//            }
//
//            double minDistance = 1111111111111111111;
//            double closestPoint[2];
//            double LineSeg[4];
//            int minIndex = -1;
//            for (int i = 0; i < cs->size() - 1; i ++) {
//                const geos::geom::Coordinate & start = cs->getAt(i);
//                const geos::geom::Coordinate & end = cs->getAt(i+1);
//
//                LineSeg[0] = start.x;
//                LineSeg[1] = start.y;
//                LineSeg[2] = end.x;
//                LineSeg[3] = end.y;
//                //
//                double dis = geo::geo_util::pt2LineSegmentDist(LineSeg, PtA, PtB);
//                if (dis < minDistance) {
//                    minDistance = dis;
//                    closestPoint[0] = PtB[0];
//                    closestPoint[1] = PtB[1];
//                    PtC[0] = LineSeg[0];
//                    PtC[1] = LineSeg[1];
//                    PtC[2] = LineSeg[2];
//                    PtC[3] = LineSeg[3];
//                    //
//                    minIndex = i;
//                }
//            }
//            min_index = minIndex;
//            PtB[0] = closestPoint[0];
//            PtB[1] = closestPoint[1];
//            //
//            return sqrt((PtA[0] - closestPoint[0]) * (PtA[0] - closestPoint[0]) +
//                        (PtA[1] - closestPoint[1]) * (PtA[1] - closestPoint[1]));
//        }


        int KDGeoUtil::pt2LineSegmentRelation(double Line[], double PtA[], double PtB[]) {
            double fLine0 = Line[0];
            double fLine1 = Line[1];
            double fLine2 = Line[2];
            double fLine3 = Line[3];
            //
            double fPtA0 = (double) (PtA[0]);
            double fPtA1 = (double) (PtA[1]);
            //
            double fPtB0 = 0.0;
            double fPtB1 = 0.0;

            double dX = fLine2 - fLine0;
            double dY = fLine3 - fLine1;

            double dR = -(fLine1 - fPtA1) * dY - (fLine0 - fPtA0) * dX;

            double dL;
            int retValue = -1;
            if (dR <= 0) {
                fPtB0 = fLine0;
                fPtB1 = fLine1;
                retValue = 0;
            } else if (dR >= (dL = dX * dX + dY * dY)) {
                fPtB0 = fLine2;
                fPtB1 = fLine3;
                retValue = 1;
            } else {
                fPtB0 = fLine0 + dR * dX / dL;
                fPtB1 = fLine1 + dR * dY / dL;
                retValue = 2;
            }

            PtB[0] = fPtB0;
            PtB[1] = fPtB1;

            return retValue;
        }


        int KDGeoUtil::pt2LineSegmentRelation(Eigen::Vector3d *lineStart,
                                              Eigen::Vector3d *lineEnd, Eigen::Vector3d *point) {
            double fLine0 = (*lineStart)(0);
            double fLine1 = (*lineStart)(1);
            double fLine2 = (*lineEnd)(0);
            double fLine3 = (*lineEnd)(1);
            //
            double fPtA0 = (double) ((*point)(0));
            double fPtA1 = (double) ((*point)(1));
            //

            double dX = fLine2 - fLine0;
            double dY = fLine3 - fLine1;

            double dR = -(fLine1 - fPtA1) * dY - (fLine0 - fPtA0) * dX;

            double dL;
            int retValue = -1;
            if (dR <= 0) {
                retValue = 0;
            } else if (dR >= (dL = dX * dX + dY * dY)) {
                retValue = 1;
            } else {
                retValue = 2;
            }

            return retValue;
        }

        bool KDGeoUtil::calIntersectParts(const shared_ptr<geos::geom::LineString> &srcLine,
                                          const shared_ptr<geos::geom::LineString> &dstLine,
                                          vector<shared_ptr<LineRelation>> &lineRelations,
                                          double bufferLen) {

            const GeometryFactory *gf = GeometryFactory::getDefaultInstance();
            Geometry *geom_buffer = srcLine->buffer(bufferLen);

            bool lastRelation = false; //上一次存储的状况
            bool firstRelation = true; //

            vector<bool> vecRelation;
            CoordinateSequence *dst_cs = dstLine->getCoordinates();
            for (int i = 0; i < dst_cs->size(); i++) {
                const Coordinate &coord = dst_cs->getAt(i);

                shared_ptr<geos::geom::Point> pointPtr(gf->createPoint(coord));
                bool curRelation = geom_buffer->intersects(pointPtr.get());

                if (curRelation != lastRelation) {
                    shared_ptr<LineRelation> relation = make_shared<LineRelation>();
                    relation->index = i;
                    relation->relation = curRelation;
                    lineRelations.emplace_back(relation);
                    lastRelation = curRelation;
                }

                if (i > 0 && !curRelation && !firstRelation) {
                    //本次和上次的点位都不在缓冲区内，有个可能是线段同缓冲区相交，需要特殊处理
                    CoordinateSequence *cl = new CoordinateArraySequence();
                    cl->add(dst_cs->getAt(i - 1));
                    cl->add(dst_cs->getAt(i));
                    shared_ptr<geos::geom::LineString> tempLine(gf->createLineString(cl));
                    if (geom_buffer->intersects(tempLine.get())) {
                        shared_ptr<LineRelation> relation1 = make_shared<LineRelation>();
                        relation1->index = i - 1;
                        relation1->relation = true;
                        lineRelations.emplace_back(relation1);

                        shared_ptr<LineRelation> relation2 = make_shared<LineRelation>();
                        relation2->index = i;
                        relation2->relation = false;
                        lineRelations.emplace_back(relation2);
                    }
                }

                //记录一下当前的状态，以便下一次判断使用
                firstRelation = curRelation;
            }

            delete geom_buffer;

            //check relation
//            cout << "check relations -----------------------" << endl;
//            for(shared_ptr<LineRelation> relation : lineRelations){
//                cout << "index:" << relation->index << ", relation: " << relation->relation << endl;
//            }

            return true;
        }

        //二维线段相交，同时z的平均值小于1米则认为两段线是相交的
        bool KDGeoUtil::IsLineStringIntersect(const shared_ptr<geos::geom::LineString> &srcLine,
                                              const shared_ptr<geos::geom::LineString> &dstLine,
                                              double bufferLen) {
            const GeometryFactory *gf = GeometryFactory::getDefaultInstance();

            vector<shared_ptr<LineRelation>> lineRelations;
            if (calIntersectParts(srcLine, dstLine, lineRelations, bufferLen)) {
                int beginIndex = 0, endIndex = 0;

                for (int i = 0; i < lineRelations.size(); i++) {
                    shared_ptr<LineRelation> relation = lineRelations[i];
                    if (relation->relation) {
                        beginIndex = relation->index - 1;
                        if (beginIndex < 0)
                            beginIndex = 0;
                    } else {
                        endIndex = relation->index;
                        if (checkIntersect(dstLine, beginIndex, endIndex, srcLine, bufferLen)) {
                            return true;
                        }
                        continue;
                    }

                    if (i == lineRelations.size() - 1) {
                        endIndex = dstLine->getNumPoints() - 1;
                        if (checkIntersect(dstLine, beginIndex, endIndex, srcLine, bufferLen)) {
                            return true;
                        }
                    }
                }
            }

            return false;
        }

        double KDGeoUtil::calLineZMean(const shared_ptr<geos::geom::LineString> &srcLine,
                                       int beginIndex, int endIndex) {
            double zValueTotal = 0.0;
            CoordinateSequence *cs = srcLine->getCoordinates();
            for (int i = beginIndex; i <= endIndex; i++) {
                const Coordinate &coord = cs->getAt(i);
                zValueTotal += coord.z;
            }

            return zValueTotal / (endIndex - beginIndex + 1);
        }

        bool KDGeoUtil::checkIntersect(const shared_ptr<geos::geom::LineString> &srcLine,
                                       int beginIndex, int endIndex,
                                       const shared_ptr<geos::geom::LineString> &dstLine,
                                       double bufferLen) {
            const GeometryFactory *gf = GeometryFactory::getDefaultInstance();

            CoordinateSequence *srccs = srcLine->getCoordinates();
            double zmean = calLineZMean(srcLine, beginIndex, endIndex);

            geos::geom::CoordinateSequence *cl = new geos::geom::CoordinateArraySequence();
            for (int j = beginIndex; j <= endIndex; j++) {
                cl->add(srccs->getAt(j));
            }

            shared_ptr<geos::geom::LineString> tempLine(gf->createLineString(cl));

            vector<shared_ptr<LineRelation>> lineRelations;
            if (calIntersectParts(tempLine, dstLine, lineRelations, bufferLen)) {
                int beginIndex = 0, endIndex = 0;

                for (int i = 0; i < lineRelations.size(); i++) {
                    shared_ptr<LineRelation> relation = lineRelations[i];
                    if (relation->relation) {
                        beginIndex = relation->index - 1;
                        if (beginIndex < 0)
                            beginIndex = 0;
                    } else {
                        endIndex = relation->index;
                        double zMeanTemp = calLineZMean(dstLine, beginIndex, endIndex);
                        if (fabs(zMeanTemp - zmean) < bufferLen) {
                            return true;
                        }
                        continue;
                    }

                    if (i == lineRelations.size() - 1) {
                        endIndex = dstLine->getNumPoints() - 1;
                        double zMeanTemp = calLineZMean(dstLine, beginIndex, endIndex);
                        if (fabs(zMeanTemp - zmean) < bufferLen) {
                            return true;
                        }
                    }
                }
            }

            return false;
        }





//        /**
//  * 创建三角形追踪区域
//  *
//  * @param startx
//  * @param starty
//  * @param endx
//  * @param endy
//  * @return
//  */
//        geos::geom::Polygon *
//        KDGeoUtil::getTraceTriGeometry(double startx, double starty, double endx, double endy, int type){
//            //虚线跟踪得更远一些，角度更小一些
//            double trace_Angle = 2.0; //2度
//            double trace_Length = 35.0; //30米
//            if (type == 1) {
//                //虚线跟踪角度大一些，距离更近一些
//                trace_Angle = 4.0;
//                trace_Length = 15;
//            }
//            return getTraceTriGeometry(startx, starty, endx, endy, trace_Angle, trace_Length);
//        }
//
//        geos::geom::Polygon *
//        KDGeoUtil::getTraceTriGeometry(double startx, double starty, double endx, double endy,
//                                       double trace_Angle, double trace_Length) {
//
//            double angle = geo::geo_util::calcAngle(startx, starty, endx, endy);
//            double angle_radian = angle; // * M_PI / 180.0;
//
//            double trace_angle_radian = trace_Angle * M_PI / 180.0;
//
//            double newx = startx + trace_Length;
//            double newy = starty;
//            double dy = atan(trace_angle_radian) * trace_Length;
//
//            const geos::geom::GeometryFactory *gf = geos::geom::GeometryFactory::getDefaultInstance();
//
//            geos::geom::CoordinateSequence *cl = new geos::geom::CoordinateArraySequence();
//            cl->add(geos::geom::Coordinate(startx, starty, 0.0));
//
//            double newrotatex, newrotatey;
//            rotate(startx, starty, newx, newy + dy, angle_radian, &newrotatex, &newrotatey);
//            cl->add(geos::geom::Coordinate(newrotatex, newrotatey, 0.0));
//
//            rotate(startx, starty, newx, newy - dy, angle_radian, &newrotatex, &newrotatey);
//            cl->add(geos::geom::Coordinate(newrotatex, newrotatey, 0.0));
//
//            cl->add(geos::geom::Coordinate(startx, starty, 0.0));
//            //创建三角形追踪区域
//            geos::geom::LinearRing *linearRing = gf->createLinearRing(cl);
//            return gf->createPolygon(linearRing, NULL);
//        }
//
//        void KDGeoUtil::rotate(double centx, double centy, double x, double y, double radius, double *newx,
//                               double *newy) {
//
//            Eigen::AngleAxisd rotation_vector(radius, Eigen::Vector3d(0, 0, 1));
//
//            Eigen::Vector3d org(x, y, 0.0);
//            Eigen::Vector3d cent(centx, centy, 0.0);
//            Eigen::Vector3d disp_org = org - cent;
//            Eigen::Vector3d v_rotated = rotation_vector * disp_org;
//            Eigen::Vector3d v_final = v_rotated + cent;
//
//            *newx = v_final[0];
//            *newy = v_final[1];
//        }
//
////        double KDGeoUtil::distance(double x1, double y1, double x2, double y2) {
////            double v1 = (x2 - x1);
////            double v2 = (y2 - y1);
////            return sqrt(v1*v1 + v2*v2);
////        }
//
//
////        //计算点到曲线段的最近的索引，并返回投影点
////        double KDGeoUtil::pt2LineDist(vector<shared_ptr<Eigen::Vector3d>> & points, double PtA[],
////                                       double PtB[], double PtC[], int &min_index) {
////
////            if ( points.size() < 2 || !PtB || !PtC) {
////                return -1;
////            }
////
////            double minDistance = 1111111111111111111;
////            double closestPoint[2];
////            double LineSeg[4];
////            int minIndex = -1;
////            for (int i = 0; i < points.size() - 1; i ++) {
////                shared_ptr<Eigen::Vector3d> start = points[i];
////                shared_ptr<Eigen::Vector3d> end = points[i+1];
////
////                LineSeg[0] = (*start)[0];
////                LineSeg[1] = (*start)[1];
////                LineSeg[2] = (*end)[0];
////                LineSeg[3] = (*end)[1];
////                //
////                double dis = geo::geo_util::pt2LineSegmentDist(LineSeg, PtA, PtB);
////                if (dis < minDistance) {
////                    minDistance = dis;
////                    closestPoint[0] = PtB[0];
////                    closestPoint[1] = PtB[1];
////                    PtC[0] = LineSeg[0];
////                    PtC[1] = LineSeg[1];
////                    PtC[2] = LineSeg[2];
////                    PtC[3] = LineSeg[3];
////                    //
////                    minIndex = i;
////                }
////            }
////            min_index = minIndex;
////            PtB[0] = closestPoint[0];
////            PtB[1] = closestPoint[1];
////            //
////            return sqrt((PtA[0] - closestPoint[0]) * (PtA[0] - closestPoint[0]) +
////                        (PtA[1] - closestPoint[1]) * (PtA[1] - closestPoint[1]));
////        }
//
//        double KDGeoUtil::pt2LineDist(geos::geom::CoordinateSequence * cs, double PtA[],
//                                      double PtB[], double PtC[], int &min_index)
//        {
//            if ( cs->size() < 2 || !PtB || !PtC) {
//                return -1;
//            }
//
//            double minDistance = 1111111111111111111;
//            double closestPoint[2];
//            double LineSeg[4];
//            int minIndex = -1;
//            for (int i = 0; i < cs->size() - 1; i ++) {
//                const geos::geom::Coordinate & start = cs->getAt(i);
//                const geos::geom::Coordinate & end = cs->getAt(i+1);
//
//                LineSeg[0] = start.x;
//                LineSeg[1] = start.y;
//                LineSeg[2] = end.x;
//                LineSeg[3] = end.y;
//                //
//                double dis = geo::geo_util::pt2LineSegmentDist(LineSeg, PtA, PtB);
//                if (dis < minDistance) {
//                    minDistance = dis;
//                    closestPoint[0] = PtB[0];
//                    closestPoint[1] = PtB[1];
//                    PtC[0] = LineSeg[0];
//                    PtC[1] = LineSeg[1];
//                    PtC[2] = LineSeg[2];
//                    PtC[3] = LineSeg[3];
//                    //
//                    minIndex = i;
//                }
//            }
//            min_index = minIndex;
//            PtB[0] = closestPoint[0];
//            PtB[1] = closestPoint[1];
//            //
//            return sqrt((PtA[0] - closestPoint[0]) * (PtA[0] - closestPoint[0]) +
//                        (PtA[1] - closestPoint[1]) * (PtA[1] - closestPoint[1]));
//        }
//
//
//        int KDGeoUtil::pt2LineSegmentRelation(double Line[], double PtA[], double PtB[]) {
//            double fLine0 = Line[0];
//            double fLine1 = Line[1];
//            double fLine2 = Line[2];
//            double fLine3 = Line[3];
//            //
//            double fPtA0 = (double) (PtA[0]);
//            double fPtA1 = (double) (PtA[1]);
//            //
//            double fPtB0 = 0.0;
//            double fPtB1 = 0.0;
//
//            double dX = fLine2 - fLine0;
//            double dY = fLine3 - fLine1;
//
//            double dR = -(fLine1 - fPtA1) * dY - (fLine0 - fPtA0) * dX;
//
//            double dL;
//            int retValue = -1;
//            if (dR <= 0) {
//                fPtB0 = fLine0;
//                fPtB1 = fLine1;
//                retValue = 0;
//            } else if (dR >= (dL = dX * dX + dY * dY)) {
//                fPtB0 = fLine2;
//                fPtB1 = fLine3;
//                retValue = 1;
//            } else {
//                fPtB0 = fLine0 + dR * dX / dL;
//                fPtB1 = fLine1 + dR * dY / dL;
//                retValue = 2;
//            }
//
//            PtB[0] = fPtB0;
//            PtB[1] = fPtB1;
//
//            return retValue;
//        }
//
//
//        int KDGeoUtil::pt2LineSegmentRelation(Eigen::Vector3d * lineStart,
//                                              Eigen::Vector3d * lineEnd, Eigen::Vector3d * point)
//        {
//            double fLine0 = (*lineStart)(0);
//            double fLine1 = (*lineStart)(1);
//            double fLine2 = (*lineEnd)(0);
//            double fLine3 = (*lineEnd)(1);
//            //
//            double fPtA0 = (double) ((*point)(0));
//            double fPtA1 = (double) ((*point)(1));
//            //
//
//            double dX = fLine2 - fLine0;
//            double dY = fLine3 - fLine1;
//
//            double dR = -(fLine1 - fPtA1) * dY - (fLine0 - fPtA0) * dX;
//
//            double dL;
//            int retValue = -1;
//            if (dR <= 0) {
//                retValue = 0;
//            } else if (dR >= (dL = dX * dX + dY * dY)) {
//                retValue = 1;
//            } else {
//                retValue = 2;
//            }
//
//            return retValue;
//        }
//
//        bool KDGeoUtil::calIntersectParts(const shared_ptr<geos::geom::LineString> & srcLine,
//                                          const shared_ptr<geos::geom::LineString> & dstLine,
//                                          vector<shared_ptr<LineRelation>> & lineRelations,
//                                          double bufferLen)
//        {
//
//            const GeometryFactory * gf = GeometryFactory::getDefaultInstance();
//            Geometry * geom_buffer = srcLine->buffer(bufferLen);
//
//            bool lastRelation = false; //上一次存储的状况
//            bool firstRelation = true; //
//
//            vector<bool> vecRelation;
//            CoordinateSequence * dst_cs = dstLine->getCoordinates();
//            for( int i = 0 ; i < dst_cs->size() ; i ++)
//            {
//                const Coordinate & coord = dst_cs->getAt(i);
//
//                shared_ptr<geos::geom::Point> pointPtr(gf->createPoint(coord));
//                bool curRelation = geom_buffer->intersects(pointPtr.get());
//
//                if(curRelation != lastRelation)
//                {
//                    shared_ptr<LineRelation> relation = make_shared<LineRelation>();
//                    relation->index = i;
//                    relation->relation = curRelation;
//                    lineRelations.emplace_back(relation);
//                    lastRelation = curRelation;
//                }
//
//                if(i > 0 && !curRelation && !firstRelation){
//                    //本次和上次的点位都不在缓冲区内，有个可能是线段同缓冲区相交，需要特殊处理
//                    CoordinateSequence * cl = new CoordinateArraySequence();
//                    cl->add(dst_cs->getAt(i-1));
//                    cl->add(dst_cs->getAt(i));
//                    shared_ptr<geos::geom::LineString> tempLine(gf->createLineString(cl));
//                    if(geom_buffer->intersects(tempLine.get()))
//                    {
//                        shared_ptr<LineRelation> relation1 = make_shared<LineRelation>();
//                        relation1->index = i-1;
//                        relation1->relation = true;
//                        lineRelations.emplace_back(relation1);
//
//                        shared_ptr<LineRelation> relation2 = make_shared<LineRelation>();
//                        relation2->index = i;
//                        relation2->relation = false;
//                        lineRelations.emplace_back(relation2);
//                    }
//                }
//
//                //记录一下当前的状态，以便下一次判断使用
//                firstRelation = curRelation;
//            }
//
//            delete geom_buffer;
//
//            //check relation
////            cout << "check relations -----------------------" << endl;
////            for(shared_ptr<LineRelation> relation : lineRelations){
////                cout << "index:" << relation->index << ", relation: " << relation->relation << endl;
////            }
//
//            return true;
//        }
//
////        bool KDGeoUtil::calIntersectPartsExt(const shared_ptr<geos::geom::LineString> & srcLine,
////                                          const shared_ptr<geos::geom::LineString> & dstLine,
////                                          vector<shared_ptr<LineRelation>> & lineRelations,
////                                          double bufferLen)
////        {
////
////            const GeometryFactory * gf = GeometryFactory::getDefaultInstance();
////            Geometry * geom_buffer = srcLine->buffer(bufferLen);
////            CoordinateSequence * src_cs = srcLine->getCoordinates();
////
////            bool lastRelation = false; //上一次存储的状况
////            bool firstRelation = true; //
////
////            vector<bool> vecRelation;
////            CoordinateSequence * dst_cs = dstLine->getCoordinates();
////            for( int i = 0 ; i < dst_cs->size() ; i ++)
////            {
////                const Coordinate & coord = dst_cs->getAt(i);
////
////                shared_ptr<geos::geom::Point> pointPtr(gf->createPoint(coord));
////                bool curRelation = false;
////                //水平相交判断
////                if(geom_buffer->intersects(pointPtr.get()))
////                {
////                    if(coordInZBuffer(src_cs, coord, bufferLen)) {
////                        curRelation = true;
////                    }
////                }
////
////                if(curRelation != lastRelation)
////                {
////                    shared_ptr<LineRelation> relation = make_shared<LineRelation>();
////                    relation->index = i;
////                    relation->relation = curRelation;
////                    lineRelations.emplace_back(relation);
////                    lastRelation = curRelation;
////                }
////
////                if(i > 0 && !curRelation && !firstRelation){
////                    //本次和上次的点位都不在缓冲区内，有个可能是线段同缓冲区相交，需要特殊处理
////                    CoordinateSequence * cl = new CoordinateArraySequence();
////                    cl->add(dst_cs->getAt(i-1));
////                    cl->add(dst_cs->getAt(i));
////
////                    shared_ptr<geos::geom::LineString> tempLine(gf->createLineString(cl));
////                    if(geom_buffer->intersects(tempLine.get()))
////                    {
////                        //检查z值
////                        if(coordInZBuffer(src_cs, dst_cs->getAt(i-1), bufferLen) &&
////                                coordInZBuffer(src_cs, dst_cs->getAt(i), bufferLen))
////                        {
////                            shared_ptr<LineRelation> relation1 = make_shared<LineRelation>();
////                            relation1->index = i-1;
////                            relation1->relation = true;
////                            lineRelations.emplace_back(relation1);
////
////                            shared_ptr<LineRelation> relation2 = make_shared<LineRelation>();
////                            relation2->index = i;
////                            relation2->relation = false;
////                            lineRelations.emplace_back(relation2);
////                        }
////                    }
////                }
////
////                //记录一下当前的状态，以便下一次判断使用
////                firstRelation = curRelation;
////            }
////
////            delete geom_buffer;
////
////            //check relation
//////            cout << "check relations -----------------------" << endl;
//////            for(shared_ptr<LineRelation> relation : lineRelations){
//////                cout << "index:" << relation->index << ", relation: " << relation->relation << endl;
//////            }
////
////            return true;
////        }
////
////        bool KDGeoUtil::coordInZBuffer(geos::geom::CoordinateSequence * cs, const Coordinate & coord, double bufferLen)
////        {
////            double PtA[2];
////            PtA[0] = coord.x;
////            PtA[1] = coord.y;
////            double PtB[2] = {0.0, 0.0};
////            double PtC[4] = {0.0, 0.0, 0.0, 0.0};
////            int min_index;
////
////            pt2LineDist(cs, PtA, PtB, PtC, min_index);
////
////            const Coordinate & compCoord = cs->getAt(min_index);
////            bool inBuffer = false;
////            if( fabs(coord.z - compCoord.z ) < bufferLen)
////                inBuffer = true;
////            return inBuffer;
////        }
//
//        //二维线段相交，同时z的平均值小于1米则认为两段线是相交的
//        bool KDGeoUtil::IsLineStringIntersect(const shared_ptr<geos::geom::LineString> & srcLine,
//                                              const shared_ptr<geos::geom::LineString> & dstLine,
//                                              double bufferLen)
//        {
//            const GeometryFactory * gf = GeometryFactory::getDefaultInstance();
//
//            vector<shared_ptr<LineRelation>> lineRelations;
//            if(calIntersectParts(srcLine, dstLine, lineRelations, bufferLen))
//            {
//                int beginIndex = 0, endIndex = 0;
//
//                for( int i = 0 ; i < lineRelations.size() ; i ++)
//                {
//                    shared_ptr<LineRelation> relation = lineRelations[i];
//                    if(relation->relation) {
//                        beginIndex = relation->index - 1;
//                        if(beginIndex < 0)
//                            beginIndex = 0;
//                    }else {
//                        endIndex = relation->index;
//                        if(checkIntersect(dstLine, beginIndex, endIndex, srcLine, bufferLen))
//                        {
//                            return true;
//                        }
//                        continue;
//                    }
//
//                    if(i == lineRelations.size() - 1) {
//                        endIndex = dstLine->getNumPoints() - 1;
//                        if(checkIntersect(dstLine, beginIndex, endIndex, srcLine, bufferLen))
//                        {
//                            return true;
//                        }
//                    }
//                }
//            }
//
//            return false;
//        }
//
//        double KDGeoUtil::calLineZMean(const shared_ptr<geos::geom::LineString> & srcLine,
//                                       int beginIndex, int endIndex)
//        {
//            double zValueTotal = 0.0;
//            CoordinateSequence * cs = srcLine->getCoordinates();
//            for( int i = beginIndex ; i <= endIndex ; i ++ )
//            {
//                const Coordinate & coord = cs->getAt(i);
//                zValueTotal += coord.z;
//            }
//
//            return zValueTotal / (endIndex - beginIndex + 1);
//        }
//
//        bool KDGeoUtil::checkIntersect(const shared_ptr<geos::geom::LineString> & srcLine,
//                                       int beginIndex, int endIndex,
//                                       const shared_ptr<geos::geom::LineString> & dstLine,
//                                       double bufferLen)
//        {
//            const GeometryFactory * gf = GeometryFactory::getDefaultInstance();
//
//            CoordinateSequence * srccs = srcLine->getCoordinates();
//            double zmean = calLineZMean(srcLine, beginIndex, endIndex);
//
//            geos::geom::CoordinateSequence *cl = new geos::geom::CoordinateArraySequence();
//            for( int j = beginIndex ; j <= endIndex ; j ++ )
//            {
//                cl->add(srccs->getAt(j));
//            }
//
//            shared_ptr<geos::geom::LineString> tempLine(gf->createLineString(cl));
//
//            vector<shared_ptr<LineRelation>> lineRelations;
//            if(calIntersectParts(tempLine, dstLine, lineRelations, bufferLen))
//            {
//                int beginIndex = 0, endIndex = 0;
//
//                for( int i = 0 ; i < lineRelations.size() ; i ++)
//                {
//                    shared_ptr<LineRelation> relation = lineRelations[i];
//                    if(relation->relation) {
//                        beginIndex = relation->index - 1;
//                        if(beginIndex < 0)
//                            beginIndex = 0;
//                    }else {
//                        endIndex = relation->index;
//                        double zMeanTemp = calLineZMean(dstLine, beginIndex, endIndex);
//                        if(fabs(zMeanTemp-zmean) < bufferLen)
//                        {
//                            return true;
//                        }
//                        continue;
//                    }
//
//                    if(i == lineRelations.size() - 1) {
//                        endIndex = dstLine->getNumPoints() - 1;
//                        double zMeanTemp = calLineZMean(dstLine, beginIndex, endIndex);
//                        if(fabs(zMeanTemp-zmean) < bufferLen)
//                        {
//                            return true;
//                        }
//                    }
//                }
//            }
//
//            return false;
//        }


        bool KDGeoUtil::getDifferencePoint(const geos::geom::Coordinate * start, const geos::geom::Coordinate * end,
                                       geos::geom::Coordinate * diffPoint, double length){
            if(start == nullptr || end == nullptr || diffPoint == nullptr)
                return false;

            double totalLen = getDistance(start, end);
            if (totalLen == 0.0) {
                diffPoint->x = start->x;
                diffPoint->y = start->y;
                diffPoint->z = start->z;
                return true;
            }

            diffPoint->x = (end->x - start->x) * length / totalLen + start->x;
            diffPoint->y = (end->y - start->y) * length / totalLen + start->y;
            diffPoint->z = (end->z - start->z) * length / totalLen + start->z;

            return true;
        }

        bool KDGeoUtil::getDifferencePoint(double x1, double y1, double x2, double y2,
                                           double & x3, double & y3, double length){
            double totalLen = distance(x1, y1, x2, y2);
            if(totalLen == 0.0){
                x3 = x1;
                y3 = y1;
                return true;
            }

            x3 = (x2 - x1) * length / totalLen + x1;
            y3 = (y2 - y1) * length / totalLen + y1;
            return true;
        }




//        bool KDGLineFittingUtil::polynomialCurveFit(std::vector<cv::Point3d> &key_point, int n, cv::Mat &A) {
//
//            //Number of key points
//            int N = key_point.size();
//
//            //构造矩阵X
//            cv::Mat X = cv::Mat::zeros(n + 1, n + 1, CV_64FC1);
//            for (int i = 0; i < n + 1; i++)
//            {
//                for (int j = 0; j < n + 1; j++)
//                {
//                    for (int k = 0; k < N; k++)
//                    {
//                        X.at<double>(i, j) = X.at<double>(i, j) +
//                                             std::pow(key_point[k].x, i + j);
//                    }
//                }
//            }
//
//            //构造矩阵Y
//            cv::Mat Y = cv::Mat::zeros(n + 1, 1, CV_64FC1);
//            for (int i = 0; i < n + 1; i++)
//            {
//                for (int k = 0; k < N; k++)
//                {
//                    Y.at<double>(i, 0) = Y.at<double>(i, 0) +
//                                         std::pow(key_point[k].x, i) * key_point[k].y;
//                }
//            }
//
//            A = cv::Mat::zeros(n + 1, 1, CV_64FC1);
//            //求解矩阵A
//            cv::solve(X, Y, A, cv::DECOMP_LU);
//            return true;
//        }

    }
}
