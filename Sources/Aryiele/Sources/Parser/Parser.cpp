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

#include <utility>
#include <llvm/ADT/STLExtras.h>
#include <Aryiele/Parser/Parser.h>
#include <Aryiele/AST/Nodes/NodeLiteralNumberFloating.h>
#include <Aryiele/AST/Nodes/NodeLiteralNumberInteger.h>
#include <Aryiele/AST/Nodes/NodeLiteralString.h>
#include <Aryiele/AST/Nodes/NodeLiteralCharacter.h>
#include <Aryiele/AST/Nodes/NodeOperationBinary.h>
#include <Aryiele/AST/Nodes/NodeStatementBlock.h>
#include <Aryiele/AST/Nodes/NodeStatementFunctionCall.h>
#include <Aryiele/AST/Nodes/NodeStatementIf.h>
#include <Aryiele/AST/Nodes/NodeStatementReturn.h>
#include <Aryiele/AST/Nodes/NodeStatementVariableDeclaration.h>
#include <Aryiele/AST/Nodes/NodeStatementWhile.h>
#include <Aryiele/AST/Nodes/NodeStatementFor.h>
#include <Aryiele/AST/Nodes/NodeOperationUnary.h>
#include <Aryiele/AST/Nodes/NodeOperationTernary.h>
#include <Aryiele/AST/Nodes/NodeStatementVariable.h>
#include <Aryiele/AST/Nodes/NodeTopNamespace.h>
#include <Aryiele/AST/Nodes/NodeStatementBreak.h>
#include <Aryiele/AST/Nodes/NodeStatementContinue.h>
#include <Aryiele/AST/Nodes/NodeStatementSwitch.h>
#include <Aryiele/AST/Nodes/NodeLiteralArray.h>
#include <Aryiele/AST/Nodes/NodeStatementArrayCall.h>
#include <Aryiele/AST/Nodes/NodeStatementCase.h>
#include <Aryiele/AST/Nodes/NodeTopFile.h>

namespace Aryiele {
    Parser::Parser() {
        m_binaryOperatorPrecedence[ParserToken_OperatorQuestionMark] = 5;
        m_binaryOperatorPrecedence[ParserToken_OperatorEqual] = 10;
        m_binaryOperatorPrecedence[ParserToken_OperatorArithmeticPlusEqual] = 10;
        m_binaryOperatorPrecedence[ParserToken_OperatorArithmeticMinusEqual] = 10;
        m_binaryOperatorPrecedence[ParserToken_OperatorArithmeticMultiplyEqual] = 10;
        m_binaryOperatorPrecedence[ParserToken_OperatorArithmeticDivideEqual] = 10;
        m_binaryOperatorPrecedence[ParserToken_OperatorArithmeticRemainderEqual] = 10;
        m_binaryOperatorPrecedence[ParserToken_OperatorLogicalAnd] = 20;
        m_binaryOperatorPrecedence[ParserToken_OperatorLogicalOr] = 20;
        m_binaryOperatorPrecedence[ParserToken_OperatorComparisonLessThan] = 30;
        m_binaryOperatorPrecedence[ParserToken_OperatorComparisonGreaterThan] = 30;
        m_binaryOperatorPrecedence[ParserToken_OperatorComparisonLessThanOrEqual] = 30;
        m_binaryOperatorPrecedence[ParserToken_OperatorComparisonGreaterThanOrEqual] = 30;
        m_binaryOperatorPrecedence[ParserToken_OperatorComparisonEqual] = 30;
        m_binaryOperatorPrecedence[ParserToken_OperatorComparisonNotEqual] = 30;
        m_binaryOperatorPrecedence[ParserToken_OperatorArithmeticPlus] = 40;
        m_binaryOperatorPrecedence[ParserToken_OperatorArithmeticMinus] = 40;
        m_binaryOperatorPrecedence[ParserToken_OperatorArithmeticRemainder] = 50;
        m_binaryOperatorPrecedence[ParserToken_OperatorArithmeticMultiply] = 50;
        m_binaryOperatorPrecedence[ParserToken_OperatorArithmeticDivide] = 50;
    }
    
    std::shared_ptr<NodeRoot> Parser::parse(const std::string& path, std::vector<ParserToken> tokens) {
        m_tokens = std::move(tokens);
        m_tokens.emplace_back("", ParserToken_EOF);
        
        while (true) {
            getNextToken();
            
            if (m_currentToken.type == ParserToken_EOF) {
                break;
            } else if (m_currentToken.type == ParserToken_KeywordFunction) {
                m_nodes.emplace_back(parseFunction());
            } else if (m_currentToken.type == ParserToken_KeywordNamespace) {
                m_nodes.emplace_back(parseNamespace());
            } else if (m_currentToken.type == ParserToken_KeywordVar) {
                m_nodes.emplace_back(parseVariableDeclaration());
            } else if (m_currentToken.type == ParserToken_KeywordLet) {
                m_nodes.emplace_back(parseVariableDeclaration(true));
            }
        }
        
        auto nodeRoot = std::make_shared<NodeRoot>(std::vector<std::shared_ptr<Node>>
            {std::make_shared<NodeTopFile>(path, m_nodes)});
        
        setParent(nodeRoot);
        
        return nodeRoot;
    }
    
    std::vector<ParserToken> Parser::convertTokens(const std::vector<LexerToken>& LexerTokens) {
        auto tokens = std::vector<ParserToken>();
        auto lastToken = LexerToken(std::string(), LexerToken_Unknown);

        for (auto& token : LexerTokens) {
            switch (token.type) {
                case LexerToken_Number:
                    if (token.content.find('.') != std::string::npos)
                        tokens.emplace_back(token.content, ParserToken_LiteralValueDecimal);
                    else
                        tokens.emplace_back(token.content, ParserToken_LiteralValueInteger);
                    break;
                case LexerToken_String:
                    tokens.emplace_back(token.content, ParserToken_LiteralValueString);
                    break;
                case LexerToken_Character:
                    tokens.emplace_back(token.content, ParserToken_LiteralValueCharacter);
                    break;
                case LexerToken_Operator:
                    // Operators.
                    if (token.content == "=")
                        tokens.emplace_back("", ParserToken_OperatorEqual);
                    // Arithmetic Operators.
                    else if (token.content == "+=")
                        tokens.emplace_back("", ParserToken_OperatorArithmeticPlusEqual);
                    else if (token.content == "-=")
                        tokens.emplace_back("", ParserToken_OperatorArithmeticMinusEqual);
                    else if (token.content == "*=")
                        tokens.emplace_back("", ParserToken_OperatorArithmeticMultiplyEqual);
                    else if (token.content == "/=")
                        tokens.emplace_back("", ParserToken_OperatorArithmeticDivideEqual);
                    else if (token.content == "%=")
                        tokens.emplace_back("", ParserToken_OperatorArithmeticRemainderEqual);
                    else if ((token.content == "+" &&
                             lastToken.type != LexerToken_Number &&
                             lastToken.type != LexerToken_Identifier) &&
                             lastToken.content != ")" &&
                             lastToken.content != "++" &&
                             lastToken.content != "--")
                        tokens.emplace_back("", ParserToken_OperatorUnaryArithmeticPlus);
                    else if (token.content == "+")
                        tokens.emplace_back("", ParserToken_OperatorArithmeticPlus);
                    else if ((token.content == "-" &&
                              lastToken.type != LexerToken_Number &&
                              lastToken.type != LexerToken_Identifier) &&
                              lastToken.content != ")" &&
                              lastToken.content != "++" &&
                              lastToken.content != "--")
                        tokens.emplace_back("", ParserToken_OperatorUnaryArithmeticMinus);
                    else if (token.content == "-")
                        tokens.emplace_back("", ParserToken_OperatorArithmeticMinus);
                    else if (token.content == "*")
                        tokens.emplace_back("", ParserToken_OperatorArithmeticMultiply);
                    else if (token.content == "/")
                        tokens.emplace_back("", ParserToken_OperatorArithmeticDivide);
                    else if (token.content == "%")
                        tokens.emplace_back("", ParserToken_OperatorArithmeticRemainder);
                    else if (token.content == "==")
                        tokens.emplace_back("", ParserToken_OperatorComparisonEqual);
                    else if (token.content == "!=")
                        tokens.emplace_back("", ParserToken_OperatorComparisonNotEqual);
                    else if (token.content == "<")
                        tokens.emplace_back("", ParserToken_OperatorComparisonLessThan);
                    else if (token.content == ">")
                        tokens.emplace_back("", ParserToken_OperatorComparisonGreaterThan);
                    else if (token.content == "<=")
                        tokens.emplace_back("", ParserToken_OperatorComparisonLessThanOrEqual);
                    else if (token.content == ">=")
                        tokens.emplace_back("", ParserToken_OperatorComparisonGreaterThanOrEqual);
                    else if (token.content == "&&")
                        tokens.emplace_back("", ParserToken_OperatorLogicalAnd);
                    else if (token.content == "||")
                        tokens.emplace_back("", ParserToken_OperatorLogicalOr);
                    else if (token.content == "!")
                        tokens.emplace_back("", ParserToken_OperatorUnaryLogicalNot);
                    else if (token.content == "++")
                        tokens.emplace_back("", ParserToken_OperatorUnaryArithmeticIncrement);
                    else if (token.content == "--")
                        tokens.emplace_back("", ParserToken_OperatorUnaryArithmeticDecrement);
                    else if (token.content == "?")
                        tokens.emplace_back("", ParserToken_OperatorQuestionMark);
                    break;
                case LexerToken_Separator:
                    // Separators
                    if (token.content == "(")
                        tokens.emplace_back("", ParserToken_SeparatorRoundBracketOpen);
                    else if (token.content == ")")
                        tokens.emplace_back("", ParserToken_SeparatorRoundBracketClosed);
                    else if (token.content == "[")
                        tokens.emplace_back("", ParserToken_SeparatorSquareBracketOpen);
                    else if (token.content == "]")
                        tokens.emplace_back("", ParserToken_SeparatorSquareBracketClosed);
                    else if (token.content == "{")
                        tokens.emplace_back("", ParserToken_SeparatorCurlyBracketOpen);
                    else if (token.content == "}")
                        tokens.emplace_back("", ParserToken_SeparatorCurlyBracketClosed);
                    else if (token.content == ";")
                        tokens.emplace_back("", ParserToken_SeparatorSemicolon);
                    else if (token.content == ":")
                        tokens.emplace_back("", ParserToken_SeparatorColon);
                    else if (token.content == ",")
                        tokens.emplace_back("", ParserToken_SeparatorComma);
                    else if (token.content == ".")
                        tokens.emplace_back("", ParserToken_SeparatorDot);
                    else if (token.content == "...")
                        tokens.emplace_back("", ParserToken_SeparatorTripleDot);
                    break;
                case LexerToken_Identifier:
                    // Boolean
                    if (token.content == "true" || token.content == "false")
                        tokens.emplace_back(token.content, ParserToken_LiteralValueBoolean);
                    // Keywords
                    else if (token.content == "func")
                        tokens.emplace_back("", ParserToken_KeywordFunction);
                    else if (token.content == "var")
                        tokens.emplace_back("", ParserToken_KeywordVar);
                    else if (token.content == "let")
                        tokens.emplace_back("", ParserToken_KeywordLet);
                    else if (token.content == "for")
                        tokens.emplace_back("", ParserToken_KeywordFor);
                    else if (token.content == "by")
                        tokens.emplace_back("", ParserToken_KeywordBy);
                    else if (token.content == "do")
                        tokens.emplace_back("", ParserToken_KeywordDo);
                    else if (token.content == "while")
                        tokens.emplace_back("", ParserToken_KeywordWhile);
                    else if (token.content == "switch")
                        tokens.emplace_back("", ParserToken_KeywordSwitch);
                    else if (token.content == "case")
                        tokens.emplace_back("", ParserToken_KeywordCase);
                    else if (token.content == "return")
                        tokens.emplace_back("", ParserToken_KeywordReturn);
                    else if (token.content == "if")
                        tokens.emplace_back("", ParserToken_KeywordIf);
                    else if (token.content == "else")
                        tokens.emplace_back("", ParserToken_KeywordElse);
                    else if (token.content == "break")
                        tokens.emplace_back("", ParserToken_KeywordBreak);
                    else if (token.content == "continue")
                        tokens.emplace_back("", ParserToken_KeywordContinue);
                    else if (token.content == "namespace")
                        tokens.emplace_back("", ParserToken_KeywordNamespace);
                    else if (token.content == "default")
                        tokens.emplace_back("", ParserToken_KeywordDefault);
                    else if (token.content == "_")
                        tokens.emplace_back("", ParserToken_KeywordEmpty);
                    else
                        tokens.emplace_back(token.content, ParserToken_Identifier);
                    break;
                case LexerToken_Space:
                    tokens.emplace_back("", ParserToken_Space);
                    break;
                case LexerToken_Newline:
                    tokens.emplace_back("", ParserToken_Newline);
                    break;
                default:
                    tokens.emplace_back(token.content, ParserToken_Unknown);
                    break;
            }

            if (token.type != LexerToken_Space && token.type != LexerToken_Newline) {
                lastToken = token;
            }
        }
    
        tokens.emplace_back("", ParserToken_EOF);

        return tokens;
    }

    ParserToken Parser::getCurrentToken() {
        return m_currentToken;
    }

    ParserToken Parser::getNextToken(bool incrementCounter) {
        if (incrementCounter) {
            m_currentToken = m_tokens[++m_currentTokenIndex];
    
            if (m_currentToken.type == ParserToken_Newline) {
                m_currentLine++;
                m_currentColumn = 0;
            } else {
                if (m_currentToken.content.empty()) {
                    m_currentColumn++;
                } else {
                    m_currentColumn += m_currentToken.content.size();
                }
            }
    
            while (m_currentToken.type == ParserToken_Space) {
                m_currentToken = m_tokens[++m_currentTokenIndex];
        
                if (m_currentToken.type == ParserToken_Newline) {
                    m_currentLine++;
                    m_currentColumn = 0;
                } else {
                    if (m_currentToken.content.empty()) {
                        m_currentColumn++;
                    } else {
                        m_currentColumn += m_currentToken.content.size();
                    }
                }
            }
    
            return m_currentToken;
        } else {
            auto counter = m_currentTokenIndex;
            auto currentToken = m_tokens[++counter];
    
            while (currentToken.type == ParserToken_Space) {
                currentToken = m_tokens[++counter];
            }
    
            return currentToken;
        }
    }
    
    int Parser::getOperatorPrecedence(ParserTokenEnum binaryOperator) {
        if(m_binaryOperatorPrecedence.find(binaryOperator) == m_binaryOperatorPrecedence.end())
            return -1;
        else
            return m_binaryOperatorPrecedence[binaryOperator] <= 0 ? -1 : m_binaryOperatorPrecedence[binaryOperator];
    }

    std::shared_ptr<NodeTopFunction> Parser::parseFunction() {
        std::string name;
        std::string type;
        std::vector<Argument> arguments;

        getNextToken();
        
        if (m_currentToken.type == ParserToken_Identifier) {
            name = m_currentToken.content;
        } else {
            PARSER_ERROR("Expected an identifier.")
        }

        getNextToken();

        if (m_currentToken.type != ParserToken_SeparatorRoundBracketOpen) {
            PARSER_ERROR("Expected an opened round bracket.")
        }

        while (true) {
            getNextToken();

            if (m_currentToken.type == ParserToken_SeparatorRoundBracketClosed)
                break;
            else if (m_currentToken.type == ParserToken_Identifier) {
                auto identifier = m_currentToken.content;

                getNextToken();
                PARSER_CHECKTOKEN(ParserToken_SeparatorColon)

                getNextToken();
                PARSER_CHECKTOKEN(ParserToken_Identifier)

                arguments.emplace_back(Argument(identifier, m_currentToken.content));
            }
            else if (m_currentToken.type == ParserToken_SeparatorComma) {
                continue;
            }
            else {
                PARSER_ERROR("expected either a closed round bracket or a variable")
            }
        }

        getNextToken();
        
        if (m_currentToken.type == ParserToken_SeparatorColon) {
            PARSER_CHECKTOKEN(ParserToken_SeparatorColon)
    
            getNextToken();
    
            if (m_currentToken.type == ParserToken_Identifier)
                type = m_currentToken.content;
            else {
                PARSER_ERROR("Expected a type name")
            }
    
            getNextToken();
            
            while (m_currentToken.type == ParserToken_Newline) {
                getNextToken();
            }
    
            PARSER_CHECKTOKEN(ParserToken_SeparatorCurlyBracketOpen)
        } else if (m_currentToken.type == ParserToken_SeparatorCurlyBracketOpen) {
            type = "Void";
        } else {
            PARSER_ERROR("expected either identifier or opened curly bracket")
        }
        
        auto expressions = parseBody();
        
        if (type == "Void") {
            bool hasReturn = false;
            
            for (auto& statement : expressions) {
                if (statement->getType() == Node_StatementReturn) {
                    hasReturn = true;
                }
            }
            
            if (!hasReturn) {
                expressions.emplace_back(std::make_shared<NodeStatementReturn>());
            }
        }

        return std::make_shared<NodeTopFunction>(name, type, arguments, expressions);
    }
    
    std::shared_ptr<Node> Parser::parseNamespace() {
        auto identifier = std::string();
        std::vector<std::shared_ptr<Node>> nodes;
        
        getNextToken();
        
        if (m_currentToken.type != ParserToken_Identifier) {
            PARSER_ERROR("expected identifier in namespace declaration")
        }
        
        identifier = m_currentToken.content;
        
        getNextToken();
        
        if (m_currentToken.type != ParserToken_SeparatorCurlyBracketOpen) {
            PARSER_ERROR("expected opened curly bracket after namespace declaration")
        }
        
        while (true) {
            getNextToken();
            
            while (m_currentToken.type == ParserToken_Newline) {
                getNextToken();
            }
            
            if (m_currentToken.type == ParserToken_KeywordFunction) {
                nodes.emplace_back(parseFunction());
            } else if (m_currentToken.type == ParserToken_KeywordNamespace) {
                nodes.emplace_back(parseNamespace());
            } else {
                break;
            }
        }
        
        return std::make_shared<NodeTopNamespace>(identifier, nodes);
    }

    std::shared_ptr<Node> Parser::parsePrimary() {
        switch (m_currentToken.type) {
            case ParserToken_LiteralValueInteger:
                return parseInteger();
            case ParserToken_LiteralValueDecimal:
                return parseDouble();
            case ParserToken_LiteralValueString:
                return parseString();
            case ParserToken_LiteralValueCharacter:
                return parseCharacter();
            case ParserToken_LiteralValueBoolean:
                return parseBoolean();
            case ParserToken_SeparatorRoundBracketOpen:
                return parseParenthese();
            case ParserToken_KeywordReturn:
                return parseReturn();
            case ParserToken_KeywordIf:
                return parseIf();
            case ParserToken_KeywordSwitch:
                return parseSwitch();
            case ParserToken_KeywordDo:
                return parseWhile(true);
            case ParserToken_KeywordWhile:
                return parseWhile(false);
            case ParserToken_KeywordFor:
                return parseFor();
            case ParserToken_Identifier:
                return parseIdentifier();
            case ParserToken_SeparatorCurlyBracketOpen:
                return parseBlock();
            case ParserToken_KeywordVar:
                return parseVariableDeclaration();
            case ParserToken_KeywordLet:
                return parseVariableDeclaration(true);
            case ParserToken_KeywordBreak:
                return parseBreak();
            case ParserToken_KeywordContinue:
                return parseContinue();
            case ParserToken_SeparatorSquareBracketOpen:
                return parseArray();
            case ParserToken_OperatorUnaryArithmeticMinus:
            case ParserToken_OperatorUnaryArithmeticPlus:
            case ParserToken_OperatorUnaryArithmeticIncrement:
            case ParserToken_OperatorUnaryArithmeticDecrement:
            case ParserToken_OperatorUnaryLogicalNot:
                return parseUnaryOperation();
            default:
                return nullptr;
        }
    }

    std::shared_ptr<Node> Parser::parseExpression() {
        auto leftExpression = parsePrimary();
    
        if (!leftExpression) {
            return nullptr;
        }
    
        if (m_currentToken.type == ParserToken_OperatorUnaryArithmeticIncrement ||
            m_currentToken.type == ParserToken_OperatorUnaryArithmeticDecrement) {
            auto unaryExpression = std::make_shared<NodeOperationUnary>(m_currentToken.type, leftExpression, false);
            getNextToken();
            return parseBinaryOperation(0, unaryExpression);
        }
        
        return parseBinaryOperation(0, leftExpression);
    }
    
    std::shared_ptr<Node> Parser::parseTernaryOperation(std::shared_ptr<Node> condition) {
        getNextToken();
    
        auto leftExpression = parseExpression();
        
        if (m_currentToken.type != ParserToken_SeparatorColon) {
            PARSER_ERROR("expected ':' in ternary operation")
        }
        
        getNextToken();
        
        auto rightExpression = parseExpression();
        
        return std::make_shared<NodeOperationTernary>(condition, leftExpression, rightExpression);
    }
    
    std::shared_ptr<Node> Parser::parseBinaryOperation(int expressionPrecedence, std::shared_ptr<Node> leftExpression) {
        while (true) {
            if (m_currentToken.type == ParserToken_OperatorQuestionMark) {
                return parseTernaryOperation(leftExpression);
            }
            
            int tokenPrecedence = getOperatorPrecedence(m_currentToken.type);
    
            if (tokenPrecedence < expressionPrecedence)
                return leftExpression;
    
            auto operationType = m_currentToken.type;
            
            getNextToken();
            
            auto rightExpression = parsePrimary();

            if (!rightExpression)
                return nullptr;

            int nextPrecedence = getOperatorPrecedence(m_currentToken.type);

            if (tokenPrecedence < nextPrecedence) {
                rightExpression = parseBinaryOperation(tokenPrecedence + 1, std::move(rightExpression));

                if (!rightExpression)
                    return nullptr;
            }

            leftExpression = std::make_shared<NodeOperationBinary>(operationType, std::move(leftExpression), rightExpression);
        }
    }
    
    std::shared_ptr<Node> Parser::parseUnaryOperation() {
        if (m_currentToken.type != ParserToken_OperatorUnaryArithmeticPlus &&
            m_currentToken.type != ParserToken_OperatorUnaryArithmeticMinus &&
            m_currentToken.type != ParserToken_OperatorUnaryArithmeticIncrement &&
            m_currentToken.type != ParserToken_OperatorUnaryArithmeticDecrement &&
            m_currentToken.type != ParserToken_OperatorUnaryLogicalNot) {
            return parsePrimary();
        }
        
        auto type = m_currentToken.type;
        
        getNextToken();
        
        auto operand = parseUnaryOperation();
        
        return std::make_shared<NodeOperationUnary>(type, operand);
    }

    std::vector<std::shared_ptr<Node>> Parser::parseBody() {
        std::vector<std::shared_ptr<Node>> expressions;

        while (true) {
            getNextToken();
            
            if (m_currentToken.type == ParserToken_SeparatorCurlyBracketClosed)
                break;

            auto expression = parseExpression();

            if (expression)
                expressions.emplace_back(expression);
        }

        return expressions;
    }
    
    std::vector<std::shared_ptr<Node>> Parser::parseCase() {
        std::vector<std::shared_ptr<Node>> expressions;
    
        while (true) {
            getNextToken();
        
            if (m_currentToken.type == ParserToken_SeparatorCurlyBracketClosed ||
                m_currentToken.type == ParserToken_KeywordCase ||
                m_currentToken.type == ParserToken_KeywordDefault) {
                break;
            }
        
            auto expression = parseExpression();
        
            if (expression) {
                expressions.emplace_back(expression);
            }
        }
    
        return expressions;
    }

    std::shared_ptr<Node> Parser::parseInteger() {
        auto result = std::make_shared<NodeLiteralNumberInteger>(std::stoi(m_currentToken.content));

        getNextToken();

        return result;
    }

    std::shared_ptr<Node> Parser::parseDouble() {
        auto result = std::make_shared<NodeLiteralNumberFloating>(std::stod(m_currentToken.content));

        getNextToken();

        return result;
    }
    
    std::shared_ptr<Node> Parser::parseString() {
        auto result = std::make_shared<NodeLiteralString>(m_currentToken.content);
        
        getNextToken();
        
        return result;
    }
    
    std::shared_ptr<Node> Parser::parseCharacter() {
        auto result = std::make_shared<NodeLiteralCharacter>(m_currentToken.content);
        
        getNextToken();
        
        return result;
    }
    
    std::shared_ptr<Node> Parser::parseBoolean() {
        auto result = std::make_shared<NodeLiteralNumberFloating>(m_currentToken.content == "true");
        
        getNextToken();
        
        return result;
    }
    
    std::shared_ptr<Node> Parser::parseArray() {
        getNextToken();
        std::vector<std::shared_ptr<Node>> elements;
        
        while (true) {
            if (m_currentToken.type == ParserToken_SeparatorSquareBracketClosed) {
                break;
            } else if (m_currentToken.type == ParserToken_SeparatorComma) {
                getNextToken();
            } else {
                elements.emplace_back(parseExpression());
            }
        }
        
        return std::make_shared<NodeLiteralArray>(elements);
    }
    
    std::shared_ptr<Node> Parser::parseArrayCall() {
        getNextToken();
        
        auto expression = parseExpression();
        std::shared_ptr<Node> subexpression = nullptr;
        
        getNextToken();
        
        if (m_currentToken.type == ParserToken_SeparatorDot) {
            getNextToken();
    
            subexpression = parseExpression();
        }
    
        return std::make_shared<NodeStatementArrayCall>(expression, subexpression);
    }

    std::shared_ptr<Node> Parser::parseIdentifier() {
        auto identifier = m_currentToken.content;
        std::shared_ptr<Node> subExpression = nullptr;
        
        getNextToken();
        
        if (m_currentToken.type == ParserToken_SeparatorRoundBracketOpen) {
            getNextToken();

            std::vector<std::shared_ptr<Node>> arguments;

            if (m_currentToken.type != ParserToken_SeparatorRoundBracketClosed) {
                while (true) {
                    if (auto arg = parseExpression()) {
                        arguments.emplace_back(arg);
                    } else {
                        return nullptr;
                    }

                    if (m_currentToken.type == ParserToken_SeparatorRoundBracketClosed) {
                        break;
                    }

                    if (m_currentToken.type != ParserToken_SeparatorComma) {
                        PARSER_ERROR("Expected ')' or ',' in argument list")
                    }

                    getNextToken();
                }
            }

            getNextToken();
            
            if (m_currentToken.type == ParserToken_SeparatorDot) {
                getNextToken();
    
                subExpression = parseExpression();
            } else if (m_currentToken.type == ParserToken_SeparatorSquareBracketOpen) {
                subExpression = parseArrayCall();
            }
            
            return std::make_shared<NodeStatementFunctionCall>(identifier, arguments, subExpression);
        } else if (m_currentToken.type == ParserToken_SeparatorSquareBracketOpen) {
            subExpression = parseArrayCall();
        }
        
        return std::make_shared<NodeStatementVariable>(identifier, subExpression);
    }

    std::shared_ptr<Node> Parser::parseParenthese() {
        getNextToken();
        
        auto expression = parseExpression();

        getNextToken();

        return expression;
    }

    std::shared_ptr<Node> Parser::parseReturn() {
        getNextToken();
        
        if (m_currentToken.type == ParserToken_SeparatorSemicolon || m_currentToken.type == ParserToken_Newline) {
            return std::make_shared<NodeStatementReturn>(nullptr);
        }
    
        return std::make_shared<NodeStatementReturn>(parseExpression());
    }

    std::shared_ptr<Node> Parser::parseIf() {
        getNextToken();
        
        std::shared_ptr<Node> condition;
        
        if (m_currentToken.type == ParserToken_SeparatorRoundBracketOpen) {
            condition = parseParenthese();
        } else {
            condition = parseExpression();
        }
        
        if (!condition) {
            PARSER_ERROR("cannot parse if condition")
        }

        PARSER_CHECKTOKEN(ParserToken_SeparatorCurlyBracketOpen)

        auto ifBody = parseBody();

        PARSER_CHECKTOKEN(ParserToken_SeparatorCurlyBracketClosed)

        std::vector<std::shared_ptr<Node>> elseBody;
        
        if (getNextToken(false).type == ParserToken_KeywordElse) {
            getNextToken();
            getNextToken();
    
            if (m_currentToken.type == ParserToken_KeywordIf) {
                elseBody.emplace_back(parseIf());

                PARSER_CHECKTOKEN(ParserToken_SeparatorCurlyBracketClosed)
            }
            else {
                PARSER_CHECKTOKEN(ParserToken_SeparatorCurlyBracketOpen)

                elseBody = parseBody();

                PARSER_CHECKTOKEN(ParserToken_SeparatorCurlyBracketClosed)
            }
        }

        return std::make_shared<NodeStatementIf>(condition, ifBody, elseBody);
    }
    
    std::shared_ptr<Node> Parser::parseFor() {
        std::shared_ptr<Node> incrementationValue;
        
        getNextToken();
        
        if (m_currentToken.type == ParserToken_SeparatorRoundBracketOpen) {
            getNextToken();
        }
    
        std::shared_ptr<Node> variable = nullptr;
        
        if (m_currentToken.type != ParserToken_KeywordEmpty) {
            if (getNextToken(false).type == ParserToken_OperatorEqual) {
                variable = parseVariableDeclaration(false, false, false);
            } else {
                variable = parseIdentifier();
            }
        } else {
            getNextToken();
        }
    
        if (m_currentToken.type != ParserToken_KeywordWhile) {
            PARSER_ERROR("expected 'while' in for declaration")
        }
        
        getNextToken();
    
        if (!isLiteralOrIdentifier(m_currentToken.type)) {
            PARSER_ERROR("expected literal value or identifier in for declaration")
        }
        
        auto condition = parseExpression();
        
        if (m_currentToken.type == ParserToken_KeywordBy) {
            getNextToken();
    
            if (!isLiteralOrIdentifier(m_currentToken.type)) {
                PARSER_ERROR("expected literal value or identifier in by in for declaration")
            }
    
            incrementationValue = parseExpression();
        }
    
        if (m_currentToken.type == ParserToken_SeparatorRoundBracketClosed) {
            getNextToken();
        }
        
        if (m_currentToken.type != ParserToken_SeparatorCurlyBracketOpen) {
            PARSER_ERROR("expected '{' in for declaration")
        }
    
        auto body = parseBody();
    
        PARSER_CHECKTOKEN(ParserToken_SeparatorCurlyBracketClosed)
        
        if (variable) {
            return std::make_shared<NodeStatementFor>(variable, condition, incrementationValue, body);
        }
        
        return std::make_shared<NodeStatementFor>(nullptr, condition, incrementationValue, body);
    }
    
    std::shared_ptr<Node> Parser::parseWhile(bool doOnce) {
        std::vector<std::shared_ptr<Node>> body;
        std::shared_ptr<Node> condition;
    
        getNextToken();
        
        if (doOnce) {
            PARSER_CHECKTOKEN(ParserToken_SeparatorCurlyBracketOpen)
            
            getNextToken();
            
            body = parseBody();
    
            PARSER_CHECKTOKEN(ParserToken_SeparatorCurlyBracketClosed)
            
            getNextToken();
            
            PARSER_CHECKTOKEN(ParserToken_KeywordWhile)
            
            getNextToken();
    
            if (m_currentToken.type == ParserToken_SeparatorRoundBracketOpen) {
                condition = parseParenthese();
            } else {
                condition = parseExpression();
            }
        } else {
            if (m_currentToken.type == ParserToken_SeparatorRoundBracketOpen) {
                condition = parseParenthese();
            } else {
                condition = parseExpression();
            }
    
            PARSER_CHECKTOKEN(ParserToken_SeparatorCurlyBracketOpen)
            
            getNextToken();
    
            body = parseBody();
    
            PARSER_CHECKTOKEN(ParserToken_SeparatorCurlyBracketClosed)
        }
        
        return std::make_shared<NodeStatementWhile>(doOnce, condition, body);
    }
    
    std::shared_ptr<Node> Parser::parseBlock() {
        return std::make_shared<NodeStatementBlock>(parseBody());
    }
    
    std::shared_ptr<Node> Parser::parseBreak() {
        getNextToken();
        
        return std::make_shared<NodeStatementBreak>();
    }
    
    std::shared_ptr<Node> Parser::parseContinue() {
        getNextToken();
        
        return std::make_shared<NodeStatementContinue>();
    }
    
    std::shared_ptr<Node> Parser::parseSwitch() {
        std::shared_ptr<Node> expression;
        std::vector<std::shared_ptr<Node>> cases;
        std::shared_ptr<NodeStatementCase> defaultCase = nullptr;
        
        getNextToken();
    
        if (m_currentToken.type == ParserToken_SeparatorRoundBracketOpen) {
            expression = parseParenthese();
        } else {
            expression = parseExpression();
        }
        
        PARSER_CHECKTOKEN(ParserToken_SeparatorCurlyBracketOpen)
    
        getNextToken();
        
        while (true) {
            if (m_currentToken.type == ParserToken_SeparatorCurlyBracketClosed) {
                break;
            } else if (m_currentToken.type == ParserToken_Newline) {
                getNextToken();
            } else if (m_currentToken.type == ParserToken_KeywordCase) {
                std::shared_ptr<Node> expressionCase = nullptr;
                std::vector<std::shared_ptr<Node>> bodyCase = std::vector<std::shared_ptr<Node>>();
                
                getNextToken();
                
    
                if (m_currentToken.type == ParserToken_SeparatorRoundBracketOpen) {
                    expressionCase = parseParenthese();
                } else {
                    expressionCase = parseExpression();
                }
                    
                PARSER_CHECKTOKEN(ParserToken_SeparatorColon)
                    
                getNextToken();
    
                bodyCase = parseCase();
                
                cases.emplace_back(std::make_shared<NodeStatementCase>(expressionCase, bodyCase));
            } else if (m_currentToken.type == ParserToken_KeywordDefault) {
                if (defaultCase) {
                    PARSER_ERROR("default already defined in switch")
                } else {
                    std::vector<std::shared_ptr<Node>> bodyCase = std::vector<std::shared_ptr<Node>>();
                    
                    getNextToken();
                    
                    PARSER_CHECKTOKEN(ParserToken_SeparatorColon)
                    
                    getNextToken();
                    
                    bodyCase = parseCase();
                    
                    cases.insert(cases.begin(), std::make_shared<NodeStatementCase>(nullptr, bodyCase));
                }
            } else {
                PARSER_ERROR("unexpected token in switch declaration")
            }
        }
        
        return std::make_shared<NodeStatementSwitch>(expression, cases);
    }
    
    std::shared_ptr<Node> Parser::parseVariableDeclaration(bool constant, bool passVar, bool multiple) {
        std::vector<std::shared_ptr<Variable>> variables;

        while (true) {
            if (passVar) {
                getNextToken();
            }
            
            PARSER_CHECKTOKEN(ParserToken_Identifier)

            auto identifier = m_currentToken.content;
            auto type = std::string();
            std::shared_ptr<Node> value = nullptr;
            
            getNextToken();
            
            if (m_currentToken.type == ParserToken_SeparatorColon) {
                getNextToken();
                
                if (m_currentToken.type == ParserToken_Identifier) {
                    type = m_currentToken.content;
                } else if (m_currentToken.type == ParserToken_SeparatorSquareBracketOpen) {
                    getNextToken();
    
                    type = "[" + m_currentToken.content + "]";
                    
                    getNextToken();
                } else {
                    PARSER_ERROR("expected typename or array typename in variable declaration")
                }
    
                getNextToken();
            }

            if (m_currentToken.type == ParserToken_OperatorEqual) {
                getNextToken();
                
                value = parseExpression();
            } else if (m_currentToken.type == ParserToken_OperatorArithmeticPlusEqual ||
                m_currentToken.type == ParserToken_OperatorArithmeticMinusEqual ||
                m_currentToken.type == ParserToken_OperatorArithmeticMultiplyEqual ||
                m_currentToken.type == ParserToken_OperatorArithmeticDivideEqual ||
                m_currentToken.type == ParserToken_OperatorArithmeticRemainderEqual) {
                PARSER_ERROR("expected initializer before using differents assignment operators")
            } else if (type.empty() && value == nullptr) {
                PARSER_ERROR("variable declaration has empty type and value")
            }
    
            if (!type.empty() || value != nullptr) {
                variables.emplace_back(std::make_shared<Variable>(identifier, type, constant, value));
            }
            
            if (!multiple || m_currentToken.type != ParserToken_SeparatorComma) {
                break;
            }
        }

        return std::make_shared<NodeStatementVariableDeclaration>(variables);
    }
    
    bool Parser::isLiteral(ParserTokenEnum type) {
        return m_currentToken.type == ParserToken_LiteralValueInteger ||
               m_currentToken.type == ParserToken_LiteralValueDecimal ||
               m_currentToken.type == ParserToken_LiteralValueString ||
               m_currentToken.type == ParserToken_LiteralValueCharacter ||
               m_currentToken.type == ParserToken_LiteralValueBoolean;
    }
    
    bool Parser::isLiteralOrIdentifier(ParserTokenEnum type) {
        return m_currentToken.type == ParserToken_Identifier || isLiteral(type);
    }
    
    bool Parser::setParent(std::shared_ptr<Node> node) {
        for (auto& child : node->children) {
            if (child) {
                child->parent = node;
    
                setParent(child);
            }
        }
    }
    
    Parser &getParser() {
        return Parser::getInstance();
    }
    
} /* Namespace Aryiele. */
