#ifndef ARYIELE_NODE_H
#define ARYIELE_NODE_H

#include <Aryiele/Core/Includes.h>
#include <Aryiele/Parser/ParserInformation.h>
#include <Aryiele/CodeGeneration/CodeGenerator.h>

namespace Aryiele
{
    class Node
    {
    public:
        virtual ~Node();

        virtual llvm::Value* GenerateCode();
        virtual void DumpInformations(std::shared_ptr<ParserInformation> parentNode);
    };

} /* Namespace Aryiele. */

#endif /* ARYIELE_NODE_H. */