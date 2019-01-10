//
// Created by gaoyanhong on 2018/3/29.
//

#include "storage/CheckErrorOutput.h"


namespace kd {
    namespace dc {

        void CheckErrorOutput::saveError(shared_ptr<DCDividerCheckError> error) {

            if (ss_.is_open()){
                ss_<< error->toString() << "\n";
            }else{
                LOG(ERROR) << error->toString();
            }
        }

        void CheckErrorOutput::saveError(shared_ptr<DCAttCheckError> error){
            if (ss_.is_open()){
                ss_<< error->toString() << "\n";
            }else{
                LOG(ERROR) << error->toString();
            }
        }

        void CheckErrorOutput::saveError(shared_ptr<DCLaneCheckError> error){
            if (ss_.is_open()){
                ss_<< error->toString() << "\n";
            }else{
                LOG(ERROR) << error->toString();
            }
        }

        void CheckErrorOutput::writeInfo(string info, bool bLongString) {
            if (ss_.is_open()){
                if (bLongString)
                    ss_<< "\"" << info.c_str() << "\"\n";
                else
                    ss_<< info.c_str() << "\n";
            }else{
                LOG(ERROR) << info.c_str();
            }
        }
    }
}

