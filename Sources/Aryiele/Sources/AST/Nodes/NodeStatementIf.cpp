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

#include <Aryiele/AST/Nodes/NodeStatementIf.h>

namespace Aryiele {
    NodeStatementIf::NodeStatementIf(std::shared_ptr<Node> condition,
                                     std::vector<std::shared_ptr<Node>> ifBody,
                                     std::vector<std::shared_ptr<Node>> elseBody,
                                     std::vector<std::vector<std::shared_ptr<Node>>> elseIfBody) :
        Condition(condition), IfBody(ifBody), ElseBody(elseBody), ElseIfBody(elseIfBody) {

    }

    void NodeStatementIf::DumpInformations(std::shared_ptr<ParserInformation> parentNode) {
        auto node = std::make_shared<ParserInformation>(parentNode, "If/Else");

        auto ifNode = std::make_shared<ParserInformation>(node, "If");
        auto ifCondition = std::make_shared<ParserInformation>(ifNode, "Condition:");
        auto ifBody = std::make_shared<ParserInformation>(ifNode, "Body:");

        Condition->DumpInformations(ifCondition);

        for (auto& i : IfBody)
            i->DumpInformations(ifBody);

        ifNode->Children.emplace_back(ifCondition);
        ifNode->Children.emplace_back(ifBody);
        node->Children.emplace_back(ifNode);

        for (auto elseIfBody : ElseIfBody) {
            auto elseNode = std::make_shared<ParserInformation>(node, "Else If");
            auto elseBody = std::make_shared<ParserInformation>(elseNode, "Body:");

            for (auto& i : elseIfBody)
                i->DumpInformations(elseBody);

            elseNode->Children.emplace_back(elseBody);
            node->Children.emplace_back(elseNode);
        }

        if (!ElseBody.empty()) {
            auto elseNode = std::make_shared<ParserInformation>(node, "Else");
            auto elseBody = std::make_shared<ParserInformation>(elseNode, "Body:");

            for (auto& i : ElseBody)
                i->DumpInformations(elseBody);

            elseNode->Children.emplace_back(elseBody);
            node->Children.emplace_back(elseNode);
        }

        parentNode->Children.emplace_back(node);
    }
    
    NodeEnum NodeStatementIf::GetType() {
        return Node_StatementIf;
    }

} /* Namespace Aryiele. */
