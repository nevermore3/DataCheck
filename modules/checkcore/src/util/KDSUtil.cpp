//
// Created by zhangxingang on 19-6-13.
//

#include <util/KDSUtil.h>
#include <util/KdsDataUtil.h>
#include <data/ErrorDataModel.h>

#include "util/KDSUtil.h"
namespace kd {
    namespace dc {


        shared_ptr<DCDivider> KDSUtil::CopyFromKDSDivider(shared_ptr<KDSDivider> kds_divider,
                                                          shared_ptr<CheckErrorOutput> error_output) {
            shared_ptr<DCDivider> dc_divider = nullptr;
            if (kds_divider != nullptr) {
                dc_divider = make_shared<DCDivider>();
                dc_divider->id_ = to_string(kds_divider->ID);
                dc_divider->dividerNo_ = kds_divider->getPropertyLong(KDSDivider::DIVIDER_NO);
                dc_divider->direction_ = kds_divider->getPropertyLong(KDSDivider::DIRECTION);
                dc_divider->rLine_ = kds_divider->getPropertyLong(KDSDivider::R_LINE);
                dc_divider->tollFlag_ = kds_divider->getPropertyLong(KDSDivider::TOLLFLAG);

                if (kds_divider->nodes.size() > 1) {
                    dc_divider->fromNodeId_ = to_string(kds_divider->nodes.front()->ID);
                    dc_divider->toNodeId_ = to_string(kds_divider->nodes.back()->ID);

                    set<long> error_node_index;
                    for (const auto &kds_node : kds_divider->nodes) {
                        shared_ptr<DCDividerNode> dc_node = CopyFromKDSDividerNode(kds_node);
                        if (dc_node) {
                            dc_divider->nodes_.emplace_back(dc_node);
                        } else {
                            // 异常
                        }
                    }

                    if (!error_node_index.empty()) {
                        shared_ptr<DCError> ptr_error = DCFieldError::createByKXS_01_024("divider", dc_divider->id_,
                                                                                         error_node_index);
                        if (error_output) {
                            error_output->saveError(ptr_error);
                        }
                    }
                } else {
                    // 异常
                }
            }
            return dc_divider;
        }

        shared_ptr<DCDividerNode> KDSUtil::CopyFromKDSDividerNode(shared_ptr<KDSNode> kds_divider_node) {
            shared_ptr<DCDividerNode> dc_divider_node = nullptr;
            if (kds_divider_node != nullptr) {
                dc_divider_node = make_shared<DCDividerNode>();
                dc_divider_node->id_ = to_string(kds_divider_node->ID);
                dc_divider_node->dashType_ = kds_divider_node->getPropertyLong(KDSDividerNode::DASHTYPE);
                dc_divider_node->coord_.lng_ = kds_divider_node->x;
                dc_divider_node->coord_.lat_ = kds_divider_node->y;
                dc_divider_node->coord_.z_ = kds_divider_node->z;
            }
            return dc_divider_node;
        }

        shared_ptr<DCDividerAttribute> KDSUtil::CopyFromKDSDA(shared_ptr<KDSDividerAttribute> kds_da,
                                                              shared_ptr<KDSDivider> kds_divider) {
            shared_ptr<DCDividerAttribute> dc_da = nullptr;
            if (kds_da != nullptr) {
                dc_da = make_shared<DCDividerAttribute>();
                dc_da->id_ = to_string(kds_da->ID);
                dc_da->virtual_ = kds_da->getPropertyLong(KDSDividerAttribute::VIRTUAL);
                dc_da->color_ = kds_da->getPropertyLong(KDSDividerAttribute::COLOR);
                dc_da->type_ = kds_da->getPropertyLong(KDSDividerAttribute::TYPE);
                dc_da->driveRule_ = kds_da->getPropertyLong(KDSDividerAttribute::DRIVE_RULE);
                dc_da->material_ = kds_da->getPropertyLong(KDSDividerAttribute::MATERIAL);
                dc_da->width_ = kds_da->getPropertyLong(KDSDividerAttribute::WIDTH);

                long divider_node_id = -1;
                for (const auto &role : kds_da->vecMemberAndRols) {
                    if (role.first == DIVIDER_NODE_ID) {
                        divider_node_id = role.second;
                    }
                }

                shared_ptr<DCDividerNode> dc_da_node = nullptr;
                if (divider_node_id != -1) {
                    for (const auto &kds_node : kds_divider->nodes) {
                        if (kds_node->ID == divider_node_id) {
                            dc_da_node = KDSUtil::CopyFromKDSDividerNode(kds_node);
                        }
                    }
                }


                if (dc_da_node) {
                    dc_da->dividerNode_ = dc_da_node;
                }
            }
            return dc_da;
        }

        bool KDSUtil::BuildDividerId2DAs(shared_ptr<ResourceManager> resource_manager,
                                         map<long, map<int, shared_ptr<kd::api::KDSDividerAttribute>>> &divId2Das,
                                         bool del_redundancy) {
            divId2Das.clear();

            vector<shared_ptr<KDSDividerAttribute>> del_das;
            const auto &mapKDSData = resource_manager->getKdsData(MODEL_NAME_DIVIDER_ATTRIBUTE);
            for (auto itr : mapKDSData) {
                shared_ptr<KDSDividerAttribute> ptrKDS_DA = std::static_pointer_cast<KDSDividerAttribute>(itr.second);

                long Divider_ID, DividerNode_ID;
                GetDividerAndNodeInfo(ptrKDS_DA.get(), Divider_ID, DividerNode_ID);

                int spidx = resource_manager->getSPIDXNormal(DividerNode_ID, Divider_ID);
                if (spidx == -1) {
                    del_das.emplace_back(ptrKDS_DA);
                    continue;
                }
                auto iter = divId2Das.find(Divider_ID);
                if (iter != divId2Das.end()) {
                    map<int, shared_ptr<KDSDividerAttribute>> &daSeq = iter->second;
                    daSeq.insert(make_pair(spidx, ptrKDS_DA));
                } else {
                    map<int, shared_ptr<KDSDividerAttribute>> daSeq;
                    daSeq[spidx] = ptrKDS_DA;
                    divId2Das[Divider_ID] = daSeq;
                }
            }

            //删除冗余da
            if (del_redundancy) {
                for (shared_ptr<KDSDividerAttribute> da : del_das) {
                    long taskId = da->getPropertyLong(da->TASKID);
                    long Divider_ID, DividerNode_ID;
                    GetDividerAndNodeInfo(da.get(), Divider_ID, DividerNode_ID);
                    LOG(ERROR) << "DA " << da->ID << "[TASK_ID:" << taskId << "] reference info [DIV_ID:" << Divider_ID
                               << ", NODE_ID:" << DividerNode_ID << "] error.";

                    KdsDataUtil::DelDividerAttribute(resource_manager, da->ID);
                }
            }

            return true;
        }

        void KDSUtil::GetDividerAndNodeInfo(const KDSRelation * relation, long & div_id, long & div_node_id){
            if(relation == nullptr){
                return;
            }

            for (const auto &role : relation->vecMemberAndRols) {
                if (role.first == DIVIDER_ID) {
                    div_id = role.second;
                } else if (role.first == DIVIDER_NODE_ID) {
                    div_node_id = role.second;
                }
            }
        }

        map<int, shared_ptr<kd::api::KDSDividerAttribute>>
        &KDSUtil::GetDividerDAs(long div_id, map<long, map<int, shared_ptr<kd::api::KDSDividerAttribute>>> &divider_da_maps_) {
            map<int, shared_ptr<kd::api::KDSDividerAttribute>> da_maps_;
            auto itemit = divider_da_maps_.find(div_id);
            if(itemit != divider_da_maps_.end()){
                return itemit->second;
            }

            return da_maps_;
        }
    }
}