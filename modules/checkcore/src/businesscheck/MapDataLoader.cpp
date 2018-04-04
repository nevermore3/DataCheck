//
// Created by gaoyanhong on 2018/3/29.
//

#include "businesscheck/MapDataLoader.h"

#include "storage/MapDataInput.h"

namespace kd {
    namespace dc {

        MapDataLoader::MapDataLoader(string basePath) {
            basePath_ = basePath;
        }

        string MapDataLoader::getId() {
            return id;
        }

        bool MapDataLoader::execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) {
            if( mapDataManager == nullptr)
                return false;

            MapDataInput mapDataInput;
            //加载车道线数据
            if(mapDataInput.loadDivider(basePath_, mapDataManager->dividers_, errorOutput)){

                for(auto recordit : mapDataManager->dividers_){
                    string divid = recordit.first;
                    shared_ptr<DCDivider> div = recordit.second;

                    if(!div->valid_)
                        continue;

                    //属性重排
                    div->sortAtts();

                    //构造空间几何属性
                    div->buildGeometryInfo();
                }
            }

            //加载车道
            if (mapDataInput.loadLane(basePath_, mapDataManager->dividers_, mapDataManager->lanes_, errorOutput)){
                for(auto recordit : mapDataManager->lanes_){
                    string laneid = recordit.first;
                    shared_ptr<DCLane> lane = recordit.second;

                    if(!lane->valid_)
                        continue;

                    //属性重排
                    lane->sortAtts();
                }
            }

            //加载车道分组
            if(mapDataInput.loadLaneGroup(basePath_, mapDataManager->lanes_, mapDataManager->laneGroups_, errorOutput)){
                for( auto recordit : mapDataManager->laneGroups_ ){

                    string lgid = recordit.first;
                    shared_ptr<DCLaneGroup> laneGroup = recordit.second;

                    if(!laneGroup->valid_)
                        continue;

                    //根据车道编号重排车道顺序
                    laneGroup->sortLanes();
                }
            }

            return true;
        }
    }
}
