//
// Created by gaoyanhong on 2018/3/30.
//


#include "businesscheck/DividerTopoCheck.h"

//thirdparty
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Point.h>
#include <DataCheckConfig.h>


namespace kd {
    namespace dc {

        string DividerTopoCheck::getId() {
            return id;
        }



        bool DividerTopoCheck::findStopLine(shared_ptr<DCDividerTopoNode> topoNode, double bufferLen,
                          const shared_ptr<geos::index::quadtree::Quadtree> & quadtree){

            const GeometryFactory *gf = GeometryFactory::getDefaultInstance();
            geos::geom::Coordinate coord(topoNode->lng_, topoNode->lat_, topoNode->z_);
            shared_ptr<geos::geom::Point> point(gf->createPoint(coord));
            shared_ptr<geos::geom::Geometry> buffer(point->buffer(bufferLen));

            vector<void*> intersect_objs;
            quadtree->query(buffer->getEnvelopeInternal(),intersect_objs);

            for(auto & intersect_ptr : intersect_objs) {
                DCObjectPL * inter_obj = (DCObjectPL *) intersect_ptr;

                //判断相交
                if(buffer->intersects(inter_obj->line_.get())){

                    //TODO 高度判断
                    return true;
                }
            }

            return false;
        }


        void DividerTopoCheck::check(shared_ptr<DCDivider> div,
                   const shared_ptr<geos::index::quadtree::Quadtree> & quadtree,
                   set<string> & divIds,
                   const map<string, shared_ptr<DCDividerTopoNode>> & topoNodes, shared_ptr<MapDataManager> mapDataManager){

            if(div == nullptr)
                return;

            //检查对象是否已经处理过了
            if(divIds.find(div->id_) != divIds.end()){
                //对象已经处理过了
                return;
            }

            divIds.insert(div->id_);

            //检查对象是否是边界线类型
            if(!isEdgeLine(div)){
                cout << "[Error] divider not edge. divider id " << div->id_ << endl;
                return;
            }

            double checkbufLen = DataCheckConfig::getInstance().getPropertyD(DataCheckConfig::OBJECT_PL_BUFFER);

            //判断起点的连接情况
            {
                string fromNodeId = div->fromNodeId_;
                auto fromNodeit = topoNodes.find(fromNodeId);
                if(fromNodeit == topoNodes.end()){
                    cout << "[Error] node " << fromNodeId << " not find topo info " << endl;
                    return;
                }

                shared_ptr<DCDividerTopoNode> topoNode = fromNodeit->second;
                bool findEdge = false;
                for( auto endRelIt : topoNode->endRels_ ){
                    string divId = endRelIt.first;

                    auto divit = mapDataManager->dividers_.find(divId);
                    if(divit != mapDataManager->dividers_.end()){
                        shared_ptr<DCDivider> div = divit->second;
                        if(isEdgeLine(div)){
                            findEdge = true;
                            break;
                        }
                    }
                }

                if(!findEdge){
                    if(!findStopLine(topoNode, checkbufLen, mapDataManager)){

                    }
                }
            }

            //判断终点的连接情况
            {

            }









        }

        //车行道边缘线在非停止线/出入口标线的地方断开
        void DividerTopoCheck::check_JH_C_4(shared_ptr<MapDataManager> mapDataManager, const map<string, shared_ptr<DCDividerTopoNode>> & topoNodes, shared_ptr<CheckErrorOutput> errorOutput){

            //空间索引保存所有停止线，以便后期在车道线停止时判断其是否有相交的情形
            shared_ptr<geos::index::quadtree::Quadtree> quadtree = make_shared<geos::index::quadtree::Quadtree>();
            for( auto plit : mapDataManager->objectPLs_ ){

                shared_ptr<DCObjectPL> pl = plit.second;
                if(!pl->valid_)
                    continue;

                if(pl->type_ == 2){ //停车让行线和减速让行线
                    quadtree->insert(pl->line_->getEnvelopeInternal(), pl.get());
                }
            }

            //检查所有的车道组
            set<string> divIds;
            for( auto lgit : mapDataManager->laneGroups_ ){

                shared_ptr<DCLaneGroup> lg = lgit.second;
                if(!lg->valid_)
                    continue;

                if(lg->lanes_.size() == 0){
                    cout << "[Error] lane group has no lane. lanegroup id " << lg->id_ << endl;
                    continue;
                }

                //检查左边缘线
                shared_ptr<DCDivider> leftDivider = lg->lanes_[0]->leftDivider_;
                if(divIds.find(leftDivider->id_) == divIds.end()){ //未处理过
                    if(!isEdgeLine(leftDivider)){
                        cout << "[Error] divider not edge. divider id " << leftDivider->id_ << endl;
                    }else{



                    }




                }





                //检查右边缘线

            }





            //检查所有车道线
            for( auto divit : mapDataManager->dividers_) {

                string divid = divit.first;
                shared_ptr<DCDivider> div = divit.second;
                if (!div->valid_)
                    continue;







            }
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

                if(div->atts_.size() == 0)
                    continue;

                //不判断路口虚拟分隔线
                if(div->atts_.size() == 1){
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
        void DividerTopoCheck::check_JH_C_6(shared_ptr<MapDataManager> mapDataManager, const map<string, shared_ptr<DCDividerTopoNode>> & topoNodes, shared_ptr<CheckErrorOutput> errorOutput){
            //判断关系
            for( auto nodeit : topoNodes) {
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
                            DCDividerCheckError::createByNode("JH_C_6", topoNode->nodeId_, topoNode->lng_, topoNode->lat_, topoNode->z_);

                    stringstream ss;
                    ss << "divider node " << topoNode->nodeId_ << " has no FDNODE relation.";
                    error->errorDesc_ = ss.str();

                    errorOutput->saveError(error);
                }

                if(topoNode->endRels_.size() == 0){
                    shared_ptr<DCDividerCheckError> error =
                            DCDividerCheckError::createByNode("JH_C_6", topoNode->nodeId_, topoNode->lng_, topoNode->lat_, topoNode->z_);

                    stringstream ss;
                    ss << "divider node " << topoNode->nodeId_ << " has no TDNODE relation.";
                    error->errorDesc_ = ss.str();

                    errorOutput->saveError(error);
                }
            }
        }

        void DividerTopoCheck::buildTopoNodes(shared_ptr<MapDataManager> mapDataManager, map<string, shared_ptr<DCDividerTopoNode>> & topoNodes){

            for( auto divit : mapDataManager->dividers_){

                string divid = divit.first;
                shared_ptr<DCDivider> div = divit.second;
                if (!div->valid_)
                    continue;

                string fromNodeId = div->fromNodeId_;
                string toNodeId = div->toNodeId_;

                //建立起点的关系
                {
                    auto topoNodeit = topoNodes.find(fromNodeId);
                    if(topoNodeit == topoNodes.end()){
                        shared_ptr<DCDividerTopoNode> topoNode = make_shared<DCDividerTopoNode>();
                        topoNode->nodeId_ = fromNodeId;
                        if(fromNodeId == div->nodes_[0]->id_){
                            topoNode->lng_ = div->nodes_[0]->coord_.lng_;
                            topoNode->lat_ = div->nodes_[0]->coord_.lat_;
                            topoNode->z_ = div->nodes_[0]->coord_.z_;

                        }else{
                            topoNode->lng_ = div->nodes_[div->nodes_.size()-1]->coord_.lng_;
                            topoNode->lat_ = div->nodes_[div->nodes_.size()-1]->coord_.lat_;
                            topoNode->z_ = div->nodes_[div->nodes_.size()-1]->coord_.z_;
                        }
                        topoNode->startRels_.insert(pair<string,string>(div->id_, div->id_));
                        topoNodes.insert(pair<string, shared_ptr<DCDividerTopoNode>>(fromNodeId, topoNode));
                    }else{

                        shared_ptr<DCDividerTopoNode> topoNode = topoNodeit->second;
                        topoNode->startRels_.insert(pair<string,string>(div->id_, div->id_));
                    }
                }

                //建立终点的关系
                {
                    auto topoNodeit = topoNodes.find(toNodeId);
                    if(topoNodeit == topoNodes.end()){
                        shared_ptr<DCDividerTopoNode> topoNode = make_shared<DCDividerTopoNode>();
                        topoNode->nodeId_ = toNodeId;

                        if(toNodeId == div->nodes_[0]->id_){
                            topoNode->lng_ = div->nodes_[0]->coord_.lng_;
                            topoNode->lat_ = div->nodes_[0]->coord_.lat_;
                            topoNode->z_ = div->nodes_[0]->coord_.z_;

                        }else{
                            topoNode->lng_ = div->nodes_[div->nodes_.size()-1]->coord_.lng_;
                            topoNode->lat_ = div->nodes_[div->nodes_.size()-1]->coord_.lat_;
                            topoNode->z_ = div->nodes_[div->nodes_.size()-1]->coord_.z_;
                        }

                        topoNode->endRels_.insert(pair<string,string>(div->id_, div->id_));
                        topoNodes.insert(pair<string, shared_ptr<DCDividerTopoNode>>(toNodeId, topoNode));
                    }else{

                        shared_ptr<DCDividerTopoNode> topoNode = topoNodeit->second;
                        topoNode->endRels_.insert(pair<string,string>(div->id_, div->id_));
                    }
                }

            }
        }

        bool DividerTopoCheck::isEdgeLine(shared_ptr<DCDivider> div){
            bool edgeLine = true;
            for( int i = 0 ; i < div->atts_.size() ; i ++ ){
                shared_ptr<DCDividerAttribute> divAtt = div->atts_[i];
                if(divAtt->virtual_ == 1) //路口虚拟分隔线可以认为是边线的一种
                    continue;

                if(divAtt->type_ == 1 || divAtt->type_ == 4){
                    continue;
                }

                return false;
            }

            return true;
        }


        bool DividerTopoCheck::execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput){

            //建立节点拓扑关系，记录所有车道线共点信息， map的key为节点id
            map<string, shared_ptr<DCDividerTopoNode>> topoNodes;
            buildTopoNodes(mapDataManager, topoNodes);

            check_JH_C_4(mapDataManager, topoNodes, errorOutput);

            check_JH_C_5(mapDataManager, errorOutput);

            check_JH_C_6(mapDataManager, topoNodes, errorOutput);

            return true;
        }
    }
}
