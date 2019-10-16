//
// Created by ubuntu on 19-9-29.
//

#ifndef AUTOHDMAP_DATACHECK_POLYLINECHECK_H
#define AUTOHDMAP_DATACHECK_POLYLINECHECK_H
#include "IMapProcessor.h"

namespace kd {
    namespace dc {
        class PolylineCheck : public IMapProcessor{
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
             * 检查停止线和关联road是否相交
             */
            void check_kxs_011_002();
        private:
            ///检查信息缓存
            std::map<string,shared_ptr<DCPolyline>> polylines;
            ///polyline关联road信息
            std::map<string,shared_ptr<DCRLORoad>> rloroad;

            const string id = "PolylineCheck";
        };
    }
}


#endif //AUTOHDMAP_DATACHECK_POLYLINECHECK_H
