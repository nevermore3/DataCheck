//
// Created by ubuntu on 19-9-27.
//

#include <DataCheckConfig.h>
#include <shp/ShpData.hpp>
#include <util/CommonUtil.h>
#include "util/shp_file_load_util.h"
#include "util/product_shp_util.h"

void ShpFileLoad::GetPolyline(long type, map<string, shared_ptr<DCPolyline>> &polylines) {

    string basePath = DataCheckConfig::getInstance().getProperty(DataCheckConfig::SHP_FILE_PATH);
    string file = basePath + "/" + kPolyline;
    ShpData shpFile(file);
    if (shpFile.isInit()) {
        int recordNums = shpFile.getRecords();
        for (size_t i = 0; i < recordNums; i++) {
            SHPObject *shpObject = shpFile.readShpObject(i);
            long type_ = shpFile.readLongField(i, TYPE);
            if (type_ != type) {
                continue;
            }
            shared_ptr<DCPolyline> polyline = make_shared<DCPolyline>();
            polyline->id_ = to_string(shpFile.readIntField(i, ID));
            polyline->type_ = type_;

            //读取空间信息
            int nVertices = shpObject->nVertices;
            set<long> error_node_index;
            for (int idx = 0; idx < nVertices; idx++) {
                shared_ptr<DCCoord> coord = make_shared<DCCoord>();
                coord->x_ = shpObject->padfX[idx];
                coord->y_ = shpObject->padfY[idx];
                coord->z_ = shpObject->padfZ[idx];

                if (!CommonUtil::CheckCoordValid(coord)) {
                    error_node_index.emplace(idx);
                }

                polyline->coords.emplace_back(coord);
            }
            polyline->buildGeometryInfo();
            polylines.insert(make_pair(polyline->id_, polyline));
        }
    }
}

void ShpFileLoad::GetRLoRoad(long type, map<string, shared_ptr<DCRLORoad>> &r_lo_road) {

    string basePath = DataCheckConfig::getInstance().getProperty(DataCheckConfig::SHP_FILE_PATH);
    string file = basePath + "/" + kRLoRoad;
    DbfData attDbfData(file);
    if (attDbfData.isInit()) {

        int recordNums = attDbfData.getRecords();
        for (size_t i = 0; i < recordNums; i++) {
            long type_ = attDbfData.readLongField(i, TYPE);
            string lo_id = attDbfData.readStringField(i, LO_ID);
            if ( type_ != type ) {
                continue;
            }
            shared_ptr<DCRLORoad> rloroad = make_shared<DCRLORoad>();
            rloroad->id_ = to_string(attDbfData.readIntField(i, ID));
            rloroad->type_ = type_;
            rloroad->e_spidx_ = attDbfData.readLongField(i, E_SPIDX);
            rloroad->s_spidx_ = attDbfData.readLongField(i, S_SPIDX);
            rloroad->road_id_ = to_string(attDbfData.readLongField(i, ROAD_ID));
            rloroad->x_ = attDbfData.readDoubleField(i, X);
            rloroad->y_ = attDbfData.readDoubleField(i, Y);
            rloroad->direction_ = attDbfData.readLongField(i, DIRECTION);
            rloroad->lo_id_ = lo_id;
            r_lo_road.insert(make_pair(rloroad->lo_id_, rloroad));
        }
    }
}