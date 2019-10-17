//
// Created by zhangxingang on 19-1-25.
//

#ifndef AUTOHDMAP_DATACHECK_ROADCHECK_H
#define AUTOHDMAP_DATACHECK_ROADCHECK_H

#include <IMapProcessor.h>
#include "SCHCheck.h"
namespace kd {
    namespace dc {
        class RoadCheck  : public IMapProcessor, public SCHCheck {
        public:
            RoadCheck(string fileName);
            /**
             * 获得对象唯一标识
             * @return 对象标识
             */
            string getId() override;

            /**
             * 进行任务处理
             * @param mapDataManager 地图数据
             * @param errorOutput 错误信息输出
             * @return 操作是否成功
             */
            bool execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) override;
        private:

            /**
             * 道路与lanegroup边缘线无交点
             * @param mapDataManager
             * @param errorOutput
             */
            void check_road_divider_intersect(shared_ptr<MapDataManager> mapDataManager,
                                              shared_ptr<CheckErrorOutput> errorOutput);

            void check_road_node_height(shared_ptr<MapDataManager> mapDataManager,
                                        shared_ptr<CheckErrorOutput> errorOutput);

            void check_road_node(shared_ptr<MapDataManager> mapDataManager,
                                 shared_ptr<CheckErrorOutput> errorOutput);

            void SetMapDataManager(shared_ptr<MapDataManager> &mapDataManager);

            bool LoadLGLaneGroupIndex();

            void DoNode2DividerSlope(long lgID, long fromIndex, long toIndex, vector<shared_ptr<DCSCHInfo>> &nodes,
                                     shared_ptr<CheckErrorOutput> &errorOutput);

            map<long, pair<long, long>> GetLaneGroupsIndex(long roadID);


            /*
             * 每个AdasNode点的坡度和关联的Divider对象中距离最近的两个形点计算出的坡度对比
             */
            void AdasNodeRelevantDividerSlope(shared_ptr<CheckErrorOutput> &errorOutput);


            /*
             * 每一Road的形状点周围1.5米内必有一个关联该Road的AdasNode
             * @param errorOutput
             */
            void RoadRelevantAdasNode(shared_ptr<CheckErrorOutput> &errorOutput);

            /*
             * AdasNode点离关联的Road的垂直距离不超过10cm
             * @param errorOutput
             */
            void AdasNodeVerticalDistance(shared_ptr<CheckErrorOutput> errorOutput);

            /*
             * 定位目标与道路关联关系存在性检查
             */
            void CheckRLORoad();

            /**
             * 结点重复
             * @param errorOutput
             * @param ptr_road
             */
            void check_road_node_repeat(shared_ptr<CheckErrorOutput> errorOutput, shared_ptr<DCRoad> ptr_road);

            /**
             * 结点间角度过大
             * @param errorOutput
             * @param ptr_road
             */
            void check_road_node_angle(shared_ptr<CheckErrorOutput> errorOutput, shared_ptr<DCRoad> ptr_road);

            /**
             * 结点间距过小
             * @param errorOutput
             * @param ptr_road
             */
            void check_road_node_distance(shared_ptr<CheckErrorOutput> errorOutput, shared_ptr<DCRoad> ptr_road);

            shared_ptr<geos::geom::LineString> get_road_line_string(const shared_ptr<MapDataManager> &mapDataManager,
                                                                    const shared_ptr<DCRoad> &ptr_road,
                                                                    const string &lane_group_id);

            bool road_divider_intersect(const shared_ptr<MapDataManager> &mapDataManager,
                                        const shared_ptr<DCRoad> &ptr_road,
                                        const string &lane_group_id,
                                        const shared_ptr<DCDivider> &ptr_divider);

            bool LoadTrafficRule();

            bool LoadRoadNode();

            bool LoadCNode();

            bool LoadCNodeConn();

            bool LoadNodeConn();

            void BuildInfo();
            //检查联通关系前数据加载
            void preCheckConn();
            //检查联通关系
            void checkCNode();

        private:
            const string id = "road_check";

            shared_ptr<MapDataManager> map_data_manager_;

            map<long, shared_ptr<DCTrafficRule>> map_traffic_rule_;

            // roadnode
            map<long, shared_ptr<DCRoadNode>> map_road_nodes_;

            //cnode
            map<long, shared_ptr<DCCNode>> map_cnodes_;

            // nodeconn
            map<long, shared_ptr<DCNodeConn>> map_node_conn_;

            // cnodeconn
            map<long, shared_ptr<DCCNodeConn>> map_cnode_conn_;
            ///map<cnode_id,vector<road_node_id>>
            map<long,vector<long>> map_cnode_node;
            ///map<froad_id,vector<troad_id>>,需要对比的cconn表数据
            map<long,vector<long>> map_froad_troad;

            // key: roadID, value:{key : from_index, value {pair<to_index, lgID>} }
            map<long, map<long, vector<pair<long, long>>>> map_road_lg_index_;
        };
    }
}
#endif //AUTOHDMAP_DATACHECK_ROADCHECK_H
