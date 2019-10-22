
#ifndef AUTOHDMAP_DATACHECK_COMMONCHECK_H
#define AUTOHDMAP_DATACHECK_COMMONCHECK_H

#include "storage/CheckErrorOutput.h"

namespace kd {
    namespace dc {
        class CommonCheck {
        public:
            /**
             * 结点间距比较函数
             * @param dc_coord_vec
             * @param dis_threshold 阈值
             * @return
             */
            static vector<shared_ptr<NodeCompareError>> DistanceCheck(const vector<shared_ptr<DCCoord>> &dc_coord_vec,
                                                                      double dis_threshold);

            static vector<shared_ptr<NodeCompareError>> DistanceCheck(const vector<shared_ptr<DCDividerNode>> &dc_node_vec,
                                                                      double dis_threshold);

            /**
             * 结点角度检查函数
             * @param dc_coord_vec
             * @param dis_threshold
             * @return
             */
            static vector<shared_ptr<NodeCompareError>> AngleCheck(const vector<shared_ptr<DCCoord>> &dc_coord_vec,
                                                                   double angle_threshold);

            static vector<shared_ptr<NodeCompareError>> AngleCheck(const vector<shared_ptr<DCDividerNode>> &dc_node_vec,
                                                                   double angle_threshold);


            /**
             * 结点高程突变检查
             * @param dc_coord_vec
             * @param height_threshold
             * @return
             */
            static vector<shared_ptr<NodeCompareError>> NodeHeightCheck(const vector<shared_ptr<DCCoord>> &dc_coord_vec,
                                                                        double height_threshold);

            static vector<shared_ptr<NodeCompareError>> NodeHeightCheck(const vector<shared_ptr<DCDividerNode>> &dc_node_vec,
                                                                        double height_threshold);
        };
    }
}


#endif //AUTOHDMAP_DATACHECK_COMMONCHECK_H
