//
// Created by ubuntu on 2019/9/10.
//

#ifndef AUTOHDMAP_DATACHECK_DIVIDERCHECK_H
#define AUTOHDMAP_DATACHECK_DIVIDERCHECK_H

#include <IMapProcessor.h>
#include "geos/index/quadtree/Quadtree.h"
#include "SCHCheck.h"

namespace kd {
    namespace dc {
        class DividerCheck : public IMapProcessor, public SCHCheck {
        public:
            explicit DividerCheck(string fileName);

            ~DividerCheck() override;

            string getId() override;

            bool execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) override;

        private:
            void SetMapDataManager(shared_ptr<MapDataManager> &mapDataManager);

            /**
             * 每一Divider的形状点周围1.5米内必有一个关联该divider的HD_DIVIDER_SCH
             * @param errorOutput
             */
            void DividerRelevantDividerSCH(shared_ptr<CheckErrorOutput> &errorOutput);


            /**
             * HD_DIVIDER_SCH点离关联的DIVIDER的垂直距离不超过10cm
             * @param errorOutput
             */
            void DividerSCHVerticalDistance(shared_ptr<CheckErrorOutput> errorOutput);

            //每个HD_DIVIDER_SCH点的坡度和关联的DIVIDER对象中距离最近的两个形点计算出的坡度对比
            void DividerSCHRelevantDividerSlope(shared_ptr<CheckErrorOutput> &errorOutput);
        private:
            const string id_ = "divider_check";

            string base_path_;

            shared_ptr<MapDataManager> map_data_manager_;

        };
    }
}
#endif //AUTOHDMAP_DATACHECK_DIVIDERCHECK_H
