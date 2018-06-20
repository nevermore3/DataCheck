//
// Created by gaoyanhong on 2018/3/29.
//

#include <data/DataManager.h>
#include "process/ModelCheckFunc.h"
#include "process/ModelFieldCheck.h"


namespace kd {
    namespace dc {


        string ModelFieldCheck::getId() {
            return id;
        }

        bool ModelFieldCheck::execute(shared_ptr<ModelDataManager> dataManager, shared_ptr<CheckErrorOutput> errorOutput) {

            for (auto taskit : dataManager->tasks_) {
                string strTaskName = taskit.first;

                //获取模型数据
                shared_ptr<DCModalData> modelData = nullptr;
                auto itdata = dataManager->modelDatas_.find(strTaskName);
                if (itdata != dataManager->modelDatas_.end()){
                    modelData = itdata->second;
                } else {
                    continue;
                };

                //获取模型配置
                shared_ptr<DCModelDefine> modelDefine = nullptr;
                auto itdef = dataManager->modelDefines_.find(strTaskName);
                if (itdef != dataManager->modelDefines_.end()){
                    modelDefine = itdef->second;
                } else {
                    continue;
                };

                //检查基础字段
                for (shared_ptr<DCFieldDefine> fieldDef : modelDefine->vecFieldDefines) {
                    if (fieldDef->valueLimit.length() == 0)
                        continue;

                    stringstream ss;
                    string fieldName = fieldDef->name;
                    switch (fieldDef->type) {
                        case DC_FIELD_TYPE_LONG:
                            checkLongValueIn(fieldDef, modelData, fieldName, errorOutput);
                            break;
                        case DC_FIELD_TYPE_DOUBLE:
                            checkDoubleValueIn(fieldDef, modelData, fieldName, errorOutput);
                            break;
                        case DC_FIELD_TYPE_VARCHAR:
                        case DC_FIELD_TYPE_TEXT:
                            checkStringValueIn(fieldDef, modelData, fieldName, errorOutput);
                            break;
                        default:
                            ss << "[Error] not support field type limit check.";
                            errorOutput->writeInfo(ss.str());
                            break;
                    }
                }
            }
            return true;
        }

        void ModelFieldCheck::checkDoubleValueIn(const shared_ptr<DCFieldDefine> fieldDef, const shared_ptr<DCModalData> modelData,
                               const string& fieldName, const shared_ptr<CheckErrorOutput> errorOutput) {

            for (shared_ptr<DCModelRecord> record : modelData->records) {

                auto valuepair = record->doubleDatas.find(fieldName);
                if (valuepair == record->doubleDatas.end()) {
                    stringstream ss;
                    ss << "[Error] not find field " << fieldName << " value.";
                    errorOutput->writeInfo(ss.str());
                    continue;
                }

                double recordValue = valuepair->second;
                if (!IsValid<double>(fieldDef->valueLimit, recordValue)){
                    stringstream ss;
                    ss << "[Error] checkDoubleValueIn : " << fieldName << "=" << recordValue << " not in '"<< fieldDef->valueLimit <<"'";
                    errorOutput->writeInfo(ss.str());
                }
            }
        }

        void ModelFieldCheck::checkLongValueIn(const shared_ptr<DCFieldDefine> fieldDef, const shared_ptr<DCModalData> modelData,
                                               const string& fieldName, const shared_ptr<CheckErrorOutput> errorOutput) {

            for (shared_ptr<DCModelRecord> record : modelData->records) {

                auto valuepair = record->longDatas.find(fieldName);
                if (valuepair == record->longDatas.end()) {
                    stringstream ss;
                    ss << "[Error] not find field " << fieldName << " value.";
                    errorOutput->writeInfo(ss.str());
                    continue;
                }

                long recordValue = valuepair->second;
                if (!IsValid<long>(fieldDef->valueLimit, recordValue)){
                    stringstream ss;
                    ss << "[Error] checkLongValueIn : " << fieldName << "=" << recordValue << " not in '"<< fieldDef->valueLimit <<"'";
                    errorOutput->writeInfo(ss.str());
                }
            }
        }

        void ModelFieldCheck::checkStringValueIn(const shared_ptr<DCFieldDefine> fieldDef, const shared_ptr<DCModalData> modelData,
                                               const string& fieldName, const shared_ptr<CheckErrorOutput> errorOutput) {

            for (shared_ptr<DCModelRecord> record : modelData->records) {

                auto valuepair = record->textDatas.find(fieldName);
                if (valuepair == record->textDatas.end()) {
                    stringstream ss;
                    ss << "[Error] not find field " << fieldName << " value.";
                    errorOutput->writeInfo(ss.str());
                    continue;
                }

                auto recordValue = valuepair->second;
                int len = recordValue.length();
                //判断值是否非空
                if (fieldDef->inputLimit == 1 && len == 0){
                    stringstream ss;
                    ss << "[Error] checkStringValueIn : " << fieldName << " value should not null.";
                    errorOutput->writeInfo(ss.str());
                }

                //判断值是否超长
                if (fieldDef->len > 0 && len > fieldDef->len){
                    stringstream ss;
                    ss << "[Error] checkStringValueIn : " << fieldName << " len=" << len << " exceed '"<< fieldDef->len <<"'";
                    errorOutput->writeInfo(ss.str());
                }

                //判断值是否超限
                if (len > 0 && !IsValid<string>(fieldDef->valueLimit, recordValue)){
                    stringstream ss;
                    ss << "[Error] checkStringValueIn : " << fieldName << "=" << recordValue << " not in '"<< fieldDef->valueLimit <<"'";
                    errorOutput->writeInfo(ss.str());
                }
            }
        }
    }
}
