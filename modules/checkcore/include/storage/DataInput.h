//
// Created by zhangxingang on 19-6-13.
//

#ifndef AUTOHDMAP_DATACHECK_DATAINPUT_H
#define AUTOHDMAP_DATACHECK_DATAINPUT_H
#include "data/DataManager.h"

#include "CheckErrorOutput.h"

namespace kd {
    namespace dc {
        class DataInput {
        public:
            DataInput(const shared_ptr<MapDataManager> &map_data_manager_,
                      const shared_ptr<CheckErrorOutput> &error_output_, const string &base_path_);

            virtual bool LoadData() = 0;

        protected:
            shared_ptr<MapDataManager> map_data_manager_;
            shared_ptr<CheckErrorOutput> error_output_;

            string base_path_;
        };
    }
}

#endif //AUTOHDMAP_DATACHECK_DATAINPUT_H