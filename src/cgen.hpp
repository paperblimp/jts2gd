#ifndef JTS2GD_CGEN
#define JTS2GD_CGEN


// built-in
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>

// local
#include "tree.hpp"





class Scope
{
    std::vector<std::unordered_set<std::string_view>> scope_hierarchy;

    public:
    
        void pop_level()
        {
            if (scope_hierarchy.size() == 0)
                panic("compiler internal error");

            this->scope_hierarchy.pop_back();
        }

        void push_level()
        {
            this->scope_hierarchy.push_back({});
        }

        void push_var_definition(std::string_view new_var)
        {
            this->scope_hierarchy.back().insert(new_var);
        }

        bool has_var(std::string_view var)
        {
            for (auto it = this->scope_hierarchy.rbegin(); it != this->scope_hierarchy.rend(); ++it)
                if (it->find(var) != it->end())
                    return true;

            return false;
        }
};








const std::unordered_map<std::string_view, std::string_view> type_table
{
    {"number", "float"},
    {"Number", "float"},

    {"string", "String"},
    {"String", "String"},

    {"Void", "void"},
    {"Null", "null"},

    {"Float", "float"},
    {"Int", "int"}
};


const std::unordered_map<std::string_view, std::string_view> function_table
{
    {"Number", "float"},
    {"Boolean", "bool"},
    {"Int", "int"},
    {"Float", "float"},

    {"push", "append"}
};


class GDScriptCGen: public Visitor
{

    private:

        uint32_t indentation = 0;
        bool func_id = false;
        Scope scope;

    public:

        std::string output;

    public:

        template <typename T, typename = std::enable_if_t<std::is_base_of_v<Element, T>>>
        void visit(T* element)
        {
            if (element != nullptr)
                element->accept(*this);
        }

    private:

        void indent(int offset = 0)
        {
            if (int64_t(this->indentation) + offset >= 0)
                this->output.append(std::string(4 * (this->indentation + offset), ' '));
        }

        void line_feed()
        {
            this->output.push_back('\n');
        }

        void visit(VarDecl&) override;
        void visit(Program&) override;
        void visit(FunctionCallPart&) override;
        void visit(ArrayIndexPart&) override;
        void visit(MemberAccessPart&) override;
        void visit(ConditionalExpr&) override;
        void visit(BinaryExpr&) override;
        void visit(UnaryExpr&) override;
        void visit(PrimaryExpr&) override;
        void visit(Block&) override;
        void visit(VarDeclStmt&) override;
        void visit(IfStmt&) override;
        void visit(WhileStmt&) override;
        void visit(ForStmt&) override;
        void visit(ContinueStmt&) override;
        void visit(BreakStmt&) override;
        void visit(ReturnStmt&) override;
        void visit(Case&) override;
        void visit(SwitchCaseStmt&) override;
        void visit(FunctionStmt&) override;
        void visit(ExpressionStmt&) override;
        void visit(EmptyStmt&) override;
        void visit(ExtendsStmt&) override;
        void visit(ClassExtendsStmt&) override;
        void visit(FunctionExpression&) override;


        void render_primary_expression(PrimaryExpr& pexpr, bool render_init, uint32_t render_start, uint32_t render_end);
        std::string_view translate_function(const std::string_view&);
        std::string_view translate_type(const std::string_view&);

};


inline std::string gen_gdscript(Program* prog)
{
    GDScriptCGen generator;
    generator.visit(prog);

    return generator.output;
}

#endif

