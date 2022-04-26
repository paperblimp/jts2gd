#ifndef JTS2GD_JS_TREE_RELEASER
#define JTS2GD_JS_TREE_RELEASER


// built-in
#include <functional>
#include <type_traits>
#include <utility>
#include <cstdint>

// local
#include "tree.hpp"
#include "tree_traverse.hpp"


// Implementation of a Traverser to free the memory 
// of all nodes in the tree (after freeing all its child nodes)
struct ReleaserTraverserBase: public TraverserBase<ReleaserTraverserBase>
{
    template <typename T, typename = std::enable_if_t<std::is_base_of_v<Element, T>>>
    inline void pre(T*)
    {

    }

    template <typename T, typename = std::enable_if_t<std::is_base_of_v<Element, T>>>
    inline void post(T* element)
    {
        delete element;
    }
};


//  Specialization of the Releaser to guarantee 
//  the deletion of the pointer to the array member 
//  vector (if it exists) of a 'PrimaryExpr'
template <>
inline void ReleaserTraverserBase::post<PrimaryExpr>(PrimaryExpr* pexpr)
{
    if (pexpr->type == PrimaryExprType::ARRAY_LITERAL)
        delete pexpr->array_members;
    delete pexpr;
}


using Releaser = Traverser<ReleaserTraverserBase>;



inline void release_program(Program* prog)
{
    Releaser().visit(prog);
}


//  Functor to create a specific instantiation 
//  of 'unique_ptr' to ensure that the tree nodes 
//  are released correctly in the event of a syntax error.
template <typename T>
struct UniqueReleaser
{
    void operator()(T* ptr)
    {
        Releaser().visit(ptr);
    }
};

#endif