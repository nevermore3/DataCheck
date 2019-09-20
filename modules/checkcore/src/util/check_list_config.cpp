#include "util/check_list_config.h"

#include <Poco/StringTokenizer.h>
#include <glog/logging.h>
#include "Poco/JSON/Parser.h"
#include "util/RequestUtil.h"
using namespace Poco;
using namespace Poco::JSON;
using namespace std;
using namespace kd::dc;
void CheckListConfig::Load(std::string fileName) {
    std::string file_content;

    std::fstream fp(fileName);
    std::string str;
    while (getline(fp, str)) {
        file_content += str;
    }
    fp.close();


    ParsseItemDesc(file_content);

    check_state_ = kd::dc::DataCheckConfig::getInstance().getPropertyI(kd::dc::DataCheckConfig::CHECK_STATE);

}

bool CheckListConfig::GetCheckList(std::string getItemUrl,string getDescUrl){
    string strJson;
    string descJson;
    CServiceRequestUtil requestUtil;
    CommonResult reqresult = requestUtil.HttpGetEx(getItemUrl, "", strJson, 60);
    if(reqresult.code == "0"){
        if( ParseCheckList(strJson)){
            reqresult = requestUtil.HttpGetEx(getDescUrl,"",descJson,60);
            if(reqresult.code == "0"){
                return ParsseItemDesc(descJson);
            }
        }
    }
    return false;
}
bool CheckListConfig::ParsseItemDesc(const string &json_result){
    try {
        // get inner
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var jsonResult = parser.parse(json_result);

        Poco::JSON::Object::Ptr obj;
        if (jsonResult.type() == typeid(Poco::JSON::Object::Ptr))
            obj = jsonResult.extract<Poco::JSON::Object::Ptr>();

        //判断返回值
//        string code = obj->getValue<string>("code");
//        if (strcmp(code.c_str(), "0") != 0) {
//            return false;
//        }
        if(obj ->has("checkItemList")) {
                Poco::JSON::Array::Ptr dataArray = obj->getArray("checkItemList");
                int totalCount = dataArray->size();

                for (long i = 0; i < totalCount; i++) {

                    Object::Ptr item = dataArray->getObject(i);
                    string code = item->get("code");
                    string desc = item->get("description");

                    map<string, string>::iterator it = check_map.find(code);
                    if (it != check_map.end()) {
                        check_map[code] = desc;
                    } else {
                        check_map.insert(make_pair(code, desc));
                    }
                }

        }
        LOG(INFO)<<"parse checkItemConfig success,item size is "<<check_map.size();
    } catch (Exception &e) {
        cout << e.what() << endl;
    }
    return true;
}
bool CheckListConfig::ParseCheckList(const std::string &json_result) {
    try {
        // get inner
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var jsonResult = parser.parse(json_result);

        Poco::JSON::Object::Ptr obj;
        if (jsonResult.type() == typeid(Poco::JSON::Object::Ptr))
            obj = jsonResult.extract<Poco::JSON::Object::Ptr>();

        //判断返回值
        std::string code = obj->getValue<std::string>("code");
        if (strcmp(code.c_str(), "0") != 0) {
            return false;
        }
        if(obj ->has("result")) {
            Object::Ptr result_obj = obj->getObject("result");

            auto dataArray = result_obj->getArray("checkItems");
            int array_size = dataArray->size();
            for (int i = 0; i < array_size; i++) {
                Dynamic::Var itemvalue = dataArray->get(i);
                string  key = itemvalue.toString();
                check_map.insert(make_pair(key, ""));
            }
        }
    } catch (Exception &e) {
        std::cout << e.what() << std::endl;
    }
    return true;
}

bool CheckListConfig::IsNeedCheck(std::string key){
    if (check_map.find(key) != check_map.end()) {
        return true;
    }

    if (kd::dc::DataCheckConfig::getInstance().getPropertyI(kd::dc::DataCheckConfig::CHECK_STATE) == kd::dc::DataCheckConfig::ALL_AUTO_CHECK) {
        return true;
    }

    return false;
}
string CheckListConfig::GetCheckItemDesc(string key){
    map<string, string>::iterator it = check_map.find(key);
    if (it != check_map.end()) {
        return check_map[key];
    }

    LOG(WARNING) << "not find " << key << " in check_map";

    return "";
}

#define DESC_NAME(NAME) NAME##_DESC

void CheckListConfig::CheckID2CheckDesc() {

    map<string, string> ID2Desc {
        //接边检查
        {CHECK_DIV_SPLIT_LOCATION_ERR, DESC_NAME(CHECK_DIV_SPLIT_LOCATION_ERR)},
        {CHECK_DIV_SPLIT_OVERLAP, DESC_NAME(CHECK_DIV_SPLIT_OVERLAP)},
        {CHECK_DIV_SPLIT_INTERSECT, DESC_NAME(CHECK_DIV_SPLIT_INTERSECT)},
        {CHECK_DIV_SPLIT_NO_CONTINUOUS_DIV, DESC_NAME(CHECK_DIV_SPLIT_NO_CONTINUOUS_DIV)},
        {CHECK_DIV_SPLIT_NO_SPLIT_ATTRIBUTE, DESC_NAME(CHECK_DIV_SPLIT_NO_SPLIT_ATTRIBUTE)},
        {CHECK_DIV_SPLIT_CONTINUOUS_Z_OUT_BUFFER, DESC_NAME(CHECK_DIV_SPLIT_CONTINUOUS_Z_OUT_BUFFER)},
        {CHECK_DIV_SPLIT_DIRECTION_ERR, DESC_NAME(CHECK_DIV_SPLIT_DIRECTION_ERR)},
        {CHECK_DIV_SPLIT_R_LINE_ERR, DESC_NAME(CHECK_DIV_SPLIT_R_LINE_ERR)},
        {CHECK_DIV_SPLIT_TOLLFLAG_ERR, DESC_NAME(CHECK_DIV_SPLIT_TOLLFLAG_ERR)},
        {CHECK_DIV_SPLIT_DA_VIRTUAL_ERR, DESC_NAME(CHECK_DIV_SPLIT_DA_VIRTUAL_ERR)},
        {CHECK_DIV_SPLIT_DA_COLOR_ERR, DESC_NAME(CHECK_DIV_SPLIT_DA_COLOR_ERR)},
        {CHECK_DIV_SPLIT_DA_TYPE_ERR, DESC_NAME(CHECK_DIV_SPLIT_DA_TYPE_ERR)},
        {CHECK_DIV_SPLIT_DA_DRIVE_RULE_ERR, DESC_NAME(CHECK_DIV_SPLIT_DA_DRIVE_RULE_ERR)},
        {CHECK_DIV_SPLIT_DA_OVERLAY_ERR, DESC_NAME(CHECK_DIV_SPLIT_DA_OVERLAY_ERR)},
        {CHECK_DIV_SPLIT_DA_MATERIAL_ERR, DESC_NAME(CHECK_DIV_SPLIT_DA_MATERIAL_ERR)},
        {CHECK_DIV_SPLIT_DA_WIDTH_ERR, DESC_NAME(CHECK_DIV_SPLIT_DA_WIDTH_ERR)},

        //源数据组织检查
        {CHECK_ITEM_KXS_ORG_001, DESC_NAME(CHECK_ITEM_KXS_ORG_001)},
        {CHECK_ITEM_KXS_ORG_002, DESC_NAME(CHECK_ITEM_KXS_ORG_002)},
        {CHECK_ITEM_KXS_ORG_003, DESC_NAME(CHECK_ITEM_KXS_ORG_003)},
        {CHECK_ITEM_KXS_ORG_004, DESC_NAME(CHECK_ITEM_KXS_ORG_004)},
        {CHECK_ITEM_KXS_ORG_005, DESC_NAME(CHECK_ITEM_KXS_ORG_005)},
        {CHECK_ITEM_KXS_ORG_006, DESC_NAME(CHECK_ITEM_KXS_ORG_006)},




    };
    ID2Desc.insert(make_pair(ID(1), DESC(1)));
    for (const auto &obj : ID2Desc) {
        check_map.insert(make_pair(obj.first, obj.second));
    }

}

