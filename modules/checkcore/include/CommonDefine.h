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
        const static std::string CHECK_ITEM_KXS_ORG_035 = "KXS_01_035";


        const static std::string CHECK_ITEM_KXS_ORG_001_DESC = "对象ID记录重复性检查";
        const static std::string CHECK_ITEM_KXS_ORG_002_DESC = "几何数据重复性检查";
        const static std::string CHECK_ITEM_KXS_ORG_003_DESC = "多边形形态错误";
        const static std::string CHECK_ITEM_KXS_ORG_004_DESC = "几何数据坐标值异常检查";
        const static std::string CHECK_ITEM_KXS_ORG_005_DESC = "";
        const static std::string CHECK_ITEM_KXS_ORG_006_DESC = "检查车道线节点间距判定是否中间有虚线丢失";
        const static std::string CHECK_ITEM_KXS_ORG_007_DESC = "车道分割线黄白线变化不一致";
        const static std::string CHECK_ITEM_KXS_ORG_008_DESC = "车行道边缘线在非停止线/出入口标线的地方断开";
        const static std::string CHECK_ITEM_KXS_ORG_009_DESC = "非路口虚拟线的车道线未构成车道";
        const static std::string CHECK_ITEM_KXS_ORG_010_DESC = "共点的车道线通行方向冲突";
        const static std::string CHECK_ITEM_KXS_ORG_011_DESC = "车道线不平滑夹角";
        const static std::string CHECK_ITEM_KXS_ORG_012_DESC = "存在长度小于0.2米的弧段";
        const static std::string CHECK_ITEM_KXS_ORG_013_DESC = "车道线高程突变";
        const static std::string CHECK_ITEM_KXS_ORG_014_DESC = "颜色、类型、通行类型属性冲突检查";
        const static std::string CHECK_ITEM_KXS_ORG_015_DESC = "车道线没有属性变化点";
        const static std::string CHECK_ITEM_KXS_ORG_016_DESC = "同一个divider上相邻两个DA属性完全相同";
        const static std::string CHECK_ITEM_KXS_ORG_017_DESC = "同一个divider上相邻两个DA距离<1米";
        const static std::string CHECK_ITEM_KXS_ORG_018_DESC = "DIVIDER的FDNODE与TDNODE应该是实际的首尾点";
        const static std::string CHECK_ITEM_KXS_ORG_019_DESC = "字段范围检查";
        const static std::string CHECK_ITEM_KXS_ORG_020_DESC = "缺少数据文件，或者数据为空";
        const static std::string CHECK_ITEM_KXS_ORG_021_DESC = "数据有效性检查";
        const static std::string CHECK_ITEM_KXS_ORG_022_DESC = "道路拓扑的数据检查";
        const static std::string CHECK_ITEM_KXS_ORG_023_DESC = "车道组与道路关联关系检查";
        const static std::string CHECK_ITEM_KXS_ORG_024_DESC = "结点坐标有效性检查";
        const static std::string CHECK_ITEM_KXS_ORG_025_DESC = "字段关系检查";
        const static std::string CHECK_ITEM_KXS_ORG_026_DESC = "外键在规格中存在, 在数据中不存在";
        const static std::string CHECK_ITEM_KXS_ORG_027_DESC = "外键的完备性检查, 记录不可缺失";
        const static std::string CHECK_ITEM_KXS_ORG_028_DESC = "";
        const static std::string CHECK_ITEM_KXS_ORG_029_DESC = "DIVIDER_SCH与关联DIVIDER距离检查";
        const static std::string CHECK_ITEM_KXS_ORG_030_DESC = "Divider点完备性检查";
        const static std::string CHECK_ITEM_KXS_ORG_031_DESC = "属性点的曲率值检查";
        const static std::string CHECK_ITEM_KXS_ORG_032_DESC = "属性点之间距离不超过1.3m";
        const static std::string CHECK_ITEM_KXS_ORG_035_DESC = "属性点的坡度和该属性点关联对象中距离最近的两个形点计算出的坡度对比";


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
        const static std::string CHECK_ITEM_KXS_LG_028 = "KXS-03-028";
        const static std::string CHECK_ITEM_KXS_LG_029 = "KXS-03-029";

        const static std::string CHECK_ITEM_KXS_LG_001_DESC = "同一个车道组内，单根车道线的长度同组内车道线平均长度不应该偏差超过";
        const static std::string CHECK_ITEM_KXS_LG_002_DESC = "分组中车道编号从零开始，从内向外递增。编号连续，不缺失，不重复";
        const static std::string CHECK_ITEM_KXS_LG_003_DESC = "车道线不存在于车道组中";
        const static std::string CHECK_ITEM_KXS_LG_004_DESC = "一条普通车道线存在于多个车道组中。两个车道组共用的双向车道线除外";
        const static std::string CHECK_ITEM_KXS_LG_005_DESC = "自动生成二维路网时，车道组要对道路全覆盖";
        const static std::string CHECK_ITEM_KXS_LG_006_DESC = "ValidityRange之间不重叠或交叉";
        const static std::string CHECK_ITEM_KXS_LG_007_DESC = "";
        const static std::string CHECK_ITEM_KXS_LG_008_DESC = "";
        const static std::string CHECK_ITEM_KXS_LG_009_DESC = "";
        const static std::string CHECK_ITEM_KXS_LG_010_DESC = "";
        const static std::string CHECK_ITEM_KXS_LG_011_DESC = "车道组未关联道路或关联道路无效";
        const static std::string CHECK_ITEM_KXS_LG_012_DESC = "车道组有效性检查";
        const static std::string CHECK_ITEM_KXS_LG_013_DESC = "车道未关联车道组";
        const static std::string CHECK_ITEM_KXS_LG_014_DESC = "车道关联多个车道组";
        const static std::string CHECK_ITEM_KXS_LG_015_DESC = "车道关联车道线无效";
        const static std::string CHECK_ITEM_KXS_LG_016_DESC = "车道线关联节点无效";
        const static std::string CHECK_ITEM_KXS_LG_017_DESC = "两个车道关联的左右车道线完全一致";
        const static std::string CHECK_ITEM_KXS_LG_018_DESC = "车道孤立不连通";
        const static std::string CHECK_ITEM_KXS_LG_019_DESC = "车道形态连通的关系丢失";
        const static std::string CHECK_ITEM_KXS_LG_020_DESC = "车道拓扑关系无效";
        const static std::string CHECK_ITEM_KXS_LG_021_DESC = "车道线冗余错误";
        const static std::string CHECK_ITEM_KXS_LG_022_DESC = "车道冗余错误";
        const static std::string CHECK_ITEM_KXS_LG_023_DESC = "Lane或Divider分组形态错误";
        const static std::string CHECK_ITEM_KXS_LG_024_DESC = "车道LaneType检查";
        const static std::string CHECK_ITEM_KXS_LG_025_DESC = "车道编号检查";
        const static std::string CHECK_ITEM_KXS_LG_026_DESC = "DIVIDER对应DA属性检查";
        const static std::string CHECK_ITEM_KXS_LG_027_DESC = "车道组没有打断，不应该存在既是入口又是出口的组";
        const static std::string CHECK_ITEM_KXS_LG_028_DESC = "车道组是否属于虚拟路口检查";


        // 道路检查
        const static std::string CHECK_ITEM_KXS_ROAD_001 = "KXS-04-001";
        const static std::string CHECK_ITEM_KXS_ROAD_002 = "KXS-04-002";
        const static std::string CHECK_ITEM_KXS_ROAD_003 = "KXS-04-003";
        const static std::string CHECK_ITEM_KXS_ROAD_004 = "KXS-04-004";
        const static std::string CHECK_ITEM_KXS_ROAD_005 = "KXS-04-005";
        const static std::string CHECK_ITEM_KXS_ROAD_006 = "KXS-04-006";
        const static std::string CHECK_ITEM_KXS_ROAD_007 = "KXS-04-007";
        const static std::string CHECK_ITEM_KXS_ROAD_008 = "KXS-04-008";
        const static std::string CHECK_ITEM_KXS_ROAD_009 = "KXS-04-009";
        const static std::string CHECK_ITEM_KXS_ROAD_010 = "KXS-04-010";
        const static std::string CHECK_ITEM_KXS_ROAD_011 = "KXS-04-011";

        const static std::string CHECK_ITEM_KXS_ROAD_001_DESC = "若车道组之间连通，则其内部的道路必然连通";
        const static std::string CHECK_ITEM_KXS_ROAD_002_DESC = "道路与车道组的两个车道边缘线应无交叉点。双向道路除外";
        const static std::string CHECK_ITEM_KXS_ROAD_003_DESC = "道路高程突变>±10厘米/米";
        const static std::string CHECK_ITEM_KXS_ROAD_004_DESC = "Road的起点终点检查";
        const static std::string CHECK_ITEM_KXS_ROAD_005_DESC = "道路RANGE索引号检查";
        const static std::string CHECK_ITEM_KXS_ROAD_006_DESC = "道路前后结点重复,前后点xy坐标相同（z值可能相同）";
        const static std::string CHECK_ITEM_KXS_ROAD_007_DESC = "道路结点出现拐点，或者角度过大";
        const static std::string CHECK_ITEM_KXS_ROAD_008_DESC = "道路形态点间距过近";
        const static std::string CHECK_ITEM_KXS_ROAD_009_DESC = "";
        const static std::string CHECK_ITEM_KXS_ROAD_010_DESC = "每一Road的形状点周围1.5米内必有一个关联该Road的AdasNode";
        const static std::string CHECK_ITEM_KXS_ROAD_011_DESC = "ADAS点到关联road的垂直距离检查";


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
        const static std::string CHECK_ITEM_KXS_LANE_023 = "KXS-05-023";

        const static std::string CHECK_ITEM_KXS_LANE_001_DESC = "如果车道组之间是“封闭”的，则车道组之间的车道不会存在孤立的车道";
        const static std::string CHECK_ITEM_KXS_LANE_002_DESC = "车道中心线与车道线在5米内不能存在两个交点（只检查组内车道线有共点的做检查）";
        const static std::string CHECK_ITEM_KXS_LANE_003_DESC = "车道中心线与本组的车道边缘线存在交叉点";
        const static std::string CHECK_ITEM_KXS_LANE_004_DESC = "";
        const static std::string CHECK_ITEM_KXS_LANE_005_DESC = "";
        const static std::string CHECK_ITEM_KXS_LANE_006_DESC = "车道面和其他车道面相交";
        const static std::string CHECK_ITEM_KXS_LANE_007_DESC = "车道面的4个角点构成夹角<45°或者>135";
        const static std::string CHECK_ITEM_KXS_LANE_008_DESC = "车道线方向冲突";
        const static std::string CHECK_ITEM_KXS_LANE_009_DESC = "车道宽度最窄处不能<2.5米";
        const static std::string CHECK_ITEM_KXS_LANE_010_DESC = "车道右侧车道线起点没有LA";
        const static std::string CHECK_ITEM_KXS_LANE_011_DESC = "同一Divider上相邻两个LA属性完全相同";
        const static std::string CHECK_ITEM_KXS_LANE_012_DESC = "孤立车道,没有进入和退出车道连接,并且没有标注应急车道属性";
        const static std::string CHECK_ITEM_KXS_LANE_013_DESC = "没有退出车道,divider拓扑存在,检查lane_connectivity是否正确";
        const static std::string CHECK_ITEM_KXS_LANE_014_DESC = "没有进入车道,divider拓扑存在,检查lane_connectivity是否正确";
        const static std::string CHECK_ITEM_KXS_LANE_015_DESC = "检查同组内中心线与中心线是否存在交叉问题（组内有共点的中心线做检查）";
        const static std::string CHECK_ITEM_KXS_LANE_016_DESC = "车道中心线前后结点重复,前后点xy坐标相同（z值可能相同）";
        const static std::string CHECK_ITEM_KXS_LANE_017_DESC = "同一条车道中心线上连续三个节点构成的夹角（绝对值）不能小于165度 (可配置)";
        const static std::string CHECK_ITEM_KXS_LANE_018_DESC = "两条相交且有车道拓扑关系的车道中心线最近的形状点构成的夹角（绝对值）不能小于170度(可配置)";
        const static std::string CHECK_ITEM_KXS_LANE_019_DESC = "";
        const static std::string CHECK_ITEM_KXS_LANE_020_DESC = "LANE点完备性检查";
        const static std::string CHECK_ITEM_KXS_LANE_021_DESC = "";
        const static std::string CHECK_ITEM_KXS_LANE_022_DESC = "LANE_SCH与关联LANE距离检查";
        const static std::string CHECK_ITEM_KXS_LANE_023_DESC = "车道中心线与车行道边缘线距离检查";


        // 定位目标
        const static std::string CHECK_ITEM_KXS_LM_001 = "KXS-06-001";
        const static std::string CHECK_ITEM_KXS_LM_002 = "KXS-06-002";

        const static std::string CHECK_ITEM_KXS_LM_001_DESC = "定位目标与道路的关联关系";
        const static std::string CHECK_ITEM_KXS_LM_002_DESC = "定位目标POINT高度值";

        // ADAS
        const static std::string CHECK_ITEM_KXS_ADAS_001 = "KXS-07-001";
        const static std::string CHECK_ITEM_KXS_ADAS_002 = "KXS-07-002";
        const static std::string CHECK_ITEM_KXS_ADAS_003 = "KXS-07-003";
        const static std::string CHECK_ITEM_KXS_ADAS_004 = "KXS-07-004";
        const static std::string CHECK_ITEM_KXS_ADAS_005 = "KXS-07-005";
        const static std::string CHECK_ITEM_KXS_ADAS_006 = "KXS-07-006";
        const static std::string CHECK_ITEM_KXS_ADAS_007 = "KXS-07-007";
        const static std::string CHECK_ITEM_KXS_ADAS_008 = "KXS-07-008";


        const static std::string CHECK_ITEM_KXS_ADAS_001_DESC = "相邻属性点间的距离不等于1米，除去与终点距离可能小于1米";
        const static std::string CHECK_ITEM_KXS_ADAS_002_DESC = "属性点与拟合曲线属性点的距离不大于10厘米";
        const static std::string CHECK_ITEM_KXS_ADAS_003_DESC = "ADAS_NODE点完备性检查";
        const static std::string CHECK_ITEM_KXS_ADAS_004_DESC = "";
        const static std::string CHECK_ITEM_KXS_ADAS_005_DESC = "";
        const static std::string CHECK_ITEM_KXS_ADAS_006_DESC = "";
        const static std::string CHECK_ITEM_KXS_ADAS_007_DESC = "ADAS_NODE坡度值域检查";
        const static std::string CHECK_ITEM_KXS_ADAS_008_DESC = "ADAS_NODE与关联ROAD距离检查";

        //长度一致性检查
        const static std::string CHECK_ITEM_KXS_LENGTH_001 = "KXS-08-001";

        const static std::string CHECK_ITEM_KXS_LENGTH_001_DESC = "长度总和一致性检查";
        //数目一致性检查
        const static std::string CHECK_ITEM_KXS_COUNT_001 = "KXS-09-001";

        const static std::string CHECK_ITEM_KXS_COUNT_001_DESC = "数量一致性检查";

        //kxf规格检查
        const static std::string CHECK_ITEM_KXS_NORM_001 = "KXS-10-001";
        const static std::string CHECK_ITEM_KXS_NORM_002 = "KXS-10-002";
        const static std::string CHECK_ITEM_KXS_NORM_003 = "KXS-10-003";

        const static std::string CHECK_ITEM_KXS_NORM_001_DESC = "KXF表描述检查";
        const static std::string CHECK_ITEM_KXS_NORM_002_DESC = "坡度变化的平滑检查";
        const static std::string CHECK_ITEM_KXS_NORM_003_DESC = "";

        ///polyine类型数据相关检查
        const static std::string CHECK_ITEM_KXS_LINE_001 ="KXS-11-01";
        const static std::string CHECK_ITEM_KXS_LINE_002 ="KXS-11-02";
        const static std::string CHECK_ITEM_KXS_LINE_001_DESC = "停止线与道路的关联关系正确性检查";

    }
}

#endif //AUTOHDMAP_DATACHECK_COMMONDEFINE_H
