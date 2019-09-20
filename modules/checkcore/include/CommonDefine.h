//
// Created by zhangxingang on 19-4-23.
//

#ifndef AUTOHDMAP_DATACHECK_COMMONDEFINE_H
#define AUTOHDMAP_DATACHECK_COMMONDEFINE_H

#include "CommonInclude.h"
namespace kd {
    namespace dc {

        ///接边检查
        const static std::string CHECK_DIV_SPLIT_LOCATION_ERR = "CHECK_DIV_SPLIT_LOCATION_ERR";
        const static std::string CHECK_DIV_SPLIT_OVERLAP = "FEM-001";//CHECK_DIV_SPLIT_OVERLAP
        const static std::string CHECK_DIV_SPLIT_INTERSECT = "FEM-002";
        const static std::string CHECK_DIV_SPLIT_NO_CONTINUOUS_DIV = "FEM-003";
        const static std::string CHECK_DIV_SPLIT_NO_SPLIT_ATTRIBUTE = "FEM-004";
        const static std::string CHECK_DIV_SPLIT_CONTINUOUS_Z_OUT_BUFFER = "FEM-006";
        const static std::string CHECK_DIV_SPLIT_DIRECTION_ERR = "FEM-007";
        const static std::string CHECK_DIV_SPLIT_R_LINE_ERR = "FEM-008";
        const static std::string CHECK_DIV_SPLIT_TOLLFLAG_ERR = "FEM-009";
        const static std::string CHECK_DIV_SPLIT_DA_VIRTUAL_ERR = "FEM-010";
        const static std::string CHECK_DIV_SPLIT_DA_COLOR_ERR = "FEM-011";
        const static std::string CHECK_DIV_SPLIT_DA_TYPE_ERR = "FEM-012";
        const static std::string CHECK_DIV_SPLIT_DA_DRIVE_RULE_ERR = "FEM-013";
        const static std::string CHECK_DIV_SPLIT_DA_OVERLAY_ERR = "FEM-014";
        const static std::string CHECK_DIV_SPLIT_DA_MATERIAL_ERR = "FEM-015";
        const static std::string CHECK_DIV_SPLIT_DA_WIDTH_ERR = "FEM-016";

        const static std::string CHECK_DIV_SPLIT_LOCATION_ERR_DESC = "split点不为车道线首末点";
        const static std::string CHECK_DIV_SPLIT_OVERLAP_DESC = "相邻任务含split点的车道线出现重叠";
        const static std::string CHECK_DIV_SPLIT_INTERSECT_DESC = "相邻任务含split点的车道线相交";
        const static std::string CHECK_DIV_SPLIT_NO_CONTINUOUS_DIV_DESC = "含split点的车道线在相邻任务框中无接边车道线";
        const static std::string CHECK_DIV_SPLIT_NO_SPLIT_ATTRIBUTE_DESC = "车道线节点缺失split属性";
        const static std::string CHECK_DIV_SPLIT_CONTINUOUS_Z_OUT_BUFFER_DESC = "任务框接边处Z坐标值相差太大";
        const static std::string CHECK_DIV_SPLIT_DIRECTION_ERR_DESC = "接边DIVIDER的通行方向错误";
        const static std::string CHECK_DIV_SPLIT_R_LINE_ERR_DESC = "接边DIVIDER的是否参考线不同";
        const static std::string CHECK_DIV_SPLIT_TOLLFLAG_ERR_DESC = "接边DIVIDER的收费站分隔线不同";
        const static std::string CHECK_DIV_SPLIT_DA_VIRTUAL_ERR_DESC = "接边DA的虚拟分隔线类型不同";
        const static std::string CHECK_DIV_SPLIT_DA_COLOR_ERR_DESC = "接边DA的分隔线颜色不同";
        const static std::string CHECK_DIV_SPLIT_DA_TYPE_ERR_DESC = "接边DA的分割线类型不同";
        const static std::string CHECK_DIV_SPLIT_DA_DRIVE_RULE_ERR_DESC = "接边DA的通行类型不同";
        const static std::string CHECK_DIV_SPLIT_DA_OVERLAY_ERR_DESC = "接边DA的覆盖类型不同";
        const static std::string CHECK_DIV_SPLIT_DA_MATERIAL_ERR_DESC = "接边DA的分隔线材质不同";
        const static std::string CHECK_DIV_SPLIT_DA_WIDTH_ERR_DESC = "接边DA的分隔线宽度不同";


        // 源数据组织检查
        const static std::string CHECK_ITEM_KXS_ORG_001 = "KXS-01-001";
        const static std::string CHECK_ITEM_KXS_ORG_002 = "KXS-01-002";
        const static std::string CHECK_ITEM_KXS_ORG_003 = "KXS-01-003";
        const static std::string CHECK_ITEM_KXS_ORG_004 = "KXS-01-004";
        const static std::string CHECK_ITEM_KXS_ORG_005 = "KXS-01-005";
        const static std::string CHECK_ITEM_KXS_ORG_006 = "KXS-01-006";
        const static std::string CHECK_ITEM_KXS_ORG_007 = "KXS-01-007";
        const static std::string CHECK_ITEM_KXS_ORG_008 = "KXS-01-008";
        const static std::string CHECK_ITEM_KXS_ORG_009 = "KXS-01-009";
        const static std::string CHECK_ITEM_KXS_ORG_010 = "KXS-01-010";
        const static std::string CHECK_ITEM_KXS_ORG_011 = "KXS-01-011";
        const static std::string CHECK_ITEM_KXS_ORG_012 = "KXS-01-012";
        const static std::string CHECK_ITEM_KXS_ORG_013 = "KXS-01-013";
        const static std::string CHECK_ITEM_KXS_ORG_014 = "KXS-01-014";
        const static std::string CHECK_ITEM_KXS_ORG_015 = "KXS-01-015";
        const static std::string CHECK_ITEM_KXS_ORG_016 = "KXS-01-016";
        const static std::string CHECK_ITEM_KXS_ORG_017 = "KXS-01-017";
        const static std::string CHECK_ITEM_KXS_ORG_018 = "KXS-01-018";
        const static std::string CHECK_ITEM_KXS_ORG_019 = "KXS-01-019";
        const static std::string CHECK_ITEM_KXS_ORG_020 = "KXS-01-020";
        const static std::string CHECK_ITEM_KXS_ORG_021 = "KXS-01-021";
        const static std::string CHECK_ITEM_KXS_ORG_022 = "KXS-01-022";
        const static std::string CHECK_ITEM_KXS_ORG_023 = "KXS-01-023";
        const static std::string CHECK_ITEM_KXS_ORG_024 = "KXS-01-024";
        const static std::string CHECK_ITEM_KXS_ORG_025 = "KXS-01-025";
        const static std::string CHECK_ITEM_KXS_ORG_026 = "KXS_01_026";
        const static std::string CHECK_ITEM_KXS_ORG_027 = "KXS_01_027";
        const static std::string CHECK_ITEM_KXS_ORG_028 = "KXS_01_028";
        const static std::string CHECK_ITEM_KXS_ORG_029 = "KXS_01_029";
        const static std::string CHECK_ITEM_KXS_ORG_030 = "KXS_01_030";

        const static std::string CHECK_ITEM_KXS_ORG_031 = "KXS_01_031";
        const static std::string CHECK_ITEM_KXS_ORG_032 = "KXS_01_032";

        //
        const static std::string CHECK_ITEM_KXS_ORG_001_DESC = "KXS-01-001";
        const static std::string CHECK_ITEM_KXS_ORG_002_DESC = "KXS-01-002";
        const static std::string CHECK_ITEM_KXS_ORG_003_DESC = "KXS-01-003";
        const static std::string CHECK_ITEM_KXS_ORG_004_DESC = "KXS-01-004";
        const static std::string CHECK_ITEM_KXS_ORG_029_DESC = "DIVIDER_SCH与关联DIVIDER距离检查";

        // 分组检查
        const static std::string CHECK_ITEM_KXS_LG_001 = "KXS-03-001";
        const static std::string CHECK_ITEM_KXS_LG_002 = "KXS-03-002";
        const static std::string CHECK_ITEM_KXS_LG_003 = "KXS-03-003";
        const static std::string CHECK_ITEM_KXS_LG_004 = "KXS-03-004";
        const static std::string CHECK_ITEM_KXS_LG_005 = "KXS-03-005";
        const static std::string CHECK_ITEM_KXS_LG_006 = "KXS-03-006";
        const static std::string CHECK_ITEM_KXS_LG_007 = "KXS-03-007";
        const static std::string CHECK_ITEM_KXS_LG_008 = "KXS-03-008";
        const static std::string CHECK_ITEM_KXS_LG_009 = "KXS-03-009";
        const static std::string CHECK_ITEM_KXS_LG_010 = "KXS-03-010";
        const static std::string CHECK_ITEM_KXS_LG_011 = "KXS-03-011";
        const static std::string CHECK_ITEM_KXS_LG_012 = "KXS-03-012";
        const static std::string CHECK_ITEM_KXS_LG_013 = "KXS-03-013";
        const static std::string CHECK_ITEM_KXS_LG_014 = "KXS-03-014";
        const static std::string CHECK_ITEM_KXS_LG_015 = "KXS-03-015";
        const static std::string CHECK_ITEM_KXS_LG_016 = "KXS-03-016";
        const static std::string CHECK_ITEM_KXS_LG_017 = "KXS-03-017";
        const static std::string CHECK_ITEM_KXS_LG_018 = "KXS-03-018";
        const static std::string CHECK_ITEM_KXS_LG_019 = "KXS-03-019";
        const static std::string CHECK_ITEM_KXS_LG_020 = "KXS-03-020";
        const static std::string CHECK_ITEM_KXS_LG_021 = "KXS-03-021";
        const static std::string CHECK_ITEM_KXS_LG_022 = "KXS-03-022";
        const static std::string CHECK_ITEM_KXS_LG_023 = "KXS-03-023";
        const static std::string CHECK_ITEM_KXS_LG_024 = "KXS-03-024";
        const static std::string CHECK_ITEM_KXS_LG_025 = "KXS-03-025";
        const static std::string CHECK_ITEM_KXS_LG_026 = "KXS-03-026";
        const static std::string CHECK_ITEM_KXS_LG_027 = "KXS-03-027";

        // 道路检查
        const static std::string CHECK_ITEM_KXS_ROAD_001 = "KXS-04-001";
        const static std::string CHECK_ITEM_KXS_ROAD_002 = "KXS-04-002";
        const static std::string CHECK_ITEM_KXS_ROAD_003 = "KXS-04-003";
        const static std::string CHECK_ITEM_KXS_ROAD_004 = "KXS-04-004";
        const static std::string CHECK_ITEM_KXS_ROAD_005 = "KXS-04-005";
        const static std::string CHECK_ITEM_KXS_ROAD_006 = "KXS-04-006";
        const static std::string CHECK_ITEM_KXS_ROAD_007 = "KXS-04-007";
        const static std::string CHECK_ITEM_KXS_ROAD_008 = "KXS-04-008";

        // 车道中心线
        const static std::string CHECK_ITEM_KXS_LANE_001 = "KXS-05-001";
        const static std::string CHECK_ITEM_KXS_LANE_002 = "KXS-05-002";
        const static std::string CHECK_ITEM_KXS_LANE_003 = "KXS-05-003";
        const static std::string CHECK_ITEM_KXS_LANE_004 = "KXS-05-004";
        const static std::string CHECK_ITEM_KXS_LANE_005 = "KXS-05-005";
        const static std::string CHECK_ITEM_KXS_LANE_006 = "KXS-05-006";
        const static std::string CHECK_ITEM_KXS_LANE_007 = "KXS-05-007";
        const static std::string CHECK_ITEM_KXS_LANE_008 = "KXS-05-008";
        const static std::string CHECK_ITEM_KXS_LANE_009 = "KXS-05-009";
        const static std::string CHECK_ITEM_KXS_LANE_010 = "KXS-05-010";
        const static std::string CHECK_ITEM_KXS_LANE_011 = "KXS-05-011";
        const static std::string CHECK_ITEM_KXS_LANE_012 = "KXS-05-012";
        const static std::string CHECK_ITEM_KXS_LANE_013 = "KXS-05-013";
        const static std::string CHECK_ITEM_KXS_LANE_014 = "KXS-05-014";
        const static std::string CHECK_ITEM_KXS_LANE_015 = "KXS-05-015";
        const static std::string CHECK_ITEM_KXS_LANE_016 = "KXS-05-016";
        const static std::string CHECK_ITEM_KXS_LANE_017 = "KXS-05-017";
        const static std::string CHECK_ITEM_KXS_LANE_018 = "KXS-05-018";
        const static std::string CHECK_ITEM_KXS_LANE_019 = "KXS-05-019";
        const static std::string CHECK_ITEM_KXS_LANE_020 = "KXS-05-020";
        const static std::string CHECK_ITEM_KXS_LANE_021 = "KXS-05-021";
        const static std::string CHECK_ITEM_KXS_LANE_022 = "KXS-05-022";

        // 定位目标
        const static std::string CHECK_ITEM_KXS_LM_001 = "KXS-06-001";
        const static std::string CHECK_ITEM_KXS_LM_002 = "KXS-06-002";

        // ADAS
        const static std::string CHECK_ITEM_KXS_ADAS_001 = "KXS-07-001";
        const static std::string CHECK_ITEM_KXS_ADAS_002 = "KXS-07-002";
        const static std::string CHECK_ITEM_KXS_ADAS_003 = "KXS-07-003";
        const static std::string CHECK_ITEM_KXS_ADAS_005 = "KXS-07-005";
        const static std::string CHECK_ITEM_KXS_ADAS_007 = "KXS-07-007";
        const static std::string CHECK_ITEM_KXS_ADAS_008 = "KXS-07-008";

        //长度一致性检查
        const static std::string CHECK_ITEM_KXS_LENGTH_001 = "KXS-08-001";
        //数目一致性检查
        const static std::string CHECK_ITEM_KXS_COUNT_001 = "KXS-09-001";

        //kxf规格检查
        const static std::string CHECK_ITEM_KXS_NORM_001 = "KXS-10-001";
        const static std::string CHECK_ITEM_KXS_NORM_002 = "KXS-10-002";
        const static std::string CHECK_ITEM_KXS_NORM_003 = "KXS-10-003";

        const static std::string ID = "ID";
        const static std::string DIVIDER_ID = "DIVIDER_ID";
        const static std::string DIVIDER_NODE_ID = "DIVIDER_NODE_ID";




        // 检查项信息统计
        class MapItemInfo {
        public:
            static MapItemInfo &getInstance() {
                static MapItemInfo instance;
                return instance;
            }

            /**
             * 获取检查项描述
             * @param checkID 检查项ID
             * @return 检查项目描述
             */
            std::string getProperty(std::string checkID);
            /**
             * 获取检查项数目
             * @return 检查项目数目
             */
            size_t getNums();

            /**
             * 建立检查项ID和检查项描述的映射
             */
            void load();
        private:
            // 检查项ID和检查项描述的映射

            std::map<std::string, std::string> map_item_info_;
        };

    }
}

#endif //AUTOHDMAP_DATACHECK_COMMONDEFINE_H
