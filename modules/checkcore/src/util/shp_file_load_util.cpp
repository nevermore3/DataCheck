//
// Created by ubuntu on 19-9-27.
//

#include <DataCheckConfig.h>

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
 void ShpFileLoad::GetFieldInfo(DBFHandle dbfHandle,vector<string> &fieldNames,vector<DBFFieldType> &field_defs){
     int field_count = DBFGetFieldCount(dbfHandle);
     for(int i = 0 ; i < field_count ; i ++){
         char field_name[32] = {0};
         int width, decimals;
         DBFFieldType field_type = DBFGetFieldInfo(dbfHandle, i, field_name, &width, &decimals);

         if(field_type == FTInteger && width == 15){
             field_type = FTLong;
         }
         if(field_name!=ID) {
             field_defs.push_back(field_type);
             fieldNames.push_back(field_name);
         }
     }
}
string ShpFileLoad::GetFieldValue(string fieldName,DBFFieldType dbfFieldType,DbfData &dbfData,int index){
    switch (dbfFieldType){
        case FTString:
            return dbfData.readStringField(index,fieldName);
        case FTInteger:
            return to_string(dbfData.readIntField(index,fieldName));
        case FTLong:
            return to_string(dbfData.readLongField(index,fieldName));
        case FTDouble:
            return to_string(dbfData.readDoubleField(index,fieldName));
        default:
            cout << "error type not support" <<dbfFieldType<< endl;
            return "";
    }
}
void ShpFileLoad::GetNodeData(string modelName,map<string, map<long, shared_ptr<KxsData>>> &kxfdata){
    if(modelName.length() == 0){
        return;
    }
    if(kxfdata.find(modelName)!= kxfdata.end()){
        kxfdata.erase(modelName);
    }

    string basePath = DataCheckConfig::getInstance().getProperty(DataCheckConfig::SHP_FILE_PATH);
    string filePath = basePath + "/" + modelName;
    ShpData shpFile(filePath);
    if (!shpFile.isInit()) {
        LOG(ERROR) << "Open shpFile :" << filePath << " Fail";
        return ;
    }

    map<long, shared_ptr<KxsData>> model_data;
    vector<string> fieldName;
    vector<DBFFieldType> field_defs;
    GetFieldInfo(shpFile.pDBF,fieldName,field_defs);

    size_t recordNums = shpFile.getRecords();
    for (size_t i = 0; i < recordNums; i++) {
        SHPObject *shpObject = shpFile.readShpObject(i);
        if (!shpObject || shpObject->nSHPType != SHPT_POINT)
            continue;

        shared_ptr<KxfNode> node = make_shared<KxfNode>();

        //读取属性信息
        int fieldSize = fieldName.size();
        node->id_ = shpFile.readIntField(i, ID);
        for(int j = 0;j<fieldSize;j++){
            node->addProperty(fieldName[j],GetFieldValue(fieldName[j],field_defs[j],shpFile,i));
        }

        size_t nVertices = shpObject->nVertices;
        if (nVertices == 1) {
            shared_ptr<DCCoord> coord = make_shared<DCCoord>();
            coord->x_ = shpObject->padfX[0];
            coord->y_ = shpObject->padfY[0];
            coord->z_ = shpObject->padfZ[0];
            node->coord_ = coord;
        }
        model_data.insert(make_pair(node->id_,node));
        SHPDestroyObject(shpObject);
    }
    kxfdata.insert(make_pair(modelName,model_data));
}

void ShpFileLoad::GetLineData(string modelName,map<string, map<long, shared_ptr<KxsData>>> &kxfdata,bool initGeom ){
    if(modelName.length() == 0){
        return;
    }
    if(kxfdata.find(modelName)!= kxfdata.end()){
        kxfdata.erase(modelName);
    }

    string basePath = DataCheckConfig::getInstance().getProperty(DataCheckConfig::SHP_FILE_PATH);
    string file = basePath + "/" + modelName;
    ShpData shpFile(file);
    if (shpFile.isInit()) {
        map<long, shared_ptr<KxsData>> model_data;
        vector<string> fieldName;
        vector<DBFFieldType> field_defs;
        GetFieldInfo(shpFile.pDBF,fieldName,field_defs);

        int fieldSize = fieldName.size();
        int recordNums = shpFile.getRecords();
        for (size_t i = 0; i < recordNums; i++) {

            shared_ptr<PolyLine> kxsdata = make_shared<PolyLine>();
            kxsdata->id_ = shpFile.readIntField(i, ID);
            for(int j = 0;j<fieldSize;j++){
                kxsdata->addProperty(fieldName[j],GetFieldValue(fieldName[j],field_defs[j],shpFile,i));
            }

            SHPObject *shpObject = shpFile.readShpObject(i);
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

                kxsdata->coords_.emplace_back(coord);
            }
            if(initGeom){
                double len;
                kxsdata->buildGeometry(kxsdata->coords_,false,len);
            }
            model_data.insert(make_pair(kxsdata->id_,kxsdata));
        }
        kxfdata.insert(make_pair(modelName,model_data));
    }
}

void ShpFileLoad::GetRelationData(string modelName,map<string, map<long, shared_ptr<KxsData>>> &kxfdata){
    if(modelName.length() == 0){
        return;
    }
    if(kxfdata.find(modelName)!= kxfdata.end()){
        kxfdata.erase(modelName);
    }


    string basePath = DataCheckConfig::getInstance().getProperty(DataCheckConfig::SHP_FILE_PATH);
    string file = basePath + "/" + modelName;
    DbfData attDbfData(file);
    if (attDbfData.isInit()) {
        map<long, shared_ptr<KxsData>> model_data;
        vector<string> fieldName;
        vector<DBFFieldType> field_defs;
        GetFieldInfo(attDbfData.pDBF,fieldName,field_defs);

        int fieldSize = fieldName.size();
        int recordNums = attDbfData.getRecords();
        for (size_t i = 0; i < recordNums; i++) {

            shared_ptr<Relation> kxsdata = make_shared<Relation>();
            kxsdata->id_ = attDbfData.readIntField(i, ID);
            for(int j = 0;j<fieldSize;j++){
                kxsdata->addProperty(fieldName[j],GetFieldValue(fieldName[j],field_defs[j],attDbfData,i));
            }
            model_data.insert(make_pair(kxsdata->id_,kxsdata));
        }
        kxfdata.insert(make_pair(modelName,model_data));
    }

}
