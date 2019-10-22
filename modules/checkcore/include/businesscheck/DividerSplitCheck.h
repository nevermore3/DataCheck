#ifndef AUTOHDMAP_DATACHECK_DIVIDERSPLITCHECK_H
#define AUTOHDMAP_DATACHECK_DIVIDERSPLITCHECK_H

#include <shp/shapefil.h>
#include <geos/geom/Polygon.h>
#include "IMapProcessor.h"
//third party
#include "geos/index/quadtree/Quadtree.h"
#include "util/GeosObjUtil.h"
#include "util/GeometryUtil.h"
#include <unordered_map>
#include <unordered_set>
#include "util/geos_obj_relation_util.h"
using namespace std;
namespace kd {
    namespace dc {
        struct DividerNodeInfo {
            ///是否含打断属性
            bool is_split;
            ///是否道路起点,否为道路尾点
            bool is_start;
            ///任务ID
            long task_id;
            ///当前道路节点
            shared_ptr<DCDividerNode> node_ptr;
            ///当前节点对应的几何对象
            shared_ptr<geos::geom::Point> geom_ptr;
            ///当前节点所在道路
            shared_ptr<DCDivider> divider_ptr;

            ///其关联的节点
            std::set<DividerNodeInfo *> conn_nodes;

        };

        struct DividerGeomInfo {
            DividerGeomInfo() {
                has_start_angle = false;
                has_end_angle = false;
                is_start_split = false;
                is_end_split = false;
                is_start_connect = false;
                is_end_connect = false;
                divider_geom_ptr = nullptr;
                divider_ptr = nullptr;
                start_ptr = nullptr;
                end_ptr = nullptr;
            }

            ///是否起始点有split属性
            bool is_start_split;
            ///是否终止点有split属性
            bool is_end_split;
            ///起始点是否已匹配
            bool is_start_connect;
            ///终止点是否已匹配
            bool is_end_connect;
            ///是否已计算开始侧道路方向
            bool has_start_angle;
            ///是否已计算结束侧道路方向
            bool has_end_angle;
            ///开始侧道路方向
            double start_angle;
            ///结束侧道路方向
            double end_engle;

            long task_id;
            shared_ptr<geos::geom::LineString> divider_geom_ptr;
            shared_ptr<DCDivider> divider_ptr;
            shared_ptr<geos::geom::Point> start_ptr;
            shared_ptr<geos::geom::Point> end_ptr;
        };
        struct GroupedDividers {
            GroupedDividers() {
                is_start = false;
                has_split = false;
                task_id = 0;
            }

            ///当前组节点是否起始
            bool is_start;
            ///当前组节点是否含有split属性(当前默认都含有,变量未使用)
            bool has_split;
            ///任务id
            long task_id;
            ///当前组节点信息列表
            std::vector<DividerNodeInfo *> node_list;
        };
        class DividerSplitCheck : public IMapProcessor {
        public:

            /**
             * 获得对象唯一标识
             * @return 对象标识
             */
            virtual string getId() override;

            /**
             * 进行任务处理
             * @param mapDataManager 地图数据
             * @return 操作是否成功
             */
            virtual bool
            execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) override;


            virtual void ShowStatisticsInfo();

            /**
             * 设置是否输出辅助信息,包括所有SPLIT节点,追踪标志
             * @param output_details
             */
            void SetOutputCheckDetails(bool output_details);

            bool IsOutputCheckDetails();

            void set_task_id(long task_id) {
                task_id_ = task_id;
            }

        private:
            /**
             * @brief 检查divider的split属性点,如split点不为首尾点,报错
             *          将其他的split属性点存入map容器
             */
            void CheckSplitLocationAndBuildSpatialIndex();

            /**
             * @brief 遍历所有split属性点,查找一定距离内有无其他split属性点,如找到,则将其从待检查容器中去除,
             *          距离阀值为node_search_buffer_len_
             */
            void RemoveConnectedSplitPoint();

            /**
             * @brief 检查重叠的含split属性的divider,将超过重叠长度阀值的点报错,并将重叠的split点排除
             *        过程未检测另一divider与当前divider重叠一端节点是否含有split属性
             */
            void CheckOverlapSplitDivider();

            /**
             * @brief 相邻任务框中，如果一个框中DIVIDER_NODE点的ISSPLIT为1（边界点），该点关联的DIVIDER车道分割线在相邻任务框有交点（交点不是首尾点）(50cm~100cm的接边容差)，如果超出容差值，则报错E1
             */
            void CheckIntersectSplitDivider();
            /**
             * @brief 排除DA是否应急车道导致的未匹配情况
             */
            void CheckEmergencyStandbyLaneSplitDivider();

            /**
             * @brief 追踪split的下一divider
             */
            void TraceDivider();

            /**
             * @brief 排除所有任务边界split点
             */
            void ExcludeTasksBorderSplit();


            void ExportErrSplitNode();

            void GetRelatedDividered(DividerNodeInfo *test_point_ptr,
                                     std::unordered_set<long> &id_list,
                                     std::list<DividerNodeInfo *> &related_dividers);

            void GroupRelatedDividers(std::list<DividerNodeInfo *> &related_dividers,
                                      std::list<shared_ptr<GroupedDividers>> &grouped_dividers);

            void CheckConnectLaneDA(DividerNodeInfo *split_node1, DividerNodeInfo *split_node2);

            void PreCheck();

            void EndCheck();

            void WriteAllSplit();

            void FilterQueryPoints(DividerNodeInfo *test_point_ptr, vector<void *> &raw_query_objs,
                                   vector<DividerNodeInfo *> &query_objs,
                                   double buffer_dis, double buffer_z, bool task_check);

            void FilterQueryPoints(DividerNodeInfo *test_point_ptr, vector<void *> &raw_query_objs,
                                   vector<DividerNodeInfo *> &query_objs,
                                   double buffer_dis,double buffer_z);
            void checkDividerInfo(const string check_id,const string check_desc,const string &da_key,long da_value1,long da_value2,DividerNodeInfo *split_node1, DividerNodeInfo *split_node2);
            void checkDA(const string check_id,const string check_desc,const string &da_key,long da_value1,long da_value2,DividerNodeInfo *split_node1, DividerNodeInfo *split_node2,const shared_ptr<DCDividerAttribute> &div_da1,const shared_ptr<DCDividerAttribute> &div_da2);
/**
 * 判读两个空间点之间的距离
 * @param point1 点1
 * @param point2 点2
 * @param buffer_size  水平容差
 * @param buffer_z 垂直容差
 * @return 容差内 true,容差外 false
 */
            bool PointIsConcurrent(shared_ptr<geos::geom::Point> point1,
                                   shared_ptr<geos::geom::Point> point2, double buffer_size, double buffer_z);
            bool zIsInBuffer(const shared_ptr<geos::geom::Point> point1, const shared_ptr<geos::geom::Point> point2, double buffer_z);
            void WriteNoSplitAttributeErr(long task_id, long div_id, shared_ptr<DCDividerNode> &node_ptr);

            void CheckSplitConnectInfo(std::vector<DividerNodeInfo *> &t_related_dividers, int &lane_num, int &conn_num);

            void GroupRelatedDividers(std::vector<DividerNodeInfo *> &div_list, double x, double y);

            void CreateTraceShapeHandle(SHPHandle &shp_ptr, DBFHandle &dbf_ptr);

            void WriteTraceGeom(SHPHandle shp_ptr, DBFHandle dbf_ptr, shared_ptr<geos::geom::Polygon> &trace_poly,
                                shared_ptr<KDSDividerNode> &node_ptr);

            void
            AddDividerNodeInfo(std::shared_ptr<DividerGeomInfo> &divider_info_ptr, DividerNodeInfo *ptr, bool make_connect);

            std::shared_ptr<DividerGeomInfo> CreateDividerGeomInfo(DividerNodeInfo *ptr, bool make_connect);

            void RemoveTwoNodeALineInGroup(std::vector<DividerNodeInfo *> &div_list, double x, double y);

            void RemoveLineWithOneNodeInGroup(std::vector<DividerNodeInfo *> &div_list,
                                              std::unordered_map<long, int> &node_count_map, double x, double y);

            bool GetRelativeDividerGroup(shared_ptr<GroupedDividers> &group, std::list<shared_ptr<GroupedDividers>> &groups,
            std::vector<shared_ptr<GroupedDividers>> &results, std::vector<bool> &unique_flags);

            bool Isvalid(long task_id);

            int GetErrorNum(const string &check_no);
            bool HasErrors();
        private:
            map<string, int> error_nums_;
            map<string, int> warning_nums_;
            ///空间索引
            shared_ptr<geos::index::quadtree::Quadtree> divider_node_quadtree_;
            ///split点信息映射
            std::unordered_map<long, shared_ptr<DividerNodeInfo>> split_node_info_map_;
            ///用于构建空间索引,防止被释放
            std::list<shared_ptr<DividerNodeInfo>> common_node_info_list_;
            ///divider信息映射
            std::unordered_map<long, shared_ptr<DividerGeomInfo>> divider_info_map_;

            ///输出辅助检查信息
            bool output_details_;
            ///接边判断Z方向容差
            double split_node_find_buffer_z = 0.5;

            double overlap_divider_err_len_ = 1.0;
            //相邻任务含split点的车道线相交buffer
            double intersect_search_buffer_len =0.5;
            //查找接边splitbuffer
            double intersect_search_buffer_z=0.2;
            //相交判断容差
            double intersect_split_divider_limit = 1.5;
            //判断相交z方向容差
            double intersect_split_divider_limit_z = 0.2;
            //交点到split点的长度限制
            double intersect_point_to_split_point_len=0.5;
            long task_id_ = 0;

        private:
            const string id_ = "divider_split_checker";

            const double node_search_buffer_z_ = 1.0;
            //切割点周边查找的距离，单位是米
            double node_search_buffer_len_ = 1.5;

            const double overlap_divider_search_buffer_ = 1.0;


            const double group_search_buffer_len_ = 10.0;

            const double group_dis_ = 4.0;

            //三角形追踪的方向夹角
            const float split_trace_angle_ = 5;

            //三角形追踪的距离
            const float split_trace_distance_ = 7.5;

            const string id = "DividerSplitCheck";
        };

    }
}
#endif //AUTOHDMAP_DATACHECK_DIVIDERSPLITCHECK_H
