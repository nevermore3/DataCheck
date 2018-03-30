//
// Created by gaoyanhong on 2018/3/29.
//

#ifndef AUTOHDMAP_DATACHECK_DIVIDERATTRIBTCHECK_H
#define AUTOHDMAP_DATACHECK_DIVIDERATTRIBTCHECK_H
#include "IMapProcessor.h"

namespace kd {
    namespace dc {

        /**
         * 车道线属性检查
         * 对应检查项：JH_C_3
         */
        class DividerAttribCheck : public IMapProcessor{

        public:
            DividerAttribCheck();

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
            void attCheck_JH_C_3(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput);

        private:

            const string id = "divider_attrib_check";

            map<long,long> yellowTypes;

            map<long,long> whiteTypes;

        };
    }
}

#endif //AUTOHDMAP_DATACHECK_DIVIDERATTRIBTCHECK_H
