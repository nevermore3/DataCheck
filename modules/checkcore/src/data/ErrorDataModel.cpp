//
// Created by gaoyanhong on 2018/3/29.
//

#include "data/ErrorDataModel.h"

namespace kd {
    namespace dc {

        string DCDividerCheckError::toString() {
            stringstream ss;
            ss << "DCDividerCheckError: " << dividerId_ << "," << nodeId_ << endl;
            return ss.str();
        }


        shared_ptr<DCDividerCheckError>
        DCDividerCheckError::createByAtt(string checkModel, shared_ptr<DCDivider> div,
                                         shared_ptr<DCDividerAttribute> att) {

            shared_ptr<DCDividerCheckError> error = make_shared<DCDividerCheckError>(checkModel);
            if(div == nullptr)
                return error;

            error->dividerId_ = div->id_;
            if( att != nullptr){
                error->nodeId_ = att->dividerNode_->id_;
                error->attId_ = att->id_;
                error->lng_ = att->dividerNode_->coord_.lng_;
                error->lat_ = att->dividerNode_->coord_.lat_;
                error->z_ = att->dividerNode_->coord_.z_;
            }else{
                error->nodeId_ = "";
                error->attId_ = "";
                error->lng_ = 0.0;
                error->lat_ = 0.0;
                error->z_ = 0.0;
            }

            return error;
        }

        shared_ptr<DCDividerCheckError>
        DCDividerCheckError::createByNode(string checkModel, shared_ptr<DCDivider> div, shared_ptr<DCDividerNode> node){
            shared_ptr<DCDividerCheckError> error = make_shared<DCDividerCheckError>(checkModel);
            if(div == nullptr)
                return error;

            error->dividerId_ = div->id_;

            if( node != nullptr){
                error->nodeId_ = node->id_;
                error->attId_ = "";
                error->lng_ = node->coord_.lng_;
                error->lat_ = node->coord_.lat_;
                error->z_ = node->coord_.z_;
            }else{
                error->nodeId_ = "";
                error->attId_ = "";
                error->lng_ = 0.0;
                error->lat_ = 0.0;
                error->z_ = 0.0;
            }

            return error;
        }
    }
}

