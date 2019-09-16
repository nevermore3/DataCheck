//
// Created by gaoyanhong on 2018/3/30.
//

#ifndef AUTOHDMAP_DATACHECK_DIVIDERTOPOCHECK_H
#define AUTOHDMAP_DATACHECK_DIVIDERTOPOCHECK_H

#include "IMapProcessor.h"

//third party
#include "geos/indexQuadtree.h"
using namespace geos::index::quadtree;

namespace kd {
    namespace dc {

        /**
         * 车道线拓扑关系检查
         * 对应检查项：JH_C_4, JH_C_5, JH_C_6
         */
        class DividerTopoCheck : public IMapProcessor {

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

        private:
            //车行道边缘线在非停止线/出入口标线的地方断开
            void check_JH_C_4(shared_ptr<MapDataManager> mapDataManager, const map<string, shared_ptr<DCDividerTopoNode>> & topoNodes,  shared_ptr<CheckErrorOutput> errorOutput);

            //存在非构成车道的车道线（非路口虚拟线）
            void check_JH_C_5(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput);

            //共点的车道线通行方向（矢量化方向+车道线方向）冲突
            void check_JH_C_6(shared_ptr<MapDataManager> mapDataManager, const map<string, shared_ptr<DCDividerTopoNode>> & topoNodes, shared_ptr<CheckErrorOutput> errorOutput);
            void save_kxs_01_010_error(shared_ptr<DCDividerTopoNode> topoNode ,shared_ptr<CheckErrorOutput> errorOutput);

            /**
             * 构建车道线共点关系
             * @param mapDataManager 地图数据
             * @param topoNodes 所有共点信息， key为节点id
             */
            void buildTopoNodes(shared_ptr<MapDataManager> mapDataManager, map<string, shared_ptr<DCDividerTopoNode>> & topoNodes);

            /**
             * 判断一个车道线是否为边线
             * @param div 车道线对象
             * @return true代表是边线，false代表不是边线
             */
            bool isEdgeLine(shared_ptr<DCDivider> div);

            /**
             * 判断一个车道线是否同边线兼容，如它是一个虚拟和道线或者出入口标线
             * @param div 被检查的车道线
             * @return true代表同边线兼容，false代表不兼容
             */
            bool isEdgeLineCompatible(shared_ptr<DCDivider> div);

            /**
             * 检查车道线起点和终点是否同边线线兼容的车道线连接，或者周围有线状对象（如停止线）
             * @param div 被检查的车道线对象
             * @param quadtree 包含所有停止线数据的空间索引
             * @param topoNodes 节点连接拓扑联系
             * @param mapDataManager 地图数据
             * @param errorOutput 错误数据输出
             */
            void checkEdgeConnectInfo(shared_ptr<DCDivider> div,
                                      const shared_ptr<geos::index::quadtree::Quadtree> &quadtree,
                                      const map<string, shared_ptr<DCDividerTopoNode>> &topoNodes,
                                      shared_ptr<MapDataManager> mapDataManager,
                                      shared_ptr<CheckErrorOutput> errorOutput);

            /**
             * 查找节点旁边是否有停止线
             * @param topoNode 节点
             * @param bufferLen 缓冲区大小
             * @param quadtree 空间索引
             * @return true代表找到停止线， false代表未找到停止线
             */
            bool findStopLine(shared_ptr<DCDividerTopoNode> topoNode, double bufferLen,
                              const shared_ptr<geos::index::quadtree::Quadtree> & quadtree);

            /**
             * 判断一个节点是否有相兼容的边缘线连接
             * @param nodeId 节点id
             * @param start 是fromnode还是tonode
             * @param topoNodes 节点连接拓扑联系
             * @param quadtree 包含所有停止线数据的空间索引
             * @param mapDataManager 地图数据
             * @return true代表找到兼容连接线，false代表未找到
             */
            bool findConnectEdge(string nodeId, bool start,
                                 const map<string, shared_ptr<DCDividerTopoNode>> & topoNodes,
                                 const shared_ptr<geos::index::quadtree::Quadtree> & quadtree,
                                 shared_ptr<MapDataManager> mapDataManager);

        private:

            const string id = "divider_topo_check";
        };
    }
}

#endif //AUTOHDMAP_DATACHECK_DIVIDERTOPOCHECK_H
