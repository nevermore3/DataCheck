//
// Created by zhangxingang on 19-6-14.
//

#ifndef AUTOHDMAP_DATACHECK_KDSDATAUTIL_H
#define AUTOHDMAP_DATACHECK_KDSDATAUTIL_H
#include <api/KDSServiceModel.h>
#include "ResourceManager.h"
using namespace kd::api;

class ResourceManager;

class KdsDataUtil {
public:

    static void DelDividerAttribute(shared_ptr<ResourceManager> &resource_manager, long divatt_id);

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    //common operation public
    static void RemoveKdsData(shared_ptr<ResourceManager> &resource_manager, const string &modelName, const long &id);

    //向某个对象增加属性
    static void AppendObjectProperty(shared_ptr<ResourceManager> &resource_manager, string data_model,
                                     long objid, const map<string,string> & propValues);

};


#endif //AUTOHDMAP_DATACHECK_KDSDATAUTIL_H
