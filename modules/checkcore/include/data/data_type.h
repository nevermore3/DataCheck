
#ifndef AUTOHDMAP_DATACHECK_DATATYPE_H
#define AUTOHDMAP_DATACHECK_DATATYPE_H


#include "DividerGeomModel.h"

namespace kd {
    namespace dc {
        class DataType {

        };


//定义lanegroup之间的连接方式
        enum EnumLaneGroupConnType {
            CONNT_UNKNOWN = -1,
            CONNT_NORMAL = 0,
            CONNT_SPLIT = 1,
            CONNT_MERGE = 2,
            CONNT_INTERSECT = 3, //交叉路口
            CONNT_DUAL = 4,  //双向
            CONNT_MULTI = 5,  //分组量测都连接了多个车道组，此种是错误的场景
            CONNT_DUAL_INTERSECT = 6, //交叉路口双向
            CONNT_SPLIT_INTERSECT = 7,//分离为虚拟组
            CONNT_MERGE_INTERSECT = 8,//虚拟组合并
            CONNT_MULTI_INTERSECT = 9, //分组两侧都连接了多个虚拟组,这种不是错误场景
        };

        enum LANE_GROUP_POSITION {
            POSITION_LEFT = 1,
            POSITION_MIDDLE = 2,
            POSITION_RIGHT = 3,
            POSITION_RETURN = 4,
        };

        class TopoDividerNodeExt {
        public:
            shared_ptr<DCDividerNode> node_;

            map<long, shared_ptr<DCDivider>> from_dividers_;

            map<long, shared_ptr<DCDivider>> to_dividers_;
        };

        //车道组
        class TopoLaneGroup {
        public:
            TopoLaneGroup() {
                conn_type_ = CONNT_UNKNOWN;
                dir_ref_div_ = nullptr;
            }

        public:
            //查找到车道组的参考车道线，同时确定其他车道线同它的方向关系
            bool BuildDividerDirectionInfo();

            //获得车道组内正向参考的车道线对象
            shared_ptr<DCDivider> GetDirectionRefDiv() { return dir_ref_div_; }

            //获得车道组内车道线的个数
            int GetDividerCount() { return dividers_.size(); }

            //根据正向车道线的参考方向，获取指定索引的车道线的正向车道线节点值, 如果车道线与返回的坐标序列相同，则返回0，否则返回1
            void GetDividerNormalNodes(int index, vector<shared_ptr<DCCoord>> &nodes);

            //按照正向方向获取车道线的首点或尾点
            shared_ptr<DCDividerNode> GetDividerNormalNode(int index, bool start);

            //获得第一个以一个节点为起点或终点车道线中的索引，如果未查找到，则返回-1；
            int GetNodeIndex(long node_id, bool start, bool left_to_right = true, bool ignore_same = false);

            //获得指定索引的车道线同车道组方向的关系
            bool GetDivDirection(int index) { return dir_infos_[index]; }

            //分析两个车道组之间的关系，-1：关系不明；0：tlg在本组的右侧；1：tlg在本组的左侧；2：tlg与本组有重叠的车道
            int Relation(const shared_ptr<TopoLaneGroup> tlg);

            //获得车道跨度的权重，用于排序。越靠在车道线左侧的关联关系，权重值越小
            int GetLaneSpanWeight(const shared_ptr<TopoLaneGroup> tlg, bool from_or_to);

            //获得车道方向的权重，用于排序。直行的值接近0，左转的分组为负数，右转的分组为正数
            double GetDirectionWeight(const shared_ptr<TopoLaneGroup> tlg, bool from_or_to);

            //获得车道组起点方向或终点方向唯一的节点数，可通过这个初步判断简单车道数
            int GetUnionNodeCount(bool start_or_end);

            //查找车道组起点或终点方向是否连接测试车道组
            bool FindTopoLaneGroup(const shared_ptr<TopoLaneGroup> tlg_check, bool from_or_to);

            //增加车道组关联信息
            void AppendTopoLaneGroup(const shared_ptr<TopoLaneGroup> tlg, bool from_or_to);

            bool DividerIsConcurrentNode(const shared_ptr<DCDivider> src_div, const shared_ptr<DCDivider> dst_div);
        public:

            //车道组id，同原始lanegroupid;
            long id_;

            //组成车道组的车道线，已经按照divider_no进行了排序
            vector<shared_ptr<DCDivider>> dividers_;

            //根据本车道组同其他车道组的关联关系，确定本车道组的拓扑连接关系
            EnumLaneGroupConnType conn_type_;

            //进入本车道组的所有车道组
            vector<shared_ptr<TopoLaneGroup>> from_lanegroups_;

            //本车道组离开时，可进入的车道组
            vector<shared_ptr<TopoLaneGroup>> to_lanegroups_;

            //正向车道线参考值
            shared_ptr<DCDivider> dir_ref_div_;

            //每个车道线与参考车道线的方向是否相同
            vector<bool> dir_infos_;

            // 连接的每一个车道组的角度
            map<long, int> f_lane_group_position_;
            map<long, int> t_lane_group_position_;
        };
    }
}
#endif //AUTOHDMAP_DATACHECK_DATATYPE_H
