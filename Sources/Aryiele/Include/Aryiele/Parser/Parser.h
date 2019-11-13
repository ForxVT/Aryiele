//==================================================================================//
//                                                                                  //
//  Copyright (c) 2019 Hugo Kindel <kindelhugo.pro@gmail.com>                       //
//                                                                                  //
//  This file is part of the Aryiele project.                                       //
//  Licensed under MIT License:                                                     //
//                                                                                  //
//  Permission is hereby granted, free of charge, to any person obtaining           //
//  a copy of this software and associated documentation files (the "Software"),    //
//  to deal in the Software without restriction, including without limitation       //
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,        //
//  and/or sell copies of the Software, and to permit persons to whom the           //
//  Software is furnished to do so, subject to the following conditions:            //
//                                                                                  //
//  The above copyright notice and this permission notice shall be included in      //
//  all copies or substantial portions of the Software.                             //
//                                                                                  //
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR      //
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        //
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE     //
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER          //
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,   //
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE   //
//  SOFTWARE.                                                                       //
//                                                                                  //
//==================================================================================//

#ifndef ARYIELE_PARSER_PARSER_H
#define ARYIELE_PARSER_PARSER_H

#include <vector>
#include <Vanir/Module/Module.h>
#include <Aryiele/Common.h>
#include <Aryiele/Lexer/LexerToken.h>
#include <Aryiele/AST/Nodes/Node.h>
#include <Aryiele/AST/Nodes/NodeFunction.h>
#include <Aryiele/Parser/ParserToken.h>
#include <Aryiele/Parser/ParserInformation.h>

namespace Aryiele {
class Parser : public Vanir::Module<Parser> {
    public:
        Parser();
    
    std::vector<std::shared_ptr<Node>> parse(std::vector<ParserToken> tokens);
        std::vector<ParserToken> convertTokens(std::vector<LexerToken> tokenizerTokens);
        static std::string getTokenName(ParserTokenEnum tokenType);
        ParserToken getCurrentToken();

    private:
        ParserToken getNextToken();
        ParserToken getPreviousToken();
        int getOperatorPrecedence(const std::string& binaryOperator);
        std::shared_ptr<NodeFunction> parseFunction();
        std::shared_ptr<Node> parsePrimary();
        std::shared_ptr<Node> parseExpression();
        std::shared_ptr<Node> parseBinaryOperation(int expressionPrecedence, std::shared_ptr<Node> leftExpression);
        std::vector<std::shared_ptr<Node>> parseBody();
        std::shared_ptr<Node> parseInteger();
        std::shared_ptr<Node> parseDouble();
        std::shared_ptr<Node> parseIdentifier();
        std::shared_ptr<Node> parseParenthese();
        std::shared_ptr<Node> parseReturn();
        std::shared_ptr<Node> parseIf();
        std::shared_ptr<Node> parseBlock();
        std::shared_ptr<Node> parseVariableDeclaration();
    
        std::map<std::string, int> m_binaryOperatorPrecedence;
        std::vector<std::shared_ptr<Node>> m_nodes;
        std::vector<ParserToken> m_tokens;
        ParserToken m_currentToken;
        int m_currentTokenIndex = -1;
    };
    
    Parser &getParser();

} /* Namespace Aryiele. */

#define PARSER_CHECKTOKEN(EXPECTEDTOKENTYPE) \
if (getParser().getCurrentToken().type != EXPECTEDTOKENTYPE) { \
    LOG_ERROR("wrong token, got '", Parser::getTokenName(m_currentToken.type), "' but expected '", Parser::getTokenName(EXPECTEDTOKENTYPE), "'"); \
    return nullptr; \
}

#define PARSER_CHECKNEXTTOKEN(EXPECTEDTOKENTYPE) { \
    getNextToken(); \
    PARSER_CHECKTOKEN(EXPECTEDTOKENTYPE); \
}

#endif /* ARYIELE_PARSER_PARSER_H. */
