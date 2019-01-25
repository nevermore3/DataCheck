//

#include <data/DataManager.h>

#include "data/DataManager.h"

namespace kd {
   namespace dc {

       shared_ptr<DCModalData> ModelDataManager::getModelData(string modelName) {
           auto itr = modelDatas_.find(modelName);
           if (itr != modelDatas_.end()) {
               return itr->second;
           }
           return nullptr;
       }

       shared_ptr<DCModelDefine> ModelDataManager::getModelDefine(string modelName) {
           auto itr = modelDefines_.find(modelName);
           if (itr != modelDefines_.end()) {
               return itr->second;
           }
           return nullptr;
       }

       void MapDataManager::insert_divider2_lane_groups(string divider, string lane_group_id) {
           auto div2_lg_iter = divider2_lane_groups_.find(divider);
           if (div2_lg_iter != divider2_lane_groups_.end()) {
               div2_lg_iter->second.insert(lane_group_id);
           } else {
               set<string> lane_groups_set;
               lane_groups_set.insert(lane_group_id);
               divider2_lane_groups_.insert(make_pair(divider, lane_groups_set));
           }
       }

       void MapDataManager::insert_fnode_id2_dividers(string fnode_id, shared_ptr<DCDivider> ptr_divider) {
           auto fnode_iter = fnode_id2_dividers_maps_.find(fnode_id);
           if (fnode_iter != fnode_id2_dividers_maps_.end()) {
               fnode_iter->second.emplace_back(ptr_divider);
           } else {
               vector<shared_ptr<DCDivider>> vec_dc_dividers;
               vec_dc_dividers.emplace_back(ptr_divider);
               fnode_id2_dividers_maps_.insert(make_pair(fnode_id, vec_dc_dividers));
           }
       }

       void MapDataManager::insert_tnode_id2_dividers(string tnode_id, shared_ptr<DCDivider> ptr_divider) {
           auto tnode_iter = tnode_id2_dividers_maps_.find(tnode_id);
           if (tnode_iter != tnode_id2_dividers_maps_.end()) {
               tnode_iter->second.emplace_back(ptr_divider);
           } else {
               vector<shared_ptr<DCDivider>> vec_dc_dividers;
               vec_dc_dividers.emplace_back(ptr_divider);
               tnode_id2_dividers_maps_.insert(make_pair(tnode_id, vec_dc_dividers));
           }
       }

       void MapDataManager::insert_node_id2_dividers(string tnode_id, shared_ptr<DCDivider> ptr_divider) {
           auto node_iter = node_id2_dividers_maps_.find(tnode_id);
           if (node_iter != node_id2_dividers_maps_.end()) {
               node_iter->second.emplace_back(ptr_divider);
           } else {
               vector<shared_ptr<DCDivider>> vec_dc_dividers;
               vec_dc_dividers.emplace_back(ptr_divider);
               node_id2_dividers_maps_.insert(make_pair(tnode_id, vec_dc_dividers));
           }
       }

       void MapDataManager::insert_lane_group2_roads(string lane_group_id, string road_id) {
            auto lane_group_iter = lane_group2_roads_maps_.find(lane_group_id);
            if (lane_group_iter != lane_group2_roads_maps_.end()) {
                lane_group_iter->second.emplace_back(road_id);
            } else {
                vector<string> road_vec;
                road_vec.emplace_back(road_id);
                lane_group2_roads_maps_.insert(make_pair(lane_group_id, road_vec));
            }
       }
   }
}