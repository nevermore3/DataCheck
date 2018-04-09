//
// Created by gaoyanhong on 2018/3/29.
//

#include "process/ModelFieldCheck.h"

//thirdparty
#include "Poco/StringTokenizer.h"

namespace kd {
    namespace dc {


        string ModelFieldCheck::getId() {
            return id;
        }

        bool ModelFieldCheck::execute(shared_ptr<DCModalData> modelData, shared_ptr<DCModelDefine> modelDefine, shared_ptr<CheckErrorOutput> errorOutput) {

            //检查基础字段
            for (shared_ptr<DCFieldDefine> fieldDef : modelDefine->vecFieldDefines) {
                if (fieldDef->valueLimit.length() == 0)
                    continue;

                stringstream ss;
                string fieldName = fieldDef->name;
                switch (fieldDef->type) {
                    case DC_FIELD_TYPE_LONG:
                        checkLongValueIn(fieldDef->valueLimit, modelData, fieldName, errorOutput);
                        break;
                    case DC_FIELD_TYPE_DOUBLE:
                        checkDoubleValueIn(fieldDef->valueLimit, modelData, fieldName, errorOutput);
                        break;
                    case DC_FIELD_TYPE_VARCHAR:
                    case DC_FIELD_TYPE_TEXT:
                        //TODO
                        ss << "[TODO] not support field type limit check .\n";
                        errorOutput->writeInfo(ss.str());
                        break;
                    default:
                        ss << "[Error] not support field type limit check .\n";
                        errorOutput->writeInfo(ss.str());
                        break;
                }
            }

            return true;
        }


        void ModelFieldCheck::checkDoubleValueIn(string valueLimit, shared_ptr<DCModalData> modelData, string fieldName, shared_ptr<CheckErrorOutput> errorOutput) {
            Poco::StringTokenizer st(valueLimit, ",");
            map<double, double> mapValueLimits;
            long limitCount = st.count();
            for (int i = 0; i < limitCount; i++) {
                double limitValue = stod(st[i]);
                mapValueLimits.insert(pair<double, double>(limitValue, limitValue));
            }

            for (shared_ptr<DCModelRecord> record : modelData->records) {

                auto valuepair = record->doubleDatas.find(fieldName);
                if (valuepair == record->doubleDatas.end()) {
                    stringstream ss;
                    ss << "[Error] not find field " << fieldName << " value. \n";
                    errorOutput->writeInfo(ss.str());
                    continue;
                }

                double recordValue = valuepair->second;

                auto limitpair = mapValueLimits.find(recordValue);
                if (limitpair == mapValueLimits.end()) {
                    stringstream ss;
                    ss << "[Error] checkLongValueIn : record value not permitted. " << recordValue << ". \n";
                    errorOutput->writeInfo(ss.str());
                }
            }
        }

        void ModelFieldCheck::checkLongValueIn(string valueLimit, shared_ptr<DCModalData> modelData, string fieldName, shared_ptr<CheckErrorOutput> errorOutput) {

            Poco::StringTokenizer st(valueLimit, ",");
            map<long, long> mapValueLimits;
            long limitCount = st.count();
            for (int i = 0; i < limitCount; i++) {
                long limitValue = stol(st[i]);
                mapValueLimits.insert(pair<long, long>(limitValue, limitValue));
            }

            for (shared_ptr<DCModelRecord> record : modelData->records) {

                auto valuepair = record->longDatas.find(fieldName);
                if (valuepair == record->longDatas.end()) {
                    stringstream ss;
                    ss << "[Error] not find field " << fieldName << " value. \n";
                    errorOutput->writeInfo(ss.str());
                    continue;
                }

                long recordValue = valuepair->second;

                auto limitpair = mapValueLimits.find(recordValue);
                if (limitpair == mapValueLimits.end()) {
                    stringstream ss;
                    ss << "[Error] checkLongValueIn : record value not permitted. " << recordValue << ". \n";
                    errorOutput->writeInfo(ss.str());
                }
            }
        }

    }
}
