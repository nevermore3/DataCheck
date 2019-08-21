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

            void LoadCheckModel();

            bool execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) override;

            void Check_kxs_08_001();

            void CheckLength(const string &modelName, const string &fileName);
        private:
            const string id_ = "length_check";

            double length_precise_;

            vector<shared_ptr<DCTask>> length_check_model_;
        };
    }
}


#endif //AUTOHDMAP_DATACHECK_LENGTHCHECK_H
