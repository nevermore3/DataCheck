//
// Created by gaoyanhong on 2018/3/20.
//

#include "util/KDConvertUtil.hpp"
namespace kd {
   namespace dc {


        int KDConvertUtil::string2int(string value, int defInt){
            if(value.length() == 0 || value == "NULL") {
                return defInt;
            }
            else {
                return stoi(value);
            }
        }

        double KDConvertUtil::string2double(string value, int defDouble){
            if(value.length() == 0 || value == "NULL") {
                return defDouble;
            }
            else {
                return stod(value);
            }
        }
    }
}