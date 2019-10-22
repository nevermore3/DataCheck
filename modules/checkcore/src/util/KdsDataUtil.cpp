
#include <util/KdsDataUtil.h>

void KdsDataUtil::DelDividerAttribute(shared_ptr<ResourceManager> &resource_manager, long divatt_id) {
    RemoveKdsData(resource_manager, MODEL_NAME_DIVIDER_ATTRIBUTE, divatt_id);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//common operation public
void KdsDataUtil::RemoveKdsData(shared_ptr<ResourceManager> &resource_manager,
                                const string &modelName, const long &id) {

    map<string, map<long, shared_ptr<KDSData>>> &mapResource = resource_manager->getMapResource();

    map<string, map<long, shared_ptr<KDSData>>>::iterator itr1 = mapResource.find(modelName);
    if (itr1 != mapResource.end()) {
        map<long, shared_ptr<KDSData>> &kdsdataset = itr1->second;
        map<long, shared_ptr<KDSData>>::iterator itr2 = kdsdataset.find(id);
        if (itr2 != kdsdataset.end()) {
            kdsdataset.erase(itr2);
        }
    }
}

void KdsDataUtil::AppendObjectProperty(shared_ptr<ResourceManager> &resource_manager, string data_model,
                                       long objid, const map<string, string> &propValues) {
    shared_ptr<KDSData> kdsdata = resource_manager->getKdsData(data_model, objid);
    if(kdsdata){
        for(auto propit : propValues){
            kdsdata->setProperty(propit.first, propit.second);
        }
    }
}