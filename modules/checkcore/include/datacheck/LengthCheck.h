//
// Created by ubuntu on 2019/8/21.
//

#ifndef AUTOHDMAP_DATACHECK_LENGTHCHECK_H
#define AUTOHDMAP_DATACHECK_LENGTHCHECK_H

#include "IMapProcessor.h"

namespace kd {
    namespace dc {

        class LengthCheck : public IMapProcessor {
        public:
            LengthCheck();

            ~LengthCheck();

            string getId() override;

            bool execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) override;


            void Check_kxs_08_001();
        private:
            const string id_ = "length_check";

            double length_precise_;

            shared_ptr<MapDataManager>osm_data_manager_;

            shared_ptr<MapDataManager>kxf_data_manager_;
        };
    }
}


#endif //AUTOHDMAP_DATACHECK_LENGTHCHECK_H
