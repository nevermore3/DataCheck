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

        //车道面的4个夹角<45°或者>135°
        void LaneShapeNormCheck::check_JH_C_15(shared_ptr<MapDataManager> mapDataManager,
                                                 shared_ptr<CheckErrorOutput> errorOutput) {

            double edgeAngle = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::DIVIDER_EDGE_ANGLE);

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

                //检查各个夹角
                shared_ptr<DCDivider> leftDiv = lane->leftDivider_;
                shared_ptr<DCDivider> rightDiv = lane->rightDivider_;

                //检查两个车道线的通行方向是否一致
                //检查左上角

                shared_ptr<DCDividerNode> node1 = leftDiv->nodes_[1];
                shared_ptr<DCDividerNode> node2 = leftDiv->nodes_[0];
                shared_ptr<DCDividerNode> node3 = rightDiv->nodes_[0];




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

            check_JH_C_15(mapDataManager, errorOutput);

            check_JH_C_17(mapDataManager, errorOutput);

            return true;
        }

    }
}