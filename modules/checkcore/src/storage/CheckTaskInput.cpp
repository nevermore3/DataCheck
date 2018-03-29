//
// Created by gaoyanhong on 2018/3/28.
//

#include "storage/CheckTaskInput.h"

#include "Poco/JSON/Parser.h"
using namespace Poco;
using namespace Poco::JSON;

namespace kd {
    namespace dc {

        string getJSONString(Poco::JSON::Object::Ptr obj, string key, string defValue = ""){
            string value;
            if(obj->has(key)){
                value = obj->getValue<std::string>(key);
            }else{
                value = defValue;
            }
            return value;
        }

        double getJSONDouble(Poco::JSON::Object::Ptr obj, string key, double defValue = 0.0){
            double value;
            if(obj->has(key)){
                value = obj->getValue<double>(key);
            }else{
                value = defValue;
            }
            return value;
        }

        long getJSONLong(Poco::JSON::Object::Ptr obj, string key, long defValue = 0){
            long value;
            if(obj->has(key)){
                value = obj->getValue<long>(key);
            }else{
                value = defValue;
            }
            return value;
        }

        bool CheckTaskInput::loadTaskInfo(string fileName, vector <shared_ptr<DCTask>> &tasks) {

            try {
                std::filebuf in;
                if (!in.open(fileName, std::ios::in)) {
                    std::cout << "[Error] fail to open task file : " << fileName << std::endl;
                    return false;
                }

                std::istream iss(&in);

                Poco::JSON::Parser parser;

                Poco::Dynamic::Var result = parser.parse(iss);
                Poco::JSON::Object::Ptr rootobj;
                if (result.type() == typeid (Poco::JSON::Object::Ptr))
                    rootobj = result.extract<Poco::JSON::Object::Ptr>();

                string taskName, dataPath, modelPath;

                //获得基本信息
                taskName = getJSONString(rootobj, "taskNmae");
                dataPath = getJSONString(rootobj, "dataPath");
                modelPath = getJSONString(rootobj, "modelPath");

                //获得任务信息
                if(!rootobj->has("tasks"))
                    return false;

                Poco::JSON::Array::Ptr taskArray = rootobj->getArray("tasks");
                int totalCount = taskArray->size();
                for( int i = 0 ; i < totalCount ; i ++){
                    Dynamic::Var value = taskArray->get(i);
                    Object::Ptr nodeObj = value.extract<Poco::JSON::Object::Ptr>();

                    string fileName = getJSONString(nodeObj, "fileName");
                    string fileType = getJSONString(nodeObj, "fileType");
                    string modelName = getJSONString(nodeObj, "modelName");

                    shared_ptr<DCTask> dctask = make_shared<DCTask>();
                    dctask->fileName = dataPath + "/" + fileName;
                    dctask->fileType = fileType;
                    dctask->modelName = modelPath + "/" + modelName;

                    tasks.emplace_back(dctask);
                }

            }catch (Exception &e) {
                cout<<"[Error] " << e.what()<<endl;
                return false;
            }
            return true;
        }

        bool CheckTaskInput::loadTaskModel(string fileName, shared_ptr<DCModelDefine> modelDefine){
            try {
                std::filebuf in;
                if (!in.open(fileName, std::ios::in)) {
                    cout << "[Error] fail to open model file : " << fileName << endl;
                    return false;
                }

                std::istream iss(&in);

                Poco::JSON::Parser parser;

                Poco::Dynamic::Var result = parser.parse(iss);
                Poco::JSON::Object::Ptr rootobj;
                if (result.type() == typeid (Poco::JSON::Object::Ptr))
                    rootobj = result.extract<Poco::JSON::Object::Ptr>();

                //获得基本信息
                modelDefine->modelName = getJSONString(rootobj, "model");
                cout << "[Debug] load model : " << modelDefine->modelName << endl;

                //获得任务信息
                if(!rootobj->has("fields") || !rootobj->has("checks"))
                    return false;

                //解析字段定义信息
                {
                    Poco::JSON::Array::Ptr taskArray = rootobj->getArray("fields");
                    int totalCount = taskArray->size();
                    for( int i = 0 ; i < totalCount ; i ++){
                        Dynamic::Var value = taskArray->get(i);
                        Object::Ptr nodeObj = value.extract<Poco::JSON::Object::Ptr>();

                        shared_ptr<DCFieldDefine> fieldDefine = make_shared<DCFieldDefine>();
                        fieldDefine->name = getJSONString(nodeObj, "name");
                        fieldDefine->type = (DCFieldType)getJSONLong(nodeObj, "type");
                        fieldDefine->len = getJSONLong(nodeObj, "lenLimit");
                        fieldDefine->inputType = getJSONString(nodeObj, "inputType");
                        fieldDefine->defValue = getJSONString(nodeObj, "defaultValue");
                        fieldDefine->inputLimit = getJSONLong(nodeObj, "inputLimit");
                        fieldDefine->valueLimit = getJSONString(nodeObj, "valueLimit");

                        modelDefine->vecFieldDefines.emplace_back(fieldDefine);
                    }
                }

                //解析字段特殊检查项
                {
                    Poco::JSON::Array::Ptr checkArray = rootobj->getArray("checks");
                    int checkCount = checkArray->size();
                    for( int i = 0 ; i < checkCount ; i ++){
                        Dynamic::Var value = checkArray->get(i);
                        Object::Ptr nodeObj = value.extract<Poco::JSON::Object::Ptr>();

                        shared_ptr<DCFieldCheckDefine> fieldCheck = make_shared<DCFieldCheckDefine>();
                        fieldCheck->fieldName = getJSONString(nodeObj, "name");
                        string func = getJSONString(nodeObj, "func");
                        if(func == "ID"){
                            fieldCheck->func = DC_FIELD_VALUE_FUNC_ID;
                        }else if(func == "GE"){
                            fieldCheck->func = DC_FIELD_VALUE_FUNC_GE;
                        }else{
                            cout << "[Error] not support operation " << func << endl;
                            continue;
                        }

                        fieldCheck->refValue = getJSONString(nodeObj, "refValue");

                        modelDefine->vecFieldChecks.emplace_back(fieldCheck);
                    }
                }

            }catch (Exception &e) {
                cout<<"[Error] " << e.what()<<endl;
                return false;
            }
            return true;
        }
    }
}
