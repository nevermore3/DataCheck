

#ifndef AUTOHDMAP_DATACHECK_KDSUTIL_H
#define AUTOHDMAP_DATACHECK_KDSUTIL_H

#include <storage/CheckErrorOutput.h>
#include "data/DataManager.h"
#include "ResourceManager.h"
#include <Poco/JSON/Parser.h>
using namespace Poco;
using namespace Poco::JSON;
namespace kd {
    namespace dc {
        class KDSUtil {
        public:
            static shared_ptr<DCDivider> CopyFromKDSDivider(shared_ptr<KDSDivider> kds_divider,
                                                            shared_ptr<CheckErrorOutput> error_output);

            static shared_ptr<DCDividerNode> CopyFromKDSDividerNode(shared_ptr<KDSNode> kds_divider_node);

            static shared_ptr<DCDividerAttribute> CopyFromKDSDA(shared_ptr<KDSDividerAttribute> kds_da,
                                                                shared_ptr<KDSDivider> kds_divider);

            static bool BuildDividerId2DAs(shared_ptr<ResourceManager> resource_manager,
                                           map<long, map<int, shared_ptr<kd::api::KDSDividerAttribute>>> &divId2Das,
                                           bool del_redundancy = false);


            static void GetDividerAndNodeInfo(const KDSRelation * relation, long & div_id, long & div_node_id);

            static map<int, shared_ptr<kd::api::KDSDividerAttribute>>
            &GetDividerDAs(long div_id, map<long, map<int, shared_ptr<kd::api::KDSDividerAttribute>>> &divider_da_maps_);
            /**
             * 根据data_id查找源数据
             * @param file_type lane,ground,pole,sign
             * @param taskId 任务号
             * @param data_type node,relation,way
             * @param data_id 数据ID
             * @param entity 查询结果
             */
            static void getResourceData(const string & file_type,const string &taskId,const string &data_type,const string & data_id,Object::Ptr & entity);

        private:
            static bool getEntityData(const string &inputJson, const string &data_type,const string data_id ,Object::Ptr & entity);


        };
    }
}

#endif //AUTOHDMAP_DATACHECK_KDSUTIL_H
