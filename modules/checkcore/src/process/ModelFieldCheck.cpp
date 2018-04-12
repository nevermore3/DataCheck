//
// Created by gaoyanhong on 2018/3/29.
//

#include <data/DataManager.h>
#include "process/ModelFieldCheck.h"

//thirdparty
#include "Poco/StringTokenizer.h"

namespace kd {
    namespace dc {


        string ModelFieldCheck::getId() {
            return id;
        }

        bool ModelFieldCheck::execute(shared_ptr<ModelDataManager> dataManager, shared_ptr<CheckErrorOutput> errorOutput) {

            for (auto taskit : dataManager->tasks_) {
                string strTaskName = taskit.first;

                //获取模型数据
                shared_ptr<DCModalData> modelData = nullptr;
                auto itdata = dataManager->modelDatas_.find(strTaskName);
                if (itdata != dataManager->modelDatas_.end()){
                    modelData = itdata->second;
                } else {
                    continue;
                };

                //获取模型配置
                shared_ptr<DCModelDefine> modelDefine = nullptr;
                auto itdef = dataManager->modelDefines_.find(strTaskName);
                if (itdef != dataManager->modelDefines_.end()){
                    modelDefine = itdef->second;
                } else {
                    continue;
                };

                //检查基础字段
                for (shared_ptr<DCFieldDefine> fieldDef : modelDefine->vecFieldDefines) {
                    if (fieldDef->valueLimit.length() == 0)
                        continue;

                    stringstream ss;
                    string fieldName = fieldDef->name;
                    switch (fieldDef->type) {
                        case DC_FIELD_TYPE_LONG:
                            checkLongValueIn(fieldDef, modelData, fieldName, errorOutput);
                            break;
                        case DC_FIELD_TYPE_DOUBLE:
                            checkDoubleValueIn(fieldDef, modelData, fieldName, errorOutput);
                            break;
                        case DC_FIELD_TYPE_VARCHAR:
                        case DC_FIELD_TYPE_TEXT:
                            checkStringValueIn(fieldDef, modelData, fieldName, errorOutput);
                            break;
                        default:
                            ss << "[Error] not support field type limit check.";
                            errorOutput->writeInfo(ss.str());
                            break;
                    }
                }
            }
            return true;
        }

        template <typename T>
        T getValue(const string& val, bool bopen = false, bool bleft = true){
            T valAccuracy, valT;
            if (typeid(T) == typeid(int)){
                valT = stoi(val.c_str());
                valAccuracy = bleft ? 1 : -1;
            } else if (typeid(T) == typeid(long)){
                valT = stol(val.c_str());
                valAccuracy = bleft ? 1 : -1;
            } else if (typeid(T) == typeid(float)){
                valT = stof(val.c_str());
                valAccuracy = bleft ? 0.0000001 : -0.0000001;
            } else if (typeid(T) == typeid(double)){
                valT = stod(val.c_str());
                valAccuracy = bleft ? 0.0000001 : -0.0000001;
            } else {
                return T();
            }

            if (bopen){
                valT += valAccuracy;
            }
            return valT;
        }

        template <typename T>
        bool getLimitRules(const string& strLimit, vector<T>& uniqueValues, map<T, T>& rangeValues){
            Poco::StringTokenizer st(strLimit, ",", Poco::StringTokenizer::Options::TOK_IGNORE_EMPTY|Poco::StringTokenizer::Options::TOK_IGNORE_EMPTY);
            for (auto val : st) {
                if (string::npos != val.find("~")){ //范围值
                    Poco::StringTokenizer rng(val, "~", Poco::StringTokenizer::Options::TOK_IGNORE_EMPTY|Poco::StringTokenizer::Options::TOK_IGNORE_EMPTY);
                    if (rng.count() != 2 || rng[0].length() < 2 || rng[1].length() < 2)
                        continue;

                    T valLeft = getValue<T>(rng[0].substr(1), (rng[0][0] == '(')?true:false, true);
                    T valRight = getValue<T>(rng[1].substr(0, rng[1].length()-1), (rng[1][rng[1].length()-1] == ')')?true:false, false);
                    rangeValues.insert(pair<T, T>(valLeft, valRight));
                } else { //唯一值
                    uniqueValues.emplace_back(getValue<T>(val));
                }
            }
            return true;
        }

        template <typename T>
        bool isInLimit(const T& value, const vector<T>& uniqueValues, const map<T, T>& rangeValues){
            if (typeid(T) == typeid(string)){
                return true;
                //TODO:
                if (uniqueValues.size() == 1){
                    T unqVal = uniqueValues[0];
                    stringstream ssunq;
                    ssunq << unqVal;
                    string strUnq = ssunq.str();

                    stringstream ss;
                    ss << value;
                    string strValue = ss.str();

                    for (auto ch : strValue){
                        if (strUnq.find(ch) == string::npos){
                            return false;
                        }
                    }
                }
                return true;
            }

            //判断值是否在取值列表内
            if (std::find(uniqueValues.begin(), uniqueValues.end(), value) != uniqueValues.end())
                return true;

            //判断值是否在取值范围内
            for (auto rngit : rangeValues){
                if (value >= rngit.first && value <= rngit.second){
                    return true;
                }
            }
            return false;
        }

        void ModelFieldCheck::checkDoubleValueIn(const shared_ptr<DCFieldDefine> fieldDef, const shared_ptr<DCModalData> modelData,
                               const string& fieldName, const shared_ptr<CheckErrorOutput> errorOutput) {
            vector<double> uniqueValues;
            map<double, double> rangeValues;
            string valueLimit = fieldDef->valueLimit;
            getLimitRules<double>(valueLimit, uniqueValues, rangeValues);

            for (shared_ptr<DCModelRecord> record : modelData->records) {

                auto valuepair = record->doubleDatas.find(fieldName);
                if (valuepair == record->doubleDatas.end()) {
                    stringstream ss;
                    ss << "[Error] not find field " << fieldName << " value.";
                    errorOutput->writeInfo(ss.str());
                    continue;
                }

                double recordValue = valuepair->second;
                if (!isInLimit<double>(recordValue, uniqueValues, rangeValues)){
                    stringstream ss;
                    ss << "[Error] checkDoubleValueIn : " << fieldName << "=" << recordValue << " not in '"<< valueLimit <<"'";
                    errorOutput->writeInfo(ss.str());
                }
            }
        }

        void ModelFieldCheck::checkLongValueIn(const shared_ptr<DCFieldDefine> fieldDef, const shared_ptr<DCModalData> modelData,
                                               const string& fieldName, const shared_ptr<CheckErrorOutput> errorOutput) {
            vector<long> uniqueValues;
            map<long, long> rangeValues;
            string valueLimit = fieldDef->valueLimit;
            getLimitRules<long>(valueLimit, uniqueValues, rangeValues);

            for (shared_ptr<DCModelRecord> record : modelData->records) {

                auto valuepair = record->longDatas.find(fieldName);
                if (valuepair == record->longDatas.end()) {
                    stringstream ss;
                    ss << "[Error] not find field " << fieldName << " value.";
                    errorOutput->writeInfo(ss.str());
                    continue;
                }

                long recordValue = valuepair->second;
                if (!isInLimit<long>(recordValue, uniqueValues, rangeValues)){
                    stringstream ss;
                    ss << "[Error] checkLongValueIn : " << fieldName << "=" << recordValue << " not in '"<< valueLimit <<"'";
                    errorOutput->writeInfo(ss.str());
                }
            }
        }

        void ModelFieldCheck::checkStringValueIn(const shared_ptr<DCFieldDefine> fieldDef, const shared_ptr<DCModalData> modelData,
                                               const string& fieldName, const shared_ptr<CheckErrorOutput> errorOutput) {
            vector<string> uniqueValues;
            map<string, string> rangeValues;
            string valueLimit = fieldDef->valueLimit;
            if (valueLimit.length() > 0){
                getLimitRules<string>(valueLimit, uniqueValues, rangeValues);
            }

            for (shared_ptr<DCModelRecord> record : modelData->records) {

                auto valuepair = record->textDatas.find(fieldName);
                if (valuepair == record->textDatas.end()) {
                    stringstream ss;
                    ss << "[Error] not find field " << fieldName << " value.";
                    errorOutput->writeInfo(ss.str());
                    continue;
                }

                auto recordValue = valuepair->second;
                int len = recordValue.length();
                //判断值是否非空
                if (fieldDef->inputLimit == 1 && len == 0){
                    stringstream ss;
                    ss << "[Error] checkStringValueIn : " << fieldName << " value should not null.";
                    errorOutput->writeInfo(ss.str());
                }

                //判断值是否超长
                if (fieldDef->len > 0 && len > fieldDef->len){
                    stringstream ss;
                    ss << "[Error] checkStringValueIn : " << fieldName << " len=" << len << " exceed '"<< fieldDef->len <<"'";
                    errorOutput->writeInfo(ss.str());
                }

                //判断值是否超限
                if (len > 0 && !isInLimit<string>(recordValue, uniqueValues, rangeValues)){
                    stringstream ss;
                    ss << "[Error] checkStringValueIn : " << fieldName << "=" << recordValue << " not in '"<< valueLimit <<"'";
                    errorOutput->writeInfo(ss.str());
                }
            }
        }
    }
}
