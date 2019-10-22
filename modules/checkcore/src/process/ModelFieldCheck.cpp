
#include <data/DataManager.h>
#include "process/ModelCheckFunc.h"
#include "process/ModelFieldCheck.h"
#include "util/product_shp_util.h"

namespace kd {
    namespace dc {


        string ModelFieldCheck::getId() {
            return id;
        }

        bool
        ModelFieldCheck::execute(shared_ptr<ModelDataManager> dataManager, shared_ptr<CheckErrorOutput> errorOutput) {

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

                //检查基础字段
                for (shared_ptr<DCFieldDefine> fieldDef : modelDefine->vecFieldDefines) {
                    if (fieldDef->valueLimit.length() == 0)
                        continue;

                    stringstream ss;
                    string fieldName = fieldDef->name;
                    switch (fieldDef->type) {
                        case DC_FIELD_TYPE_LONG:
                            checkLongValueIn(strTaskName, fieldDef, modelData, fieldName, errorOutput);
                            break;
                        case DC_FIELD_TYPE_DOUBLE:
                            checkDoubleValueIn(strTaskName, fieldDef, modelData, fieldName, errorOutput);
                            break;
                        case DC_FIELD_TYPE_VARCHAR:
                        case DC_FIELD_TYPE_TEXT:
                            checkStringValueIn(strTaskName, fieldDef, modelData, fieldName, errorOutput);
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

        void ModelFieldCheck::checkDoubleValueIn(const string &task_name, const shared_ptr<DCFieldDefine> &fieldDef,
                                                 const shared_ptr<DCModalData> &modelData,
                                                 const string &fieldName,
                                                 const shared_ptr<CheckErrorOutput> &errorOutput) {

            for (shared_ptr<DCModelRecord> record : modelData->records) {

                auto valuepair = record->doubleDatas.find(fieldName);
                if (valuepair == record->doubleDatas.end()) {
                    long id_ = record->longDatas.find(ID)->second;
                    stringstream ss;
                    ss << task_name << " 没有找到字段" << fieldName << " value."<< "',ID="<<id_;
//                    errorOutput->writeInfo(ss.str());
                    shared_ptr<DCError> ptr_error = DCFieldError::createByKXS_01_019(ss.str());
                    errorOutput->saveError(ptr_error);
                    continue;
                }

                double recordValue = valuepair->second;
                if (!IsValid<double>(fieldDef->valueLimit, recordValue)) {
                    long id_ = record->longDatas.find(ID)->second;
                    stringstream ss;
                    ss << task_name << " 检查double类型 : " << fieldName << "=" << recordValue << " not in '"
                       << fieldDef->valueLimit << "',ID="<<id_;
                    shared_ptr<DCError> ptr_error = DCFieldError::createByKXS_01_019(ss.str());
                    errorOutput->saveError(ptr_error);
                }
            }
        }

        void ModelFieldCheck::checkLongValueIn(const string &task_name, const shared_ptr<DCFieldDefine> &fieldDef,
                                               const shared_ptr<DCModalData> &modelData,
                                               const string &fieldName,
                                               const shared_ptr<CheckErrorOutput> &errorOutput) {

            for (shared_ptr<DCModelRecord> record : modelData->records) {
                auto valuepair = record->longDatas.find(fieldName);
                if (valuepair == record->longDatas.end()) {
                    long id_ = record->longDatas.find(ID)->second;
                    stringstream ss;
                    ss << task_name << " 没有找到字段" << fieldName << " value."<< "',ID="<<id_;
                    shared_ptr<DCError> ptr_error = DCFieldError::createByKXS_01_019(ss.str());
                    errorOutput->saveError(ptr_error);
//                    errorOutput->writeInfo(ss.str());
                    continue;
                }

                long recordValue = valuepair->second;
                if (!IsValid<long>(fieldDef->valueLimit, recordValue)) {
                    long id_ = record->longDatas.find(ID)->second;
                    stringstream ss;
                    ss << task_name << "检查long类型 : " << fieldName << "=" << recordValue << " not in '"
                       << fieldDef->valueLimit << "'，ID="<<id_;
                    shared_ptr<DCError> ptr_error = DCFieldError::createByKXS_01_019(ss.str());
                    errorOutput->saveError(ptr_error);
                    //                    errorOutput->writeInfo(ss.str());
                }
            }
        }

        void ModelFieldCheck::checkStringValueIn(const string &task_name, const shared_ptr<DCFieldDefine> &fieldDef,
                                                 const shared_ptr<DCModalData> &modelData,
                                                 const string &fieldName,
                                                 const shared_ptr<CheckErrorOutput> &errorOutput) {

            for (shared_ptr<DCModelRecord> record : modelData->records) {

                auto valuepair = record->textDatas.find(fieldName);
                if (valuepair == record->textDatas.end()) {
                    long id_ = record->longDatas.find(ID)->second;
                    stringstream ss;
                    ss << task_name << " 没有找到字段" << fieldName << " value."<< "',ID="<<id_;
//                    errorOutput->writeInfo(ss.str());
                    shared_ptr<DCError> ptr_error = DCFieldError::createByKXS_01_019(ss.str());
                    errorOutput->saveError(ptr_error);
                    continue;
                }

                auto recordValue = valuepair->second;
                int len = recordValue.length();
                //判断值是否非空
                if (fieldDef->inputLimit == 1 && len == 0) {
                    stringstream ss;
                    ss << task_name << " 检查string类型非空 : " << fieldName << " value should not null.";
//                    errorOutput->writeInfo(ss.str());
                    shared_ptr<DCError> ptr_error = DCFieldError::createByKXS_01_019(ss.str());
                    errorOutput->saveError(ptr_error);
                }

                //判断值是否超长
                if (fieldDef->len > 0 && len > fieldDef->len) {
                    stringstream ss;
                    ss << task_name << " 检查string类型是否超长 : " << fieldName << " len=" << len << " exceed '"
                       << fieldDef->len << "'";
                    shared_ptr<DCError> ptr_error = DCFieldError::createByKXS_01_019(ss.str());
                    errorOutput->saveError(ptr_error);
//                    errorOutput->writeInfo(ss.str());
                }

                //判断值是否超限
                if (len > 0 && !IsValid<string>(fieldDef->valueLimit, recordValue)) {
                    long id_ = record->longDatas.find(ID)->second;
                    stringstream ss;
                    ss << task_name << "检查string类型 : " << fieldName << "=" << recordValue << " not in '"
                       << fieldDef->valueLimit << "',ID="<<id_;
                    shared_ptr<DCError> ptr_error = DCFieldError::createByKXS_01_019(ss.str());
                    errorOutput->saveError(ptr_error);
//                    errorOutput->writeInfo(ss.str());
                }
            }
        }
    }
}
