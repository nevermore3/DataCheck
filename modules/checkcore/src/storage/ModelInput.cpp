#include "storage/ModelInput.h"
#include <storage/CheckTaskInput.h>
#include <storage/ModelInput.h>
#include <process/ModelCheckFunc.h>


namespace kd {
    namespace dc {
        ModelInput::ModelInput(const string &model_path_) : model_path_(model_path_) {
            model_data_manager_ = make_shared<ModelDataManager>();
        }

        bool ModelInput::LoadModel() {
            CheckTaskInput taskInput;
            if (taskInput.loadTaskInfo(model_path_, model_data_manager_->tasks_)) {
                for (auto task : model_data_manager_->tasks_) {
                    CheckTaskInput taskInput;
                    string resource_path = task.second->modelName + ".json";
                    shared_ptr<DCModelDefine> model_define = make_shared<DCModelDefine>();
                    if (taskInput.loadTaskModel(resource_path, model_define)) {
                        if (model_data_manager_->modelDefines_.find(task.first) ==
                            model_data_manager_->modelDefines_.end()) {
                            model_data_manager_->modelDefines_.emplace(task.first, model_define);
                        }
                    }
                }

                return true;
            }

            return false;
        }

        shared_ptr<DCModelDefine> ModelInput::GetModelDefine(string model_name) {
            shared_ptr<DCModelDefine> ret_model_define = nullptr;
            auto model_iter = model_data_manager_->modelDefines_.find(model_name);
            if (model_iter != model_data_manager_->modelDefines_.end()){
                ret_model_define = model_iter->second;
            }
            return ret_model_define;
        }


        shared_ptr<DCModalData> ModelInput::GetModelData(string model_name) {
            shared_ptr<DCModalData> ret_model_data = nullptr;
            auto model_iter = model_data_manager_->modelDatas_.find(model_name);
            if (model_iter != model_data_manager_->modelDatas_.end()){
                ret_model_data = model_iter->second;
            }
            return ret_model_data;
        }


        bool ModelInput::CheckModelField(const shared_ptr<CheckErrorOutput> &error_output) {
            for (auto task : model_data_manager_->tasks_) {
                string model_name = task.first;

                //获取模型数据
                shared_ptr<DCModalData> model_data = GetModelData(model_name);
                if (model_data == nullptr) {
                    continue;
                }

                //获取模型配置
                shared_ptr<DCModelDefine> model_define = GetModelDefine(model_name);
                if (model_define == nullptr) {
                    continue;
                }

                //检查基础字段
                for (const shared_ptr<DCFieldDefine> &fieldDef : model_define->vecFieldDefines) {
                    if (fieldDef->valueLimit.length() == 0)
                        continue;

                    stringstream ss;
                    string field_name = fieldDef->name;
                    switch (fieldDef->type) {
                        case DC_FIELD_TYPE_LONG:
                            checkLongValueIn(model_name, fieldDef, model_data, field_name, error_output);
                            break;
                        case DC_FIELD_TYPE_DOUBLE:
                            checkDoubleValueIn(model_name, fieldDef, model_data, field_name, error_output);
                            break;
                        case DC_FIELD_TYPE_VARCHAR:
                        case DC_FIELD_TYPE_TEXT:
                            checkStringValueIn(model_name, fieldDef, model_data, field_name, error_output);
                            break;
                        default:
                            ss << "[Error] not support field type limit check.";
                            error_output->writeInfo(ss.str());
                            break;
                    }
                }
            }
            return false;
        }

        void ModelInput::checkDoubleValueIn(const string &task_name, const shared_ptr<DCFieldDefine> &fieldDef,
                                            const shared_ptr<DCModalData> &modelData,
                                            const string &fieldName,
                                            const shared_ptr<CheckErrorOutput> &errorOutput) {

            for (shared_ptr<DCModelRecord> record : modelData->records) {

                auto valuepair = record->double_data_maps_.find(fieldName);
                if (valuepair == record->double_data_maps_.end()) {
                    stringstream ss;
                    ss << task_name << " 没有找到字段" << fieldName << " value.";
                    shared_ptr<DCError> ptr_error = DCFieldError::createByKXS_01_019(ss.str());
                    errorOutput->saveError(ptr_error);
                    continue;
                }

                for (auto recordValue : valuepair->second) {
                    if (!IsValid<double>(fieldDef->valueLimit, recordValue)) {
                        stringstream ss;
                        ss << task_name << " 检查double类型 : " << fieldName << "=" << recordValue << " not in '"
                           << fieldDef->valueLimit << "'";
                        shared_ptr<DCError> ptr_error = DCFieldError::createByKXS_01_019(ss.str());
                        errorOutput->saveError(ptr_error);
                    }
                }
            }
        }

        void ModelInput::checkLongValueIn(const string &task_name, const shared_ptr<DCFieldDefine> &fieldDef,
                                          const shared_ptr<DCModalData> &modelData,
                                          const string &fieldName,
                                          const shared_ptr<CheckErrorOutput> &errorOutput) {

            for (shared_ptr<DCModelRecord> record : modelData->records) {

                auto valuepair = record->long_data_maps_.find(fieldName);
                if (valuepair == record->long_data_maps_.end()) {
                    stringstream ss;
                    ss << task_name << " 没有找到字段" << fieldName << " value.";
                    shared_ptr<DCError> ptr_error = DCFieldError::createByKXS_01_019(ss.str());
                    errorOutput->saveError(ptr_error);
                    continue;
                }

                for (auto recordValue : valuepair->second) {
                    if (!IsValid<long>(fieldDef->valueLimit, recordValue)) {
                        stringstream ss;
                        ss << task_name << "检查long类型 : " << fieldName << "=" << recordValue << " not in '"
                           << fieldDef->valueLimit << "'";
                        shared_ptr<DCError> ptr_error = DCFieldError::createByKXS_01_019(ss.str());
                        errorOutput->saveError(ptr_error);
                    }
                }
            }
        }

        void ModelInput::checkStringValueIn(const string &task_name, const shared_ptr<DCFieldDefine> &fieldDef,
                                            const shared_ptr<DCModalData> &modelData,
                                            const string &fieldName,
                                            const shared_ptr<CheckErrorOutput> &errorOutput) {

            for (shared_ptr<DCModelRecord> record : modelData->records) {

                auto valuepair = record->text_data_maps_.find(fieldName);
                if (valuepair == record->text_data_maps_.end()) {
                    stringstream ss;
                    ss << task_name << " 没有找到字段 " << fieldName << " value.";
                    shared_ptr<DCError> ptr_error = DCFieldError::createByKXS_01_019(ss.str());
                    errorOutput->saveError(ptr_error);
                    continue;
                }

                for (auto recordValue : valuepair->second) {
                    int len = recordValue.length();
                    //判断值是否非空
                    if (fieldDef->inputLimit == 1 && len == 0) {
                        stringstream ss;
                        ss << task_name << " 检查string类型非空 : " << fieldName << " value should not null.";
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
                    }

                    //判断值是否超限
                    if (len > 0 && !IsValid<string>(fieldDef->valueLimit, recordValue)) {
                        stringstream ss;
                        ss << task_name << "检查string类型 : " << fieldName << "=" << recordValue << " not in '"
                           << fieldDef->valueLimit << "'";
                        shared_ptr<DCError> ptr_error = DCFieldError::createByKXS_01_019(ss.str());
                        errorOutput->saveError(ptr_error);
                    }
                }
            }
        }

    }
}
