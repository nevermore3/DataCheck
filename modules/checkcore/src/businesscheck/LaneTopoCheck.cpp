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

            errorOutput->writeInfo("[LaneTopoCheck]\n" + make_shared<DCLaneCheckError>("")->getHeader(), false);
            check_JH_C_22(mapDataManager, errorOutput);

            return true;
        }

        //存在没有有进入车道和退出车道的车道
        void LaneTopoCheck::check_JH_C_22(shared_ptr<MapDataManager> mapDataManager,
                                          shared_ptr<CheckErrorOutput> errorOutput) {

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
            for(auto laneit : mapDataManager->lanes_){

                shared_ptr<DCLane> lane = laneit.second;

                //查找以本车道为fromLane和toLane关联的记录数
                int fromLaneCount = fromLaneConns.count(lane->id_);
                int toLaneCount = toLaneConns.count(lane->id_);

                if(fromLaneCount == 0 && toLaneCount == 0){

                    shared_ptr<DCLaneCheckError> error =
                            DCLaneCheckError::createByNode("JH_C_22", lane, nullptr);
                    error->errorDesc_ = "lane no connected lane.";
                    error->checkDesc_ = "存在没有进入车道和退出车道的车道";
                    errorOutput->saveError(error);

                }else if(fromLaneCount == 0 && toLaneCount > 0){
                    shared_ptr<DCLaneCheckError> error =
                            DCLaneCheckError::createByNode("JH_C_22", lane, nullptr);
                    error->errorDesc_ = "lane no exit.";
                    error->checkDesc_ = "存在没有进入车道和退出车道的车道";
                    errorOutput->saveError(error);

                }else if(fromLaneCount > 0 && toLaneCount == 0){
                    shared_ptr<DCLaneCheckError> error =
                            DCLaneCheckError::createByNode("JH_C_22", lane, nullptr);
                    error->errorDesc_ = "lane no enter.";
                    error->checkDesc_ = "存在没有进入车道和退出车道的车道";
                    errorOutput->saveError(error);
                }
            }
        }
    }
}

