//
// Created by yuanjinfa on 2018/4/16.
//

#ifndef AUTOHDMAP_DATACHECK_MODELCHECK_H
#define AUTOHDMAP_DATACHECK_MODELCHECK_H

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <typeinfo>

using namespace std;

//thirdparty
#include "Poco/StringTokenizer.h"


namespace kd {
    namespace dc {

        template <typename T>
        T GetRangeValue(std::string val, bool bopen = false, bool bleft = true){
            T valAccuracy, valT;
            if (typeid(T) == typeid(int)){
                valT = atoi(val.c_str());
                valAccuracy = bleft ? 1 : -1;
            } else if (typeid(T) == typeid(long)){
                valT = atol(val.c_str());
                valAccuracy = bleft ? 1 : -1;
            } else if (typeid(T) == typeid(float)){
                valT = atof(val.c_str());
                valAccuracy = bleft ? 0.0000001 : -0.0000001;
            } else if (typeid(T) == typeid(double)){
                valT = atof(val.c_str());
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
        bool GetLimitRules(const string &strLimit, std::vector<T> &uniqueValues, std::map<T, T> &rangeValues){
            Poco::StringTokenizer st(strLimit, ",", Poco::StringTokenizer::Options::TOK_IGNORE_EMPTY|Poco::StringTokenizer::Options::TOK_IGNORE_EMPTY);
            for (auto val : st) {
                if (string::npos != val.find("~")){ //范围值
                    Poco::StringTokenizer rng(val, "~", Poco::StringTokenizer::Options::TOK_IGNORE_EMPTY|Poco::StringTokenizer::Options::TOK_IGNORE_EMPTY);
                    if (rng.count() != 2 || rng[0].length() < 2 || rng[1].length() < 2)
                        continue;

                    T valLeft = GetRangeValue<T>(rng[0].substr(1), (rng[0][0] == '(') ? true : false, true);
                    T valRight = GetRangeValue<T>(rng[1].substr(0, rng[1].length() - 1),
                                                  (rng[1][rng[1].length() - 1] == ')') ? true : false, false);
                    rangeValues.insert(pair<T, T>(valLeft, valRight));
                } else { //唯一值
                    uniqueValues.emplace_back(GetRangeValue<T>(val));
                }
            }
            return true;
        }

        template <typename T>
        bool IsValid(const T &value, const std::vector<T> &uniqueValues, const std::map<T, T> &rangeValues){
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

        template <typename T>
        bool IsValid(const string& rules, const T &value){

            std::vector<T> uniqueValues;
            std::map<T, T> rangeValues;
            GetLimitRules(rules, uniqueValues, rangeValues);

            return IsValid(value, uniqueValues, rangeValues);
        }

        template <typename T>
        T GetValue(const string& value){
            if (typeid(T) == typeid(long)){
                return atol(value.c_str());
            } else if (typeid(T) == typeid(double)) {
                return atof(value.c_str());
            } else if (typeid(T) == typeid(string)) {
                return string(value.c_str());
            } else {
                return T();
            }
        }
    }
}

#endif //AUTOHDMAP_DATACHECK_MODELCHECK_H
