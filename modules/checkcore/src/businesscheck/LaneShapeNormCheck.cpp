//
// Created by gaoyanhong on 2018/4/2.
//

//third party
#include "geos/indexQuadtree.h"
#include "geos/geom/GeometryFactory.h"
#include "geos/geom/CoordinateSequence.h"
#include "geos/geom/CoordinateArraySequence.h"
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

        //计算线段的角度
        double calcAngle(double sLng, double sLat, double eLng, double eLat){
            double y = eLat - sLat;
            double x = eLng - sLng;
            return (0 == x) ? 90.0 : (atan(y/x)*180/3.1415926);
        }

        //构建多边形
        shared_ptr<geos::geom::Polygon> createPolygon(const shared_ptr<geos::geom::LineString> leftline,
                                                      const shared_ptr<geos::geom::LineString> rightline){

            geos::geom::CoordinateSequence* csLeft = leftline->getCoordinates();
            geos::geom::CoordinateSequence* csRight = rightline->getCoordinates();

            //判断左右分割线的构建多边形是否存在自相交
            geos::geom::Coordinate lsCoord = csLeft->getAt(0);
            geos::geom::Coordinate leCoord = csLeft->getAt(csLeft->getSize()-1);
            geos::geom::Coordinate rsCoord = csRight->getAt(0);
            geos::geom::Coordinate reCoord = csRight->getAt(csRight->getSize()-1);
            double LineA[] = {lsCoord.x, lsCoord.y, rsCoord.x, rsCoord.y};
            double LineB[] = {leCoord.x, leCoord.y, reCoord.x, reCoord.y};
            bool bReverse = false;
            if (geo::geo_util::isLineSegmentCross(LineA, LineB)){
                bReverse = true;
            }

            //构建多边形点序列
            geos::geom::CoordinateSequence * cs = new geos::geom::CoordinateArraySequence();
            for (int i = 0; i < csLeft->getSize(); ++i) {
                cs->add(csLeft->getAt(i));
            }

            if (bReverse){
                for (int j = 0; j < csRight->getSize(); ++j) {
                    cs->add(csRight->getAt(j));
                }
            }else{
                for (int j = csRight->getSize()-1; j >= 0; --j) {
                    cs->add(csRight->getAt(j));
                }
            }
            cs->add(csLeft->getAt(0));

            //构建geos多边形对象
            const geos::geom::GeometryFactory *gf = geos::geom::GeometryFactory::getDefaultInstance();
            geos::geom::LinearRing* lr = gf->createLinearRing(cs);
            shared_ptr<geos::geom::Polygon> poly(gf->createPolygon(lr, NULL));
            //const geos::geom::Envelope* env = poly->getEnvelopeInternal();//不调用此接口返回智能指针后面在调用getEnvelopeInternal就会异常
            return poly;
        }

        //缓冲范围内查找车道
        bool findPloygons(const shared_ptr<geos::geom::Polygon> poly, double bufferLen,
                          const shared_ptr<geos::index::quadtree::Quadtree> & quadtree, vector<shared_ptr<DCLane>>& rtnLanes){

            const GeometryFactory *gf = GeometryFactory::getDefaultInstance();
            const geos::geom::Envelope* envelope = poly->getEnvelopeInternal();
            shared_ptr<geos::geom::Geometry> geoEnv(gf->toGeometry(envelope));
            shared_ptr<geos::geom::Geometry> geoEnvBuffer(geoEnv->buffer(bufferLen));
            vector<void*> lanes;
            quadtree->query(geoEnvBuffer->getEnvelopeInternal(), lanes);
            for (auto lane : lanes) {
                rtnLanes.push_back(make_shared<DCLane>(*((DCLane*)lane)));
            }
            return (rtnLanes.size()>0)?true:false;
        }

        //车道面和其他车道面相交
        void LaneShapeNormCheck::check_JH_C_14(shared_ptr<MapDataManager> mapDataManager,
                                               shared_ptr<CheckErrorOutput> errorOutput) {
            double bufferLen = 1;
            double overlapArea = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::LANE_OVERLAP_AREA);
            map<shared_ptr<DCLane>, shared_ptr<geos::geom::Polygon>> mLanePoly;

            //构建车道多边形
            for (auto recordit : mapDataManager->lanes_) {
                shared_ptr<DCLane> lane = recordit.second;
                if (!lane->valid_)
                    continue;

                if(lane->leftDivider_ == nullptr || lane->leftDivider_->nodes_.size() < 2 ||
                   lane->rightDivider_ == nullptr || lane->rightDivider_->nodes_.size() < 2)
                {
                    stringstream ss;
                    ss << "[Error] lane divider info error.";
                    errorOutput->writeInfo(ss.str());
                    continue;
                }

                shared_ptr<geos::geom::Polygon> poly = createPolygon(lane->leftDivider_->line_, lane->rightDivider_->line_);
                mLanePoly.emplace(make_pair(lane, poly));
            }

            //构建空间索引存储车道面信息
            shared_ptr<geos::index::quadtree::Quadtree> quadtree = make_shared<geos::index::quadtree::Quadtree>();
            for (auto itPoly : mLanePoly){
                quadtree->insert(itPoly.second->getEnvelopeInternal(), itPoly.first.get());
            }

            //判断每个车道面关联车道面空间关系
            for (auto itPoly : mLanePoly) {
                shared_ptr<DCLane> curLane = itPoly.first;
                shared_ptr<geos::geom::Polygon> curPoly = itPoly.second;

                vector<shared_ptr<DCLane>> vlanes;
                if (findPloygons(curPoly, bufferLen, quadtree, vlanes)){
                    for (auto itLane : vlanes){
                        shared_ptr<DCLane> nxtLane = itLane;
                        auto itNxtLane = mLanePoly.find(nxtLane);
                        if (nxtLane == curLane || itNxtLane == mLanePoly.end())
                            continue;

                        shared_ptr<geos::geom::Polygon> nxtPoly = itNxtLane->second;
                        if (!(curPoly.get())->intersects(nxtPoly.get()))
                            continue;

                        //获取多边形相交部分
                        shared_ptr<geos::geom::Geometry> geo((curPoly.get())->intersection(nxtPoly.get()));
                        if (nullptr == geo)
                            continue;

                        geos::geom::GeometryTypeId geometryTypeId = geo->getGeometryTypeId();
                        if (geo->getCoordinates()->size() < 3
                            || !(geos::geom::GEOS_POLYGON == geometryTypeId
                                 || geos::geom::GEOS_MULTIPOLYGON == geometryTypeId))
                            continue;

                        //判断重叠面积是否超过限制
                        double area = geo->getArea();
                        if (area > overlapArea){
                            shared_ptr<DCLaneCheckError> error = DCLaneCheckError::createByAtt("KXS-05-006", curLane, nullptr);
                            error->checkDesc_ = "车道面和其他车道面相交";
                            stringstream ss;
                            ss << "lane_id:" << curLane->id_ << "与lane_id:" << nxtLane->id_ << "相交";
                            error->errorDesc_ = ss.str();
                            errorOutput->saveError(error);
                        }
                    }
                }
            }
        }


        //车道面的4个夹角<45°或者>135°
        void LaneShapeNormCheck::check_JH_C_15(shared_ptr<MapDataManager> mapDataManager,
                                                 shared_ptr<CheckErrorOutput> errorOutput) {
            double edgeMaxAngle = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::LANE_EDGE_MAX_ANGLE);
            double edgeMinAngle = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::LANE_EDGE_MIN_ANGLE);

            for (auto recordit : mapDataManager->lanes_) {
                shared_ptr<DCLane> lane = recordit.second;

                if (!lane->valid_)
                    continue;

                if(lane->leftDivider_ == nullptr || lane->leftDivider_->nodes_.size() < 2 ||
                        lane->rightDivider_ == nullptr || lane->rightDivider_->nodes_.size() < 2)
                {
                    stringstream ss;
                    ss << "[Error] lane divider info error.";
                    errorOutput->writeInfo(ss.str());
                    continue;
                }

                //检查车道面四个角点的夹角
                shared_ptr<DCDividerNode> lsNode = lane->getPassDividerNode(true, true);
                shared_ptr<DCDividerNode> leNode = lane->getPassDividerNode(true, false);
                shared_ptr<DCDividerNode> rsNode = lane->getPassDividerNode(false, true);
                shared_ptr<DCDividerNode> reNode = lane->getPassDividerNode(false, false);

                auto ls0 = lsNode->coord_;
                auto ls1 = leNode->coord_;
                auto le1 = lsNode->coord_;
                auto le0 = leNode->coord_;
                if (lsNode->id_ == lane->leftDivider_->nodes_[0]->id_){
                    ls1 = lane->leftDivider_->nodes_[1]->coord_;
                    le1 = lane->leftDivider_->nodes_[lane->leftDivider_->nodes_.size()-2]->coord_;
                }else{
                    ls1 = lane->leftDivider_->nodes_[lane->leftDivider_->nodes_.size()-2]->coord_;
                    le1 = lane->leftDivider_->nodes_[1]->coord_;
                }

                auto rs0 = rsNode->coord_;
                auto rs1 = reNode->coord_;
                auto re1 = rsNode->coord_;
                auto re0 = reNode->coord_;
                if (rsNode->id_ == lane->rightDivider_->nodes_[0]->id_){
                    rs1 = lane->rightDivider_->nodes_[1]->coord_;
                    re1 = lane->rightDivider_->nodes_[lane->rightDivider_->nodes_.size()-2]->coord_;
                }else{
                    rs1 = lane->rightDivider_->nodes_[lane->rightDivider_->nodes_.size()-2]->coord_;
                    re1 = lane->rightDivider_->nodes_[1]->coord_;
                }

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

                double angle = 0.0;
                string corner = "";
                string nodeId = "";
                bool bError = true;
                if (ltAngle < edgeMinAngle || ltAngle > edgeMaxAngle){
                    angle = ltAngle;
                    corner = "left_start";
                    nodeId = lsNode->id_;
                }else if (lbAngle < edgeMinAngle || lbAngle > edgeMaxAngle){
                    angle = lbAngle;
                    corner = "left_end";
                    nodeId = leNode->id_;
                }else if (rtAngle < edgeMinAngle || rtAngle > edgeMaxAngle){
                    angle = rtAngle;
                    corner = "right_start";
                    nodeId = rsNode->id_;
                }else if (rbAngle < edgeMinAngle || rbAngle > edgeMaxAngle) {
                    angle = rbAngle;
                    corner = "right_end";
                    nodeId = reNode->id_;
                }else {
                    bError = false;
                }

                if (bError){
                    shared_ptr<DCLaneCheckError> error = DCLaneCheckError::createByAtt("KXS-05-007", lane, nullptr);
                    error->checkDesc_ = "车道面的4个角点构成夹角<45°或者>135°";
                    stringstream ss;
                    ss << "lane_id:" << lane->id_ << ",left_divider:" << lane->leftDivider_->id_ << ",right_divider:"
                       << lane->rightDivider_->id_;
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
                    stringstream ss;
                    ss << "[Error] divider no spatial info.";
                    errorOutput->writeInfo(ss.str());
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
                                        DCLaneCheckError::createByNode("KXS-05-009", lane, rightDiv->nodes_[i]);
                                error->checkDesc_ = "车道宽度最窄处不能<2.5米";
                                stringstream ss;
                                ss << "divider:" << rightDiv->id_ << ",nodeid:" << dcNode->id_ << "与divider:"
                                   << leftDiv->id_ << "距离" << length;
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
                                        DCLaneCheckError::createByNode("KXS-05-009", lane, rightDiv->nodes_[i]);
                                error->checkDesc_ = "车道宽度最大不能>7米";
                                stringstream ss;
                                ss << "divider:" << rightDiv->id_ << "，nodeid:" << dcNode->id_ << "与divider:"
                                   << leftDiv->id_ << "距离" << length;
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

            errorOutput->writeInfo("[LaneShapeNormCheck]\n" + make_shared<DCLaneCheckError>("")->getHeader(), false);

            check_JH_C_14(mapDataManager, errorOutput);

            check_JH_C_15(mapDataManager, errorOutput);

            check_JH_C_17(mapDataManager, errorOutput);

            return true;
        }

    }
}