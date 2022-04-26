#ifndef JTS2GD_JS_TREE
#define JTS2GD_JS_TREE


// built-in
#include <vector>
#include <cstdint>

// local
#include "globals.hpp"




struct VarDecl;
struct Program;
struct FunctionCallPart;
struct ArrayIndexPart;
struct MemberAccessPart;
struct ConditionalExpr;
struct BinaryExpr;
struct UnaryExpr;
struct PrimaryExpr;
struct Block;
struct VarDeclStmt;
struct IfStmt;
struct WhileStmt;
struct ForStmt;
struct ContinueStmt;
struct BreakStmt;
struct ReturnStmt;
struct Case;
struct SwitchCaseStmt;
struct FunctionStmt;
struct ExpressionStmt;
struct EmptyStmt;
struct ExtendsStmt;
struct ClassExtendsStmt;
struct FunctionExpression;

struct Visitor
{
    virtual void visit(VarDecl&) = 0;
    virtual void visit(Program&) = 0;
    virtual void visit(FunctionCallPart&) = 0;
    virtual void visit(ArrayIndexPart&) = 0;
    virtual void visit(MemberAccessPart&) = 0;
    virtual void visit(ConditionalExpr&) = 0;
    virtual void visit(BinaryExpr&) = 0;
    virtual void visit(UnaryExpr&) = 0;
    virtual void visit(PrimaryExpr&) = 0;
    virtual void visit(Block&) = 0;
    virtual void visit(VarDeclStmt&) = 0;
    virtual void visit(IfStmt&) = 0;
    virtual void visit(WhileStmt&) = 0;
    virtual void visit(ForStmt&) = 0;
    virtual void visit(ContinueStmt&) = 0;
    virtual void visit(BreakStmt&) = 0;
    virtual void visit(ReturnStmt&) = 0;
    virtual void visit(Case&) = 0;
    virtual void visit(SwitchCaseStmt&) = 0;
    virtual void visit(FunctionStmt&) = 0;
    virtual void visit(ExpressionStmt&) = 0;
    virtual void visit(EmptyStmt&) = 0;
    virtual void visit(ExtendsStmt&) = 0;
    virtual void visit(ClassExtendsStmt&) = 0;
    virtual void visit(FunctionExpression&) = 0;
};

struct Element
{
    virtual void accept(Visitor&) = 0;
};








class Statement: public Element
{
    public:
        virtual ~Statement() = default;
};

class Expression: public Element
{
    public:
        virtual ~Expression() = default;
};


struct MemberExprPart: public Element
{
    virtual ~MemberExprPart() = default;
};


struct VarDecl: public Element
{
    const Token* var;
    const Token* type = nullptr;      // nullptr if no type has been specified
    Expression* init_value = nullptr; // nullptr if there is no initialization expression

    VarDecl() = default;

    VarDecl(VarDecl&& other)
    {
        this->var = other.var;
        this->type = other.type;
        this->init_value = other.init_value;

        other.init_value = nullptr;
    }

    void accept(Visitor& v) override
    {
        v.visit(*this);
    }

    virtual ~VarDecl() = default;
};



struct Program: public Element
{
    std::vector<Statement*> stmts;
    std::vector<FunctionExpression*> function_expressions;

    void accept(Visitor& v) override
    {
        v.visit(*this);
    }

    virtual ~Program() = default;
};


// ####################################################
// #                                                  #
// #                    Member                        #
// #                                                  #
// ####################################################



struct FunctionCallPart: public MemberExprPart
{
    std::vector<Expression*> args;

    void accept(Visitor& v) override
    {
        v.visit(*this);
    }
};

struct MemberAccessPart: public MemberExprPart
{
    const Token* member;

    void accept(Visitor& v) override
    {
        v.visit(*this);
    }
};

struct ArrayIndexPart: public MemberExprPart
{
    Expression* index = nullptr;

    void accept(Visitor& v) override
    {
        v.visit(*this);
    }
};




// ####################################################
// #                                                  #
// #                  Expressions                     #
// #                                                  #
// ####################################################


struct ConditionalExpr: public Expression
{
    Expression* cond = nullptr;
    Expression* expr1 = nullptr;
    Expression* expr2 = nullptr;

    void accept(Visitor& v) override
    {
        v.visit(*this);
    }
};

struct BinaryExpr: public Expression
{
    const Token* oprt;
    Expression* left = nullptr;
    Expression* right = nullptr;

    void accept(Visitor& v) override
    {
        v.visit(*this);
    }
};

struct UnaryExpr: public Expression
{
    const Token* oprt;
    Expression* value = nullptr;

    void accept(Visitor& v) override
    {
        v.visit(*this);
    }
};

enum class PrimaryExprType
{
    IDENTIFIER,
    EXPRESSION,
    LITERAL,
    ARRAY_LITERAL
};

struct PrimaryExpr: public Expression
{
    union
    {
        const Token* identifier = nullptr;
        const Token* literal;
        Expression* expr;
        std::vector<Expression*>* array_members;
    };

    PrimaryExprType type;
    std::vector<MemberExprPart*> parts;

    void accept(Visitor& v) override
    {
        v.visit(*this);
    }
};


// ####################################################
// #                                                  #
// #                   Statements                     #
// #                                                  #
// ####################################################


struct Block: public Statement
{
    std::vector<Statement*> stmts;

    void accept(Visitor& v) override
    {
        v.visit(*this);
    }
};


enum class VarDeclStmtType
{
    VAR,
    CONST
};

inline const char* VarDeclStmtTypeRepr[]
{
    "var",
    "const"
};

struct VarDeclStmt: public Statement
{
    std::vector<VarDecl*> decls;
    VarDeclStmtType type;

    void accept(Visitor& v) override
    {
        v.visit(*this);
    }
};

struct IfStmt: public Statement
{
    Expression* cond = nullptr;
    Statement* body = nullptr;
    Statement* else_block = nullptr; // nullptr if not used

    void accept(Visitor& v) override
    {
        v.visit(*this);
    }
};

struct WhileStmt: public Statement
{
    Expression* cond = nullptr;
    Statement* body = nullptr;

    void accept(Visitor& v) override
    {
        v.visit(*this);
    }
};


struct ForStmt: public Statement
{
    const Token* init_var_decl = nullptr;

    bool for_of = false;
    
    Expression* of_expr = nullptr;

    Statement* init_expr = nullptr;  // nullptr if not used
    Expression* cond = nullptr; // nullptr if not used
    Expression* post = nullptr; // nullptr if not used
    Statement* block = nullptr;

    void accept(Visitor& v) override
    {
        v.visit(*this);
    }
};


struct ContinueStmt: public Statement
{
    void accept(Visitor& v) override
    {
        v.visit(*this);
    }
};
struct BreakStmt: public Statement
{
    void accept(Visitor& v) override
    {
        v.visit(*this);
    }
};
struct ReturnStmt: public Statement
{

    Expression* value = nullptr; // nullptr case has no return value

    void accept(Visitor& v) override
    {
        v.visit(*this);
    }
};

struct Case: public Element
{
    std::vector<Expression*> comp_values; // nullptr if default clause
    std::vector<Statement*> stmts;

    void accept(Visitor& v) override
    {
        v.visit(*this);
    }

    virtual ~Case() = default;
};

struct SwitchCaseStmt: public Statement
{

    Expression* match_value = nullptr;
    std::vector<Case*> case_clauses;

    void accept(Visitor& v) override
    {
        v.visit(*this);
    }
};


struct FunctionStmt: public Statement
{
    const Token* name;
    std::vector<VarDecl*> params;      // nullptr if it has no parameters
    const Token* type;                 // nullptr if no type has been specified
    std::vector<Statement*> func_body;

    void accept(Visitor& v) override
    {
        v.visit(*this);
    }
};

struct ExpressionStmt: Statement
{
    Expression* expr = nullptr;

    void accept(Visitor& v) override
    {
        v.visit(*this);
    }
};

struct EmptyStmt: public Statement
{
    void accept(Visitor& v) override
    {
        v.visit(*this);
    }
};

struct ExtendsStmt: public Statement
{
    const Token* name;

    void accept(Visitor& v) override
    {
        v.visit(*this);
    }
};

struct ClassExtendsStmt: public Statement
{
    const Token* class_name; // useless for now
    const Token* extended;

    std::vector<Statement*> body;

    void accept(Visitor& v) override
    {
        v.visit(*this);
    }
};

struct FunctionExpression: Element
{
    Token name;
    Token literal;
    std::string name_value;
    std::string literal_value;
    std::vector<VarDecl*> params;      // nullptr if it has no parameters
    
    bool expression_body = false;
    Expression* expression = nullptr;
    std::vector<Statement*> func_body; 

    void accept(Visitor& v) override
    {
        v.visit(*this);
    }

    virtual ~FunctionExpression() = default;
};


#endif