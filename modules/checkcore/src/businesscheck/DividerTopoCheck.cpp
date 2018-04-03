//
// Created by gaoyanhong on 2018/3/30.
//

#include "businesscheck/DividerTopoCheck.h"

namespace kd {
    namespace dc {

        string DividerTopoCheck::getId() {
            return id;
        }

        //车行道边缘线在非停止线/出入口标线的地方断开
        void DividerTopoCheck::check_JH_C_4(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput){

            

        }

        //存在非构成车道的车道线（非路口虚拟线）
        void DividerTopoCheck::check_JH_C_5(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput){

            //统计出已经构建了车道的车道线id
            map<string, string> divRelIds;
            for( auto laneit : mapDataManager->lanes_ ){

                shared_ptr<DCLane> lane = laneit.second;
                if(!lane->valid_)
                    continue;

                string leftDivId = lane->leftDivider_->id_;
                string rightDivId = lane->rightDivider_->id_;

                divRelIds.insert(pair<string,string>(leftDivId, leftDivId));
                divRelIds.insert(pair<string,string>(rightDivId, rightDivId));
            }

            //检查所有车道线
            for( auto divit : mapDataManager->dividers_) {

                string divid = divit.first;
                shared_ptr<DCDivider> div = divit.second;
                if (!div->valid_)
                    continue;

                //不判断路口虚拟分隔线
                if(div->atts_.size() == 0){
                    shared_ptr<DCDividerAttribute> divAtt = div->atts_[0];
                    if(divAtt->virtual_ == 1){
                        continue;
                    }
                }

                //检查车道线id是否使用过
                if(divRelIds.find(divid) == divRelIds.end()){
                    shared_ptr<DCDividerCheckError> error =
                            DCDividerCheckError::createByNode("JH_C_5", div, nullptr);

                    error->errorDesc_ = "divider not used by lane.";
                    errorOutput->saveError(error);
                }
            }
        }


        //共点的车道线通行方向（矢量化方向+车道线方向）冲突
        void DividerTopoCheck::check_JH_C_6(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput){

            //建立节点拓扑关系，记录所有车道线共点信息， map的key为节点id
            map<string, shared_ptr<DCDividerTopoNode>> topoNodes_;
            for( auto divit : mapDataManager->dividers_){

                string divid = divit.first;
                shared_ptr<DCDivider> div = divit.second;
                if (!div->valid_)
                    continue;

                string fromNodeId = div->fromNodeId_;
                string toNodeId = div->toNodeId_;

                //建立起点的关系
                {
                    auto topoNodeit = topoNodes_.find(fromNodeId);
                    if(topoNodeit == topoNodes_.end()){
                        shared_ptr<DCDividerTopoNode> topoNode = make_shared<DCDividerTopoNode>();
                        topoNode->nodeId_ = fromNodeId;
                        if(fromNodeId == div->nodes_[0]->id_){
                            topoNode->lng = div->nodes_[0]->coord_.lng_;
                            topoNode->lat = div->nodes_[0]->coord_.lat_;
                            topoNode->z = div->nodes_[0]->coord_.z_;

                        }else{
                            topoNode->lng = div->nodes_[div->nodes_.size()-1]->coord_.lng_;
                            topoNode->lat = div->nodes_[div->nodes_.size()-1]->coord_.lat_;
                            topoNode->z = div->nodes_[div->nodes_.size()-1]->coord_.z_;
                        }
                        topoNode->startRels_.insert(pair<string,string>(div->id_, div->id_));
                        topoNodes_.insert(pair<string, shared_ptr<DCDividerTopoNode>>(fromNodeId, topoNode));
                    }else{

                        shared_ptr<DCDividerTopoNode> topoNode = topoNodeit->second;
                        topoNode->startRels_.insert(pair<string,string>(div->id_, div->id_));
                    }
                }

                //建立终点的关系
                {
                    auto topoNodeit = topoNodes_.find(toNodeId);
                    if(topoNodeit == topoNodes_.end()){
                        shared_ptr<DCDividerTopoNode> topoNode = make_shared<DCDividerTopoNode>();
                        topoNode->nodeId_ = toNodeId;

                        if(toNodeId == div->nodes_[0]->id_){
                            topoNode->lng = div->nodes_[0]->coord_.lng_;
                            topoNode->lat = div->nodes_[0]->coord_.lat_;
                            topoNode->z = div->nodes_[0]->coord_.z_;

                        }else{
                            topoNode->lng = div->nodes_[div->nodes_.size()-1]->coord_.lng_;
                            topoNode->lat = div->nodes_[div->nodes_.size()-1]->coord_.lat_;
                            topoNode->z = div->nodes_[div->nodes_.size()-1]->coord_.z_;
                        }

                        topoNode->endRels_.insert(pair<string,string>(div->id_, div->id_));
                        topoNodes_.insert(pair<string, shared_ptr<DCDividerTopoNode>>(toNodeId, topoNode));
                    }else{

                        shared_ptr<DCDividerTopoNode> topoNode = topoNodeit->second;
                        topoNode->endRels_.insert(pair<string,string>(div->id_, div->id_));
                    }
                }

            }

            //判断关系
            for( auto nodeit : topoNodes_) {
                shared_ptr<DCDividerTopoNode> topoNode = nodeit.second;

                if(topoNode == nullptr)
                    continue;

                int relCount = topoNode->startRels_.size() + topoNode->endRels_.size();
                if( relCount == 1 ){
                    //边缘点的情况，不处理
                    continue;
                }

                //排除了只有一个关联车道线的情况后，如果某个方向的关联信息为0，则说明有问题
                if(topoNode->startRels_.size() == 0){
                    shared_ptr<DCDividerCheckError> error =
                            DCDividerCheckError::createByNode("JH_C_6", topoNode->nodeId_, topoNode->lng, topoNode->lat, topoNode->z);

                    stringstream ss;
                    ss << "divider node " << topoNode->nodeId_ << " has no FDNODE relation.";
                    error->errorDesc_ = ss.str();

                    errorOutput->saveError(error);
                }

                if(topoNode->endRels_.size() == 0){
                    shared_ptr<DCDividerCheckError> error =
                            DCDividerCheckError::createByNode("JH_C_6", topoNode->nodeId_, topoNode->lng, topoNode->lat, topoNode->z);

                    stringstream ss;
                    ss << "divider node " << topoNode->nodeId_ << " has no TDNODE relation.";
                    error->errorDesc_ = ss.str();

                    errorOutput->saveError(error);
                }
            }
        }



        bool DividerTopoCheck::execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput){

            check_JH_C_4(mapDataManager, errorOutput);

            check_JH_C_5(mapDataManager, errorOutput);

            check_JH_C_6(mapDataManager, errorOutput);

            return true;
        }
    }
}

