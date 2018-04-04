//
// Created by gaoyanhong on 2018/4/2.
//

#include <geos/geom/GeometryFactory.h>
#include "businesscheck/LaneShapeNormCheck.h"

//core
#include "geom/geo_util.h"

using namespace geo;

//module
#include "DataCheckConfig.h"
#include "util/KDGeoUtil.hpp"

using namespace kd::automap;

namespace kd {
    namespace dc {


        string LaneShapeNormCheck::getId() {
            return id;
        }


        double calAngle(shared_ptr<DCDivider> leftDiv, bool leftStart, shared_ptr<DCDivider> rightDiv, bool rightStart){
            return 0.0;
        }

        double calcAngle(double sLng, double sLat, double eLng, double eLat){
            double y = eLat - sLat;
            double x = eLng - sLng;
            return (0 == x) ? 90.0 : (atan(y/x)*180/3.1415926);
        }

        //车道面和其他车道面相交
        void LaneShapeNormCheck::check_JH_C_14(shared_ptr<MapDataManager> mapDataManager,
                                               shared_ptr<CheckErrorOutput> errorOutput) {

            double overlapArea = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::LANE_OVERLAP_AREA);

//            map<shared_ptr<DCLane>, geos::geom::Polygon> vLanePoly;
//
//            for (auto recordit : mapDataManager->lanes_) {
//                shared_ptr<DCLane> lane = recordit.second;
//                if (!lane->valid_)
//                    continue;
//
//                if(lane->leftDivider_ == nullptr || lane->leftDivider_->nodes_.size() < 2 ||
//                   lane->rightDivider_ == nullptr || lane->rightDivider_->nodes_.size() < 2)
//                {
//                    cout << "[Error] lane divider info error." << endl;
//                    continue;
//                }
//
//                //构建车道多边形
//                shared_ptr<DCDivider> leftDiv = lane->leftDivider_;
//                shared_ptr<DCDivider> rightDiv = lane->rightDivider_;
//                //geos::geom::Polygon poly();
//                //vLanePoly.emplace(make_pair(lane, poly));
//            };
//
//            for (auto itPoly : vLanePoly){
//                ;
//            }
        }


        //车道面的4个夹角<45°或者>135°
        void LaneShapeNormCheck::check_JH_C_15(shared_ptr<MapDataManager> mapDataManager,
                                                 shared_ptr<CheckErrorOutput> errorOutput) {

            double edgeMaxAngle = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::LANE_EDGE_MAX_ANGLE);
            double edgeMinAngle = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::LANE_EDGE_MIN_ANGLE);

            for (auto recordit : mapDataManager->lanes_) {
                shared_ptr<DCLane> lane = recordit.second;
                //约定：此处默认约定首先进行了"JH_C_16"检查项，对方向不匹配的现象进行了标识，标识了的对象不再进行夹角检查
                if (!lane->valid_)
                    continue;

                if(lane->leftDivider_ == nullptr || lane->leftDivider_->nodes_.size() < 2 ||
                        lane->rightDivider_ == nullptr || lane->rightDivider_->nodes_.size() < 2)
                {
                    cout << "[Error] lane divider info error." << endl;
                    continue;
                }

                //检查车道面四个角点的夹角
                shared_ptr<DCDivider> leftDiv = lane->leftDivider_;
                shared_ptr<DCDivider> rightDiv = lane->rightDivider_;
                auto ls0 = leftDiv->nodes_[0]->coord_;//corner point
                auto ls1 = leftDiv->nodes_[1]->coord_;
                auto le1 = leftDiv->nodes_[leftDiv->nodes_.size()-2]->coord_;
                auto le0 = leftDiv->nodes_[leftDiv->nodes_.size()-1]->coord_;//corner point
                auto rs0 = rightDiv->nodes_[0]->coord_;//corner point
                auto rs1 = rightDiv->nodes_[1]->coord_;
                auto re1 = rightDiv->nodes_[rightDiv->nodes_.size()-2]->coord_;
                auto re0 = rightDiv->nodes_[rightDiv->nodes_.size()-1]->coord_;//corner point

                double ltAngle = fabs(calcAngle(ls0.lng_, ls0.lat_, ls1.lng_, ls1.lat_)
                                      - calcAngle(ls0.lng_, ls0.lat_, rs0.lng_, rs0.lat_));
                ltAngle =  (ltAngle > 180)? (360 - ltAngle) : ltAngle;

                double lbAngle = fabs(calcAngle(rs0.lng_, rs0.lat_, rs1.lng_, rs1.lat_)
                                      - calcAngle(rs0.lng_, rs0.lat_, ls0.lng_, ls0.lat_));
                lbAngle =  (lbAngle > 180)? (360 - lbAngle) : lbAngle;

                double rtAngle = fabs(calcAngle(le0.lng_, le0.lat_, le1.lng_, le1.lat_)
                                      - calcAngle(le0.lng_, le0.lat_, re0.lng_, re0.lat_));
                rtAngle =  (rtAngle > 180)? (360 - rtAngle) : rtAngle;

                double rbAngle = fabs(calcAngle(re0.lng_, re0.lat_, le0.lng_, le0.lat_)
                                      - calcAngle(re0.lng_, re0.lat_, re1.lng_, re1.lat_));
                rbAngle =  (rbAngle > 180)? (360 - rbAngle) : rbAngle;

                if (ltAngle < edgeMinAngle || ltAngle > edgeMaxAngle
                        ||lbAngle < edgeMinAngle || lbAngle > edgeMaxAngle
                        ||rtAngle < edgeMinAngle || rtAngle > edgeMaxAngle
                        ||rbAngle < edgeMinAngle || rbAngle > edgeMaxAngle){
                    shared_ptr<DCLaneCheckError> error =
                            DCLaneCheckError::createByAtt("JH_C_15", lane, nullptr);
                    stringstream ss;
                    ss << "lane face corner is not in " << edgeMinAngle << "~" << edgeMaxAngle;
                    error->errorDesc_ = ss.str();
                    errorOutput->saveError(error);
                    lane->valid_ = false;
                }
            };
        }


        //车道宽度最窄处不能<2.5米，最大不能>7米
        void LaneShapeNormCheck::check_JH_C_17(shared_ptr<MapDataManager> mapDataManager,
                                                            shared_ptr<CheckErrorOutput> errorOutput) {

            double laneWidthMax = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::LANE_WIDTH_MAX);
            double laneWidthMin = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::LANE_WIDTH_MIN);

            const GeometryFactory * gf = GeometryFactory::getDefaultInstance();

            for (auto recordit : mapDataManager->lanes_) {
                shared_ptr<DCLane> lane = recordit.second;
                if (!lane->valid_)
                    continue;

                shared_ptr<DCDivider> leftDiv = lane->leftDivider_;
                shared_ptr<DCDivider> rightDiv = lane->rightDivider_;

                if(leftDiv == nullptr || leftDiv->line_ == nullptr || rightDiv == nullptr || rightDiv->line_ == nullptr){
                    cout << "[Error] divider no spatial info." << endl;
                    continue;
                }

                //检查最小距离
                {
                    shared_ptr<Geometry> minWidthBuffer(leftDiv->line_->buffer(laneWidthMin));
                    geos::geom::CoordinateSequence *cs = rightDiv->line_->getCoordinates();
                    for (int i = 0; i < cs->size(); i++) {
                        const geos::geom::Coordinate &coord = cs->getAt(i);
                        shared_ptr<geos::geom::Point> point(gf->createPoint(coord));
                        //目前只判断了平面距离，立交桥是否出错？ //TODO
                        if (minWidthBuffer->contains(point.get())) {

                            //计算距离
                            shared_ptr<DCDividerNode> dcNode = rightDiv->nodes_[i];
                            double PtA[2];
                            PtA[0] = dcNode->coord_.lng_;
                            PtA[1] = dcNode->coord_.lat_;
                            double PtB[2] = {0.0, 0.0};
                            double PtC[4] = {0.0, 0.0, 0.0, 0.0};
                            int index;

                            double length = KDGeoUtil::pt2LineDist(leftDiv->line_->getCoordinates(), PtA, PtB, PtC,
                                                                   index);
                            if (length < laneWidthMin) {
                                shared_ptr<DCLaneCheckError> error =
                                        DCLaneCheckError::createByNode("JH_C_17", lane, rightDiv->nodes_[i]);

                                stringstream ss;
                                ss << "[divider:" << rightDiv->id_ << "][nodeid:[" << dcNode->id_ << "] distance to ";
                                ss << "[divider:" << leftDiv->id_ << "] is " << length;
                                error->errorDesc_ = ss.str();

                                errorOutput->saveError(error);
                                break;
                            }
                        }
                    }
                }


                //检查最大距离
                {
                    shared_ptr<Geometry> maxWidthBuffer(leftDiv->line_->buffer(laneWidthMax));
                    geos::geom::CoordinateSequence *cs = rightDiv->line_->getCoordinates();
                    for (int i = 0; i < cs->size(); i++) {
                        const geos::geom::Coordinate &coord = cs->getAt(i);
                        shared_ptr<geos::geom::Point> point(gf->createPoint(coord));
                        //目前只判断了平面距离，立交桥是否出错？ //TODO
                        if (!maxWidthBuffer->contains(point.get())) {

                            //计算距离
                            shared_ptr<DCDividerNode> dcNode = rightDiv->nodes_[i];
                            double PtA[2];
                            PtA[0] = dcNode->coord_.lng_;
                            PtA[1] = dcNode->coord_.lat_;
                            double PtB[2] = {0.0, 0.0};
                            double PtC[4] = {0.0, 0.0, 0.0, 0.0};
                            int index;

                            double length = KDGeoUtil::pt2LineDist(leftDiv->line_->getCoordinates(), PtA, PtB, PtC,
                                                                   index);
                            if (length > laneWidthMax) {
                                shared_ptr<DCLaneCheckError> error =
                                        DCLaneCheckError::createByNode("JH_C_17", lane, rightDiv->nodes_[i]);

                                stringstream ss;
                                ss << "[divider:" << rightDiv->id_ << "][nodeid:[" << dcNode->id_ << "] distance to ";
                                ss << "[divider:" << leftDiv->id_ << "] is " << length;
                                error->errorDesc_ = ss.str();

                                errorOutput->saveError(error);
                                break;
                            }
                        }
                    }
                }
            }
        }

        bool LaneShapeNormCheck::execute(shared_ptr<MapDataManager> mapDataManager,
                                            shared_ptr<CheckErrorOutput> errorOutput) {
            if (mapDataManager == nullptr)
                return false;

            //check_JH_C_14(mapDataManager, errorOutput);

            check_JH_C_15(mapDataManager, errorOutput);

            check_JH_C_17(mapDataManager, errorOutput);

            return true;
        }

    }
}