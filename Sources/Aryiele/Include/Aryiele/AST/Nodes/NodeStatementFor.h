//==================================================================================//
//                                                                                  //
//  Copyright (c) 2019 Hugo Kindel <kindelhugo.pro@gmail.com>                      //
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

#ifndef ARYIELE_NODESTATEMENTFOR_H
#define ARYIELE_NODESTATEMENTFOR_H

#include <Aryiele/Common.h>
#include <Aryiele/AST/Nodes/Node.h>
#include <Aryiele/AST/Nodes/NodeStatementVariableDeclaration.h>
#include <Aryiele/AST/Variable.h>

namespace Aryiele {
    struct NodeStatementFor : public Node {
        NodeStatementFor(std::shared_ptr<Node> variable, std::shared_ptr<Node> condition,
            std::shared_ptr<Node> incrementalValue, std::vector<std::shared_ptr<Node>> body);
    
        void dumpAST(std::shared_ptr<ParserInformation> parentNode) override;
        NodeEnum getType() override;
    
        std::shared_ptr<Node> condition;
        std::shared_ptr<Node> incrementalValue;
        std::vector<std::shared_ptr<Node>> body;
        std::shared_ptr<Node> variable;
    };
    
} /* Namespace Aryiele. */

#endif /* ARYIELE_NODESTATEMENTFOR_H. */
