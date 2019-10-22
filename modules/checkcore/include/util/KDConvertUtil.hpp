

#ifndef AUTOHDMAP_FUSIONCORE_KDCONVERTUTIL_H
#define AUTOHDMAP_FUSIONCORE_KDCONVERTUTIL_H

#include "CommonInclude.h"

namespace kd {
   namespace dc {
        class KDConvertUtil{

        public:

            static int string2int(string value, int defInt = 0);

            static double string2double(string value, int defDouble = 0.0);

        };

    }
}

#endif //AUTOHDMAP_FUSIONCORE_KDCONVERTUTIL_H
