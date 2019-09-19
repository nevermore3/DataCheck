//
// Created by gaoyanhong on 2018/3/29.
//

#include "DataCheckConfig.h"

//thirdparty
#include <Poco/StringTokenizer.h>
#include <DataCheckConfig.h>


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
        const string DataCheckConfig::LANE_OVERLAP_AREA = "lane_overlap_area";
        const string DataCheckConfig::LANE_DIVIDER_DIRANGLE = "lane_divider_dir_angle";
        const string DataCheckConfig::LANE_NODE_ANGLE = "lane_node_angle";
        const string DataCheckConfig::LANE_ANGLE = "lane_angle";
        const string DataCheckConfig::LANE_CURVATURE = "lane_curvature";

        const string DataCheckConfig::OBJECT_PL_BUFFER = "object_pl_buffer";

        const string DataCheckConfig::CONFIG_FILE_PATH = "config_file_path";
        const string DataCheckConfig::MODEL_FILE_PATH = "model_file_path";
        const string DataCheckConfig::TASK_FILE = "task_file";
        const string DataCheckConfig::SQL_TASK_FILE = "sql_task_file";
        const string DataCheckConfig::UPDATE_REGION = "update_region";

        const string DataCheckConfig::DIVIDER_LENGTH_RATIO = "divider_length_ratio";
        const string DataCheckConfig::LANE_INTERSECT_LENGTH = "lane_intersect_length";
        const string DataCheckConfig::ADAS_NODE_DISTANCE = "adas_node_distance";
        const string DataCheckConfig::ADAS_NODE_DISTANCE_ACCURACY = "adas_node_distance_accuracy";
        const string DataCheckConfig::ADAS_NODE_CURVATURE_DISTANCE = "adas_node_curvature_distance";
        const string DataCheckConfig::ADAS_NODE_MAX_CURVATURE = "adas_node_max_curvature";
        const string DataCheckConfig::ADAS_NODE_MAX_SLOPE = "adas_node_max_slope";
        const string DataCheckConfig::ROAD_NODE_ANGLE = "road_node_angle";
        const string DataCheckConfig::ROAD_NODE_DISTANCE = "road_node_distance";

        const string DataCheckConfig::JSON_DATA_INPUT = "json_data_input";

        const string DataCheckConfig::RESOURCE_FILE_PATH = "resource_file_path";
        const string DataCheckConfig::RESOURCE_FILE = "resource_file";

        const string DataCheckConfig::ERR_JSON_PATH = "err_json_path";
        const string DataCheckConfig::OUTPUT_PATH = "output_path";
        const string DataCheckConfig::CHECK_FILE_PATH ="check_file_path";

        const string DataCheckConfig::SHP_FILE_PATH ="shp_data_input";
        const string DataCheckConfig::DB_INPUT_FILE ="db_input_file";

        const string DataCheckConfig::CHECK_STATE ="check_state";

        //数据一致性检查
        const string DataCheckConfig::LENGTH_CHECK_FILE = "length_check_file";
        const string DataCheckConfig::COUNT_CHECK_FILE = "count_check_file";
        const string DataCheckConfig::LENGTH_PRECISE = "length_precise";

        ///接边检查
        const string DataCheckConfig::CONN_INPUTPATH = "conn_inputPath";
        const string DataCheckConfig::CONN_OUTPATH = "conn_outputPath";
        const string DataCheckConfig::CONN_TASKBOUNDPATH="conn_taskBoundPath";
        const string DataCheckConfig::CONN_CHECKFILEPATH="conn_checkFilePath";

        // 坡度检查
        const string DataCheckConfig::AVG_SLOPE_ERROR = "avg_slope_error";
        const string DataCheckConfig::ADAS_NODE_DIVIDER_SLOPE = "adas_node_divider_slope";
        const string DataCheckConfig::CHECK_ALL_FILE = "check_all_file";

        int DataCheckConfig::load(string fileName) {
            int ret = 0;
            try {
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
                } else {
                    LOG(ERROR) << "读取配置文件错误,查看当前路径下" << fileName << "是否有效";
                    ret = 1;
                }
            } catch (const exception& e) {
                LOG(ERROR) << e.what();
                ret = 1;
            }

            return ret;
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

        void DataCheckConfig::addProperty(string key, string value) {
            m_properties.insert(make_pair(key, value));
        }

        void DataCheckConfig::setProperty(string key, string value) {
            auto iter = m_properties.find(key);
            if (iter != m_properties.end()) {
                iter->second = value;
            }
        }

        bool DataCheckConfig::getPropertyBool(string key){
            map<string, string>::iterator it = m_properties.find(key);
            if (it != m_properties.end()) {

                string value = it->second;
                if(value == "t" || value == "T"){
                    return true;
                }else{
                    return false;
                }
            }

            LOG(WARNING) << "not fine " << key << " properties in config";

            return false;
        }

        string DataCheckConfig::getTaskId() {
            return task_id;
        }

        void DataCheckConfig::setTaskId(string id) {
            task_id = id;
        }

    }
}
