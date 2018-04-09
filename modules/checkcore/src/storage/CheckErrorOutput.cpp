//
// Created by gaoyanhong on 2018/3/29.
//

#include "storage/CheckErrorOutput.h"


namespace kd {
    namespace dc {

        void CheckErrorOutput::saveError(shared_ptr<DCDividerCheckError> error) {

            if (outputFile_){
                ss_<< error->toString() << "\n";
            }else{
                cout << error->toString() << endl;
            }
        }

        void CheckErrorOutput::saveError(shared_ptr<DCAttCheckError> error){
            if (outputFile_){
                ss_<< error->toString() << "\n";
            }else{
                cout << error->toString() << endl;
            }
        }

        void CheckErrorOutput::saveError(shared_ptr<DCLaneCheckError> error){
            if (outputFile_){
                ss_<< error->toString() << "\n";
            }else{
                cout << error->toString() << endl;
            }
        }

        void CheckErrorOutput::writeInfo(string info) {
            if (outputFile_){
                ss_<< info.c_str() << "\n";
            }else{
                cout << info.c_str() << endl;
            }
        }
    }
}

