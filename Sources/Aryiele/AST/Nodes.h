#ifndef ARYIELE_NODES_H
#define ARYIELE_NODES_H

namespace Aryiele
{
    enum Nodes
    {
        Nodes_Unknown,

        Nodes_Constant_Double,
        Nodes_Constant_Integer,

        Nodes_Function_Definition,
        Nodes_Function_Prototype,

        Nodes_Operation_Binary,

        Nodes_Statement_FunctionCall,
        Nodes_Statement_If,

        Nodes_Variable
    };

} /* Namespace Aryiele. */

#endif /* ARYIELE_NODES_H. */
