//
// Created by gaoyanhong on 2018/3/29.
//

#include "process/ModelBussCheck.h"

namespace kd {
    namespace dc {


        string ModelBussCheck::getId() {
            return id;
        }

        bool ModelBussCheck::execute(shared_ptr<DCModalData> modelData, shared_ptr<DCModelDefine> modelDefine) {

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

            return false;
        }

        void ModelBussCheck::checkFieldIdentify(shared_ptr<DCModalData> modelData, shared_ptr<DCFieldDefine> fieldDef) {
            switch (fieldDef->type) {
                case DC_FIELD_TYPE_LONG:
                    checkLongFieldIdentify(modelData, fieldDef->name);
                    break;
                case DC_FIELD_TYPE_DOUBLE:
                    checkDoubleFieldIdentify(modelData, fieldDef->name);
                    break;
                case DC_FIELD_TYPE_VARCHAR:
                case DC_FIELD_TYPE_TEXT:
                    checkStringFieldIdentify(modelData, fieldDef->name);
                    break;
                default:
                    cout << "[Error] checkFieldIdentify not support field type :" << fieldDef->type << endl;
                    break;
            }
        }


        void ModelBussCheck::checkLongFieldIdentify(shared_ptr<DCModalData> modelData, string fieldName){
            //TODO
            //multimap
        }

        void ModelBussCheck::checkDoubleFieldIdentify(shared_ptr<DCModalData> modelData, string fieldName){
            //TODO
        }

        void ModelBussCheck::checkStringFieldIdentify(shared_ptr<DCModalData> modelData, string fieldName){
            //TODO
        }

    }
}
