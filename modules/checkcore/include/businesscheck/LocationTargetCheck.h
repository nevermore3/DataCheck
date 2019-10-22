

#ifndef AUTOHDMAP_DATACHECK_LOCATIONTARGETCHECK_H
#define AUTOHDMAP_DATACHECK_LOCATIONTARGETCHECK_H

#include "IMapProcessor.h"
/**
 * 定位目标相关检查
 */
namespace kd {
    namespace dc {
        class LocationTargetCheck : public IMapProcessor{
        public:
            /**
             * 获得对象唯一标识
             * @return 对象标识
             */
            string getId() override;

            /**
             * 进行任务处理
             * @param mapDataManager 地图数据
             * @param errorOutput 错误信息输出
             * @return 操作是否成功
             */
            bool execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) override;

        private:
            /**
             * 清理内存
             */
            void clearMeomery();
            /**
             * 数据准备
             */
            void preCheck();
            /**
             * 交通灯分组正确性检查
             */
            void check_kxs_06_003();
            /**
             * 创建交通灯与分组的关系
             */
            void BuildLight2Group();
            /**
             * 创建交通灯与道路关系
             */
            void BuildLight2Road();
        private:
            ///交通灯与分组的关系
            map<long,long> map_light_id_to_group_id;
            ///交通灯与道路关系
            map<long,long> map_light_id_road_id;

            const string id = "LocationTargetCheck";
        };
    }
}
#endif //AUTOHDMAP_DATACHECK_LOCATIONTARGETCHECK_H
