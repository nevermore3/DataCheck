//
// Created by gaoyanhong on 2018/3/28.
//

#include "storage/ModelDataInput.h"

//thirdparty
#include <shp/shapefil.h>
#include <shp/ShpData.hpp>

namespace kd {
    namespace dc {


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

            //读取数据
            int record_nums = shpData.getRecords();
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

            //读取数据
            int record_nums = shpData.getRecords();
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

            //读取数据
            int record_nums = dbfData.getRecords();
            for (int i = 0; i < record_nums; i++) {
                //遍历各个字段
                shared_ptr<DCModelRecord> record = make_shared<DCModelRecord>();
                for (shared_ptr<DCFieldDefine> field : vecFieldDefines) {

                    string fieldName = field->name;

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

            //读取数据
            int record_nums = shpData.getRecords();
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

