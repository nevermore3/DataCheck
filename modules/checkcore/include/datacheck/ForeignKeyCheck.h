//
// Created by ubuntu on 2019/9/2.
//

#ifndef AUTOHDMAP_DATACHECK_FOREIGNKEYCHECK_H
#define AUTOHDMAP_DATACHECK_FOREIGNKEYCHECK_H

#include "IModelProcessor.h"
namespace kd {
    namespace dc {

        class ForeignKeyCheck : public IModelProcessor {
        public:
            ForeignKeyCheck();

            ~ForeignKeyCheck();

            string getId() override;

            virtual bool execute(shared_ptr<ModelDataManager> modelDataManager,
                                 shared_ptr<CheckErrorOutput> errorOutput) override;

            void CheckForeignKeyExist(shared_ptr<ModelDataManager> modelDataManager,
                                      shared_ptr<CheckErrorOutput> errorOutput);

            void CheckForeignKeyIntegrity(shared_ptr<ModelDataManager> modelDataManager,
                                          shared_ptr<CheckErrorOutput> errorOutput);

            bool CheckForeignKey(shared_ptr<ModelDataManager> modelDataManager,
                                 shared_ptr<CheckErrorOutput> errorOutput,
                                 string modelName);

        private:
            const string id_ = "foreign key check";

            CppSQLite3::Database *pDataBase;
        };

    }
}


#endif //AUTOHDMAP_DATACHECK_FOREIGNKEYCHECK_H
