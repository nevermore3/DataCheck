//
// Created by zhangxingang on 19-1-18.
//

#ifndef AUTOHDMAP_DATACHECK_COMMONUTIL_H
#define AUTOHDMAP_DATACHECK_COMMONUTIL_H

#include <data/DataManager.h>
#include "data/DividerGeomModel.h"
#include "CommonInclude.h"

namespace kd {
    namespace dc {
        class CommonUtil {
        public:
            /**
             * 获取road
             * @param mapDataManager
             * @param road_id
             * @return 不存在返回null
             */
            static shared_ptr<DCRoad> get_road(shared_ptr<MapDataManager> mapDataManager, string road_id);

            /**
             * 获取lane group关联的道路
             * @param mapDataManager
             * @param lane_group
             * @return 不存在返回null
             */
            static shared_ptr<DCRoad> get_road_by_lg(const shared_ptr<MapDataManager> &mapDataManager,
                                                     const string &lane_group_id);

            /**
             * 获取lane group关联的道路，可能是多条道路
             * @param mapDataManager
             * @param lane_group_id
             * @return
             */
            static set<string> get_roads_by_lg(const shared_ptr<MapDataManager> &mapDataManager,
                                                              const string &lane_group_id);

            /**
             * 获取lane group
             * @param mapDataManager
             * @param lane_group_id
             * @return 不存在返回null
             */
            static shared_ptr<DCLaneGroup> get_lane_group(const shared_ptr<MapDataManager> &mapDataManager,
                                                          const string &lane_group_id);

            /**
             * 获取lane group
             * @param mapDataManager
             * @param divider_id
             * @return
             */
            static set<string> get_lane_groups_by_divider(shared_ptr<MapDataManager> mapDataManager, string divider_id);

            /**
             * 获取divider
             * @param mapDataManager
             * @param divider
             * @return 不存在返回null
             */
            static shared_ptr<DCDivider> get_divider(const shared_ptr<MapDataManager> &mapDataManager,
                                                     const string &divider);

            /**
             * 获取连接的divider
             * @param mapDataManager
             * @param divider
             * @param is_front 是否是当前首点连接
             * @return
             */
            static set<string> get_conn_divider(const shared_ptr<MapDataManager> &mapDataManager,
                                               const shared_ptr<DCDivider> &ptr_divider, bool is_front);

            /**
             * 获取连接的divider,主要是为了双向参考线方向不确定使用
             * @param mapDataManager
             * @param lg
             * @param divider
             * @param is_front 是否是当前首点连接
             * @return
             */
            static set<shared_ptr<DCDivider>> get_ref_conn_divider(const shared_ptr<MapDataManager> &mapDataManager,
                                                                   const string &lg,
                                                                   const shared_ptr<DCDivider> &ptr_divider,
                                                                   bool is_front);

            /**
             * 获取lane group组内lane集合，已排序
             * @param mapDataManager
             * @param lane_group_id
             * @return
             */
            static vector<shared_ptr<DCLane>> get_lanes_by_lg(const shared_ptr<MapDataManager> &mapDataManager,
                                                              const string &lane_group_id);

            /**
             * 获取lane group组内divider集合，已排序
             * @param mapDataManager
             * @param lane_group_id
             * @return
             */
            static vector<shared_ptr<DCDivider>> get_dividers_by_lg(const shared_ptr<MapDataManager> &mapDataManager,
                                                                    const string &lane_group_id);

            /**
             * 获取左右divider之间的车道集合，左右divider需要在同一个组
             * @param mapDataManager
             * @param left_divider
             * @param right_divider
             * @return
             */
            static vector<shared_ptr<DCLane>>
            get_lanes_between_dividers(const shared_ptr<MapDataManager> &mapDataManager,
                                       const shared_ptr<DCDivider> &left_divider,
                                       const shared_ptr<DCDivider> &right_divider, bool same = false);

            /**
             * 获取2个divider node距离
             * @param divider_node1
             * @param divider_node2
             * @return
             */
            static double get_length_between_divider_nodes(const shared_ptr<DCDividerNode> &divider_node1,
                                                           const shared_ptr<DCDividerNode> &divider_node2);

            static shared_ptr<geos::geom::LineString> get_line_string(const vector<shared_ptr<DCCoord>> &nodes);

            static shared_ptr<geos::geom::LineString>
            get_divider_line_string(const vector<shared_ptr<DCDividerNode>> &nodes);

            /**
             * divider是否是相同数字化方向的
             * @param left_divider
             * @param right_divider
             * @return
             */
            static bool check_dividers_same_direction(const shared_ptr<DCDivider> &left_divider,
                                                      const shared_ptr<DCDivider> &right_divider);

            /**
             * 返回node到DIVIDER的最短距离
             * @param divider_node
             * @param divider
             * @return
             */
            static double get_min_distance_from_divider(const shared_ptr<DCDividerNode> &divider_node,
                                                        const shared_ptr<DCDivider> &divider);

            static long GetMaxDividerNo(const shared_ptr<MapDataManager> &mapDataManager, const string &lane_group);

            /**
             * 获取距离divider首尾点一定距离的divider node
             * @param ptr_divider
             * @param length
             * @param is_front 是否从首点开始
             * @return
             */
            static shared_ptr<DCDividerNode> get_distance_node(shared_ptr<DCDivider> ptr_divider,
                                                               double length, bool is_front = true);

            /**
             * 计算点在线的左侧还是右侧
             * @param f_ptr_node
             * @param t_ptr_node
             * @param ptr_node
             * @return 返回值为1则在左侧，0为在线段上，-1为右侧, -2参数错误
             */
            static int NodeOrentationOfDivider(shared_ptr<DCDividerNode> f_ptr_node,
                                               shared_ptr<DCDividerNode> t_ptr_node,
                                               shared_ptr<DCDividerNode> ptr_node);

            static bool CheckCoordValid(DCCoord coord);

            static bool CheckCoordValid(shared_ptr<DCCoord> coord);

            static bool CheckCoordAngle(shared_ptr<DCCoord> ptr_coord1, shared_ptr<DCCoord> ptr_coord2,
                                        shared_ptr<DCCoord> ptr_coord3, double angle_threthold, double &angle);

        private:

            /**
             * 左右divider是否是相同组
             * @param mapDataManager
             * @param left_divider
             * @param right_divider
             * @param lane_group
             * @return
             */
            static bool is_same_lane_group(const shared_ptr<MapDataManager> &mapDataManager,
                                           const shared_ptr<DCDivider> &left_divider,
                                           const shared_ptr<DCDivider> &right_divider, string &lane_group);

            /**
            * 判断两条线是否方向相同
            * @return
            */
            static bool calLaneSameDir(double firstNode1X, double firstNode1Y, double firstNode2X, double firstNode2Y,
                                       double secondNode1X, double secondNode1Y, double secondNode2X,
                                       double secondNode2Y, double angleLimit = 90.0);
        };
    }
}


#endif //AUTOHDMAP_DATACHECK_COMMONUTIL_H
