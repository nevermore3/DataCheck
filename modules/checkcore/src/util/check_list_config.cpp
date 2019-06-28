#include "util/check_list_config.h"

#include <Poco/StringTokenizer.h>
#include <glog/logging.h>
#include "Poco/JSON/Parser.h"

using namespace Poco;
using namespace Poco::JSON;

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

void CheckListConfig::ParseCheckList(const std::string &json_result) {
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
//            return;
//        }
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
}

bool CheckListConfig::IsNeedCheck(std::string key){
    if (check_list_.find(key) != check_list_.end()) {
        return true;
    }

    return false;
}
