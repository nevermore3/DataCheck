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
        {CHECK_ITEM_KXS_ORG_007, DESC_NAME(CHECK_ITEM_KXS_ORG_007)},
        {CHECK_ITEM_KXS_ORG_008, DESC_NAME(CHECK_ITEM_KXS_ORG_008)},
        {CHECK_ITEM_KXS_ORG_009, DESC_NAME(CHECK_ITEM_KXS_ORG_009)},
        {CHECK_ITEM_KXS_ORG_010, DESC_NAME(CHECK_ITEM_KXS_ORG_010)},
        {CHECK_ITEM_KXS_ORG_011, DESC_NAME(CHECK_ITEM_KXS_ORG_011)},
        {CHECK_ITEM_KXS_ORG_012, DESC_NAME(CHECK_ITEM_KXS_ORG_012)},
        {CHECK_ITEM_KXS_ORG_013, DESC_NAME(CHECK_ITEM_KXS_ORG_013)},
        {CHECK_ITEM_KXS_ORG_014, DESC_NAME(CHECK_ITEM_KXS_ORG_014)},
        {CHECK_ITEM_KXS_ORG_015, DESC_NAME(CHECK_ITEM_KXS_ORG_015)},
        {CHECK_ITEM_KXS_ORG_016, DESC_NAME(CHECK_ITEM_KXS_ORG_016)},
        {CHECK_ITEM_KXS_ORG_017, DESC_NAME(CHECK_ITEM_KXS_ORG_017)},
        {CHECK_ITEM_KXS_ORG_018, DESC_NAME(CHECK_ITEM_KXS_ORG_018)},
        {CHECK_ITEM_KXS_ORG_019, DESC_NAME(CHECK_ITEM_KXS_ORG_019)},
        {CHECK_ITEM_KXS_ORG_020, DESC_NAME(CHECK_ITEM_KXS_ORG_020)},
        {CHECK_ITEM_KXS_ORG_021, DESC_NAME(CHECK_ITEM_KXS_ORG_021)},
        {CHECK_ITEM_KXS_ORG_022, DESC_NAME(CHECK_ITEM_KXS_ORG_022)},
        {CHECK_ITEM_KXS_ORG_023, DESC_NAME(CHECK_ITEM_KXS_ORG_023)},
        {CHECK_ITEM_KXS_ORG_024, DESC_NAME(CHECK_ITEM_KXS_ORG_024)},
        {CHECK_ITEM_KXS_ORG_025, DESC_NAME(CHECK_ITEM_KXS_ORG_025)},
        {CHECK_ITEM_KXS_ORG_026, DESC_NAME(CHECK_ITEM_KXS_ORG_026)},
        {CHECK_ITEM_KXS_ORG_027, DESC_NAME(CHECK_ITEM_KXS_ORG_027)},
        {CHECK_ITEM_KXS_ORG_028, DESC_NAME(CHECK_ITEM_KXS_ORG_028)},
        {CHECK_ITEM_KXS_ORG_029, DESC_NAME(CHECK_ITEM_KXS_ORG_029)},
        {CHECK_ITEM_KXS_ORG_030, DESC_NAME(CHECK_ITEM_KXS_ORG_030)},
        {CHECK_ITEM_KXS_ORG_031, DESC_NAME(CHECK_ITEM_KXS_ORG_031)},
        {CHECK_ITEM_KXS_ORG_032, DESC_NAME(CHECK_ITEM_KXS_ORG_032)},
        {CHECK_ITEM_KXS_ORG_035, DESC_NAME(CHECK_ITEM_KXS_ORG_035)},

        //// 分组检查
        {CHECK_ITEM_KXS_LG_001, DESC_NAME(CHECK_ITEM_KXS_LG_001)},
        {CHECK_ITEM_KXS_LG_002, DESC_NAME(CHECK_ITEM_KXS_LG_002)},
        {CHECK_ITEM_KXS_LG_003, DESC_NAME(CHECK_ITEM_KXS_LG_003)},
        {CHECK_ITEM_KXS_LG_004, DESC_NAME(CHECK_ITEM_KXS_LG_004)},
        {CHECK_ITEM_KXS_LG_005, DESC_NAME(CHECK_ITEM_KXS_LG_005)},
        {CHECK_ITEM_KXS_LG_006, DESC_NAME(CHECK_ITEM_KXS_LG_006)},
        {CHECK_ITEM_KXS_LG_007, DESC_NAME(CHECK_ITEM_KXS_LG_007)},
        {CHECK_ITEM_KXS_LG_008, DESC_NAME(CHECK_ITEM_KXS_LG_008)},
        {CHECK_ITEM_KXS_LG_009, DESC_NAME(CHECK_ITEM_KXS_LG_009)},
        {CHECK_ITEM_KXS_LG_010, DESC_NAME(CHECK_ITEM_KXS_LG_010)},
        {CHECK_ITEM_KXS_LG_011, DESC_NAME(CHECK_ITEM_KXS_LG_011)},
        {CHECK_ITEM_KXS_LG_012, DESC_NAME(CHECK_ITEM_KXS_LG_012)},
        {CHECK_ITEM_KXS_LG_013, DESC_NAME(CHECK_ITEM_KXS_LG_013)},
        {CHECK_ITEM_KXS_LG_014, DESC_NAME(CHECK_ITEM_KXS_LG_014)},
        {CHECK_ITEM_KXS_LG_015, DESC_NAME(CHECK_ITEM_KXS_LG_015)},
        {CHECK_ITEM_KXS_LG_016, DESC_NAME(CHECK_ITEM_KXS_LG_016)},
        {CHECK_ITEM_KXS_LG_017, DESC_NAME(CHECK_ITEM_KXS_LG_017)},
        {CHECK_ITEM_KXS_LG_018, DESC_NAME(CHECK_ITEM_KXS_LG_018)},
        {CHECK_ITEM_KXS_LG_019, DESC_NAME(CHECK_ITEM_KXS_LG_019)},
        {CHECK_ITEM_KXS_LG_020, DESC_NAME(CHECK_ITEM_KXS_LG_020)},
        {CHECK_ITEM_KXS_LG_021, DESC_NAME(CHECK_ITEM_KXS_LG_021)},
        {CHECK_ITEM_KXS_LG_022, DESC_NAME(CHECK_ITEM_KXS_LG_022)},
        {CHECK_ITEM_KXS_LG_023, DESC_NAME(CHECK_ITEM_KXS_LG_023)},
        {CHECK_ITEM_KXS_LG_024, DESC_NAME(CHECK_ITEM_KXS_LG_024)},
        {CHECK_ITEM_KXS_LG_025, DESC_NAME(CHECK_ITEM_KXS_LG_025)},
        {CHECK_ITEM_KXS_LG_026, DESC_NAME(CHECK_ITEM_KXS_LG_026)},
        {CHECK_ITEM_KXS_LG_027, DESC_NAME(CHECK_ITEM_KXS_LG_027)},
        {CHECK_ITEM_KXS_LG_028, DESC_NAME(CHECK_ITEM_KXS_LG_028)},
        {CHECK_ITEM_KXS_LG_029, DESC_NAME(CHECK_ITEM_KXS_LG_028)},

        //道路检查
        {CHECK_ITEM_KXS_ROAD_001, DESC_NAME(CHECK_ITEM_KXS_ROAD_001)},
        {CHECK_ITEM_KXS_ROAD_002, DESC_NAME(CHECK_ITEM_KXS_ROAD_002)},
        {CHECK_ITEM_KXS_ROAD_003, DESC_NAME(CHECK_ITEM_KXS_ROAD_003)},
        {CHECK_ITEM_KXS_ROAD_004, DESC_NAME(CHECK_ITEM_KXS_ROAD_004)},
        {CHECK_ITEM_KXS_ROAD_005, DESC_NAME(CHECK_ITEM_KXS_ROAD_005)},
        {CHECK_ITEM_KXS_ROAD_006, DESC_NAME(CHECK_ITEM_KXS_ROAD_006)},
        {CHECK_ITEM_KXS_ROAD_007, DESC_NAME(CHECK_ITEM_KXS_ROAD_007)},
        {CHECK_ITEM_KXS_ROAD_008, DESC_NAME(CHECK_ITEM_KXS_ROAD_008)},
        {CHECK_ITEM_KXS_ROAD_009, DESC_NAME(CHECK_ITEM_KXS_ROAD_009)},
        {CHECK_ITEM_KXS_ROAD_010, DESC_NAME(CHECK_ITEM_KXS_ROAD_010)},

        //车道中心线检查
        {CHECK_ITEM_KXS_LANE_001, DESC_NAME(CHECK_ITEM_KXS_LANE_001)},
        {CHECK_ITEM_KXS_LANE_002, DESC_NAME(CHECK_ITEM_KXS_LANE_002)},
        {CHECK_ITEM_KXS_LANE_003, DESC_NAME(CHECK_ITEM_KXS_LANE_003)},
        {CHECK_ITEM_KXS_LANE_004, DESC_NAME(CHECK_ITEM_KXS_LANE_004)},
        {CHECK_ITEM_KXS_LANE_005, DESC_NAME(CHECK_ITEM_KXS_LANE_005)},
        {CHECK_ITEM_KXS_LANE_006, DESC_NAME(CHECK_ITEM_KXS_LANE_006)},
        {CHECK_ITEM_KXS_LANE_007, DESC_NAME(CHECK_ITEM_KXS_LANE_007)},
        {CHECK_ITEM_KXS_LANE_008, DESC_NAME(CHECK_ITEM_KXS_LANE_008)},
        {CHECK_ITEM_KXS_LANE_009, DESC_NAME(CHECK_ITEM_KXS_LANE_009)},
        {CHECK_ITEM_KXS_LANE_010, DESC_NAME(CHECK_ITEM_KXS_LANE_010)},
        {CHECK_ITEM_KXS_LANE_011, DESC_NAME(CHECK_ITEM_KXS_LANE_011)},
        {CHECK_ITEM_KXS_LANE_012, DESC_NAME(CHECK_ITEM_KXS_LANE_012)},
        {CHECK_ITEM_KXS_LANE_013, DESC_NAME(CHECK_ITEM_KXS_LANE_013)},
        {CHECK_ITEM_KXS_LANE_014, DESC_NAME(CHECK_ITEM_KXS_LANE_014)},
        {CHECK_ITEM_KXS_LANE_015, DESC_NAME(CHECK_ITEM_KXS_LANE_015)},
        {CHECK_ITEM_KXS_LANE_016, DESC_NAME(CHECK_ITEM_KXS_LANE_016)},
        {CHECK_ITEM_KXS_LANE_017, DESC_NAME(CHECK_ITEM_KXS_LANE_017)},
        {CHECK_ITEM_KXS_LANE_018, DESC_NAME(CHECK_ITEM_KXS_LANE_018)},
        {CHECK_ITEM_KXS_LANE_019, DESC_NAME(CHECK_ITEM_KXS_LANE_019)},
        {CHECK_ITEM_KXS_LANE_020, DESC_NAME(CHECK_ITEM_KXS_LANE_020)},
        {CHECK_ITEM_KXS_LANE_021, DESC_NAME(CHECK_ITEM_KXS_LANE_021)},
        {CHECK_ITEM_KXS_LANE_022, DESC_NAME(CHECK_ITEM_KXS_LANE_022)},
        {CHECK_ITEM_KXS_LANE_023, DESC_NAME(CHECK_ITEM_KXS_LANE_023)},

        //定位目标
        {CHECK_ITEM_KXS_LM_001, DESC_NAME(CHECK_ITEM_KXS_LM_001)},
        {CHECK_ITEM_KXS_LM_002, DESC_NAME(CHECK_ITEM_KXS_LM_002)},


        //ADAS
        {CHECK_ITEM_KXS_ADAS_001, DESC_NAME(CHECK_ITEM_KXS_ADAS_001)},
        {CHECK_ITEM_KXS_ADAS_002, DESC_NAME(CHECK_ITEM_KXS_ADAS_002)},
        {CHECK_ITEM_KXS_ADAS_003, DESC_NAME(CHECK_ITEM_KXS_ADAS_003)},
        {CHECK_ITEM_KXS_ADAS_004, DESC_NAME(CHECK_ITEM_KXS_ADAS_004)},
        {CHECK_ITEM_KXS_ADAS_005, DESC_NAME(CHECK_ITEM_KXS_ADAS_005)},
        {CHECK_ITEM_KXS_ADAS_006, DESC_NAME(CHECK_ITEM_KXS_ADAS_006)},
        {CHECK_ITEM_KXS_ADAS_007, DESC_NAME(CHECK_ITEM_KXS_ADAS_007)},
        {CHECK_ITEM_KXS_ADAS_008, DESC_NAME(CHECK_ITEM_KXS_ADAS_008)},

        //长度一致性检查
        {CHECK_ITEM_KXS_LENGTH_001, DESC_NAME(CHECK_ITEM_KXS_LENGTH_001)},
        //数目一致性检查
        {CHECK_ITEM_KXS_COUNT_001, DESC_NAME(CHECK_ITEM_KXS_COUNT_001)},

        //kxf规格检查
        {CHECK_ITEM_KXS_NORM_001, DESC_NAME(CHECK_ITEM_KXS_NORM_001)},
        {CHECK_ITEM_KXS_NORM_002, DESC_NAME(CHECK_ITEM_KXS_NORM_002)},
        {CHECK_ITEM_KXS_NORM_003, DESC_NAME(CHECK_ITEM_KXS_NORM_003)},

        {CHECK_ITEM_KXS_LINE_001,DESC_NAME(CHECK_ITEM_KXS_LINE_001)},
        {CHECK_ITEM_KXS_LINE_002,DESC_NAME(CHECK_ITEM_KXS_LINE_001)}
    };

    for (const auto &obj : ID2Desc) {
        check_map.insert(make_pair(obj.first, obj.second));
    }

}

