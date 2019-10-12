//
// Created by gaoyanhong on 2018/3/28.
//

#include "data/MapDataModel.h"

namespace kd {
    namespace dc {
        shared_ptr<DCFieldDefine> DCModelDefine::getFieldDefine(string fieldName) {
            for(shared_ptr<DCFieldDefine> fieldDefine : vecFieldDefines ){

                if(fieldDefine != nullptr && fieldDefine->name == fieldName){
                    return fieldDefine;
                }
            }

            for(shared_ptr<DCFieldDefine> fieldDefine : vecMemberAndRols ){

                if(fieldDefine != nullptr && fieldDefine->name == fieldName){
                    return fieldDefine;
                }
            }
            return nullptr;
        }

        string KxsData::getProperty(const string &columnName, const string &defValue) {
            auto itr = kxf_data_map_.find(columnName);
            if (itr != kxf_data_map_.end() && itr->second != "NULL") {
                return itr->second;
            }
            return defValue;
        }
        long KxsData::getPropertyLong(const string &columnName, const string &defValue) {
            string value;
            try {
                auto itr = kxf_data_map_.find(columnName);
                if (itr != kxf_data_map_.end() && itr->second != "NULL") {
                    value = itr->second;
                    return itr->second.empty() ? 0 : stol(itr->second);
                }
                return defValue.empty() ? 0 : stol(defValue);
            } catch (std::invalid_argument &ex) {
                cout << "invalid_argument: " << ex.what() << "\n";
                cout << "getPropertyLong key:" << columnName << " value=" << value << "\n";
                return -1;
            } catch (std::exception &ex) {
                cout << "exception: " << ex.what() << "\n";
                return -1;
            }
        }
        double KxsData::getPropertyDouble(string columnName,const string &defValue) {
            auto itr = kxf_data_map_.find(columnName);
            if (itr != kxf_data_map_.end() && itr->second != "NULL") {
                return itr->second.empty() ? 0.0 : stod(itr->second);
            }
            return defValue.empty() ? 0.0 : stod(defValue);
        }
        void KxsData::addProperty(const string &columnName,const string &value){
            kxf_data_map_.insert(make_pair(columnName,value));
        }
    }
}
