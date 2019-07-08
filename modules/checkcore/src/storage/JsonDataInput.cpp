//
// Created by zhangxingang on 19-6-13.
//

#include <storage/JsonDataInput.h>

#include <shp/shapefil.h>
#include <shp/ShpData.hpp>
#include <util/KDSUtil.h>

namespace kd {
    namespace dc {

        JsonDataInput::JsonDataInput(const shared_ptr<MapDataManager> &map_data_manager_,
                                     const shared_ptr<CheckErrorOutput> &error_output_,
                                     const string &base_path_, const string &model_path_,
                                     shared_ptr<ResourceManager> resource_manager)
                : DataInput(map_data_manager_, error_output_, base_path_), ModelInput(model_path_),
                  resource_manager_(resource_manager) {

        }

        bool JsonDataInput::LoadData() {
            //加载车道线数据
            if(LoadDivider()){

                for(auto recordit : map_data_manager_->dividers_){
                    string divid = recordit.first;
                    shared_ptr<DCDivider> div = recordit.second;

                    if(!div->valid_)
                        continue;

                    //属性重排
                    div->sortAtts();

                    //构造空间几何属性
                    div->buildGeometryInfo();
                }
            }

            if (LoadLaneGroup()) {

            }
            return false;
        }

        bool JsonDataInput::LoadDivider() {
            map<long, map<int, shared_ptr<kd::api::KDSDividerAttribute>>> divider2_da_maps;
            KDSUtil::BuildDividerId2DAs(resource_manager_, divider2_da_maps);
            auto &dividers = map_data_manager_->dividers_;
            //读取节点信息
            const auto &kds_data_map = resource_manager_->getKdsData(MODEL_NAME_DIVIDER);

            for (auto kds_data : kds_data_map) {
                shared_ptr<KDSDivider> kds_divider = std::static_pointer_cast<KDSDivider>(kds_data.second);

                LoadModelField(MODEL_NAME_DIVIDER, kds_divider);

                string taskid = kds_divider->getProperty(KDSDivider::TASKID);
                string flag = kds_divider->getProperty(KDSDivider::FLAG);
                auto divider_iter = dividers.find(to_string(kds_divider->ID));
                if (divider_iter == dividers.end()) {
                    shared_ptr<DCDivider> dc_divider = KDSUtil::CopyFromKDSDivider(kds_divider, error_output_);
                    if (dc_divider) {
                        // 构建fnode与divider关系
                        map_data_manager_->insert_fnode_id2_dividers(dc_divider->fromNodeId_, dc_divider);
                        map_data_manager_->insert_node_id2_dividers(dc_divider->fromNodeId_, dc_divider);

                        // 构建tnode与divider关系
                        map_data_manager_->insert_tnode_id2_dividers(dc_divider->toNodeId_, dc_divider);
                        map_data_manager_->insert_node_id2_dividers(dc_divider->toNodeId_, dc_divider);

                        const auto &da_maps = KDSUtil::GetDividerDAs(stol(dc_divider->id_), divider2_da_maps);
                        if(da_maps.size()>0) {
                            for (auto da : da_maps) {
                                shared_ptr<DCDividerAttribute> dc_da = KDSUtil::CopyFromKDSDA(da.second, kds_divider);
                                if (dc_da) {
                                    dc_divider->atts_.emplace_back(dc_da);
                                } else {
                                    int k = 0;
                                }
                            }
                        }

                        dc_divider->task_id_ = taskid;
                        dc_divider->flag_ = flag;
                        dividers.emplace(dc_divider->id_, dc_divider);
                    }
                } else {
                    // divider 重复
                }
            }

            return true;
        }

        bool JsonDataInput::LoadLane() {
            return false;
        }

        bool JsonDataInput::LoadLaneGroup() {
            const auto &kds_data_map = resource_manager_->getKdsData(MODEL_NAME_R_DIVIDER_DREF);

            for (auto kds_data : kds_data_map) {
                shared_ptr<KDSR_Divider_DREF> kds_dref = std::static_pointer_cast<KDSR_Divider_DREF>(kds_data.second);
                LoadModelField(MODEL_NAME_R_DIVIDER_DREF, kds_dref);
                LoadModelRelation(MODEL_NAME_R_DIVIDER_DREF, kds_dref);
                shared_ptr<DCLaneGroup> dc_lane_group = make_shared<DCLaneGroup>();
                dc_lane_group->id_ = to_string(kds_dref->ID);
                long divider_id = -1;
                for (const auto &role : kds_dref->vecMemberAndRols) {
                    if (role.first == DIVIDER_ID) {
                        divider_id = role.second;
                        if (divider_id != -1) {
                            map_data_manager_->insert_divider2_lane_groups(to_string(divider_id), to_string(kds_dref->ID));
                        }
                    }
                }

                map_data_manager_->laneGroups_.emplace(dc_lane_group->id_, dc_lane_group);


            }
            return true;
        }

        bool JsonDataInput::LoadLaneConnectivity() {
            return false;
        }

        bool JsonDataInput::LoadRoad() {
            return false;
        }

        bool JsonDataInput::LoadLaneGroupLogicInfo() {
            return false;
        }

        void JsonDataInput::LoadModelField(string model_name, shared_ptr<KDSData> kds_data) {
            auto model_define = GetModelDefine(model_name);
            if (model_define) {
                shared_ptr<DCModelRecord> record = make_shared<DCModelRecord>();
                for (shared_ptr<DCFieldDefine> field : model_define->vecFieldDefines) {

                    string field_name = field->name;

                    switch (field->type) {
                        case DC_FIELD_TYPE_VARCHAR: {
                            string value = kds_data->getProperty(field_name);
                            record->textDatas.insert(pair<string, string>(field_name, value));
                        }
                            break;
                        case DC_FIELD_TYPE_LONG: {
                            if (field_name == ID) {
                                record->insert_long_data(field_name, kds_data->ID);
                            } else {
                                long value = kds_data->getPropertyLong(field_name);
                                record->insert_long_data(field_name, value);
                            }
                        }
                            break;
                        case DC_FIELD_TYPE_DOUBLE: {
                            double value = kds_data->getPropertyDouble(field_name);
                            record->insert_double_data(field_name, value);
                        }
                            break;
                        case DC_FIELD_TYPE_TEXT: {
                            string value = kds_data->getProperty(field_name);
                            record->insert_text_data(field_name, value);
                        }
                            break;
                        default:
                            stringstream ss;
                            ss << "[Error] field type error. file='" << field_name << "', fieldName='" << field_name
                               << "' type is " << field->type;
                            error_output_->writeInfo(ss.str());
                            break;
                    }
                }

                auto model_data_iter = model_data_manager_->modelDatas_.find(model_name);
                if (model_data_iter != model_data_manager_->modelDatas_.end()) {
                    model_data_iter->second->records.emplace_back(record);
                } else {
                    shared_ptr<DCModalData> model_data = make_shared<DCModalData>();
                    model_data->records.emplace_back(record);
                    model_data_manager_->modelDatas_.emplace(model_name, model_data);
                }
            }
        }

        void JsonDataInput::LoadModelRelation(string model_name, shared_ptr<KDSRelation> kds_relation) {
            auto model_define = GetModelDefine(model_name);
            if (model_define) {
                shared_ptr<DCModelRecord> record = make_shared<DCModelRecord>();
                for (shared_ptr<DCFieldDefine> field : model_define->vecMemberAndRols) {

                    string field_name = field->name;

                    switch (field->type) {
                        case DC_RELATION_TYPE_LONG: {
                            for (const auto &role : kds_relation->vecMemberAndRols) {
                                if (role.first == field_name) {
                                    record->insert_long_data(field_name, role.second);
                                }
                            }
                        }
                            break;
                        default:
                            stringstream ss;
                            ss << "[Error] field type error. file='" << field_name << "', fieldName='" << field_name
                               << "' type is " << field->type;
                            error_output_->writeInfo(ss.str());
                            break;
                    }
                }

                auto model_data_iter = model_data_manager_->modelDatas_.find(model_name);
                if (model_data_iter != model_data_manager_->modelDatas_.end()) {
                    model_data_iter->second->records.emplace_back(record);
                } else {
                    shared_ptr<DCModalData> model_data = make_shared<DCModalData>();
                    model_data->records.emplace_back(record);
                    model_data_manager_->modelDatas_.emplace(model_name, model_data);
                }
            }
        }


        void JsonDataInput::CheckModelRelation() {
            for (auto model_define : model_data_manager_->modelDefines_) {
                string model_name = model_define.first;
                shared_ptr<DCModelDefine> model = model_define.second;
                for (auto relation : model->vecRelation) {
                    string member = relation->member;
                    string rel_model_name = relation->model;
                    string rule = relation->rule;
                    shared_ptr<DCFieldDefine> field = model->getFieldDefine(rule);
                    if (nullptr == field) {
                        continue;
                    }

                    shared_ptr<DCModalData> model_data = GetModelData(model_name);
                    if (model_data == nullptr) {
                        break;
                    }
                    for (auto record : model_data->records) {
                        if (field->type == DC_FIELD_TYPE_LONG || field->type == DC_RELATION_TYPE_LONG) {
                            auto rel_iter = record->long_data_maps_.find(rule);
                            if (rel_iter != record->long_data_maps_.end()) {
                                for (auto value : rel_iter->second) {
                                    auto kds_data = resource_manager_->getKdsData(rel_model_name, value);
                                    if (kds_data == nullptr) {
                                        // 关系不存在
                                        auto ptr_error = DCRelationCheckError::createByKXS_01_25(model_name, rule,
                                                                                                 rel_model_name);

                                        error_output_->saveError(ptr_error);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}