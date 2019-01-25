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
            static set<string> get_ref_conn_divider(const shared_ptr<MapDataManager> &mapDataManager, const string &lg,
                                                const shared_ptr<DCDivider> &ptr_divider, bool is_front);

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
                                       const shared_ptr<DCDivider> &right_divider);

            /**
             * 获取点集合长度
             * @param ptr_coords
             * @return
             */
            static double get_length_of_coords(const vector<shared_ptr<DCCoord>> &ptr_coords);

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
        };
    }
}


#endif //AUTOHDMAP_DATACHECK_COMMONUTIL_H
