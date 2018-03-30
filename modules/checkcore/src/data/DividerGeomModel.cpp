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
        // DCivider
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

                Coordinates::ll2utm(node->coord_.lat_, node->coord_.lng_, X0, Y0, zone0);

                cl->add(geos::geom::Coordinate(X0, Y0, node->coord_.z_));
            }

            if (cl->size() < 2) {
                this->valid_ = false;
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

    }
}
