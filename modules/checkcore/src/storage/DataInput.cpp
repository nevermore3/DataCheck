//
// Created by zhangxingang on 19-6-13.
//

#include "storage/DataInput.h"

namespace kd {
    namespace dc {

        DataInput::DataInput(const shared_ptr<MapDataManager> &map_data_manager_,
                             const shared_ptr<CheckErrorOutput> &error_output_, const string &base_path_)
                : map_data_manager_(map_data_manager_), error_output_(error_output_), base_path_(base_path_) {}
    }
}
