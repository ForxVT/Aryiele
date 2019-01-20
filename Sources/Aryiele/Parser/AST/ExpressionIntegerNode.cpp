#include <Aryiele/Parser/AST/ExpressionIntegerNode.h>

namespace Aryiele
{
    ExpressionIntegerNode::ExpressionIntegerNode(int value) :
            m_value(value)
    {

    }

    void ExpressionIntegerNode::DumpInformations(std::shared_ptr<ParserInformation> parentNode)
    {
        auto node = std::make_shared<ParserInformation>(parentNode, "Integer");
        auto body = std::make_shared<ParserInformation>(node, "Value: " + std::to_string(m_value));

        node->Children.emplace_back(body);
        parentNode->Children.emplace_back(node);
    }

} /* Namespace Aryiele. */