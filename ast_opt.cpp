//
//  ast_opt.cpp
//
//  Copyright (c) 2021 Sergiy Yevtushenko. All rights reserved.
//  MIT License
//

#include <set>
#include <string>

std::set<std::string> filter_safe { // NOLINT(cert-err58-cpp)
        "BlockEnd",
        "BlockStart",
        "LP",
        "RP",
        "Semicolon",
        "SequenceSign",
        "eof"
};

std::set<std::string> filter_if_empty { // NOLINT(cert-err58-cpp)
        "OptionalEllipsis"
};

std::set<std::string> filter_unsafe { // NOLINT(cert-err58-cpp)
        "Arrow",
        "BitOr",
        "Colon",
        "Comma",
        "Eq",
        "GT",
        "LSqB",
        "LT",
        "RSqB",
};

std::set<std::string> unsafe_parents { // NOLINT(cert-err58-cpp)
        "AndType",
        "ApiRefs",
        "ArrayConstruction",
        "ArrowDecl",
        "ClassFields",
        "Comprehension",
        "ComprPipeline",
        "Constant",
        "ExpressionList",
        "ForSetup",
        "ImportList",
        "LimitedTypeName",
        "MatchCase",
        "MethodExpression",
        "MultiparamLambda",
        "NamedAssignment",
        "NamedAssignmentList",
        "NamedRange",
        "NameList",
        "OptionalEllipsis",
        "OptionalNameList",
        "OrType",
        "ParallelAssignment",
        "RequiredNameList",
        "SingleParamLambda",
        "TupleConstruction",
        "TupleDecl",
        "Type",
        "TypeArguments",
        "TypedArgList",
        "TypeNameList",
        "UnitDecl",
        "ValueReference"
};
