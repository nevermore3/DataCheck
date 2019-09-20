//
// Created by ubuntu on 9/19/19.
//
#include "CommonDefine.h"
#include "CommonInclude.h"

namespace kd {
    namespace dc {

        string MapItemInfo::getProperty(string checkID) {
            map<string, string>::iterator it = map_item_info_.find(checkID);
            if (it != map_item_info_.end()) {
                return it->second;
            }
            return "描述不存在";
        }

        size_t MapItemInfo::getNums() {
            return map_item_info_.size();
        }


    }
}
