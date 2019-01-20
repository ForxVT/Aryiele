#ifndef ARYIELE_EXPRESSIONINTEGERNODE_H
#define ARYIELE_EXPRESSIONINTEGERNODE_H

#include <Aryiele/Core/Includes.h>
#include <Aryiele/Parser/AST/ExpressionNode.h>

namespace Aryiele
{
    class ExpressionIntegerNode : public ExpressionNode
    {
    public:
        explicit ExpressionIntegerNode(int value);

        void DumpInformations(std::shared_ptr<ParserInformation> parentNode) override;

    protected:
        int m_value;
    };

} /* Namespace Aryiele. */

#endif /* ARYIELE_EXPRESSIONINTEGERNODE_H. */