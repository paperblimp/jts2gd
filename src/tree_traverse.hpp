#ifndef JTS2GD_JS_TREE_TRAVERSE
#define JTS2GD_JS_TREE_TRAVERSE


// built-in
#include <type_traits>
#include <cstdint>

// local
#include "tree.hpp"



//
//  Visitor Traverser
//
//
//  Modular system to implement actions before and
//  after finding a certain node in the tree.
//
//  Useful for cases where only a few nodes need special treatment.
//
//  All nodes need to inherit from the 'Element' class 
//  to ensure the implementation of the 'accept' function.



template <typename Impl>
struct TraverserBase
{

    //
    // '_pre' && '_post'
    //
    //  Interface for the 'pre' and 'post' functions 
    //  that will be declared in the implementation class.


    template <typename T, typename = std::enable_if_t<std::is_base_of_v<Element, T>>>
    inline void _post(T* element)
    {
        static_cast<Impl&>(*this).template post<T>(element);
    }

    template <typename T, typename = std::enable_if_t<std::is_base_of_v<Element, T>>>
    inline void _pre(T* element)
    {
        static_cast<Impl&>(*this).template pre<T>(element);
    }
};



template <class Base>
class Traverser: public Visitor, public Base
{

    public:

        // Interface to visit other nodes that checks if the pointer is null.
        template <typename T, typename = std::enable_if_t<std::is_base_of_v<Element, T>>>
        inline void visit(T* element)
        {
            if (element != nullptr)
                element->accept(*this);
        }

    private:

        void visit(VarDecl& var_decl) override
        {
            this->_pre(&var_decl);

            if (var_decl.init_value)
                this->visit(var_decl.init_value);

            this->_post(&var_decl);
        }

        void visit(Program& prog) override
        {
            this->_pre(&prog);

            for (auto fexpr: prog.function_expressions)
                this->visit(fexpr);

            for (auto stmt: prog.stmts)
                this->visit(stmt);
            
            this->_post(&prog);
        }

        void visit(FunctionCallPart& fcall) override
        {
            this->_pre(&fcall);

            for (auto expr: fcall.args)
                this->visit(expr);
            
            this->_post(&fcall);
        }

        void visit(MemberAccessPart& maccess) override
        {
            this->_pre(&maccess);
            this->_post(&maccess);
        }

        void visit(ArrayIndexPart& arr_idx) override
        {
            this->_pre(&arr_idx);

            this->visit(arr_idx.index);
            
            this->_post(&arr_idx);
        }

        void visit(ConditionalExpr& cexpr) override
        {
            this->_pre(&cexpr);

            this->visit(cexpr.cond);
            this->visit(cexpr.expr1);
            this->visit(cexpr.expr2);
            
            this->_post(&cexpr);
        }

        void visit(BinaryExpr& bexpr) override
        {
            this->_pre(&bexpr);

            this->visit(bexpr.left);
            this->visit(bexpr.right);
            
            this->_post(&bexpr);
        }

        void visit(UnaryExpr& uexpr) override
        {
            this->_pre(&uexpr);

            this->visit(uexpr.value);
            
            this->_post(&uexpr);
        }

        void visit(PrimaryExpr& pexpr) override
        {
            this->_pre(&pexpr);

            for (auto part: pexpr.parts)
                this->visit(part);

            if (pexpr.type == PrimaryExprType::EXPRESSION)
                this->visit(pexpr.expr);

            else if (pexpr.type == PrimaryExprType::ARRAY_LITERAL)
                for (auto member: *pexpr.array_members)
                    this->visit(member);
            
            this->_post(&pexpr);
        }

        void visit(Block& blk) override
        {
            this->_pre(&blk);

            for (auto stmt: blk.stmts)
                this->visit(stmt);
            
            this->_post(&blk);
        }

        void visit(VarDeclStmt& vdecl_stmt) override
        {
            this->_pre(&vdecl_stmt);

            for (auto& decl: vdecl_stmt.decls)
                this->visit(decl);
            
            this->_post(&vdecl_stmt);
        }

        void visit(IfStmt& ifstmt) override
        {
            this->_pre(&ifstmt);

            this->visit(ifstmt.cond);
            this->visit(ifstmt.body);
            this->visit(ifstmt.else_block);
            
            this->_post(&ifstmt);
        }

        void visit(WhileStmt& wstmt) override
        {
            this->_pre(&wstmt);

            this->visit(wstmt.cond);
            this->visit(wstmt.body);
            
            this->_post(&wstmt);
        }

        void visit(ForStmt& fstmt) override
        {
            this->_pre(&fstmt);

            if (fstmt.for_of)
            {
                this->visit(fstmt.of_expr);
            }
            else
            {
                this->visit(fstmt.init_expr);
                this->visit(fstmt.cond);
                this->visit(fstmt.post);
            }
            
            this->visit(fstmt.block);

            this->_post(&fstmt);
        }

        void visit(ContinueStmt& cstmt) override
        {
            this->_pre(&cstmt);
            this->_post(&cstmt);
        }
        void visit(BreakStmt& bstmt) override
        {
            this->_pre(& bstmt);
            this->_post(& bstmt);
        }
        void visit(ReturnStmt& rstmt) override
        {
            this->_pre(& rstmt);
            this->visit(rstmt.value);
            this->_post(& rstmt);
        }

        void visit(Case& cs) override
        {
            this->_pre(&cs);

            for (auto expr: cs.comp_values)
                this->visit(expr);

            for (auto stmt: cs.stmts)
                this->visit(stmt);
            
            this->_post(&cs);
        }

        void visit(SwitchCaseStmt& scstmt) override
        {
            this->_pre(&scstmt);

            this->visit(scstmt.match_value);

            for (auto& c: scstmt.case_clauses)
                this->visit(c);
            
            this->_post(&scstmt);
        }

        void visit(FunctionStmt& fcall) override
        {
            this->_pre(&fcall);

            for (auto& vdecl: fcall.params)
                this->visit(vdecl);

            for (auto stmt: fcall.func_body)
                this->visit(stmt);
            
            this->_post(&fcall);
        }

        void visit(ExpressionStmt& expr) override
        {
            this->_pre(&expr);

            this->visit(expr.expr);
            
            this->_post(&expr);
        }

        void visit(EmptyStmt& estmt) override
        {
            this->_pre(&estmt);
            this->_post(&estmt);
        }

        void visit(ExtendsStmt& estmt) override
        {
            // Different implementation from the others due to a strange MSVC error.
            // Probably a template inferencing error,
            // So I made things a little more explicit to make the compiler work easier.
            
            this->template _pre<ExtendsStmt>(&estmt);
            this->template _post<ExtendsStmt>(&estmt);
        }

        void visit(ClassExtendsStmt& cestmt) override
        {
            this->_pre(&cestmt);

            for (auto stmt: cestmt.body)
                this->visit(stmt);
            
            this->_post(&cestmt);
        }

        void visit(FunctionExpression& fexpr) override
        {
            this->_pre(&fexpr);
            
            for (auto param: fexpr.params)
                this->visit(param);

            if (fexpr.expression_body)
                this->visit(fexpr.expression);
            else
                for (auto stmt: fexpr.func_body)
                    this->visit(stmt);
            
            this->_post(&fexpr);
        }
        
};



#endif
