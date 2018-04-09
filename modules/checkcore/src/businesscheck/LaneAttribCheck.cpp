//
// Created by gaoyanhong on 2018/4/1.
//

//core
#include "geom/geo_util.h"

using namespace geo;

#include "businesscheck/LaneAttribCheck.h"

#include "DataCheckConfig.h"

#include "util/KDGeoUtil.hpp"

using namespace kd::automap;

namespace kd {
    namespace dc {

        LaneAttribCheck::LaneAttribCheck() {

        }

        string LaneAttribCheck::getId() {
            return id;
        }

        void LaneAttribCheck::check_JH_C_16(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput){
            for (auto recordit : mapDataManager->lanes_) {

                double limitAngle = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::LANE_DIVIDER_DIRANGLE);

                shared_ptr<DCLane> lane = recordit.second;
                if (!lane->valid_)
                    continue;

                if (lane->leftDivider_ == nullptr  || lane->rightDivider_ == nullptr) {
                    lane->valid_ = false;
                    cout << "[Error] lane divider info error." << endl;
                    continue;
                }

                shared_ptr<DCDivider> leftDiv = lane->leftDivider_;
                shared_ptr<DCDivider> rightDiv = lane->rightDivider_;
                if (leftDiv->nodes_.size() < 2 || rightDiv->nodes_.size() < 2){
                    continue;
                }

                //先通过方向属性判断是否冲突
                if ((leftDiv->direction_ != 4 && leftDiv->direction_ == 4)
                    ||(leftDiv->direction_ == 4 && leftDiv->direction_ != 4)){
                    shared_ptr<DCLaneCheckError> error =
                            DCLaneCheckError::createByAtt("JH_C_16", lane, nullptr);
                    stringstream ss;
                    ss << "two divider direction is conflict. [left divider:" << leftDiv->id_;
                    ss << "],[right divider:" << rightDiv->id_ << "]";
                    errorOutput->saveError(error);
                    lane->valid_ = false;
                } else if (leftDiv->direction_ == 1 || rightDiv->direction_ == 1){
                    continue;
                }

                //通过左车道分割线方向获取同行方向起始终止点坐标
                string lId = leftDiv->fromNodeId_;
                DCCoord lscoord = leftDiv->nodes_[0]->coord_;//通行方向起点
                DCCoord lecoord = leftDiv->nodes_[leftDiv->nodes_.size()-1]->coord_;//通行方向终点
                if (leftDiv->direction_ == 3){
                    lscoord = leftDiv->nodes_[leftDiv->nodes_.size()-1]->coord_;
                    lecoord = leftDiv->nodes_[0]->coord_;
                }else /*if (leftDiv->direction_ == 2)*/{
                    lscoord = leftDiv->nodes_[0]->coord_;
                    lecoord = leftDiv->nodes_[leftDiv->nodes_.size()-1]->coord_;
                }

                //通过右车道分割线方向获取同行方向起始终止点坐标
                string rId = rightDiv->fromNodeId_;
                DCCoord rscoord = rightDiv->nodes_[0]->coord_;//通行方向起点
                DCCoord recoord = rightDiv->nodes_[rightDiv->nodes_.size()-1]->coord_;//通行方向终点
                if (rightDiv->direction_ == 3){
                    rscoord = rightDiv->nodes_[rightDiv->nodes_.size()-1]->coord_;
                    recoord = rightDiv->nodes_[0]->coord_;
                }else /*if (rightDiv->direction_ == 2)*/{
                    rscoord = rightDiv->nodes_[0]->coord_;
                    recoord = rightDiv->nodes_[rightDiv->nodes_.size()-1]->coord_;
                }

                //检查矢量化方向是否相同，两条车道分割线的首尾连线夹角是否为锐角
                double leftAngle = geo_util::calcAngle(lscoord.lng_, lscoord.lat_, lecoord.lng_, lecoord.lat_);
                double rightAngle = geo_util::calcAngle(rscoord.lng_, rscoord.lat_, recoord.lng_, recoord.lat_);
                double fAngle = fabs(leftAngle - rightAngle);
                if (fAngle > limitAngle){
                    shared_ptr<DCLaneCheckError> error =
                            DCLaneCheckError::createByAtt("JH_C_16", lane, nullptr);
                    stringstream ss;
                    ss << "two divider direction is conflict. [left divider:" << leftDiv->id_;
                    ss << "],[right divider:" << rightDiv->id_ << "] intersect angle:" << fAngle;
                    errorOutput->saveError(error);
                    lane->valid_ = false;
                }

                /*//检查两个车道线的通行方向是否一致
                if( (leftDiv->direction_ == 2 && rightDiv->direction_ == 3) ||
                        (leftDiv->direction_ == 3 && rightDiv->direction_ == 2) ||
                        leftDiv->direction_ == 4 || rightDiv->direction_ == 4){

                    shared_ptr<DCLaneCheckError> error =
                            DCLaneCheckError::createByAtt("JH_C_16", lane, nullptr);
                    stringstream ss;
                    ss << "two divider direction not match. [left direction:" << leftDiv->direction_;
                    ss << "],[right direction:" << rightDiv->direction_ << "]";
                    error->errorDesc_ = ss.str();

                    lane->valid_ = false;

                    errorOutput->saveError(error);
                }*/
            }
        }

        //车道右侧车道线起点没有LA
        void LaneAttribCheck::check_JH_C_19(shared_ptr<MapDataManager> mapDataManager,
                                               shared_ptr<CheckErrorOutput> errorOutput) {
            for (auto recordit : mapDataManager->lanes_) {
                shared_ptr<DCLane> lane = recordit.second;
                if (!lane->valid_)
                    continue;

                if (lane->atts_.size() == 0){
                    //车道线没有属性变化点
                    shared_ptr<DCLaneCheckError> error =
                            DCLaneCheckError::createByAtt("JH_C_19", lane, nullptr);
                    error->errorDesc_ = "lane no la";

                    errorOutput->saveError(error);
                    continue;
                }

                int nodeIndex = lane->getAttNodeIndex( lane->atts_[0]->dividerNode_ );
                if( nodeIndex != 0){
                    //车道线起点没有属性变化点
                    shared_ptr<DCLaneCheckError> error =
                            DCLaneCheckError::createByAtt("JH_C_19", lane, nullptr);
                    error->errorDesc_ = "lane no la on start point";

                    errorOutput->saveError(error);
                    continue;
                }
            }
        }

        //同一Divider上相邻两个LA属性完全一样
        void LaneAttribCheck::check_JH_C_21(shared_ptr<MapDataManager> mapDataManager,
                                               shared_ptr<CheckErrorOutput> errorOutput){
            for (auto recordit : mapDataManager->lanes_) {
                shared_ptr<DCLane> lane = recordit.second;
                if (!lane->valid_)
                    continue;

                if(lane->atts_.size() <= 1){
                    continue;
                }

                for( int i = 1 ; i < lane->atts_.size() ; i ++ ){

                    shared_ptr<DCLaneAttribute> la1 = lane->atts_[i-1];
                    shared_ptr<DCLaneAttribute> la2 = lane->atts_[i];

                    if(la1 != nullptr && la1->typeSame(la2)){
                        shared_ptr<DCLaneCheckError> error =
                                DCLaneCheckError::createByAtt("JH_C_21", lane, la1);

                        stringstream ss;
                        ss << "la [id:"<< la1->id_ << "] same as la [id:" << la2->id_ << "]";
                        error->errorDesc_ = ss.str();
                        errorOutput->saveError(error);
                    }
                }
            }
        }

        //同一Divider上相邻两个LA距离<1米
        void LaneAttribCheck::check_JH_C_20(shared_ptr<MapDataManager> mapDataManager,
                                               shared_ptr<CheckErrorOutput> errorOutput){

            double laSpaceLen = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::LA_SPACE_LEN);

            for (auto recordit : mapDataManager->lanes_) {
                shared_ptr<DCLane> lane = recordit.second;
                if (!lane->valid_)
                    continue;

                if(lane->atts_.size() <= 1){
                    continue;
                }

                for( int i = 1 ; i < lane->atts_.size() ; i ++ ){

                    shared_ptr<DCLaneAttribute> la1 = lane->atts_[i-1];
                    shared_ptr<DCLaneAttribute> la2 = lane->atts_[i];

                    if(la1 == nullptr || la1->dividerNode_ == nullptr || la2 == nullptr || la2->dividerNode_ == nullptr){
                        continue;
                    }

                    auto node1 = la1->dividerNode_;
                    auto node2 = la2->dividerNode_;
                    double distance = KDGeoUtil::distanceLL(node1->coord_.lng_, node1->coord_.lat_, node2->coord_.lng_, node2->coord_.lat_);

                    if(distance < laSpaceLen){
                        shared_ptr<DCLaneCheckError> error =
                                DCLaneCheckError::createByAtt("JH_C_20", lane, la1);

                        stringstream ss;
                        ss << "length from la [id:"<< la1->id_ << "] to la [id:" << la2->id_ << "] is " << distance << " meter";
                        error->errorDesc_ = ss.str();
                        errorOutput->saveError(error);
                    }
                }
            }
        }


        bool LaneAttribCheck::execute(shared_ptr<MapDataManager> mapDataManager,
                                         shared_ptr<CheckErrorOutput> errorOutput) {
            if (mapDataManager == nullptr)
                return false;

            errorOutput->writeInfo("<LaneAttribCheck>\n" + make_shared<DCLaneCheckError>("")->getHeader());

            check_JH_C_16(mapDataManager, errorOutput);

            check_JH_C_19(mapDataManager, errorOutput);

            check_JH_C_20(mapDataManager, errorOutput);

            check_JH_C_21(mapDataManager, errorOutput);

            return true;
        }

    }
}

