//
// Created by gaoyanhong on 2018/3/28.
//

#include "util/DCFieldCheckUtil.h"

//thirdparty
#include "Poco/StringTokenizer.h"

namespace kd {
    namespace dc {

        void DCFieldCheckUtil::checkLongFieldIdentify(shared_ptr<DCModalData> modelData, string fieldName){
            //TODO
            //multimap
        }

        void DCFieldCheckUtil::checkDoubleFieldIdentify(shared_ptr<DCModalData> modelData, string fieldName){
            //TODO
        }

        void DCFieldCheckUtil::checkStringFieldIdentify(shared_ptr<DCModalData> modelData, string fieldName){
            //TODO
        }

        void DCFieldCheckUtil::checkDoubleValueIn(string valueLimit, shared_ptr<DCModalData> modelData, string fieldName) {
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
                    cout << "[Error] not find field " << fieldName << " value. " << endl;
                    continue;
                }

                double recordValue = valuepair->second;

                auto limitpair = mapValueLimits.find(recordValue);
                if (limitpair == mapValueLimits.end()) {
                    cout << "[Error] checkLongValueIn : record value not permitted. " << recordValue << endl;
                }
            }
        }

        void DCFieldCheckUtil::checkLongValueIn(string valueLimit, shared_ptr<DCModalData> modelData, string fieldName) {

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
                    cout << "[Error] not find field " << fieldName << " value. " << endl;
                    continue;
                }

                long recordValue = valuepair->second;

                auto limitpair = mapValueLimits.find(recordValue);
                if (limitpair == mapValueLimits.end()) {
                    cout << "[Error] checkLongValueIn : record value not permitted. " << recordValue << endl;
                }
            }
        }
    }
}
