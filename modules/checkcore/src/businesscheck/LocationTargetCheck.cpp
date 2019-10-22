
#include "businesscheck/LocationTargetCheck.h"

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
    return id;
}
void LocationTargetCheck::clearMeomery() {
    data_manager()->clearData(kRLoRoad);
    data_manager()->clearData(kTrafficLight);
    data_manager()->clearData(kTrafficLightGroup);
}
void LocationTargetCheck::preCheck() {
    data_manager()->initRelation(kRLoRoad);
    data_manager()->initPolyline(kTrafficLight);
    data_manager()->initRelation(kTrafficLightGroup);
    BuildLight2Group();
    BuildLight2Road();
}
void LocationTargetCheck::check_kxs_06_003() {
    map<long,long> lg_id_2_road_id;
    auto triffic_light = data_manager()->getKxfData(kTrafficLight);
    error_output()->addCheckItemInfo(CHECK_ITEM_KXS_LM_003,map_light_id_to_group_id.size());

    for(auto it:map_light_id_to_group_id){
        auto light_id_2_road = map_light_id_road_id.find(it.first);
        if(light_id_2_road!=map_light_id_road_id.end()){
            auto lg_2_road=lg_id_2_road_id.find(it.second);
            if(lg_2_road == lg_id_2_road_id.end()){
                lg_id_2_road_id.insert(make_pair(it.second,light_id_2_road->second));
            }else{
                if(lg_2_road->second != light_id_2_road->second){
                    auto kxfdata = triffic_light.find(it.first);
                    if(kxfdata != triffic_light.end()) {
                        shared_ptr<PolyLine> triffic_light = std::static_pointer_cast<PolyLine>(kxfdata->second);
                        auto error = DCLocationTargetError::createByKXS_06_003(it.first, it.second,
                                                                               light_id_2_road->second,
                                                                               triffic_light->coords_[0]);
                        error_output()->saveError(error);

                    }
                }
            }
        }
    }

}

void LocationTargetCheck::BuildLight2Group(){
    auto light_group_data = data_manager()->getKxfData(kTrafficLightGroup);
    for(auto it:light_group_data){
        auto kxfdata = it.second;
        long lg_id = kxfdata->getPropertyLong(TLG_ID);
        long l_id = kxfdata->getPropertyLong(TL_ID);
        if(map_light_id_to_group_id.find(l_id) == map_light_id_to_group_id.end()){
            map_light_id_to_group_id.insert(make_pair(l_id,lg_id));
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
        if(map_light_id_road_id.find(l_id) == map_light_id_road_id.end()){
            map_light_id_road_id.insert(make_pair(l_id,r_id));
        }else{
            LOG(INFO)<<" exist more triffic light in HD_TRAFFIC_LIGHT_GROUP,light id is "<<l_id;
        }
    }
}

