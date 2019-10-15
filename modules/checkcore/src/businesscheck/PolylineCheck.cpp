//
// Created by ubuntu on 19-9-29.
//

#include <util/shp_file_load_util.h>
#include "businesscheck/PolylineCheck.h"
#include "util/KDGeoUtil.hpp"

using namespace kd::automap;

string PolylineCheck::getId() {
    return id;
}

bool PolylineCheck::execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) {

    set_data_manager(mapDataManager);
    set_error_output(errorOutput);

    preCheck();
    ///停止线与道路的关联关系正确性检查
    check_kxs_011_002();


    clearMeomery();
}

void PolylineCheck::clearMeomery() {
    polylines.clear();
    rloroad.clear();
}

void PolylineCheck::preCheck() {
    set<string> line_ids;

    ShpFileLoad::GetPolyline(3, polylines);
    for (auto polyline:polylines) {
        line_ids.insert(polyline.first);
    }
    ShpFileLoad::GetRLoRoad(2, rloroad);
}

void PolylineCheck::check_kxs_011_002() {
    auto roads = data_manager()->roads_;
    for (auto polyline:polylines) {
        string polyline_id = polyline.first;
        auto lo_road = rloroad.find(polyline.first);
        if (lo_road == rloroad.end()) {
            shared_ptr<PolyLineError> polylineError = PolyLineError::createByKXS_011_002(1,polyline.first,
                                                                                        polyline.second->coords[0]);
            error_output()->saveError(polylineError);
            continue;
        }
        string road_id = lo_road->second->road_id_;
        auto road = roads.find(lo_road->second->road_id_);
        if(road == roads.end()){
            LOG(INFO)<<"not find road "<<road_id;
            continue;
        }
        CoordinateSequence *intersections = nullptr;
        bool iscross = KDGeoUtil::isLineCross(road->second->line_.get(), polyline.second->line_.get(), &intersections);
        if (!iscross) {
            shared_ptr<PolyLineError> polylineError = PolyLineError::createByKXS_011_002(2,polyline.first,
                                                                                        polyline.second->coords[0]);
            error_output()->saveError(polylineError);
        }
    }
    error_output()->addCheckItemInfo(CHECK_ITEM_KXS_LINE_002, polylines.size());
}