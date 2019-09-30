//
// Created by gaoyanhong on 2018/3/28.
//

#include "storage/ModelDataInput.h"

//thirdparty
#include <shp/shapefil.h>
#include <shp/ShpData.hpp>

namespace kd {
    namespace dc {

        struct DCField{
            string Name;
            int Type;
            int Width;
            int Decimal;

            DCField(){
                Name = "";
                Type = FTInteger;
                Width = 0;
                Decimal = 0;
            }
            DCField(string name, int type, int width, int decimal){
                Name = name;
                Type = type;
                Width = width;
                Decimal = decimal;
            }

            bool operator ==(const DCField& dcf) const {
                return (dcf.Name == Name && dcf.Type == Type);
                //return (dcf.Name == Name && dcf.Type == Type && dcf.Width == Width && dcf.Decimal == Decimal);
            }
        };

        DBFFieldType GetDBFFieldType(DCFieldType tp){
            if (tp == DCFieldType::DC_FIELD_TYPE_LONG){
                return DBFFieldType::FTInteger;
            } else if (tp == DCFieldType::DC_FIELD_TYPE_DOUBLE){
                return DBFFieldType::FTDouble;
            } else if (tp == DCFieldType::DC_FIELD_TYPE_TEXT || tp == DCFieldType::DC_FIELD_TYPE_VARCHAR){
                return DBFFieldType::FTString;
            } else {
                return DBFFieldType::FTInteger;
            }
        }

        bool VerifyFields(const string &fileName, const vector<shared_ptr<DCFieldDefine>> &vecFieldDefines, const DBFHandle pDBF,
                          shared_ptr<CheckErrorOutput> errorOutput, vector<string>& dbfFieldNames){
            string simpleFileName = fileName.substr(fileName.find_last_of('/') + 1);
            map<string, DCField> defFields;
            for (auto itFld : vecFieldDefines){
                string fldName = itFld->name;
                if (defFields.find(fldName) == defFields.end()){
                    defFields.insert(make_pair(fldName, DCField(fldName, GetDBFFieldType(itFld->type), itFld->len, 0)));
                } else {
                    // 配置文件中字段冗余
                    stringstream ss;
                    ss << "配置文件 " << simpleFileName << " 中的字段 "<< fldName << " 冗余";
                    shared_ptr<DCError> error = DCFieldError::createByKXS_01_019(ss.str());
                    errorOutput->saveError(error);
                }
            }

            map<string, DCField> dbfFields;
            for (int iFld = 0; iFld < pDBF->nFields; ++iFld) {
                char szfldName[64] = "";
                int fldWidth = 0, fldDecimal = 0;
                DBFFieldType fldType = DBFGetFieldInfo(pDBF, iFld, szfldName, &fldWidth, &fldDecimal);
                string fldName = string(szfldName);
                if (dbfFields.find(fldName) == dbfFields.end()){
                    dbfFields.insert(make_pair(fldName, DCField(fldName, fldType, fldWidth, fldDecimal)));
                } else {
                    // 字段重复
                    stringstream ss;
                    ss << "文件 " << simpleFileName << " 中的字段 "<< fldName << " 是重复字段";
                    shared_ptr<DCError> error = DCFieldError::createByKXS_01_019(ss.str());
                    errorOutput->saveError(error);
                }

                if (defFields.find(fldName) == defFields.end()){
                    // 字段冗余
                    stringstream ss;
                    ss << "文件 " << simpleFileName << " 中的字段 "<< fldName << " 冗余, 在规格中该字段不存在";
                    shared_ptr<DCError> error = DCFieldError::createByKXS_01_019(ss.str());
                    errorOutput->saveError(error);
                } else {
                    // 字段类型和规格中的不相等
                    if(!(defFields[fldName] == dbfFields[fldName])){
                        stringstream ss;
                        ss << "文件 " << simpleFileName << " 中的字段 "<< fldName << "  的类型和规格中的不同";
                        shared_ptr<DCError> error = DCFieldError::createByKXS_01_019(ss.str());
                        errorOutput->saveError(error);
                    }
                }
            }

            for (auto itDefFld : defFields) {
                if (dbfFields.find(itDefFld.first) == dbfFields.end()){
                    // 缺少字段
                    stringstream ss;
                    ss << "文件 " << simpleFileName << " 中缺少字段 "<< itDefFld.first;
                    shared_ptr<DCError> error = DCFieldError::createByKXS_01_019(ss.str());
                    errorOutput->saveError(error);
                } else {
                    dbfFieldNames.push_back(itDefFld.first);
                }
            }

            return true;
        }

        bool ModelDataInput::loadPointFile(const string &fileName,
                                           const vector<shared_ptr<DCFieldDefine>> &vecFieldDefines,
                                           shared_ptr<DCModalData> modelData, shared_ptr<CheckErrorOutput> errorOutput) {

            //加载文件
            ShpData shpData(fileName);
            if (!shpData.isInit()) {
                stringstream ss;
                ss << "[Error] open shp file " << fileName << " error.";
                errorOutput->writeInfo(ss.str());
                return false;
            }

            vector<string> dbfFieldNames;
            VerifyFields(fileName, vecFieldDefines, shpData.pDBF, errorOutput, dbfFieldNames);

            //读取数据
            int record_nums = shpData.getRecords();
            if (record_nums <= 0) {
                return false;
            }
            // 数据不能为空， 防止过多打印，只打印一次
            bool strNotNull = false;
            bool textNotNull = false;
            for (int i = 0; i < record_nums; i++) {
                SHPObject *shpObject = shpData.readShpObject(i);
                if (!shpObject || !(shpObject->nSHPType == SHPT_POINT || shpObject->nSHPType == SHPT_POINTZ)) {
                    stringstream ss;
                    ss << "[Error] file='" << fileName << "', object=" << i << " is null or type not point";
                    errorOutput->writeInfo(ss.str());
                    continue;
                }
                //遍历各个字段
                shared_ptr<DCModelRecord> record = make_shared<DCModelRecord>();
                for (shared_ptr<DCFieldDefine> field : vecFieldDefines) {

                    string fieldName = field->name;
                    if (std::find(dbfFieldNames.begin(),dbfFieldNames.end(), fieldName)==dbfFieldNames.end())
                        continue;

                    switch (field->type) {
                        case DC_FIELD_TYPE_VARCHAR: {
                            string value = shpData.readStringField(i, fieldName);
                            if (!strNotNull && field->inputLimit == 1 && value.empty()) {
                                stringstream ss;
                                ss << "文件 " << fileName << " 中的字段 "<< fieldName << " 不能为空";
                                shared_ptr<DCError> error = DCFieldError::createByKXS_01_019(ss.str());
                                errorOutput->saveError(error);
                                strNotNull = true;
                            }
                            record->textDatas.insert(pair<string, string>(fieldName, value));
                        }
                            break;
                        case DC_FIELD_TYPE_LONG: {
                            long value = shpData.readIntField(i, fieldName);
                            record->longDatas.insert(pair<string, long>(fieldName, value));
                        }
                            break;
                        case DC_FIELD_TYPE_DOUBLE: {
                            double value = shpData.readDoubleField(i, fieldName);
                            record->doubleDatas.insert(pair<string, double>(fieldName, value));
                        }
                            break;
                        case DC_FIELD_TYPE_TEXT: {
                            string value = shpData.readStringField(i, fieldName);
                            if (!textNotNull && field->inputLimit == 1 && value.empty()) {
                                stringstream ss;
                                ss << "文件 " << fileName << " 中的字段 "<< fieldName << " 不能为空";
                                shared_ptr<DCError> error = DCFieldError::createByKXS_01_019(ss.str());
                                errorOutput->saveError(error);
                                textNotNull = true;
                            }
                            record->textDatas.insert(pair<string, string>(fieldName, value));
                        }
                            break;
                        default:
                            stringstream ss;
                            ss << "[Error] field type error. file='" << fileName << "', fieldName='" << fieldName << "' type is " << field->type;
                            errorOutput->writeInfo(ss.str());
                            break;
                    }
                }

                modelData->records.emplace_back(record);
            }
            return true;
        }

        bool
        ModelDataInput::loadArcFile(const string &fileName, const vector<shared_ptr<DCFieldDefine>> &vecFieldDefines,
                                    shared_ptr<DCModalData> modelData, shared_ptr<CheckErrorOutput> errorOutput) {
            //加载文件
            ShpData shpData(fileName);
            if (!shpData.isInit()) {
                stringstream ss;
                ss << "[Error] open shp file " << fileName << " error.";
                errorOutput->writeInfo(ss.str());
                return false;
            }

            vector<string> dbfFieldNames;
            VerifyFields(fileName, vecFieldDefines, shpData.pDBF, errorOutput, dbfFieldNames);

            //读取数据
            int record_nums = shpData.getRecords();
            if (record_nums <= 0) {
                return false;
            }
            for (int i = 0; i < record_nums; i++) {
                SHPObject *shpObject = shpData.readShpObject(i);
                if (!shpObject || !(shpObject->nSHPType == SHPT_ARCZ || shpObject->nSHPType == SHPT_ARC)) {
                    stringstream ss;
                    ss << "[Error] file='" << fileName << "', object=" << i << " is null or type not arc";
                    errorOutput->writeInfo(ss.str());
                    continue;
                }

                int num = vecFieldDefines.size();
                //遍历各个字段
                shared_ptr<DCModelRecord> record = make_shared<DCModelRecord>();
                for (shared_ptr<DCFieldDefine> field : vecFieldDefines) {

                    string fieldName = field->name;
                    if (std::find(dbfFieldNames.begin(),dbfFieldNames.end(), fieldName)==dbfFieldNames.end())
                        continue;

                    switch (field->type) {
                        case DC_FIELD_TYPE_VARCHAR: {
                            string value = shpData.readStringField(i, fieldName);
                            record->textDatas.insert(pair<string, string>(fieldName, value));
                        }
                            break;
                        case DC_FIELD_TYPE_LONG: {
                            long value = shpData.readIntField(i, fieldName);
                            record->longDatas.insert(pair<string, long>(fieldName, value));
                        }
                            break;
                        case DC_FIELD_TYPE_DOUBLE: {
                            double value = shpData.readDoubleField(i, fieldName);
                            record->doubleDatas.insert(pair<string, double>(fieldName, value));
                        }
                            break;
                        case DC_FIELD_TYPE_TEXT: {
                            string value = shpData.readStringField(i, fieldName);
                            record->textDatas.insert(pair<string, string>(fieldName, value));
                        }
                            break;
                        default:
                            stringstream ss;
                            ss << "[Error] field type error. file='" << fileName << "', fieldName='" << fieldName << "' type is " << field->type;
                            errorOutput->writeInfo(ss.str());
                            break;
                    }
                }

                modelData->records.emplace_back(record);
            }
            return true;
        }

        bool
        ModelDataInput::loadDBFFile(const string &fileName, const vector<shared_ptr<DCFieldDefine>> &vecFieldDefines,
                                    shared_ptr<DCModalData> modelData, shared_ptr<CheckErrorOutput> errorOutput) {
            //加载文件
            DbfData dbfData(fileName);
            if (!dbfData.isInit()) {
                stringstream ss;
                ss << "[Error] open dbf file " << fileName << " error.";
                errorOutput->writeInfo(ss.str());
                return false;
            }

            vector<string> dbfFieldNames;
            VerifyFields(fileName, vecFieldDefines, dbfData.pDBF, errorOutput, dbfFieldNames);

            //读取数据
            int record_nums = dbfData.getRecords();
            if (record_nums <= 0) {
                return false;
            }
            for (int i = 0; i < record_nums; i++) {
                //遍历各个字段
                shared_ptr<DCModelRecord> record = make_shared<DCModelRecord>();
                for (shared_ptr<DCFieldDefine> field : vecFieldDefines) {

                    string fieldName = field->name;
                    if (std::find(dbfFieldNames.begin(),dbfFieldNames.end(), fieldName)==dbfFieldNames.end())
                        continue;

                    switch (field->type) {
                        case DC_FIELD_TYPE_VARCHAR: {
                            string value = dbfData.readStringField(i, fieldName);
                            record->textDatas.insert(pair<string, string>(fieldName, value));
                        }
                            break;
                        case DC_FIELD_TYPE_LONG: {
                            long value = dbfData.readIntField(i, fieldName);
                            record->longDatas.insert(pair<string, long>(fieldName, value));
                        }
                            break;
                        case DC_FIELD_TYPE_DOUBLE: {
                            double value = dbfData.readDoubleField(i, fieldName);
                            record->doubleDatas.insert(pair<string, double>(fieldName, value));
                        }
                            break;
                        case DC_FIELD_TYPE_TEXT: {
                            string value = dbfData.readStringField(i, fieldName);
                            record->textDatas.insert(pair<string, string>(fieldName, value));
                        }
                            break;
                        default:
                            stringstream ss;
                            ss << "[Error] field type error. file='" << fileName << "', fieldName='" << fieldName << "' type is " << field->type;
                            errorOutput->writeInfo(ss.str());
                            break;
                    }
                }

                modelData->records.emplace_back(record);
            }
            return true;
        }

        bool
        ModelDataInput::loadPolygonFile(const string &fileName, const vector<shared_ptr<DCFieldDefine>> &vecFieldDefines,
                                    shared_ptr<DCModalData> modelData, shared_ptr<CheckErrorOutput> errorOutput) {
            //加载文件
            ShpData shpData(fileName);
            if (!shpData.isInit()) {
                stringstream ss;
                ss << "[Error] open shp file " << fileName << " error.";
                errorOutput->writeInfo(ss.str());
                return false;
            }

            vector<string> dbfFieldNames;
            VerifyFields(fileName, vecFieldDefines, shpData.pDBF, errorOutput, dbfFieldNames);

            //读取数据
            int record_nums = shpData.getRecords();
            if (record_nums <= 0) {
                return false;
            }
            for (int i = 0; i < record_nums; i++) {
                SHPObject *shpObject = shpData.readShpObject(i);
                if (!shpObject || !(shpObject->nSHPType == SHPT_POLYGON || shpObject->nSHPType == SHPT_POLYGONZ || shpObject->nSHPType == SHPT_POLYGONM)) {
                    stringstream ss;
                    ss << "[Error] file='" << fileName << "', object=" << i << " is null or type not polygon";
                    errorOutput->writeInfo(ss.str());
                    continue;
                }

                int num = vecFieldDefines.size();
                //遍历各个字段
                shared_ptr<DCModelRecord> record = make_shared<DCModelRecord>();
                for (shared_ptr<DCFieldDefine> field : vecFieldDefines) {

                    string fieldName = field->name;
                    if (std::find(dbfFieldNames.begin(),dbfFieldNames.end(), fieldName)==dbfFieldNames.end())
                        continue;

                    switch (field->type) {
                        case DC_FIELD_TYPE_VARCHAR: {
                            string value = shpData.readStringField(i, fieldName);
                            record->textDatas.insert(pair<string, string>(fieldName, value));
                        }
                            break;
                        case DC_FIELD_TYPE_LONG: {
                            long value = shpData.readIntField(i, fieldName);
                            record->longDatas.insert(pair<string, long>(fieldName, value));
                        }
                            break;
                        case DC_FIELD_TYPE_DOUBLE: {
                            double value = shpData.readDoubleField(i, fieldName);
                            record->doubleDatas.insert(pair<string, double>(fieldName, value));
                        }
                            break;
                        case DC_FIELD_TYPE_TEXT: {
                            string value = shpData.readStringField(i, fieldName);
                            record->textDatas.insert(pair<string, string>(fieldName, value));
                        }
                            break;
                        default:
                            stringstream ss;
                            ss << "[Error] field type error. file='" << fileName << "', fieldName='" << fieldName << "' type is " << field->type;
                            errorOutput->writeInfo(ss.str());
                            break;
                    }
                }

                modelData->records.emplace_back(record);

            }
            return true;
        }
    }
}

