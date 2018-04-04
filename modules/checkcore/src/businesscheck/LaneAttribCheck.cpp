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
                shared_ptr<DCLane> lane = recordit.second;
                if (!lane->valid_)
                    continue;

                if (lane->leftDivider_ == nullptr  || lane->rightDivider_ == nullptr) {
                    lane->valid_ = false;
                    cout << "[Error] lane divider info error." << endl;
                    continue;
                }

                //检查矢量化方向是否相同，两条车道分割线的首尾连线夹角是否为锐角
                const double IntersectAngleLimit = 90;
                shared_ptr<DCDivider> leftDiv = lane->leftDivider_;
                shared_ptr<DCDivider> rightDiv = lane->rightDivider_;
                double leftAngle = 0.0, rightAngle = 0.0;
                if (leftDiv->nodes_.size() >= 2){
                    leftAngle = geo_util::calcAngle(leftDiv->nodes_[0]->coord_.lng_, leftDiv->nodes_[0]->coord_.lat_,
                                                    leftDiv->nodes_[leftDiv->nodes_.size()-1]->coord_.lng_,
                                                    leftDiv->nodes_[leftDiv->nodes_.size()-1]->coord_.lat_);
                }

                if (rightDiv->nodes_.size() >= 2){
                    rightAngle = geo_util::calcAngle(rightDiv->nodes_[0]->coord_.lng_, rightDiv->nodes_[0]->coord_.lat_,
                                                     rightDiv->nodes_[rightDiv->nodes_.size()-1]->coord_.lng_,
                                                     rightDiv->nodes_[rightDiv->nodes_.size()-1]->coord_.lat_);
                }

                double fAngle = fabs(leftAngle - rightAngle);
                if (fAngle > IntersectAngleLimit){
                    shared_ptr<DCLaneCheckError> error =
                            DCLaneCheckError::createByAtt("JH_C_16", lane, nullptr);
                    stringstream ss;
                    ss << "two divider vector direction not match. [left divider:" << leftDiv->id_;
                    ss << "],[right divider:" << rightDiv->id_ << "],[intersect angle:" << fAngle << "]";
                    errorOutput->saveError(error);
                    lane->valid_ = false;
                }

                //检查两个车道线的通行方向是否一致
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
                }
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

            check_JH_C_16(mapDataManager, errorOutput);

            check_JH_C_19(mapDataManager, errorOutput);

            check_JH_C_20(mapDataManager, errorOutput);

            check_JH_C_21(mapDataManager, errorOutput);

            return true;
        }

    }
}

