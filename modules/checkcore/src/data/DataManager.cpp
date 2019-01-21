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
   }
}