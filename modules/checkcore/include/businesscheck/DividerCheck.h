//
// Created by ubuntu on 2019/9/10.
//

#ifndef AUTOHDMAP_DATACHECK_DIVIDERCHECK_H
#define AUTOHDMAP_DATACHECK_DIVIDERCHECK_H

#include <IMapProcessor.h>
#include "geos/index/quadtree/Quadtree.h"

namespace kd {
    namespace dc {
        class DividerCheck : public IMapProcessor {
        public:
            explicit DividerCheck();

            ~DividerCheck() override;

            string getId() override;

            bool execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) override;

        private:
            bool LoadDividerSCH();


            /**
             * 每一Divider的形状点周围1.5米内必有一个关联该divider的HD_DIVIDER_SCH
             * @param errorOutput
             */
            void DividerRelevantDividerSCH(shared_ptr<CheckErrorOutput> errorOutput);

            /**
             * 相邻HD_DIVIDER_SCH点之间距离不能超过1.3m
             * @param errorOutput
             */
            void AdjacentDividerSCHNodeDistance(shared_ptr<CheckErrorOutput> errorOutput);

            /**
             * HD_DIVIDER_SCH点离关联的DIVIDER的垂直距离不超过10cm
             * @param errorOutput
             */
            void DividerSCHVerticalDistance(shared_ptr<CheckErrorOutput> errorOutput);
        private:
            const string id_ = "divider_check";

            string base_path_;

            shared_ptr<MapDataManager> map_data_manager_;

        private:
            unordered_map<long, map<long, shared_ptr<DCDivideSCH>>> map_divider_sch_;
        };
    }
}
#endif //AUTOHDMAP_DATACHECK_DIVIDERCHECK_H
