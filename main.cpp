//
//  main.cpp
//
//  Copyright (c) 2020 Sergiy Yevtushenko. All rights reserved.
//  MIT License
//

#include <functional>
#include <fstream>
#include <sstream>
#include "peglib.h"

std::function<void(size_t, size_t, const std::string &)> makeErrorLogger(std::string &errorText) {
    return [&](size_t ln, size_t col, const std::string &msg) mutable {
        errorText += "  at " + std::to_string(ln) + ":" + std::to_string(col) + "::" + msg + "\n";
    };
}

bool parse_grammar(const std::string &text, peg::parser &peg, std::string &errorText) {
    peg.log = makeErrorLogger(errorText);
    return peg.load_grammar(text.c_str(), text.size());
}

bool parse_code(const std::string &text, peg::parser &peg, std::string &errorText, std::shared_ptr<peg::Ast> &ast) {
    peg.enable_ast();
    peg.log = makeErrorLogger(errorText);
    return peg.parse_n(text.data(), text.size(), ast);
}

std::string lint(const std::string &grammarText, const std::string &codeText, std::string &fileName, bool opt_mode = true) {
    std::string grammarErrors;
    std::string codeErrors = fileName + "\n";
    std::string astText;
    std::string astOptimizedText;

    peg::parser peg;
    auto ret = parse_grammar(grammarText, peg, grammarErrors);

    if (ret && peg) {
        std::shared_ptr<peg::Ast> ast;
        parse_code(codeText, peg, codeErrors, ast);
        if (ast) {
            astText = peg::ast_to_s(ast);
            astOptimizedText = peg::ast_to_s(peg.optimize_ast(ast, opt_mode));
        }
    }

    std::string json;
    json += "Grammar errors: " + grammarErrors + "\n";

    if (!codeErrors.empty()) {
        json += "Code errors: " + codeErrors + "\n";
        json += "AST (Optimized):\n" + astOptimizedText;
    }

    return json;
}

int main(int argc, const char **argv) {
    std::string grammarText = (R"(
Program <- Use* Constant* Type* Extension* Function* Unsafe*

Use <- _ 'use' QualifiedName ImportList? Semicolon
Constant <- AccessModifier? _ 'const' (KW_val / TypeName) Name Eq Expression Semicolon
Type <- AnnotationRef* AccessModifier? KW_type TypeName Eq (ApiDecl / AnnotationDecl / CompoundTypeDecl / TupleDecl / TypeAliasDecl / ClassDecl)
Extension <- AccessModifier? _ 'extend' TypeName ApiRefs? ClassBlock
Function <- AnnotationRef* AccessModifier? MethodSignature MethodBody
Unsafe <- _ 'unsafe' BlockStart Constant* Type* Function* BlockEnd

ImportList <- BlockStart NameWithAlias (Comma NameWithAlias)* BlockEnd
NameWithAlias <- Name (_ 'as'  Name)?
AnnotationRef <- _ '@' Name (LP Expression? (Comma Expression)* RP)?

TupleDecl <- LP TypeName? (Comma TypeName)* (Comma Ellipsis)? RP Semicolon
ApiDecl <- _ 'api' CommonApiBody
TypeAliasDecl <- ArgumentType Semicolon

AnnotationDecl <- (KW_type / KW_fn) _ 'annotation' DataDecl BlockStart Statement* BlockEnd

CompoundTypeDecl <- TypeName ( AndType / OrType)? (Semicolon / CommonApiDecl)
AndType <- (Comma TypeName)+
OrType <- (BitOr TypeName)+

CommonApiDecl <- ApiRefs? CommonApiBody
ApiRefs <- Colon TypeNameList

CommonApiBody <- BlockStart Constant* (MethodDecl / StaticMethodImpl)* NamedClassDecl* BlockEnd

MethodDecl <- MethodSignature Semicolon

MethodSignature <- TypeVarsDecl? (TypedFunction/UntypedFunction)
TypedFunction <- TypeName Name LP TypedArgList? RP
UntypedFunction <- KW_fn Name OptionalNameList
OptionalNameList <- LP NameList? RP
ClassDecl <- KW_class ClassBody

NamedClassDecl <- AccessModifier? KW_class TypeName ClassBody
ClassBody <- DataDecl? ApiRefs? ClassBlock
DataDecl <- LP TypedArgList RP
ClassBlock <- BlockStart Constant* Constructor* Destructor? (StaticMethodImpl / Function / MethodImpl)* NamedClassDecl* BlockEnd
StaticMethodImpl <- AccessModifier? _ 'static' MethodSignature MethodBody
Constructor <- AccessModifier? Name LP TypedArgList? RP MethodBody
Destructor <- AccessModifier? _ '#' Name LP RP MethodBody
AccessModifier <- _ 'pub'

MethodImpl <- _ 'impl' Name OptionalNameList MethodBody
MethodBody <- MethodExpression / MethodBlockBody
MethodExpression <- Eq Expression Semicolon

MethodBlockBody <- BlockStart Statement* BlockEnd
Statement <- (ForStatement / IfExpression / SelectExpression / MatchExpression / ReturnStatement / VariableDeclaration / Assignment / IterableStatement / MethodBlockBody / ValueStatement)
ValueStatement <- Value Semicolon
VariableDeclaration <- VarTitle Assignment
VarTitle <- KW_val / KW_var / TypeName
Assignment <- AssignmentExpr Semicolon
AssignmentExpr <- RegularAssignment / ParallelAssignment
RegularAssignment <- QualifiedName AssignOps Expression
ParallelAssignment <- RequiredNameList Eq (TupleConstruction / Expression)
TupleConstruction <- LP Expression? (Comma Expression)* (Comma Ellipsis)? RP
RequiredNameList <- LP NameList RP
ForStatement <- KW_for LP VarTitle Name Colon Expression RP MethodBlockBody
ReturnStatement <- KW_return Expression? Semicolon
IterableStatement <- Iterable Call* Semicolon
Call <- MethodCall / CallParameters / Index
MethodCall <- Dot QualifiedName Call?

TypeName <- Name TypeArguments? BitAnd? ArrayDecl*
ArrayDecl <- LSqB RSqB
TypeArguments <- LT TypeNameList (Comma Ellipsis)? GT
TypeVarsDecl <- LT LimitedTypeName (Comma LimitedTypeName)* GT
LimitedTypeName <- Name (Colon TypeName ('+'/'-')?)?
TypeNameList <- TypeName (Comma TypeName)*
TypedArgList <- TypedArgument (Comma TypedArgument)* (Comma Ellipsis)?
TypedArgument <- ArgumentType Name
ArgumentType <- TypeName (Arrow Ellipsis? TypeName)*

NameList <- Name (Comma Name)*

Expression <- IfExpression / SelectExpression / MatchExpression / TernaryExpression / KW_break / KW_continue / Expr

IfExpression <- KW_if NestedExpr MethodBlockBody (KW_else MethodBlockBody)?
SelectExpression <- KW_select NestedExpr BlockStart SelectCase* Default? BlockEnd
MatchExpression <- KW_match NestedExpr BlockStart MatchCase* Default? BlockEnd
TernaryExpression <- NestedExpr _ '?' Expression Colon Expression

SelectCase <- KW_case CaseExpr Arrow Expression Semicolon
MatchCase <- TypeName Arrow Expression Semicolon
Default <- KW_default Arrow Expression Semicolon
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
Product <- (Cast / UnaryOps)* (MethodReference / Value / NestedExpr / Ellipsis)
Cast <- LP TypeName RP
MethodReference <- QualifiedName? DoubleColon Name

Value <- LiteralValue Call* PostfixOps?
LiteralValue <- CharLiteral / NumLiteral / StringLiteral / Iterable / ObjConstruction / ArrayConstruction / TupleConstruction / Lambda / QualifiedName
Index <- LSqB Expression RSqB
Lambda <- (TypeName? CallParameters / Name) Arrow Ellipsis? (Expression / MethodBlockBody)
NestedExpr <- LP Expression RP
ObjConstruction <- TypeName? BlockStart (Expression BitOr)? AssignmentExpr (Comma AssignmentExpr)* BlockEnd
ArrayConstruction <- LSqB (Expression (Comma Expression)*)? RSqB

Iterable <- Range / Comprehension
Range <- LSqB NumLiteral SequenceSign NumLiteral (Comma NumLiteral)? RSqB
OpenRange <- LSqB NumLiteral SequenceSign (NumLiteral (Comma NumLiteral)?)? RSqB
Comprehension <- LSqB Expr BitOr Name LeftArrow OpenRange (ProductCompr / ParallelCompr) (OutputCondition)?RSqB
ProductCompr <- (Comma Name LeftArrow OpenRange)*
ParallelCompr <- (BitOr Name LeftArrow OpenRange)*
OutputCondition <- Comma Expression
CallParameters <- LP (Expression (Comma Expression)* (Comma Ellipsis)?)? RP

SumOps <- _ < ('+'/'-') >
MulOps <- _ < ('*' / '/' / '%') >
UnaryOps <- _ < ('-' / '+' / '--' / '++' / '!' / '~') >
PostfixOps <- _ < ('--' / '++') >
AssignOps <- _ < (Eq / '+=' / '-=' / '^=' / '|=' / '&=' ) >
EqualityOps <- _ < ('==' / '!=') >
CompareOps <- _ < ('<=' / '>=' / LT / GT / KW_isA) >
ShiftOps <- _ < ('<<' / '>>' / '>>>') >

NumLiteral <- (HexNumLiteral / OctNumLiteral / BinNumLiteral / FloatNumLiteral / DecNumLiteral) NumSuffix?

HexNumLiteral <- _ <'0x' [0-9a-fA-F] [0-9a-fA-F_]* > _
OctNumLiteral <- _ <'0o' [0-7] [0-7_]* > _
BinNumLiteral <- _ <'0b' [01] [01_]* > _
FloatNumLiteral <- _ < DecSegment _ '.' DecSegment _ (('e'/'E') _ ('+' / '-')? DecSegment _)? > _
DecNumLiteral <- _ <DecSegment> _
DecSegment <- [0-9][0-9_]*

NumSuffix <- 'i8' / 'u8' / 'i16' / 'u16' / 'i32' / 'u32' / 'i64' / 'u64' / 'f16' / 'f32' / 'f64'

QualifiedName <- _ < Name (Dot Name )* > _
Name <- _ < NameSegment > _
NameSegment <- [_$@a-zA-Z] [_$a-zA-Z0-9]*

KW_break <- _ 'break'
KW_case <- _ 'case'
KW_class <- _ 'class'
KW_continue <- _ 'continue'
KW_default <- _ 'default'
KW_else <- _ 'else'
KW_fn <- _ 'fn'
KW_for <- _ 'for'
KW_if <- _ 'if'
KW_isA <- _ 'isA'
KW_match <- _ 'match'
KW_return <- _ 'return'
KW_select <- _ 'select'
KW_type <- _ 'type'
KW_val <- _ 'val' _
KW_var <- _ 'var'

StringLiteral <- StringSegment*
CharLiteral <- _ < [\'] < ('\'' / ![\'] .) > [\'] > _
StringSegment <- _ < ["] < ('\\"' / !["] .)* > ["] > _
Semicolon <- _ ';' _
Ellipsis <- _ '...'
Arrow <- _ '->'
LeftArrow <- _ '<-'
LogicalAnd <- _ '&&'
LogicalOr <- _ '||'
BitAnd <- _ '&'
BitOr <- _ '|'
BitXor <- _ '^'
Comma <- _ ','
BlockStart <- _ '{'
BlockEnd <- _ '}' _
SequenceSign <- _ '..'
LP <- _ '('
RP <- _ ')'
LSqB <- _ '['
RSqB <- _ ']'
Dot <- _ '.'
Colon <- _ ':'
DoubleColon <- _ <'::' > _
Eq <- _ '='
LT <- _ '<'
GT <- _ '>'
~_ <- [ \t\r\n]* Comment?
Comment <- '//' (!End .)* &End _?
End <- Eol / Eof
Eol <- '\r\n' / '\n' / '\r'
Eof <- !.
    )");

    if (argc > 1) {
        std::string  fileName = argv[1];
        std::ifstream ifs(fileName);
        std::string code((std::istreambuf_iterator<char>(ifs)),
                         (std::istreambuf_iterator<char>()));

        std::cout << lint(grammarText, code, fileName, true) << std::endl;
    } else {
        std::cout << "Usage: cbc \"text\" " << std::endl;
    }
    return 0;
}
