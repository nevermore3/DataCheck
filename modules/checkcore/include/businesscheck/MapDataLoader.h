//
// Created by gaoyanhong on 2018/3/29.
//

#ifndef AUTOHDMAP_DATACHECK_MAPDATALOADER_H
#define AUTOHDMAP_DATACHECK_MAPDATALOADER_H

#include "IMapProcessor.h"

namespace kd {
    namespace dc {
        class MapDataLoader : public IMapProcessor{

        public:
            MapDataLoader(string basePath);

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
            virtual bool execute(shared_ptr<MapDataManager> map_data_manager, shared_ptr<CheckErrorOutput> error_output) override ;

        private:

            const string id = "map_data_loader";

            //基础数据路径
            string basePath_;

            shared_ptr<MapDataManager> map_data_manager_;
            shared_ptr<CheckErrorOutput> error_output_;

        };
    }
}

#endif //AUTOHDMAP_DATACHECK_MAPDATALOADER_H
