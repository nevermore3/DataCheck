//
// Created by gaoyanhong on 2018/3/29.
//

#include "process/ModelBussCheck.h"

namespace kd {
    namespace dc {


        string ModelBussCheck::getId() {
            return id;
        }

        bool ModelBussCheck::execute(shared_ptr<DCModalData> modelData, shared_ptr<DCModelDefine> modelDefine, shared_ptr<CheckErrorOutput> errorOutput) {

            vector<shared_ptr<DCFieldCheckDefine>> vecFieldChecks;
            for (shared_ptr<DCFieldCheckDefine> check : modelDefine->vecFieldChecks) {

                shared_ptr<DCFieldDefine> fieldDefine = modelDefine->getFieldDefine(check->fieldName);
                if (fieldDefine == nullptr) {
                    stringstream ss;
                    ss << "[Error] field check oper not find field " << check->fieldName << " define \n";
                    errorOutput->writeInfo(ss.str());
                    continue;
                }

                switch (check->func) {
                    case DC_FIELD_VALUE_FUNC_ID:
                        checkFieldIdentify(modelData, fieldDefine, errorOutput);
                        break;
                    case DC_FIELD_VALUE_FUNC_GE:
                        break;
                    default:
                        errorOutput->writeInfo("[TODO] function need to implement.\n");
                        break;
                }
            }

            return false;
        }

        void ModelBussCheck::checkFieldIdentify(shared_ptr<DCModalData> modelData, shared_ptr<DCFieldDefine> fieldDef, shared_ptr<CheckErrorOutput> errorOutput) {
            switch (fieldDef->type) {
                case DC_FIELD_TYPE_LONG:
                    checkLongFieldIdentify(modelData, fieldDef->name, errorOutput);
                    break;
                case DC_FIELD_TYPE_DOUBLE:
                    checkDoubleFieldIdentify(modelData, fieldDef->name, errorOutput);
                    break;
                case DC_FIELD_TYPE_VARCHAR:
                case DC_FIELD_TYPE_TEXT:
                    checkStringFieldIdentify(modelData, fieldDef->name, errorOutput);
                    break;
                default:
                    stringstream ss;
                    ss << "[Error] checkFieldIdentify not support field type :" << fieldDef->type << "\n";
                    errorOutput->writeInfo(ss.str());
                    break;
            }
        }


        void ModelBussCheck::checkLongFieldIdentify(shared_ptr<DCModalData> modelData, string fieldName, shared_ptr<CheckErrorOutput> errorOutput){
            //TODO
            //multimap
            for (shared_ptr<DCModelRecord> record : modelData->records) {

                auto valuepair = record->longDatas.find(fieldName);
                if (valuepair == record->longDatas.end()) {
                    stringstream ss;
                    ss << "[Error] not find field " << fieldName << " value. \n";
                    errorOutput->writeInfo(ss.str());
                    continue;
                }

                multimapid.insert(make_pair(valuepair->second,valuepair->first));
                double recordValue = valuepair->second;
                auto idpair = multimapid.find(recordValue);
                if(idpair == multimapid.end()){
                    continue;
                }
                int num = multimapid.count(recordValue);
                if (num != 1) {
                    stringstream ss;
                    ss << "[Error] checkLongValueIn : ID value not unique. " << recordValue << ". \n";
                    errorOutput->writeInfo(ss.str());
                }
            }
        }

        void ModelBussCheck::checkDoubleFieldIdentify(shared_ptr<DCModalData> modelData, string fieldName, shared_ptr<CheckErrorOutput> errorOutput){
            //TODO
        }

        void ModelBussCheck::checkStringFieldIdentify(shared_ptr<DCModalData> modelData, string fieldName, shared_ptr<CheckErrorOutput> errorOutput){
            //TODO
        }

    }
}
