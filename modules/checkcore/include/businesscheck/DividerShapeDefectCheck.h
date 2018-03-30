//
// Created by gaoyanhong on 2018/3/29.
//

#ifndef AUTOHDMAP_DATACHECK_DIVIDERSHAPEDEFECTCHECK_H
#define AUTOHDMAP_DATACHECK_DIVIDERSHAPEDEFECTCHECK_H
#include "IMapProcessor.h"

namespace kd {
    namespace dc {

        /**
         * 车道线形状缺失检查
         * 对应检查项：JH_C_1, JH_C_2
         */
        class DividerShapeDefectCheck : public IMapProcessor{

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

            void check_JH_C_1_AND_JH_C_2(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput);

            void checkShapeDefect(shared_ptr<DCDivider> div, shared_ptr<DCDividerAttribute> divAtt,
                                  int beginNodexIdx, int endNodeIdx, shared_ptr<CheckErrorOutput> errorOutput);

            void checkShapeDefect( string checkModel, double distLimit, shared_ptr<DCDivider> div, shared_ptr<DCDividerAttribute> divAtt,
                                   int beginNodexIdx, int endNodeIdx, shared_ptr<CheckErrorOutput> errorOutput);

            void getComparePair(shared_ptr<DCDivider> div, int begin, int end, vector<pair<int,int>> & pairs);


        private:

            const string id = "divider_shape_defect_check";

        };
    }
}

#endif //AUTOHDMAP_DATACHECK_DIVIDERSHAPEDEFECTCHECK_H
