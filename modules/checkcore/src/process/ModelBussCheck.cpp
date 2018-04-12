//
// Created by gaoyanhong on 2018/3/29.
//

#include "process/ModelBussCheck.h"

namespace kd {
    namespace dc {


        string ModelBussCheck::getId() {
            return id;
        }

        //记录字段值
        template <typename T>
        void insertValue(const T& data, map<T, int>& values){
            auto itVal = values.find(data);
            if (itVal == values.end()){
                values.insert(make_pair(data, 1));
            } else {
                ++itVal->second;
            }
        }

        /*
        //获取字段值
        template <typename T>
        bool getFieldValues(const shared_ptr<DCModalData> modelData, const string& fieldName, map<T, int>& values){
            for (shared_ptr<DCModelRecord> record : modelData->records) {
                if (typeid(long) == typeid(T)){
                    auto longpair = record->longDatas.find(fieldName);
                    if (longpair == record->longDatas.end()) {
                        return false;
                    }
                    insertValue<T>(longpair->second, values);
                } else if (typeid(double) == typeid(T)){
                    auto doublepair = record->doubleDatas.find(fieldName);
                    if (doublepair == record->doubleDatas.end()) {
                        return false;
                    }
                    insertValue<T>(doublepair->second, values);
                } else if (typeid(string) == typeid(T)) {
                    auto texpair = record->textDatas.find(fieldName);
                    if (texpair == record->textDatas.end()) {
                        return false;
                    }
                    insertValue<string>(texpair->second, values);
                } else {
                    return false;
                }
            }
            return true;
        }

        //检查字段值是否唯一
        template <typename T>
        void checkFieldValueIdentify(shared_ptr<DCModalData> modelData, string fieldName, shared_ptr<CheckErrorOutput> errorOutput){
            map<T, int> mValues;
            if (!getFieldValues<T>(modelData, fieldName, mValues)){
                stringstream ss;
                ss << "[Error] get field values fail. " << fieldName;
                errorOutput->writeInfo(ss.str());
                return;
            }

            for (auto itVal : mValues){
                if (itVal.second > 1){
                    stringstream ss;
                    ss << "[Error] checkValueIn: field value not unique. " << fieldName << "=" << itVal.first;
                    errorOutput->writeInfo(ss.str());
                }
            }
        }*/

        bool ModelBussCheck::execute(shared_ptr<ModelDataManager> dataManager, shared_ptr<CheckErrorOutput> errorOutput) {

            for (auto taskit : dataManager->tasks_) {
                string strTaskName = taskit.first;

                //获取模型数据
                shared_ptr<DCModalData> modelData = nullptr;
                auto itdata = dataManager->modelDatas_.find(strTaskName);
                if (itdata != dataManager->modelDatas_.end()) {
                    modelData = itdata->second;
                } else {
                    continue;
                };

                //获取模型配置
                shared_ptr<DCModelDefine> modelDefine = nullptr;
                auto itdef = dataManager->modelDefines_.find(strTaskName);
                if (itdef != dataManager->modelDefines_.end()) {
                    modelDefine = itdef->second;
                } else {
                    continue;
                };

                vector<shared_ptr<DCFieldCheckDefine>> vecFieldChecks;
                for (shared_ptr<DCFieldCheckDefine> check : modelDefine->vecFieldChecks) {

                    shared_ptr<DCFieldDefine> fieldDefine = modelDefine->getFieldDefine(check->fieldName);
                    if (fieldDefine == nullptr) {
                        stringstream ss;
                        ss << "[Error] field check oper not find field " << check->fieldName;
                        errorOutput->writeInfo(ss.str());
                        continue;
                    }

                    switch (check->func) {
                        case DC_FIELD_VALUE_FUNC_ID:
                            checkFieldIdentify(modelData, fieldDefine, errorOutput);
                            break;
                        case DC_FIELD_VALUE_FUNC_GE:
                            break;
                        default:
                            errorOutput->writeInfo("[TODO] function need to implement.");
                            break;
                    }
                }
            }

            return true;
        }

        void ModelBussCheck::checkFieldIdentify(shared_ptr<DCModalData> modelData, shared_ptr<DCFieldDefine> fieldDef, shared_ptr<CheckErrorOutput> errorOutput) {
            switch (fieldDef->type) {
                case DC_FIELD_TYPE_LONG:
                    //checkFieldValueIdentify<long>(modelData, fieldDef->name, errorOutput);
                    checkLongFieldIdentify(modelData, fieldDef->name, errorOutput);
                    break;
                case DC_FIELD_TYPE_DOUBLE:
                    //checkFieldValueIdentify<double>(modelData, fieldDef->name, errorOutput);
                    checkDoubleFieldIdentify(modelData, fieldDef->name, errorOutput);
                    break;
                case DC_FIELD_TYPE_VARCHAR:
                case DC_FIELD_TYPE_TEXT:
                    //checkFieldValueIdentify<string>(modelData, fieldDef->name, errorOutput);
                    checkStringFieldIdentify(modelData, fieldDef->name, errorOutput);
                    break;
                default:
                    stringstream ss;
                    ss << "[Error] checkFieldIdentify not support field type :" << fieldDef->type;
                    errorOutput->writeInfo(ss.str());
                    break;
            }
        }

        bool getLongValues(const shared_ptr<DCModalData> modelData, const string& fieldName, map<long, int>& values){
            for (shared_ptr<DCModelRecord> record : modelData->records) {
                auto longpair = record->longDatas.find(fieldName);
                if (longpair == record->longDatas.end()) {
                    return false;
                }
                insertValue<long>(longpair->second, values);
            }
            return true;
        }

        bool getDoubleValues(const shared_ptr<DCModalData> modelData, const string& fieldName, map<double, int>& values){
            for (shared_ptr<DCModelRecord> record : modelData->records) {
                auto doublepair = record->doubleDatas.find(fieldName);
                if (doublepair == record->doubleDatas.end()) {
                    return false;
                }
                insertValue<double>(doublepair->second, values);
            }
            return true;
        }

        bool getStringValues(const shared_ptr<DCModalData> modelData, const string& fieldName, map<string, int>& values){
            for (shared_ptr<DCModelRecord> record : modelData->records) {
                auto texpair = record->textDatas.find(fieldName);
                if (texpair == record->textDatas.end()) {
                    return false;
                }
                insertValue<string>(texpair->second, values);
            }
            return true;
        }

        void ModelBussCheck::checkLongFieldIdentify(shared_ptr<DCModalData> modelData, string fieldName, shared_ptr<CheckErrorOutput> errorOutput){
            map<long, int> mValues;
            if (!getLongValues(modelData, fieldName, mValues)){
                stringstream ss;
                ss << "[Error] get field values fail. " << fieldName;
                errorOutput->writeInfo(ss.str());
                return;
            }

            for (auto itVal : mValues){
                if (itVal.second > 1){
                    stringstream ss;
                    ss << "[Error] checkValueIn: field value not unique. " << fieldName << "=" << itVal.first;
                    errorOutput->writeInfo(ss.str());
                }
            }
        }

        void ModelBussCheck::checkDoubleFieldIdentify(shared_ptr<DCModalData> modelData, string fieldName, shared_ptr<CheckErrorOutput> errorOutput){
            map<long, int> mValues;
            if (!getLongValues(modelData, fieldName, mValues)){
                stringstream ss;
                ss << "[Error] get field values fail. " << fieldName;
                errorOutput->writeInfo(ss.str());
                return;
            }

            for (auto itVal : mValues){
                if (itVal.second > 1){
                    stringstream ss;
                    ss << "[Error] checkValueIn: field value not unique. " << fieldName << "=" << itVal.first;
                    errorOutput->writeInfo(ss.str());
                }
            }
        }

        void ModelBussCheck::checkStringFieldIdentify(shared_ptr<DCModalData> modelData, string fieldName, shared_ptr<CheckErrorOutput> errorOutput){
            map<long, int> mValues;
            if (!getLongValues(modelData, fieldName, mValues)){
                stringstream ss;
                ss << "[Error] get field values fail. " << fieldName;
                errorOutput->writeInfo(ss.str());
                return;
            }

            for (auto itVal : mValues){
                if (itVal.second > 1){
                    stringstream ss;
                    ss << "[Error] checkValueIn: field value not unique. " << fieldName << "=" << itVal.first;
                    errorOutput->writeInfo(ss.str());
                }
            }
        }
    }
}
