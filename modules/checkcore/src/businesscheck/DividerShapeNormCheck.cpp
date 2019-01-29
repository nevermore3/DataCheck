//
// Created by gaoyanhong on 2018/3/30.
//

#include "businesscheck/DividerShapeNormCheck.h"

//core
#include "geom/geo_util.h"

using namespace geo;

//module
#include "DataCheckConfig.h"
#include "util/KDGeoUtil.hpp"

using namespace kd::automap;

namespace kd {
    namespace dc {


        string DividerShapeNormCheck::getId() {
            return id;
        }

        //存在夹角<135°的弧段
        void DividerShapeNormCheck::check_JH_C_7(shared_ptr<MapDataManager> mapDataManager,
                                                 shared_ptr<CheckErrorOutput> errorOutput) {

            double edgeAngle = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::DIVIDER_EDGE_ANGLE);

            for (auto recordit : mapDataManager->dividers_) {
                shared_ptr<DCDivider> div = recordit.second;
                if (!div->valid_)
                    continue;

                int nodeCount = div->nodes_.size();
                if (nodeCount <= 2)
                    continue;

                for (int i = 2; i < nodeCount; i++) {

                    auto node1 = div->nodes_[i - 2];
                    auto node2 = div->nodes_[i - 1];
                    auto node3 = div->nodes_[i];

                    double angle1 = geo_util::calcAngle(node1->coord_.lng_, node1->coord_.lat_, node2->coord_.lng_,
                                                        node2->coord_.lat_);
                    double angle2 = geo_util::calcAngle(node2->coord_.lng_, node2->coord_.lat_, node3->coord_.lng_,
                                                        node3->coord_.lat_);

                    double dAngle = fabs(angle1 - angle2);
                    if (dAngle > 180)
                        dAngle = 360 - dAngle;
                    if (dAngle > (180-edgeAngle)) {
                        shared_ptr<DCDividerCheckError> error =
                                DCDividerCheckError::createByNode("KXS-01-011", div, node1);
                        error->checkDesc_ = "车道线不平滑夹角<135";
                        stringstream ss;
                        ss << "divider:" << div->id_ << ",node_id1:" << node1->id_ << ",node_id2:" << node2->id_
                        << ",node_id3:" << node3->id_ << "夹角：" << dAngle << "度";
                        error->errorDesc_ = ss.str();
                        error->checkLevel_ = LEVEL_WARNING;

                        errorOutput->saveError(error);
                    }
                }
            };
        }


        //存在长度小于0.2米的弧段; 车道线高程突变>±10厘米/米
        void DividerShapeNormCheck::check_JH_C_8_AND_JH_C_9(shared_ptr<MapDataManager> mapDataManager,
                                                            shared_ptr<CheckErrorOutput> errorOutput) {

            double nodespace = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::DIVIDER_NODE_SPACE_LEN);
            double heightchange = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::DIVIDER_HEIGHT_CHANGE_PER_METER);

            for (auto recordit : mapDataManager->dividers_) {
                shared_ptr<DCDivider> div = recordit.second;
                if (!div->valid_)
                    continue;

                int nodeCount = div->nodes_.size();
                if (nodeCount < 2)
                    continue;

                for (int i = 1; i < nodeCount; i++) {

                    auto node1 = div->nodes_[i - 1];
                    auto node2 = div->nodes_[i];

                    //间距判断
                    double distance = KDGeoUtil::distanceLL(node1->coord_.lng_, node1->coord_.lat_, node2->coord_.lng_, node2->coord_.lat_);
                    if(distance < nodespace){
                        shared_ptr<DCDividerCheckError> error =
                                DCDividerCheckError::createByNode("KXS-01-012", div, node1);
                        error->checkDesc_ = "车道线两个节点长度<0.2米";
                        stringstream ss;
                        ss << "divider:" << div->id_ << ",node_id:" << node1->id_ << "与node_id:" << node2->id_
                           << "距离" << distance << "米";
                        error->errorDesc_ = ss.str();

                        errorOutput->saveError(error);
                    }

                    //坡度判断
                    double slopLimit = distance * heightchange;
                    double realDeltaZ = node1->coord_.z_ - node2->coord_.z_;
                    if(fabs(realDeltaZ) > slopLimit ){
                        shared_ptr<DCDividerCheckError> error =
                                DCDividerCheckError::createByNode("KXS-01-013", div, node1);
                        error->checkDesc_ = "车道线高程突变>±10厘米";
                        stringstream ss;
                        ss << "divider:" << div->id_ << ",node_id:" << node1->id_ << ",node_id:" << node2->id_;
                        error->errorDesc_ = ss.str();

                        errorOutput->saveError(error);
                    }
                }
            }
        }

        bool DividerShapeNormCheck::execute(shared_ptr<MapDataManager> mapDataManager,
                                            shared_ptr<CheckErrorOutput> errorOutput) {
            if (mapDataManager == nullptr)
                return false;

            errorOutput->writeInfo("[DividerShapeNormCheck]\n" + make_shared<DCDividerCheckError>("")->getHeader(), false);

            check_JH_C_7(mapDataManager, errorOutput);

            check_JH_C_8_AND_JH_C_9(mapDataManager, errorOutput);

            return true;
        }

    }
}


