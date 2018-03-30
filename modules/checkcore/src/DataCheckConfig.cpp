//
// Created by gaoyanhong on 2018/3/29.
//

#include "DataCheckConfig.h"

//thirdparty
#include <Poco/StringTokenizer.h>


namespace kd {
    namespace dc {

        const string DataCheckConfig::DOTLINE_DEFECT_LEN = "dotline_defect_len";
        const string DataCheckConfig::BUSELINE_DEFECT_LEN = "busline_defect_len";

        void DataCheckConfig::load(string fileName) {
            ifstream in(fileName.c_str());
            if (in.is_open()) {
                char buffer[256];
                while (!in.eof()) {
                    in.getline(buffer, 256);

                    //去掉空行或注释行
                    if (strlen(buffer) == 0 || buffer[0] == '#')
                        continue;

                    //解析数据
                    Poco::StringTokenizer st(buffer, ":");
                    if (st.count() >= 2) {
                        string key = st[0];
                        stringstream ss;
                        for(int i = 1; i < st.count() ; i ++){
                            ss << st[i];
                            if( i < st.count() - 1){
                                ss << ":";
                            }
                        }

                        m_properties.insert(pair<string, string>(key, ss.str()));
                    }
                }
            }
        }

        string DataCheckConfig::getProperty(string key) {
            map<string, string>::iterator it = m_properties.find(key);
            if (it != m_properties.end()) {
                return it->second;
            }

            return "";
        }

        double DataCheckConfig::getPropertyD(string key)
        {
            string value = getProperty(key);
            if(value.length() == 0 || value == "NULL"){
                return 0.0;
            }else{
                return ::stod(value);
            }
        }

        int DataCheckConfig::getPropertyI(string key)
        {
            string value = getProperty(key);
            if(value.length() == 0 || value == "NULL"){
                return 0;
            }else{
                return ::stoi(value);
            }
        }

    }
}
