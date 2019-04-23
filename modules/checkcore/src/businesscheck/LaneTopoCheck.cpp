//
// Created by gaoyanhong on 2018/4/5.
//

#include "businesscheck/LaneTopoCheck.h"

namespace kd {
    namespace dc {

        string LaneTopoCheck::getId() {
            return id;
        }

        bool LaneTopoCheck::execute(shared_ptr<MapDataManager> mapDataManager,
                                    shared_ptr<CheckErrorOutput> errorOutput) {
            check_JH_C_22(mapDataManager, errorOutput);
            return true;
        }

        //存在没有有进入车道和退出车道的车道
        void LaneTopoCheck::check_JH_C_22(shared_ptr<MapDataManager> mapDataManager,
                                          shared_ptr<CheckErrorOutput> errorOutput) {
            const auto &f_node2_divider_maps = mapDataManager->fnode_id2_dividers_maps_;
            const auto &t_node2_divider_maps = mapDataManager->tnode_id2_dividers_maps_;


            //构造查询map
            multimap<string, string> fromLaneConns;
            multimap<string, string> toLaneConns;

            for( auto laneConnit : mapDataManager->laneConnectivitys_){

                shared_ptr<DCLaneConnectivity> laneConn = laneConnit.second;
                string fromLane = to_string(laneConn->fLaneId_);
                string toLane = to_string(laneConn->tLaneId_);

                fromLaneConns.insert(make_pair(fromLane, toLane));
                toLaneConns.insert(make_pair(toLane, fromLane));
            }

            //遍历所有的车道
            //修改错误描述,以及对孤立车道的检查逻辑,增加了对应急车道的判断以及前后DIVIDER连接关系判断,减少了一些冗余的错误输出
            for(auto laneit : mapDataManager->lanes_){

                shared_ptr<DCLane> lane = laneit.second;

                //查找以本车道为fromLane和toLane关联的记录数
                int fromLaneCount = fromLaneConns.count(lane->id_);
                int toLaneCount = toLaneConns.count(lane->id_);

                if (fromLaneCount == 0 && toLaneCount == 0) {
                    bool is_emergency = false;

                    for (const auto &attr : lane->atts_) {
                        if (attr->laneType_ == 9) {
                            is_emergency = true;
                            break;
                        }
                    }

                    if (!is_emergency) {
                        shared_ptr<DCLaneCheckError> error =
                                DCLaneCheckError::createByNode("KXS-05-012", lane, nullptr);
                        error->errorDesc_ = "lane_id:";
                        error->errorDesc_ += lane->id_;
                        error->errorDesc_ += "孤立车道,没有进入车道和退出车道";
                        error->checkDesc_ = "孤立车道,没有进入和退出车道连接,并且没有标注应急车道属性";
                        errorOutput->saveError(error);
                    }

                } else if (fromLaneCount == 0 && toLaneCount > 0) {
                    auto right_divider = lane->rightDivider_;
                    auto div_back_node = right_divider->nodes_.back();
                    if (f_node2_divider_maps.find(div_back_node->id_) != f_node2_divider_maps.end()) {
                        shared_ptr<DCLaneCheckError> error =
                                DCLaneCheckError::createByNode("KXS-05-013", lane, nullptr);
                        error->errorDesc_ = "lane_id:";
                        error->errorDesc_ += lane->id_;
                        error->errorDesc_ += "没有退出车道连接";
                        error->checkDesc_ = "没有退出车道,divider拓扑存在,检查lane_connectivity是否正确";
                        errorOutput->saveError(error);
                    }
                } else if (fromLaneCount > 0 && toLaneCount == 0) {
                    auto right_divider = lane->rightDivider_;
                    auto div_front_node = right_divider->nodes_.front();
                    if (t_node2_divider_maps.find(div_front_node->id_) != t_node2_divider_maps.end()) {
                        shared_ptr<DCLaneCheckError> error =
                                DCLaneCheckError::createByNode("KXS-05-014", lane, nullptr);
                        error->errorDesc_ = "lane_id:";
                        error->errorDesc_ += lane->id_;
                        error->errorDesc_ += "没有进入车道连接";
                        error->checkDesc_ = "没有进入车道,divider拓扑存在,检查lane_connectivity是否正确";
                        errorOutput->saveError(error);
                    }
                }
            }
        }
    }
}

