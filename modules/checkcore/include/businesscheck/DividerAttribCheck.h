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
         * 对应检查项：JH_C_3,JH_C_10,JH_C_11,JH_C_12,JH_C_13
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
            //黄（虚线、实线、双虚线、双实线、左实右虚、左虚右实）变其他，白（虚线、实线、双虚线、双实线、左实右虚、左虚右实）变其他，都可以变公交专用车道
            void check_JH_C_3(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput);

            //字段匹配关系：
            // 虚拟分隔线（1、2、3）时，分隔线类型为0，颜色类型为0，通行类型为0；
            // 颜色为2时，分隔线类型为4、8、9、10、12、13、15、17、11；
            // 颜色为1时，分割线类型为1、2、3、4、5、6、7、11、14、16、18；
            // 通行类型为1，分隔线类型为1、3、4、7、10、11、13、18；
            // 通行类型为2，分隔线类型为2、4、5、6、8、9、12、18；
            // 通行类型为3，分隔线类型为14、15；
            // 通行类型为4，分隔线类型为16、17；
            void check_JH_C_10(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput);

            //起点没有有DA
            void check_JH_C_11(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput);

            //同一个divider上相邻两个DA属性完全一样
            void check_JH_C_12(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput);

            //同一个divider上相邻两个DA距离<1米
            void check_JH_C_13(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput);

        private:

            const string id = "divider_attrib_check";

            map<long,long> yellowTypes;

            map<long,long> whiteTypes;

            map<long,map<long,long>> colorDividerTypeMaps;

            map<long,map<long,long>> driveRuleDividerTypeMaps;

        };
    }
}

#endif //AUTOHDMAP_DATACHECK_DIVIDERATTRIBTCHECK_H
