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
         * 对应检查项：JH_C_6, JH_C_5
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


            void buildTopoNodes(shared_ptr<MapDataManager> mapDataManager, map<string, shared_ptr<DCDividerTopoNode>> & topoNodes);

            bool isEdgeLine(shared_ptr<DCDivider> div);

            void check(shared_ptr<DCDivider> div, const shared_ptr<geos::index::quadtree::Quadtree> & quadtree,
                                         set<string> & divIds, const map<string, shared_ptr<DCDividerTopoNode>> & topoNodes, shared_ptr<MapDataManager> mapDataManager);


            /**
             * 查找节点旁边是否有停止线
             * @param topoNode 节点
             * @param bufferLen 缓冲区大小
             * @param quadtree 空间索引
             * @return true代表找到停止线， false代表未找到停止线
             */
            bool findStopLine(shared_ptr<DCDividerTopoNode> topoNode, double bufferLen,
                              const shared_ptr<geos::index::quadtree::Quadtree> & quadtree);

        private:

            const string id = "divider_topo_check";
        };
    }
}

#endif //AUTOHDMAP_DATACHECK_DIVIDERTOPOCHECK_H
