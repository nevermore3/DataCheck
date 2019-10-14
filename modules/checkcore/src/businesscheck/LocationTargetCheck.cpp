//
// Created by ubuntu on 19-10-14.
//
#include "businesscheck/LocationTargetCheck.h"
#include "util/product_shp_util.h"
bool LocationTargetCheck:: execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput){

    set_data_manager(mapDataManager);
    set_error_output(errorOutput);

    preCheck();
    LOG(INFO)<<"preCheck finished";


    check_kxs_06_003();




    clearMeomery();
    LOG(INFO)<<"clearMeomery finished";
}
string LocationTargetCheck::getId() {

}
void LocationTargetCheck::clearMeomery() {
    data_manager()->clearData(kRLoRoad);
    data_manager()->clearData(kTrafficLight);
    data_manager()->clearData(kTrafficLightGroup);
}
void LocationTargetCheck::preCheck() {
    data_manager()->initRelation(kRLoRoad);
    data_manager()->initRelation(kTrafficLight);
    data_manager()->initRelation(kTrafficLightGroup);
    BuildLight2Group();
    BuildLight2Road();
}
void LocationTargetCheck::check_kxs_06_003() {
    map<long,long> lg_id_2_road_id;


}

void LocationTargetCheck::BuildLight2Group(){
    auto light_group_data = data_manager()->getKxfData(kTrafficLightGroup);
    for(auto it:light_group_data){
        auto kxfdata = it.second;
        long lg_id = kxfdata->getPropertyLong(TLG_ID);
        long l_id = kxfdata->getPropertyLong(TL_ID);
        if(map_light_id_to_group_id.find(l_id) == map_light_id_to_group_id.end()){
            map_light_id_to_group_id.insert(l_id,lg_id);
        }else{
            LOG(INFO)<<" exist more triffic light in HD_TRAFFIC_LIGHT_GROUP,light id is "<<l_id;
        }
    }
}

void LocationTargetCheck::BuildLight2Road(){
    auto light_group_data = data_manager()->getKxfData(kRLoRoad);
    for(auto it:light_group_data){
        auto kxfdata = it.second;
        if(kxfdata->getPropertyLong(TYPE)!=5){
            continue;
        }
        long r_id = kxfdata->getPropertyLong(ROAD_ID);
        long l_id = kxfdata->getPropertyLong(LO_ID);
        if(map_light_id_to_group_id.find(l_id) == map_light_id_to_group_id.end()){
            map_light_id_to_group_id.insert(l_id,r_id);
        }else{
            LOG(INFO)<<" exist more triffic light in HD_TRAFFIC_LIGHT_GROUP,light id is "<<l_id;
        }
    }
}

