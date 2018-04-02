//
// Created by gaoyanhong on 2018/3/29.
//

#include "data/ErrorDataModel.h"

namespace kd {
    namespace dc {

        /////////////////////////////////////////////////////////////////////////////////////////
        // DCkError
        /////////////////////////////////////////////////////////////////////////////////////////

        DCError::DCError(string checkModel){
            checkModel_ = checkModel;
        }

        /////////////////////////////////////////////////////////////////////////////////////////
        // DCAttCheckError
        /////////////////////////////////////////////////////////////////////////////////////////
        DCAttCheckError::DCAttCheckError(string checkModel): DCError(checkModel){

        }

        DCAttCheckError::DCAttCheckError(string checkModel, string modelName, string fieldName, string recordId):
                DCError(checkModel){
            modelName_ = modelName;
            fieldName_ = fieldName;
            recordId_  = recordId;
        }

        string DCAttCheckError::getHeader(){
            return "checkModel,modelName,fieldName,recordId,errorDesc";
        }

        string DCAttCheckError::toString() {
            stringstream ss;
            ss << checkModel_ << "," << modelName_ << "," << fieldName_ << "," << recordId_ << "," << errorDesc_;
            return ss.str();
        }

        /////////////////////////////////////////////////////////////////////////////////////////
        // DCRelationCheckError
        /////////////////////////////////////////////////////////////////////////////////////////
        DCRelationCheckError::DCRelationCheckError(string checkModel) : DCError(checkModel){

        }

        DCRelationCheckError::DCRelationCheckError(string checkModel, string modelName, string fieldName,
                                                   string refModelName, string refFieldName):
                DCError(checkModel){
            modelName_ = modelName;
            fieldName_ = fieldName;
            refModelName_ = refModelName;
            refFieldName_ = refFieldName;
        }

        string DCRelationCheckError::getHeader(){
            return "checkModel,modelName,fieldName,refModelName,refFieldName,recordValue,errorDesc";
        }

        string DCRelationCheckError::toString() {
            stringstream ss;
            ss << checkModel_ << "," << modelName_ << "," << fieldName_ << "," << refModelName_;
            ss << "," << refFieldName_ << "," << recordValue_ << "," << errorDesc_;
            return ss.str();
        }


        /////////////////////////////////////////////////////////////////////////////////////////
        // DCDividerCheckError
        /////////////////////////////////////////////////////////////////////////////////////////
        DCDividerCheckError::DCDividerCheckError(string checkModel) : DCError(checkModel) {

        }

        string DCDividerCheckError::getHeader(){
            return "checkModel,dividerId,attId,nodeId,lng,lat,z,errorDesc";
        }

        string DCDividerCheckError::toString() {
            stringstream ss;
            ss << checkModel_<< "," << dividerId_ << "," << attId_ << "," << nodeId_;
            ss << "," << setprecision(12) << lng_ << "," << lat_ << "," << z_ << "," << errorDesc_;
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

