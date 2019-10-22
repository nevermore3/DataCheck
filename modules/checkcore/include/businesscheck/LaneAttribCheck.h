#ifndef AUTOHDMAP_DATACHECK_LANEATTRIBCHECK_H
#define AUTOHDMAP_DATACHECK_LANEATTRIBCHECK_H

#include "IMapProcessor.h"

namespace kd {
    namespace dc {

        /**
         * 车道属性检查
         * 对应检查项：JH_C_19,JH_C_20,JH_C_21
         */
        class LaneAttribCheck : public IMapProcessor{

        public:
            LaneAttribCheck();

        public:

            /**
             * 获得对象唯一标识
             * @return 对象标识
             */
            virtual string getId() override ;

            /**
             * 进行任务处理
             * @param mapDataManager 地图数据
             * @return 操作是否成功
             */
            virtual bool execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) override ;


        private:

            //车道左右边线的通行方向（矢量化方向+车道线方向）冲突
            void check_JH_C_16(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput);

            //车道右侧车道线起点没有LA
            void check_JH_C_19(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput);

            //同一Divider上相邻两个LA属性完全一样
            void check_JH_C_21(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput);

            //同一Divider上相邻两个LA距离<1米
            void check_JH_C_20(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput);

        private:

            const string id = "lane_attrib_check";

        };
    }
}

#endif //AUTOHDMAP_DATACHECK_LANEATTRIBCHECK_H
