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

#include <Aryiele/Lexer/Lexer.h>
#include <Aryiele/Lexer/LexerTable.h>

namespace Aryiele {
    std::vector<LexerToken> Lexer::Tokenize(const std::string& filepath) {
        std::ifstream file;
        std::string expression;

        file.open(filepath.c_str());

        if (file.fail())
            return m_tokens;

        while (getline(file, expression))
            UseFiniteStateMachine(expression);

        RemoveComments();
        DetailTokens();

        return m_tokens;
    }

    void Lexer::UseFiniteStateMachine(std::string expression) {
        LexerToken currentToken;
        std::vector<LexerToken> tokens;
        auto currentTransitionState = LexerTokens_Reject;
        auto previousTransitionState = LexerTokens_Reject;
        std::string currentTokenExpression;

        for (unsigned x = 0; x < expression.length();) {
            const auto currentCharacter = expression[x];
            const auto column = GetTransitionTableColumn(currentCharacter);

            currentTransitionState = static_cast<LexerTokens>(LexerTable[currentTransitionState][column]);

            if (currentTransitionState == LexerTokens_Reject) {
                currentToken.Content = currentTokenExpression;
                currentToken.Type = previousTransitionState;
                currentTokenExpression = "";

                tokens.push_back(currentToken);
            }
            else {
                currentTokenExpression += currentCharacter;
                ++x;
            }

            previousTransitionState = currentTransitionState;
        }

        if (!currentTokenExpression.empty()) {
            currentToken.Content = currentTokenExpression;
            currentToken.Type = currentTransitionState;

            tokens.push_back(currentToken);
        }

        currentToken.Content = std::string();
        currentToken.Type = LexerTokens_Newline;

        tokens.push_back(currentToken);

        m_tokens.insert(m_tokens.end(), tokens.begin(), tokens.end());
    }

    void Lexer::RemoveComments() {
        LexerToken currentToken;
        std::vector<LexerToken> tokens;
        auto isInCommentSingleLine = false;
        auto isInCommentMultiLine = false;

        for (auto& token : m_tokens) {
            const auto lastToken = currentToken;
            currentToken = token;

            if (isInCommentSingleLine) {
                if (currentToken.Type == LexerTokens_Newline)
                    isInCommentSingleLine = false;
            }
            else if (isInCommentMultiLine) {
                if (currentToken.Content == "*/")
                    isInCommentMultiLine = false;
            }
            else if (currentToken.Content == "//") {
                isInCommentSingleLine = true;
            }
            else if (currentToken.Content == "/*") {
                isInCommentMultiLine = true;
            }
            else {
                tokens.emplace_back(currentToken);
            }
        }

        m_tokens.clear();

        m_tokens = tokens;
    }

    void Lexer::DetailTokens() {
        LexerToken currentToken;
        std::vector<LexerToken> tokens;
        std::string inTextQuote;
        std::string currentText;
        auto isInText = false;

        for (auto& token : m_tokens) {
            const auto lastToken = currentToken;
            currentToken = token;

            if (currentToken.Type == LexerTokens_Newline)
                continue;
            if (currentToken.Type == LexerTokens_Space && !isInText)
                continue;

            if (isInText) {
                if (currentToken.Type == LexerTokens_StringQuote && currentToken.Content == inTextQuote) {
                    if (currentText[currentText.length() - 1] == '\\')
                    {
                        currentText.pop_back();
                        currentText += inTextQuote;
                    }
                    else
                    {
                        currentToken.Content = currentText;
                        currentToken.Type = LexerTokens_String;

                        tokens.emplace_back(currentToken);

                        isInText = false;
                        currentText = "";
                        inTextQuote = "";
                    }
                }
                else {
                    currentText += currentToken.Content;
                }
            }
            else if (currentToken.Type == LexerTokens_StringQuote) {
                inTextQuote = currentToken.Content;
                isInText = true;
            }
            else {
                tokens.emplace_back(currentToken);
            }
        }

        m_tokens.clear();

        m_tokens = tokens;
    }

    LexerTokens Lexer::GetTransitionTableColumn(char currentCharacter) {
        if (currentCharacter == '{' || currentCharacter == '}' ||
            currentCharacter == '[' || currentCharacter == ']' ||
            currentCharacter == '(' || currentCharacter == ')' ||
            currentCharacter == ';' || currentCharacter == ',' || currentCharacter == ':') {
            return LexerTokens_Separator;
        }
        else if (currentCharacter == '\"' || currentCharacter == '\'') {
            return LexerTokens_StringQuote;
        }
        else if (isspace(currentCharacter)) {
            return LexerTokens_Space;
        }
        else if (isdigit(currentCharacter) || currentCharacter == '.') {
            return LexerTokens_Number;
        }
        else if (isalpha(currentCharacter) || currentCharacter == '_') {
            return LexerTokens_Identifier;
        }
        else if (ispunct(currentCharacter)) {
            return LexerTokens_Operator;
        }

        return LexerTokens_Unknown;
    }

    std::string Lexer::GetTokenName(LexerToken tokenType) {
        switch (tokenType.Type) {
            case LexerTokens_Number:
                return "Number";
            case LexerTokens_String:
                return "String";
            case LexerTokens_Operator:
                return "Operator";
            case LexerTokens_Identifier:
                return "Identifier";
            case LexerTokens_Separator:
                return "Separator";
            case LexerTokens_Space:
                return "Space";
            case LexerTokens_Newline:
                return "Newline";
            case LexerTokens_StringQuote:
                return "StringQuote";
            case LexerTokens_Unknown:
                return "Unknown";
            default:
                return "Error";
        }
    }
    
    Lexer &GetLexer() {
        return Lexer::getInstance();
    }

} /* Namespace Aryiele. */