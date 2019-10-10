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

            bool LoadLGLaneGroupIndex();

            void DoNode2DividerSlope(long lgID, long fromIndex, long toIndex, vector<shared_ptr<DCSCHInfo>> &nodes,
                                     shared_ptr<CheckErrorOutput> &errorOutput);

            map<long, pair<long, long>> GetLaneGroupsIndex(long roadID);


            /*
             * 每个AdasNode点的坡度和关联的Divider对象中距离最近的两个形点计算出的坡度对比
             */
            void AdasNodeRelevantDividerSlope(shared_ptr<CheckErrorOutput> &errorOutput);

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

            void BuildNodeID2Road();

            //
            void CheckNodesAndCNodeRelation(shared_ptr<CheckErrorOutput> &errorOutput);
            //检查联通关系前数据加载
            void preCheckConn();
            //检查联通关系
            void checkCNode();
            /**
             * 根据进入road获取关联的退出道路
             * @param from_road_id
             * @param t_road_set
             */
            void getTRoadByFRoad(long from_road_id,set<long> &t_road_set);
            /**
             * 查找复杂路口内部道路可通达的道路
             * @param from_road_id 进入道路
             * @param insideRoad 内部道路
             * @param t_road_ids 需要对比的退出道路集合
             * @param checkedNodes 已经遍历过的内部道路集合
             */
            void findAccessibleRoad(long cnode_id,long from_road_id,shared_ptr<DCRoad> insideRoad,long t_road_end_node_id,set<long> &t_road_ids,set<long> &checkedNodes);

            /*
             * 道路通行方向孤立检查
             */
            void CheckIsolatedRoad();

            /*
             * 道路等级连通性检查
             */
            void CheckRoadGradesInterConnection();
        private:
            const string id = "road_check";
            int item_data_total=0;
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

            // key : nodeID, value: {roads}
            map<long, vector<shared_ptr<DCRoad>>> map_node_id_to_froad_;

            map<long, vector<shared_ptr<DCRoad>>> map_node_id_to_troad_;
            ///map<froad_id,cnode_id>
            map<long,set<long>> map_froad_to_cnode;
            ///map<troad,cnode_id>
            map<long,set<long>> map_troad_to_cnode;


            // key: roadID, value:{key : from_index, value {pair<to_index, lgID>} }
            map<long, map<long, vector<pair<long, long>>>> map_road_lg_index_;

        };
    }
}
#endif //AUTOHDMAP_DATACHECK_ROADCHECK_H
