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
                                     const shared_ptr<CheckErrorOutput> &error_output_, const string &base_path_,
                                     shared_ptr<ResourceManager> resource_manager)
                : DataInput(map_data_manager_, error_output_, base_path_), resource_manager_(resource_manager) {

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

                auto divider_iter = dividers.find(to_string(kds_divider->ID));
                if (divider_iter == dividers.end()) {
                    shared_ptr<DCDivider> dc_divider = KDSUtil::CopyFromKDSDivider(kds_divider);
                    if (dc_divider) {
                        dividers.emplace(dc_divider->id_, dc_divider);

                        // 构建fnode与divider关系
                        map_data_manager_->insert_fnode_id2_dividers(dc_divider->fromNodeId_, dc_divider);
                        map_data_manager_->insert_node_id2_dividers(dc_divider->fromNodeId_, dc_divider);

                        // 构建tnode与divider关系
                        map_data_manager_->insert_tnode_id2_dividers(dc_divider->toNodeId_, dc_divider);
                        map_data_manager_->insert_node_id2_dividers(dc_divider->toNodeId_, dc_divider);

                        auto da_maps = KDSUtil::GetDividerDAs(stol(dc_divider->id_), divider2_da_maps);
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
            return false;
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
    }
}