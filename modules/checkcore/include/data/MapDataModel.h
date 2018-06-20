//
// Created by gaoyanhong on 2018/3/28.
//

#ifndef AUTOHDMAP_DATACHECK_MAPDATAMODEL_H
#define AUTOHDMAP_DATACHECK_MAPDATAMODEL_H

#include "CommonInclude.h"

namespace kd {
    namespace dc {

        struct DCRelField{
            string Table;
            string Field;
            string Value;
        };

        class DCFieldDefine;
        class DCFieldCheckDefine;
        class DCModelRecord;
        class DCRelationDefine;

        class DCModelDefine{

        public:
            shared_ptr<DCFieldDefine> getFieldDefine(string fieldName);
        public:
            //模型名称
            string modelName;

            //字段定义信息
            vector<shared_ptr<DCFieldDefine>> vecFieldDefines;

            //字段特殊检查项
            vector<shared_ptr<DCFieldCheckDefine>> vecFieldChecks;

            //关联关系信息
            vector<shared_ptr<DCRelationDefine>> vecRelation;

            //map<long,map<string,string>> mapRelation;

            vector<pair<vector<DCRelField>, vector<DCRelField>>> mapRelation;

        };

        enum DCFieldType{
            DC_FIELD_TYPE_VARCHAR = 0,
            DC_FIELD_TYPE_LONG = 1,
            DC_FIELD_TYPE_DOUBLE = 2,
            DC_FIELD_TYPE_TEXT = 3
        };

        class DCFieldDefine{
        public:
            //字段标识名（英文字母开头，可由英文字母、数字 和下划线组成）
            string name;

            //类型（varchar、long、double、text，默认为 varchar）
            DCFieldType type;

            //长度（仅类型为varchar有效）
            int len;

            //输入方式（text,select,checkbox,radio,textarea，默认为text ）
            string inputType;

            //默认值
            string defValue;

            //输入限制（0 无限制，1 不能为空）
            int inputLimit;

            //取值说明（或可选值，格式为“1：”）
            string valueLimit;
        };

        enum DCFieldValueFunc{
            DC_FIELD_VALUE_FUNC_NOLIMIT = 0,
            DC_FIELD_VALUE_FUNC_ID = 1,
            DC_FIELD_VALUE_FUNC_IN = 2,
            DC_FIELD_VALUE_FUNC_GT = 3,
            DC_FIELD_VALUE_FUNC_GE = 4,
            DC_FIELD_VALUE_FUNC_LT = 5,
            DC_FIELD_VALUE_FUNC_LE = 6
        };

        class DCFieldCheckDefine{
        public:
            //字段名
            string fieldName;

            //检查类型
            DCFieldValueFunc func;

            //检查参考值
            string refValue;
        };


        /**
         * 模型数据
         */
        class DCModalData{
        public:

            vector<shared_ptr<DCModelRecord>> records;

        };

        /**
         * 模型记录
         */
        class DCModelRecord{
        public:

            map<string,string> textDatas;

            map<string,double> doubleDatas;

            map<string,long> longDatas;
        };

        class DCRelationDefine{
        public:

            string member;

            string rule;
        };

    }
}

#endif //AUTOHDMAP_DATACHECK_MAPDATAMODEL_H
