//
// Created by gaoyanhong on 2018/3/30.
//

#ifndef AUTOHDMAP_DATACHECK_DIVIDERSHAPENORMCHECK_H
#define AUTOHDMAP_DATACHECK_DIVIDERSHAPENORMCHECK_H

#include "IMapProcessor.h"

namespace kd {
    namespace dc {

        /**
         * 车道线形状规范化检查
         * 对应检查项：JH_C_7,JH_C_8,JH_C_9
         */
        class DividerShapeNormCheck : public IMapProcessor{

        public:
            DividerShapeNormCheck();

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
            //存在夹角<135°的弧段
            void check_JH_C_7(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput);

            //存在长度小于0.2米的弧段; 车道线高程突变>±10厘米/米
            void check_JH_C_8_AND_JH_C_9(shared_ptr<MapDataManager> mapDataManager,
                                         shared_ptr<CheckErrorOutput> errorOutput);


        private:

            const string id = "divider_shape_norm_check";
        };
    }
}
#endif //AUTOHDMAP_DATACHECK_DIVIDERSHAPENORMCHECK_H
