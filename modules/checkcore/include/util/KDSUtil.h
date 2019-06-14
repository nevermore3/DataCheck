//
// Created by zhangxingang on 19-6-13.
//

#ifndef AUTOHDMAP_DATACHECK_KDSUTIL_H
#define AUTOHDMAP_DATACHECK_KDSUTIL_H

#include "data/DataManager.h"
#include "ResourceManager.h"

namespace kd {
    namespace dc {
        class KDSUtil {
        public:
            static shared_ptr<DCDivider> CopyFromKDSDivider(shared_ptr<KDSDivider> kds_divider);

            static shared_ptr<DCDividerNode> CopyFromKDSDividerNode(shared_ptr<KDSNode> kds_divider_node);

            static shared_ptr<DCDividerAttribute> CopyFromKDSDA(shared_ptr<KDSDividerAttribute> kds_da);

            static bool BuildDividerId2DAs(shared_ptr<ResourceManager> resource_manager,
                                           map<long, map<int, shared_ptr<kd::api::KDSDividerAttribute>>> &divId2Das,
                                           bool del_redundancy = false);


            static void GetDividerAndNodeInfo(const KDSRelation * relation, long & div_id, long & div_node_id);

            static map<int, shared_ptr<kd::api::KDSDividerAttribute>>
            &GetDividerDAs(long div_id, map<long, map<int, shared_ptr<kd::api::KDSDividerAttribute>>> &divider_da_maps_);


        };
    }
}

#endif //AUTOHDMAP_DATACHECK_KDSUTIL_H
