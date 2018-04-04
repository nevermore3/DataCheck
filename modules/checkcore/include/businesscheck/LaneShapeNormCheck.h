//
// Created by gaoyanhong on 2018/4/2.
//

#ifndef AUTOHDMAP_DATACHECK_LANESHAPENORMCHECK_H
#define AUTOHDMAP_DATACHECK_LANESHAPENORMCHECK_H

#include "IMapProcessor.h"

namespace kd {
    namespace dc {

        /**
         * 车道形状规范化检查
         * 对应检查项：JH_C_15,JH_C_,JH_C_
         */
        class LaneShapeNormCheck : public IMapProcessor {

        public:

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
            //车道面和其他车道面相交
            void check_JH_C_14(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput);

            //车道面的4个夹角<45°或者>135°
            void check_JH_C_15(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput);

            //车道宽度最窄处不能<2.5米，最大不能>7米
            void check_JH_C_17(shared_ptr<MapDataManager> mapDataManager,
                                         shared_ptr<CheckErrorOutput> errorOutput);


        private:

            const string id = "lane_shape_norm_check";

        };
    }
}

#endif //AUTOHDMAP_DATACHECK_LANESHAPENORMCHECK_H
