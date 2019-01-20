#include <Aryiele/Parser/AST/ExpressionDoubleNode.h>

namespace Aryiele
{
    ExpressionDoubleNode::ExpressionDoubleNode(double value) :
        m_value(value)
    {

    }

    void ExpressionDoubleNode::DumpInformations(std::shared_ptr<ParserInformation> parentNode)
    {
        auto node = std::make_shared<ParserInformation>(parentNode, "Double");
        auto body = std::make_shared<ParserInformation>(node, "Value: " + std::to_string(m_value));

        node->Children.emplace_back(body);
        parentNode->Children.emplace_back(node);
    }

} /* Namespace Aryiele. */