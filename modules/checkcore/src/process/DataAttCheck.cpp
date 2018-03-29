//
// Created by gaoyanhong on 2018/3/28.
//


#include "process/DataAttCheck.h"

//thirdparty
#include <Poco/StringTokenizer.h>

using namespace Poco;

//module
#include "storage/CheckTaskInput.h"
#include "storage/ModelDataInput.h"

#include "util/DCFieldCheckUtil.h"


namespace kd {
    namespace dc {

        DataAttCheck::DataAttCheck(string basePath, string taskName) {
            basePath_ = basePath;
            taskName_ = taskName;
        }

        string DataAttCheck::getId() {
            return id;
        }

        bool DataAttCheck::execute() {
            CheckTaskInput taskInput;

            vector<shared_ptr<DCTask>> tasks;
            string taskPath = basePath_ + "/" + taskName_;
            taskInput.loadTaskInfo(taskPath, tasks);
            cout << "[Debug] task size is " << tasks.size() << endl;

            for (shared_ptr<DCTask> task : tasks) {

                string modelPath = basePath_ + "/" + task->modelName + ".json";

                shared_ptr<DCModelDefine> modelDefine = make_shared<DCModelDefine>();

                if (taskInput.loadTaskModel(modelPath, modelDefine)) {

                    string dataFilePath = basePath_ + "/" + task->fileName;

                    shared_ptr<DCModalData> modelData = make_shared<DCModalData>();
                    ModelDataInput dataInput;
                    if (task->fileType == "point") {
                        dataInput.loadPointFile(dataFilePath, modelDefine->vecFieldDefines, modelData);
                    } else if (task->fileType == "arc") {
                        dataInput.loadArcFile(dataFilePath, modelDefine->vecFieldDefines, modelData);
                    } else if (task->fileType == "dbf") {
                        dataInput.loadDBFFile(dataFilePath, modelDefine->vecFieldDefines, modelData);
                    } else {
                        cout << "[Error] data file type error " << task->fileType << endl;
                    }

                    cout << "[Debug] load data count " << modelData->records.size() << endl;

                    check(modelData, modelDefine);
                }
            }

            return true;
        }





        void checkFieldIdentify(shared_ptr<DCModalData> modelData, shared_ptr<DCFieldDefine> fieldDef) {
            switch (fieldDef->type) {
                case DC_FIELD_TYPE_LONG:
                    DCFieldCheckUtil::checkLongFieldIdentify(modelData, fieldDef->name);
                    break;
                case DC_FIELD_TYPE_DOUBLE:
                    DCFieldCheckUtil::checkDoubleFieldIdentify(modelData, fieldDef->name);
                    break;
                case DC_FIELD_TYPE_VARCHAR:
                case DC_FIELD_TYPE_TEXT:
                    DCFieldCheckUtil::checkStringFieldIdentify(modelData, fieldDef->name);
                    break;
                default:
                    cout << "[Error] checkFieldIdentify not support field type :" << fieldDef->type << endl;
                    break;
            }
        }


        void DataAttCheck::check(shared_ptr<DCModalData> modelData, shared_ptr<DCModelDefine> modelDefine) {

            //检查基础字段
            for (shared_ptr<DCFieldDefine> fieldDef : modelDefine->vecFieldDefines) {
                if (fieldDef->valueLimit.length() == 0)
                    continue;

                string fieldName = fieldDef->name;
                switch (fieldDef->type) {
                    case DC_FIELD_TYPE_LONG:
                        DCFieldCheckUtil::checkLongValueIn(fieldDef->valueLimit, modelData, fieldName);
                        break;
                    case DC_FIELD_TYPE_DOUBLE:
                        DCFieldCheckUtil::checkDoubleValueIn(fieldDef->valueLimit, modelData, fieldName);
                        break;
                    case DC_FIELD_TYPE_VARCHAR:
                    case DC_FIELD_TYPE_TEXT:
                        //TODO
                        cout << "[TODO] not support field type limit check ." << endl;
                        break;
                    default:
                        cout << "[Error] not support field type limit check ." << endl;
                        break;
                }
            }

            //检查属性
            vector<shared_ptr<DCFieldCheckDefine>> vecFieldChecks;
            for (shared_ptr<DCFieldCheckDefine> check : modelDefine->vecFieldChecks) {

                shared_ptr<DCFieldDefine> fieldDefine = modelDefine->getFieldDefine(check->fieldName);
                if (fieldDefine == nullptr) {
                    cout << "[Error] field check oper not find field " << check->fieldName << " define " << endl;
                    continue;
                }

                switch (check->func) {
                    case DC_FIELD_VALUE_FUNC_ID:
                        checkFieldIdentify(modelData, fieldDefine);
                        break;
                    case DC_FIELD_VALUE_FUNC_GE:
                        break;
                    default:
                        cout << "[TODO] function need to implement." << endl;
                        break;
                }
            }
        }

    }
}
