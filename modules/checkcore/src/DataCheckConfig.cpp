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
        const string DataCheckConfig::DA_SPACE_LEN = "da_space_len";

        const string DataCheckConfig::DIVIDER_EDGE_ANGLE = "divider_edge_angle";
        const string DataCheckConfig::DIVIDER_NODE_SPACE_LEN = "divider_node_space_len";
        const string DataCheckConfig::DIVIDER_HEIGHT_CHANGE_PER_METER = "divider_height_change_per_meter";

        const string DataCheckConfig::LA_SPACE_LEN = "la_space_len";
        const string DataCheckConfig::LANE_EDGE_MAX_ANGLE = "lane_edge_max_angle";
        const string DataCheckConfig::LANE_EDGE_MIN_ANGLE = "lane_edge_min_angle";
        const string DataCheckConfig::LANE_WIDTH_MAX = "lane_width_max";
        const string DataCheckConfig::LANE_WIDTH_MIN = "lane_width_min";

        const string DataCheckConfig::OBJECT_PL_BUFFER = "object_pl_buffer";



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
