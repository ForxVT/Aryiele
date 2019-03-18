#include "ParserPrecedence.h"

namespace Aryiele
{
    // TODO: Finish it.
    ParserPrecedence::ParserPrecedence()
    {
        m_binaryOperatorPrecedence[";"] = -1;

        m_binaryOperatorPrecedence["<"] = 1;
        m_binaryOperatorPrecedence[">"] = 1;
        m_binaryOperatorPrecedence["<="] = 1;
        m_binaryOperatorPrecedence[">="] = 1;

        m_binaryOperatorPrecedence["+"] = 2;
        m_binaryOperatorPrecedence["-"] = 2;

        m_binaryOperatorPrecedence["*"] = 4;
        m_binaryOperatorPrecedence["/"] = 4;
    }

    int ParserPrecedence::GetPrecedence(const std::string& binaryOperator)
    {
        if(m_binaryOperatorPrecedence.find(binaryOperator) == m_binaryOperatorPrecedence.end())
            return -1;
        else
            return m_binaryOperatorPrecedence[binaryOperator] <= 0 ? -1 : m_binaryOperatorPrecedence[binaryOperator];
    }

} /* Namespace Aryiele. */