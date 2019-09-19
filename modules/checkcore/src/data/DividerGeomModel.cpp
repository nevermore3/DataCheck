//
// Created by gaoyanhong on 2018/3/1.
//

#include "data/DividerGeomModel.h"

//thirdprty
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Point.h>
#include "geos/geom/CoordinateArraySequence.h"

using namespace geos::geom;

//core
#include <core/structure/KCoordinates.h>
#include <mvg/Coordinates.hpp>
#include <data/DividerGeomModel.h>

#include "geom/geo_util.h"

using namespace kd::automap;


namespace kd {
   namespace dc {

        /////////////////////////////////////////////////////////////////////////////////////////
        //  KDDividerAttribute
        /////////////////////////////////////////////////////////////////////////////////////////
        bool DCDividerAttribute::typeSame(shared_ptr<DCDividerAttribute> dividerAtt) {
            if (dividerAtt == nullptr)
                return false;


            if (this->virtual_ == dividerAtt->virtual_ && this->color_ == dividerAtt->color_ &&
                this->type_ == dividerAtt->type_ && this->driveRule_ == dividerAtt->driveRule_ &&
                this->material_ == dividerAtt->material_ && this->width_ == dividerAtt->width_ ) {
                return true;
            }

            return false;
        }

//        bool DCDividerAttribute::valueSame(shared_ptr<DCDividerAttribute> srcDividerAtt) {
//
//            if (this->virtual_ != srcDividerAtt->virtual_ ||
//                this->color_ != srcDividerAtt->color_ ||
//                this->type_ != srcDividerAtt->type_ ||
//                this->driveRule_ != srcDividerAtt->driveRule_ ||
//                this->material_ != srcDividerAtt->material_ ||
//                this->width_ != srcDividerAtt->width_)
//                return false;
//
//            return true;
//        }

        bool DCDividerAttribute::copyBaseInfo(shared_ptr<DCDividerAttribute> srcDividerAtt) {
            if (srcDividerAtt == nullptr)
                return false;

            this->virtual_ = srcDividerAtt->virtual_;
            this->color_ = srcDividerAtt->color_;
            this->type_ = srcDividerAtt->type_;
            this->driveRule_ = srcDividerAtt->driveRule_;
            this->material_ = srcDividerAtt->material_;
            this->width_ = srcDividerAtt->width_;

            return true;
        }


        /////////////////////////////////////////////////////////////////////////////////////////
        // DCDivider
        /////////////////////////////////////////////////////////////////////////////////////////
        bool DCDivider::isValid() {
            if (line_ == nullptr || len_ == 0.0)
                return false;

            return valid_;
        }

        bool DCDivider::checkValid() {
            //至少有两个节点
            if (nodes_.size() < 2) {
                valid_ = false;
                return false;
            }

            //至少有一个属性变化点
            if (atts_.size() == 0) {
                valid_ = false;
                return false;
            }

            //空间几何对象属性检查
            if (line_ == nullptr || len_ == 0.0) {
                valid_ = false;
                return false;
            }

            //各项参数检查，如数据的取值范围
            //TODO

            return true;
        }

        string DCDivider::toString() {
            return "";
        }

        void DCDivider::sortAtts() {
            if (atts_.size() <= 1)
                return;

            //获取原有顺序
            vector<pair<int, int>> attIndexes;
            for (int i = 0; i < atts_.size(); i++) {
                shared_ptr<DCDividerAttribute> &att = atts_[i];

                int nodeIndex = getAttNodeIndex(att->dividerNode_);

                attIndexes.emplace_back(pair<int, int>(i, nodeIndex));
            }

            //排序
            sort(attIndexes.begin(), attIndexes.end(), [](const pair<int, int> &v1, const pair<int, int> &v2) {
                if (v1.second < v2.second) {
                    return true;
                } else {
                    return false;
                }
            });

            //根据排序后的顺序重制列表
            std::vector<shared_ptr<DCDividerAttribute>> attsTemp;
            for (pair<int, int> &attIndex : attIndexes) {
                attsTemp.emplace_back(atts_[attIndex.first]);
            }

            atts_.swap(attsTemp);
        }

        int DCDivider::getAttNodeIndex(shared_ptr<DCDividerNode> node) {
            if(node == nullptr)
                return -1;

            for (int i = 0; i < nodes_.size(); i++) {
                shared_ptr<DCDividerNode> &tmpNode = nodes_[i];
                if (tmpNode->id_ == node->id_)
                    return i;
            }

            return -1;
        }

        bool DCDivider::buildGeometryInfo() {

            //创建linestring
            CoordinateSequence *cl = new CoordinateArraySequence();
            for (shared_ptr<DCDividerNode> node : nodes_) {
                double X0, Y0;
                char zone0[8] = {0};

                Coordinates::ll2utm(node->coord_->y_, node->coord_->x_, X0, Y0, zone0);

                cl->add(geos::geom::Coordinate(X0, Y0, node->coord_->z_));
            }

            if (cl->size() < 2) {
                this->valid_ = false;
                delete cl;
                return false;
            }

            const GeometryFactory *gf = GeometryFactory::getDefaultInstance();
            geos::geom::LineString *lineString = gf->createLineString(cl);
            if (lineString) {
                line_.reset(lineString);

                //计算线段距离轨迹的平均距离
                double lenTotal = 0.0;
                int segmentCount = cl->size() - 1;
                for (int i = 0; i < segmentCount; i++) {
                    const Coordinate &coord1 = cl->getAt(i);
                    const Coordinate &coord2 = cl->getAt(i + 1);

                    double dx = coord1.x - coord2.x;
                    double dy = coord1.y - coord2.y;

                    double lenTemp = sqrt(dx*dx + dy*dy);
                    lenTotal += lenTemp;
                }

                len_ = lenTotal;
                return true;
            } else {
                delete cl;
                this->valid_ = false;
                return false;
            }
        }


        /////////////////////////////////////////////////////////////////////////////////////////
        // DCLane
        /////////////////////////////////////////////////////////////////////////////////////////

        void DCLane::sortAtts() {
            if (atts_.size() <= 1)
                return;

            //获取原有顺序
            vector<pair<int, int>> attIndexes;
            for (int i = 0; i < atts_.size(); i++) {
                shared_ptr<DCLaneAttribute> &att = atts_[i];

                int nodeIndex = getAttNodeIndex(att->dividerNode_);

                attIndexes.emplace_back(pair<int, int>(i, nodeIndex));
            }

            //排序
            sort(attIndexes.begin(), attIndexes.end(), [](const pair<int, int> &v1, const pair<int, int> &v2) {
                if (v1.second < v2.second) {
                    return true;
                } else {
                    return false;
                }
            });

            //根据排序后的顺序重制列表
            std::vector<shared_ptr<DCLaneAttribute>> attsTemp;
            for (pair<int, int> &attIndex : attIndexes) {
                attsTemp.emplace_back(atts_[attIndex.first]);
            }

            atts_.swap(attsTemp);
        }

        int DCLane::getAttNodeIndex(shared_ptr<DCDividerNode> node) {
            if(node == nullptr || rightDivider_ == nullptr)
                return -1;

            for (int i = 0; i < rightDivider_->nodes_.size(); i++) {
                shared_ptr<DCDividerNode> &tmpNode = rightDivider_->nodes_[i];
                if (tmpNode->id_ == node->id_)
                    return i;
            }

            return -1;
        }

        bool DCLane::buildGeometryInfo() {
            //创建linestring
            CoordinateSequence *cl = new CoordinateArraySequence();
            for (const auto &node : coords_) {
                double X0, Y0;
                char zone0[8] = {0};

                Coordinates::ll2utm(node->y_, node->x_, X0, Y0, zone0);

                cl->add(geos::geom::Coordinate(X0, Y0, node->z_));
            }

            if (cl->size() < 2) {
                this->valid_ = false;
                delete cl;
                return false;
            }

            const GeometryFactory *gf = GeometryFactory::getDefaultInstance();
            geos::geom::LineString *lineString = gf->createLineString(cl);
            if (lineString) {
                line_.reset(lineString);
                return true;
            } else {
                delete cl;
                this->valid_ = false;
                return false;
            }
        }

        shared_ptr<DCDividerNode> DCLane::getPassDividerNode(bool left, bool start) {
            if (leftDivSNode_ == nullptr && leftDivENode_ == nullptr
                && rightDivSNode_ == nullptr && rightDivENode_ == nullptr){

                //left divider
                bool bLeftNoIsuueDir = false;
                shared_ptr<DCDividerNode> lsNode = nullptr, leNode = nullptr;
                if (leftDivider_->direction_ == 2){
                    lsNode = leftDivider_->nodes_[0];
                    leNode = leftDivider_->nodes_[leftDivider_->nodes_.size()-1];
                }else if(leftDivider_->direction_ == 3){
                    lsNode = leftDivider_->nodes_[leftDivider_->nodes_.size()-1];
                    leNode = leftDivider_->nodes_[0];
                }else {
                    bLeftNoIsuueDir = true;
                    string sId = leftDivider_->fromNodeId_.c_str();
                    string eId = leftDivider_->toNodeId_.c_str();
                    shared_ptr<DCDividerNode> tmpNode = leftDivider_->nodes_[0];
                    if (tmpNode->id_ == sId){
                        lsNode = leftDivider_->nodes_[0];
                        leNode = leftDivider_->nodes_[leftDivider_->nodes_.size()-1];
                    }else if(tmpNode->id_ == eId){
                        lsNode = leftDivider_->nodes_[leftDivider_->nodes_.size()-1];
                        leNode = leftDivider_->nodes_[0];
                    } else {
                        lsNode = leftDivider_->nodes_[0];
                        leNode = leftDivider_->nodes_[leftDivider_->nodes_.size()-1];
                    }
                }
                //right divider
                bool bRightNoIsuueDir = false;
                shared_ptr<DCDividerNode> rsNode = nullptr, reNode = nullptr;
                if (rightDivider_->direction_ == 2){
                    rsNode = rightDivider_->nodes_[0];
                    reNode = rightDivider_->nodes_[rightDivider_->nodes_.size()-1];
                }else if(rightDivider_->direction_ == 3){
                    rsNode = rightDivider_->nodes_[rightDivider_->nodes_.size()-1];
                    reNode = rightDivider_->nodes_[0];
                }else {
                    bRightNoIsuueDir = true;
                    string sId = rightDivider_->fromNodeId_.c_str();
                    string eId = rightDivider_->toNodeId_.c_str();
                    shared_ptr<DCDividerNode> tmpNode = rightDivider_->nodes_[0];
                    if (tmpNode->id_ == sId){
                        rsNode = rightDivider_->nodes_[0];
                        reNode = rightDivider_->nodes_[rightDivider_->nodes_.size()-1];
                    }else if(tmpNode->id_ == eId){
                        rsNode = rightDivider_->nodes_[rightDivider_->nodes_.size()-1];
                        reNode = rightDivider_->nodes_[0];
                    }else {
                        rsNode = rightDivider_->nodes_[0];
                        reNode = rightDivider_->nodes_[rightDivider_->nodes_.size()-1];
                    }
                }
                //根据方向判定通行方向的起始终止点
                if (bLeftNoIsuueDir || bRightNoIsuueDir){
                    double LineA[] = {lsNode->coord_->x_, lsNode->coord_->y_, rsNode->coord_->x_, rsNode->coord_->y_};
                    double LineB[] = {leNode->coord_->x_, leNode->coord_->y_, reNode->coord_->x_, reNode->coord_->y_};
                    bool bCross = geo::geo_util::isLineSegmentCross(LineA, LineB);
                    if (bCross){
                        if (!bLeftNoIsuueDir && bRightNoIsuueDir){
                            shared_ptr<DCDividerNode> tmpNode = rsNode;
                            rsNode = reNode;
                            reNode = tmpNode;
                        } else if (bLeftNoIsuueDir && !bRightNoIsuueDir) {
                            shared_ptr<DCDividerNode> tmpNode = lsNode;
                            lsNode = leNode;
                            leNode = tmpNode;
                        } else{
                            //无法确定通行方向时,按中国右侧通行原则右侧一定为外侧分割线,左侧分割线双向几率较大，直接调转左分割线
                            shared_ptr<DCDividerNode> tmpNode = lsNode;
                            lsNode = leNode;
                            leNode = tmpNode;
                        }
                    }
                }

                leftDivSNode_ = lsNode;
                leftDivENode_ = leNode;
                rightDivSNode_ = rsNode;
                rightDivENode_ = reNode;
            }
            if (left && start){
                return leftDivSNode_;
            }else if(left && !start){
                return leftDivENode_;
            }else if(!left && start){
                return rightDivSNode_;
            }else if(!left && !start){
                return rightDivENode_;
            }else{
                return nullptr;
            }
        }


        /////////////////////////////////////////////////////////////////////////////////////////
        //  DCLaneAttribute
        /////////////////////////////////////////////////////////////////////////////////////////
        bool DCLaneAttribute::typeSame(shared_ptr<DCLaneAttribute> laneAtt) {
            if (laneAtt == nullptr)
                return false;


            if (this->laneType_ == laneAtt->laneType_ && this->subType_ == laneAtt->subType_ &&
                this->direction_ == laneAtt->direction_ && this->width_ == laneAtt->width_ &&
                this->maxSpeed_ == laneAtt->maxSpeed_ && this->minSpeed_ == laneAtt->minSpeed_ &&
                this->smType_ == laneAtt->smType_ && this->status_ == laneAtt->status_) {
                return true;
            }

            return false;
        }


        /////////////////////////////////////////////////////////////////////////////////////////
        //  DCLaneGroup
        /////////////////////////////////////////////////////////////////////////////////////////
        void DCLaneGroup::sortLanes(){
            if (lanes_.size() <= 1)
                return;

            //获取原有顺序
            vector<pair<long, long>> laneNumIndex;
            for (int i = 0; i < lanes_.size(); i++) {
                shared_ptr<DCLane> & lane = lanes_[i];

                long laneNum = lane->laneNo_;

                laneNumIndex.emplace_back(pair<int, int>(i, laneNum));
            }

            //排序
            sort(laneNumIndex.begin(), laneNumIndex.end(), [](const pair<int, int> &v1, const pair<int, int> &v2) {
                if (v1.second < v2.second) {
                    return true;
                } else {
                    return false;
                }
            });

            //根据排序后的顺序重制列表
            std::vector<shared_ptr<DCLane>> lanesTemp;
            for (pair<long, long> & laneIndex : laneNumIndex) {
                lanesTemp.emplace_back(lanes_[laneIndex.first]);
            }

            lanes_.swap(lanesTemp);
        }


        /////////////////////////////////////////////////////////////////////////////////////////
        //  DCObjectPL
        /////////////////////////////////////////////////////////////////////////////////////////
        bool DCObjectPL::buildGeometryInfo(){
            //创建linestring
            CoordinateSequence *cl = new CoordinateArraySequence();
            for (shared_ptr<DCCoord> coord : coords_) {
                double X0, Y0;
                char zone0[8] = {0};

                Coordinates::ll2utm(coord->y_, coord->x_, X0, Y0, zone0);

                cl->add(geos::geom::Coordinate(X0, Y0, coord->z_));
            }

            if (cl->size() < 2) {
                this->valid_ = false;
                delete cl;
                return false;
            }

            const GeometryFactory *gf = GeometryFactory::getDefaultInstance();
            geos::geom::LineString *lineString = gf->createLineString(cl);
            if (lineString) {
                line_.reset(lineString);
                return true;
            } else {
                delete cl;
                this->valid_ = false;
                return false;
            }
        }

        /////////////////////////////////////////////////////////////////////////////////////////
        //  DCRoad
        /////////////////////////////////////////////////////////////////////////////////////////
        bool DCRoad::buildGeometryInfo() {
            //创建linestring
            CoordinateSequence *cl = new CoordinateArraySequence();
            for (const auto &node : nodes_) {
                double X0, Y0;
                char zone0[8] = {0};

                Coordinates::ll2utm(node->y_, node->x_, X0, Y0, zone0);

                cl->add(geos::geom::Coordinate(X0, Y0, node->z_));
            }

            if (cl->size() < 2) {
                this->valid_ = false;
                delete cl;
                return false;
            }

            const GeometryFactory *gf = GeometryFactory::getDefaultInstance();
            geos::geom::LineString *lineString = gf->createLineString(cl);
            if (lineString) {
                line_.reset(lineString);

                //计算线段距离轨迹的平均距离
                double lenTotal = 0.0;
                size_t segmentCount = cl->size() - 1;
                for (size_t i = 0; i < segmentCount; i++) {
                    const Coordinate &coord1 = cl->getAt(i);
                    const Coordinate &coord2 = cl->getAt(i + 1);

                    double dx = coord1.x - coord2.x;
                    double dy = coord1.y - coord2.y;

                    double lenTemp = sqrt(dx*dx + dy*dy);
                    lenTotal += lenTemp;
                }

                len_ = lenTotal;
                return true;
            } else {
                delete cl;
                this->valid_ = false;
                return false;
            }
        }
    }
}
