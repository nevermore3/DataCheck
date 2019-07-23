#include "util/check_list_config.h"

#include <Poco/StringTokenizer.h>
#include <glog/logging.h>
#include "Poco/JSON/Parser.h"
#include "util/RequestUtil.h"
using namespace Poco;
using namespace Poco::JSON;
using namespace std;
void CheckListConfig::Load(std::string fileName) {
    std::string file_content;

    std::fstream fp(fileName);
    std::string str;
    while (getline(fp, str)) {
        file_content += str;
    }
    fp.close();

    ParseCheckList(file_content);
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
                ParsseItemDesc(descJson);
            }
            return true;
        }
    }else{
        return false;
    }
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
        string code = obj->getValue<string>("code");
        if (strcmp(code.c_str(), "0") != 0) {
            return false;
        }
        if(obj ->has("result")) {
            Poco::JSON::Array::Ptr dataArray = obj->getArray("result");

            int totalCount = dataArray->size();

            for (long i = 0; i < totalCount; i++) {

                Object::Ptr item = dataArray->getObject(i);
                string code = item->get("code");
                string desc = item->get("description");

                map<string, string>::iterator it = check_map.find(code);
                if (it != check_map.end()) {
                    check_map[code] =desc;
                } else {
                    check_map.insert(make_pair(code, desc));
                }
            }
        }
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

    return false;
}
string CheckListConfig::GetCheckItemDesc(string key){
    map<string, string>::iterator it = check_map.find(key);
    if (it != check_map.end()) {
        return check_map[key];
    }

    LOG(WARNING) << "not fine " << key << " in check_map";

    return "";
}