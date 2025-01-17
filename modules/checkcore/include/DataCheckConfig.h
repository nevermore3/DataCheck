

#ifndef AUTOHDMAP_DATACHECK_DATACHECKCONFIG_H
#define AUTOHDMAP_DATACHECK_DATACHECKCONFIG_H

#include "CommonInclude.h"

namespace kd {
    namespace dc {
        class DataCheckConfig {
        public:
            const static int TOPO_AUTO_CHECK = 1;
            const static int ALL_AUTO_CHECK = 2;

            const static string DOTLINE_DEFECT_LEN;
            const static string BUSELINE_DEFECT_LEN;
            const static string DA_SPACE_LEN;
            const static string DIVIDER_EDGE_ANGLE;
            const static string DIVIDER_NODE_SPACE_LEN;
            const static string DIVIDER_HEIGHT_CHANGE_PER_METER;

            const static string LA_SPACE_LEN;
            const static string LANE_EDGE_MAX_ANGLE;
            const static string LANE_EDGE_MIN_ANGLE;
            const static string LANE_WIDTH_MAX;
            const static string LANE_WIDTH_MIN;
            const static string LANE_OVERLAP_AREA;
            const static string LANE_DIVIDER_DIRANGLE;
            const static string LANE_NODE_ANGLE;
            const static string LANE_ANGLE;

            const static string OBJECT_PL_BUFFER;

            const static string CONFIG_FILE_PATH;
            const static string MODEL_FILE_PATH;
            const static string TASK_FILE;
            const static string SQL_TASK_FILE;
            const static string UPDATE_REGION;

            const static string DIVIDER_LENGTH_RATIO;
            const static string LANE_INTERSECT_LENGTH;
            const static string ADAS_NODE_DISTANCE;
            const static string ADAS_NODE_DISTANCE_ACCURACY;
            const static string ADAS_NODE_CURVATURE_DISTANCE;
            const static string ADAS_NODE_MAX_CURVATURE;
            const static string ADAS_NODE_MAX_SLOPE;

            const static string ROAD_NODE_ANGLE;
            const static string ROAD_NODE_DISTANCE;

            const static string JSON_DATA_INPUT;
            const static string RESOURCE_FILE_PATH;
            const static string RESOURCE_FILE;
            const static string ERR_JSON_PATH;
            const static string OUTPUT_PATH;
            const static string CHECK_FILE_PATH;
            const static string NODE_RELEVANT_OBJ_SLOPE_ERROR;
            const static string SCH_HEADING_ERROR;
            const static string SCH_SLOPE_ERROR;
            const static string SCH_CURVATURE_ERROR;

            const static string SHP_FILE_PATH;
            const static string DB_INPUT_FILE;
            const static string CHECK_STATE;

            //数据一致性检查
            const static string LENGTH_CHECK_FILE;
            const static string COUNT_CHECK_FILE;

            const static string LENGTH_PRECISE;

            ///接边检查
            const static string CONN_INPUTPATH;
            const static string CONN_OUTPATH;
            const static string CONN_TASKBOUNDPATH;
            const static string CONN_CHECKFILEPATH;
            const static string AVG_SLOPE_ERROR;
            const static string CHECK_ALL_FILE;

        public:
            static DataCheckConfig & getInstance() {
                static DataCheckConfig instance;
                return instance;
            }

            /**
             * 加载配置文件
             * @param fileName 配置文件
             */
            int load(string fileName);

            /**
             * 获取配置属性
             * @param key 配置属性关键字
             * @return 配置属性值
             */
            string getProperty(string key);

            /**
             * 获取配置属性，该属性值类型为double
             * @param key 配置属性关键字
             * @return 配置属性值
             */
            double getPropertyD(string key);

            /**
             * 获取配置属性，该属性值类型为int
             * @param key 配置属性关键字
             * @return 配置属性值
             */
            int getPropertyI(string key);

            void addProperty(string key, string value);

            void setProperty(string key, string value);

            string getTaskId();

            void setTaskId(string id);
            //获得bool类型的参数， 如果没找到关键字则返回false
            bool getPropertyBool(string key);
        private:
            //所有配置属性
            map<string, string> m_properties;
            string task_id;

        };
    }
}

#endif //AUTOHDMAP_DATACHECK_DATACHECKCONFIG_H
