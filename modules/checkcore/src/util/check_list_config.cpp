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

bool CheckListConfig::GetCheckList(std::string url){
    string strJson;
    CServiceRequestUtil requestUtil;
    CommonResult reqresult = requestUtil.HttpGetEx(url, "", strJson, 60);
    if(reqresult.code == "0"){
        return ParseCheckList(strJson);
    }else{
        return false;
    }
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
                check_list_.emplace(itemvalue.toString());
            }
        }
    } catch (Exception &e) {
        std::cout << e.what() << std::endl;
    }
    return true;
}

bool CheckListConfig::IsNeedCheck(std::string key){
    if (check_list_.find(key) != check_list_.end()) {
        return true;
    }

    return false;
}
