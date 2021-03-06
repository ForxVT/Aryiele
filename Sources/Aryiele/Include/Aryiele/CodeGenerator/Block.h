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

#ifndef ARYIELE_CODEGENERATOR_BLOCK_H
#define ARYIELE_CODEGENERATOR_BLOCK_H

#include <llvm/IR/Instructions.h>
#include <Aryiele/Common.h>
#include <Aryiele/CodeGenerator/BlockVariable.h>

namespace Aryiele {
    struct Block {
        Block(std::shared_ptr<Block> parent = nullptr,
              std::map<std::string, std::shared_ptr<BlockVariable>> variables = std::map<std::string, std::shared_ptr<BlockVariable>>(),
              std::vector<std::shared_ptr<Block>> children = std::vector<std::shared_ptr<Block>>());

        std::shared_ptr<Block> parent = nullptr;
        std::map<std::string, std::shared_ptr<BlockVariable>> variables;
        std::vector<std::shared_ptr<Block>> children;
    };

} /* Namespace Aryiele. */

#endif /* ARYIELE_CODEGENERATOR_BLOCK_H. */
