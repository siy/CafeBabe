//
//  main.cpp
//
//  Copyright (c) 2020 Sergiy Yevtushenko. All rights reserved.
//  MIT License
//

#include <assert.h>
#include <iostream>
#include "peglib.h"

using namespace std;
using namespace peg;

int main() {
    parser parser(R"(
        Program <- Uses (Type/Function/Constant/Unsafe/Extension)*
        Uses <- Use*
        Use <- _ 'use' PackageName Semicolon
        PackageName <- Name ('.' Name )*
        Unsafe <- _ 'unsafe' BlockStart (Type/Function)* BlockEnd
        Extension <- _ 'extend' TypeName ApiRefs? ClassBlock
        Type <- _ 'type' TypeName '=' (ApiDecl / CompoundTypeDecl / TypeAliasDecl / ClassDecl)
        Function <- MethodSignature MethodBody
        ApiDecl <- _ 'api' CommonApiBody
        TypeAliasDecl <- ArgumentType Semicolon
        CompoundTypeDecl <- TypeName ( AndType / OrType)? (Semicolon / CommonApiDecl)
        AndType <- (Comma TypeName)+
        OrType <- (BitOr TypeName)+
        CommonApiDecl <- ApiRefs? CommonApiBody
        ApiRefs <- _ ':' CommonApiRefs
        CommonApiRefs <- TypeNameList
        CommonApiBody <- BlockStart MethodDecl* (StaticMethodImpl / NamedClassDecl / Constant)* BlockEnd
        MethodDecl <- MethodSignature Semicolon
        MethodSignature <- TypeVarsDecl? (_ 'fn' / TypeName) Name '(' _ TypedArgList? ')' 
        ClassDecl <- _ 'class' DataDecl? ApiRefs? (ClassBlock / Semicolon)
        NamedClassDecl <- _ 'class' TypeName ClassBody
        ClassBody <- DataDecl? ApiRefs? ClassBlock
        ClassBlock <-  BlockStart Constant* (Function / MethodImpl)* BlockEnd
        DataDecl <- _ '(' TypedArgList ')'
        #TODO: use of 'var' is allowed by grammar but should be denied by compiler
        Constant <- _ 'const' VariableDeclaration
        StaticMethodImpl <- _ 'static' MethodSignature MethodBody
        MethodImpl <- _ 'impl' Name '(' NameList? ')' MethodBody
        MethodBody <- MethodExpression / MethodBlockBody
        MethodExpression <- _ '=' Expression Semicolon
        MethodBlockBody <- BlockStart Statement* BlockEnd
        Statement <- ForStatement / IfExpression / SelectExpression / MatchExpression / ReturnStatement / VariableDeclaration / Assignment / IterableStatement / MethodBlockBody
        VariableDeclaration <- _ VarTitle Assignment
        VarTitle <- 'val' / 'var' / TypeName
        Assignment <- AssignmentExpr Semicolon
        AssignmentExpr <-  RegularAssignment / ParallelAssignment
        RegularAssignment <- Name AssignOps Expression
        ParallelAssignment <- _ '(' NameList ')' _ '=' (TupleDef / Expression)
        TupleDef <- _ '(' Expression (Comma Expression) ')' _
        ForStatement <- _ 'for' _ '(' VarTitle Name ':' Expression ')' MethodBlockBody
        FunctionType <- TypeName (Arrow TypeName)+
        ReturnStatement <- _ 'return' Expression? Semicolon
        IterableStatement <- _ Iterable MethodCall? Semicolon
        TypeName <- Name TypeArguments? ReferenceDecl?
        ReferenceDecl <- _ '&' _
        TypeArguments <- _ '<' TypeNameList Ellipsis? '>' _
        TypeVarsDecl <- _ '<' NameList '>' _
        TypeNameList <- TypeName (Comma TypeName)*
        TypedArgList <- TypedArgument (Comma TypedArgument)* Ellipsis?
        TypedArgument <- ArgumentType Name
        ArgumentType <- TypeName (Arrow TypeName)*
        NameList <- Name (Comma Name)*
        #TODO: incomplete
        Expression <- IfExpression / SelectExpression / MatchExpression / TernaryExpression / Expr
        IfExpression <- _ 'if' _ NestedExpr MethodBlockBody (_ 'else' MethodBlockBody)?
        SelectExpression <- _ 'select' NestedExpr BlockStart SelectCase* Default? BlockEnd
        MatchExpression <- _ 'match' NestedExpr BlockStart MatchCase* Default? BlockEnd
        TernaryExpression <- _ NestedExpr _ '?' Expr _ ':' Expr
        #TODO: might be incorrect
        SelectCase <- _ 'case' CaseExpr Arrow Expr Semicolon
        MatchCase <- _  TypeName Arrow Expr Semicolon
        Default <- _ 'default' Arrow Expr Semicolon
        CaseExpr <- (NumLiteral / StringLiteral / Iterable / ObjConstruction)
        Expr <- Or (LogicalOr Or)*
        Or <- And (LogicalAnd And)*
        And <- BitwiseOr (BitOr BitwiseOr)*
        BitwiseOr <- BitwiseXor (BitXor BitwiseXor)*
        BitwiseXor <- BitwiseAnd (BitAnd BitwiseAnd)*
        BitwiseAnd <- Equality (EqualityOps Equality)*
        Equality <- Compare (CompareOps Compare)*
        Compare <- Shift (ShiftOps Shift)*
        Shift <- Sum (SumOps Sum)*
        Sum <- Product (MulOps Product)*
        Product <- (Cast / UnaryOps)* UnaryValue
        UnaryValue <-  ( Value / NestedExpr) PostfixOps?
        Cast <- _ '(' TypeName ')'
        Value <- LiteralValue Call?
        LiteralValue <- NumLiteral / StringLiteral / Iterable / ObjConstruction / Lambda / Name
        Call <- (MethodCall / CallParameters / Name)*
        MethodCall <- _ '.' Call
        Lambda <- (CallParameters / Name) Arrow (Expression / MethodBlockBody)
        NestedExpr <- _ '(' Expr ')' 
        ObjConstruction <- _ TypeName? BlockStart (Expr BitOr)? AssignmentExpr (',' AssignmentExpr)* BlockEnd
        SumOps <-  _ < ('+'/'-') >
        MulOps <-  _ < ('*' / '/' / '%') >
        UnaryOps <-  _ < ('-' / '+' / '--' / '++' / '!' / '~') >
        PostfixOps <-  _ < ('--' / '++') >
        AssignOps <- _ < ('=' / '+=' / '-=' / '^=' / '|=' / '&=' ) >
        EqualityOps <- _ < ('==' / '!=') >
        CompareOps <- _ < ('<=' / '>=' / '<' / '>' / 'isA') >
        ShiftOps <-  _ < ('<<' / '>>' / '>>>') >
        #TODO: incomplete
        Iterable <- _ '[' (Range / Comprehension) ']' _
        Range <- NumLiteral '..' NumLiteral (_ ',' NumLiteral)?
        Comprehension <- 
        CallParameters <- _ '(' (Expr (Comma Expr)*)? ')'
        BlockStart <- _ '{' _
        BlockEnd <- _ '}' _
        NumLiteral <- _ < [0-9_]+ ('.'[0-9_]+)?NumSuffix? > _
        NumSuffix <- 'i8' / 'u8' / 'i16' / 'u16' / 'i32' / 'u32' / 'i64' / 'u64' / 'f16' / 'f32' / 'f64'
        Name <- _ < [_$a-zA-Z] [_$a-zA-Z0-9]* > _
        StringLiteral <- _ < ["] < (!["] .)* > ["] >
        Semicolon <- _ ';'_
        Ellipsis <- _ '...' _
        Arrow <- _ '->' _
        LogicalAnd <- _ '&&' _
        LogicalOr <- _ '||' _
        BitAnd <- _ '&' _
        BitOr <- _ '|' _
        BitXor <- _ '^' _
        Comma <- _ ',' _
        ~_ <- [ \t\r\n]* Comment?
        Comment <- '//' (!End .)* &End _?
        End <- Eol / Eof
        Eol <- '\r\n' / '\n' / '\r'
        Eof <- !.
    )");

    assert(parser);

    parser.enable_ast();

    for (auto& rule : parser.get_rule_names())
        std::cout << rule << ' ';

    return 0;
}