#ifndef AUTOHDMAP_DATACHECK_LENGTHCHECK_H
#define AUTOHDMAP_DATACHECK_LENGTHCHECK_H

#include "IMapProcessor.h"

namespace kd {
    namespace dc {

        class DataCheckModel;
        class LengthCheck : public IMapProcessor {
        public:
            LengthCheck();

            ~LengthCheck();

            string getId() override;

            bool execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) override;

            void Check_kxs_08_001();

            void CheckLength(const shared_ptr<DataCheckModel> &checkModel);
        private:
            const string id_ = "length_check";

            double length_precise_;

            vector<shared_ptr<DataCheckModel>> length_check_model_;
        };
    }
}


#endif //AUTOHDMAP_DATACHECK_LENGTHCHECK_H
