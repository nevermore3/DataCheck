#ifndef AUTOHDMAP_DATACHECK_COUNTCHECK_H
#define AUTOHDMAP_DATACHECK_COUNTCHECK_H

#include "IMapProcessor.h"

namespace kd {
    namespace dc {
        class DataCheckModel;
        class CountCheck : public IMapProcessor {
        public:
            CountCheck();

            ~CountCheck() {};

            string getId() override ;

            bool execute(shared_ptr<MapDataManager> mapDataManager, shared_ptr<CheckErrorOutput> errorOutput) override;

            void Check_kxs_08_002();

            void CheckCount(const shared_ptr<DataCheckModel> &chechModel);

        private:
            const string id_ = "Count Check";

            vector<shared_ptr<DataCheckModel>> count_check_model_;
        };

    }
}

#endif //AUTOHDMAP_DATACHECK_COUNTCHECK_H
