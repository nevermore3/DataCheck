//
// Created by ubuntu on 19-9-27.
//

#include <DataCheckConfig.h>
#include <shp/ShpData.hpp>
#include <util/CommonUtil.h>
#include "util/shp_file_load_util.h"

void ShpFileLoad::GetPolyline(long type,map<string,shared_ptr<DCPolyline>> &polylines) {

    string basePath = DataCheckConfig::getInstance().getProperty(DataCheckConfig::SHP_FILE_PATH);
    string file = basePath + "/HD_POLYLINE";
    ShpData shpFile(file);
    if (shpFile.isInit()) {
        int recordNums = shpFile.getRecords();
        for (size_t i = 0; i < recordNums; i++) {
            SHPObject *shpObject = shpFile.readShpObject(i);
            long type_ = shpFile.readLongField(i, "TYPE");
            if(type_!=type){
                continue;
            }
            shared_ptr<DCPolyline> polyline = make_shared<DCPolyline>();
            polyline->id_ = shpFile.readStringField(i, "ID");
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

            polylines.insert(make_pair(polyline->id_,polyline));
        }
    }
}