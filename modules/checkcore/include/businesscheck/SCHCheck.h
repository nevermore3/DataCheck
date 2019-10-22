
#ifndef AUTOHDMAP_DATACHECK_SCHCHECK_H
#define AUTOHDMAP_DATACHECK_SCHCHECK_H

#include "CommonInclude.h"
#include "data/DividerGeomModel.h"
#include "storage/CheckErrorOutput.h"
#include "data/DataManager.h"
/*
 * Divider Lane Road 属性点处理
 */

namespace kd {
    namespace dc {
        class SCHCheck {
        public:
            SCHCheck(string &objName);

            // 坡度值检查
            void SlopeValueCheck(shared_ptr<CheckErrorOutput> &errorOutput);

            // 航向角检查
            void HeadingValueCheck(shared_ptr<CheckErrorOutput> &errorOutput);

            //曲率值检查
            void CurvatureValueCheck(shared_ptr<CheckErrorOutput> &errorOutput);

            // 相邻的属性点之间距离不能超过1.3m
            void CheckAdjacentNodeDistance(shared_ptr<CheckErrorOutput> &errorOutput);

            bool LoadData();

            /*
             * 属性点的坡度
             * 和 属性点关联对象 最近的两个形点计算出的坡度 对比
             * 相差不能超过0.035
             */
            void SCHNodeRelevantObjectSlope(long objID,
                                            vector<shared_ptr<DCSCHInfo>> &nodes,
                                            vector<shared_ptr<DCCoord>> &coords,
                                            shared_ptr<CheckErrorOutput> &errorOutput);

            void SCHNodeRelevantObjectSlope(long objID,
                                            vector<shared_ptr<DCSCHInfo>> &nodes,
                                            vector<shared_ptr<DCDividerNode>> &coords,
                                            shared_ptr<CheckErrorOutput> &errorOutput);
        public:
            // key: 对象ID,  value : 和对象关联的 属性点集合
            map<long, vector<shared_ptr<DCSCHInfo>>> map_obj_schs_;

        private:
            // 对象名称: "HD_DIVIDER_SCH"、"HD_LANE_SCH"、"ADAS_NODE"
            string file_name_;
        };


    }
}
#endif //AUTOHDMAP_DATACHECK_SCHCHECK_H
