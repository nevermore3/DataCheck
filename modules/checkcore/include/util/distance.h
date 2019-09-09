//
// Created by ubuntu on 2019/9/9.
//

#ifndef AUTOHDMAP_DATACHECK_DISTANCE_H
#define AUTOHDMAP_DATACHECK_DISTANCE_H

#include "data/DividerGeomModel.h"

namespace kd {
    namespace dc {

        class Distance {
        public:

            static double distance(const shared_ptr<DCCoord> pt,
                                   const vector<shared_ptr<DCCoord>>& line,
                                   shared_ptr<DCCoord> pFoot,
                                   int32_t* pSeg);


            static double distance(const shared_ptr<DCCoord> pt,
                                   const vector<shared_ptr<DCDividerNode>> &nodes,
                                   shared_ptr<DCCoord> pFoot,
                                   int32_t *pSeg);

            static double distance(const shared_ptr<DCCoord> pt1, const shared_ptr<DCCoord> pt2);




        };
    }
}


#endif //AUTOHDMAP_DATACHECK_DISTANCE_H
