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

        bool CheckTaskInput::loadTaskInfo(string fileName, map<string,shared_ptr<DCTask>> &tasks) {

            try {
                std::filebuf in;
                if (!in.open(fileName, std::ios::in)) {
                    std::cout << "[Error] fail to open task file : " << fileName << std::endl;
                    return false;
                }

                Poco::JSON::Parser parser;
                std::istream iss(&in);
                Poco::Dynamic::Var result = parser.parse(iss);
                Poco::JSON::Object::Ptr rootobj;
                if (result.type() == typeid (Poco::JSON::Object::Ptr))
                    rootobj = result.extract<Poco::JSON::Object::Ptr>();

                //获得基本信息
                string taskName = getJSONString(rootobj, "taskName");
                string dataPath = getJSONString(rootobj, "dataPath");
                string modelPath = getJSONString(rootobj, "modelPath");

                dataPath = (dataPath.length()>0)?(dataPath + "/"):string("");
                modelPath = (modelPath.length()==0)?(DIR_CONFIG + string("model/")):modelPath;

                //获得任务信息
                if (!rootobj->has("tasks"))
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
                    dctask->name = modelName;
                    dctask->fileName = dataPath + fileName;
                    dctask->fileType = fileType;
                    dctask->modelName = modelPath + modelName;
                    tasks.insert(make_pair(modelName, dctask));
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

                //parse relation
                {
                    Poco::JSON::Array::Ptr relArray = rootobj->getArray("relation");
                    int relCount = relArray->size();
                    for( int i = 0 ; i < relCount ; i ++){
                        Dynamic::Var value = relArray->get(i);
                        Object::Ptr nodeObj = value.extract<Poco::JSON::Object::Ptr>();

                        shared_ptr<DCRelationDefine> reldef = make_shared<DCRelationDefine>();
                        reldef->member = getJSONString(nodeObj, "member");
                        reldef->rule = getJSONString(nodeObj, "rule");

                        modelDefine->vecRelation.emplace_back(reldef);
                    }
                }
                //parse table relation
                {
                    Poco::JSON::Array::Ptr relArray = rootobj->getArray("relationcheck");
                    int relCount = relArray->size();
                    for( int i = 0 ; i < relCount ; i ++){
                        Dynamic::Var value = relArray->get(i);
                        Object::Ptr nodeObj = value.extract<Poco::JSON::Object::Ptr>();

                        string model = getJSONString(nodeObj, "table");
                        string nodetype = getJSONString(nodeObj, "node_type");
                        string field = getJSONString(nodeObj, "field");

                        map<string,string> mapfiletable;
                        mapfiletable.insert(make_pair(model,field));
                        modelDefine->mapRelation.insert(make_pair(stol(nodetype),mapfiletable));
                    }
                }
            }catch (Exception &e) {
                cout<<"[Error] " << fileName.c_str() << ", info:" << e.what()<<endl;
                return false;
            }
            return true;
        }
    }
}
