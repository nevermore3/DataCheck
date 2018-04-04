//
// Created by gaoyanhong on 2018/3/29.
//

#ifndef AUTOHDMAP_DATACHECK_DATACHECKCONFIG_H
#define AUTOHDMAP_DATACHECK_DATACHECKCONFIG_H

#include "CommonInclude.h"

namespace kd {
    namespace dc {
        class DataCheckConfig {
        public:
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

            const static string OBJECT_PL_BUFFER;




        public:
            static DataCheckConfig & getInstance() {
                static DataCheckConfig instance;
                return instance;
            }

            /**
             * 加载配置文件
             * @param fileName 配置文件
             */
            void load(string fileName);

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


        private:
            //所有配置属性
            map<string, string> m_properties;

        };
    }
}

#endif //AUTOHDMAP_DATACHECK_DATACHECKCONFIG_H
