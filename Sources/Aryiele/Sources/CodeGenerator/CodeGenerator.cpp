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

#include <llvm/Transforms/Scalar/SimplifyCFG.h>
#include <llvm/Transforms/Scalar/Reassociate.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <Aryiele/CodeGenerator/CodeGenerator.h>
#include <Aryiele/Parser/Parser.h>
#include <Aryiele/AST/Nodes/Node.h>

namespace Aryiele {
    CodeGenerator::CodeGenerator() {
        m_module = std::make_shared<llvm::Module>("Aryiele", m_context);
        m_dataLayout = std::make_shared<llvm::DataLayout>(m_module.get());
        m_blockStack = std::make_shared<BlockStack>();
    }

    void CodeGenerator::generateCode(std::vector<std::shared_ptr<Node>> nodes) {
        // PRINTF TODO: Extern
        std::vector<llvm::Type *> Doubles(1, llvm::Type::getInt32Ty(m_context));
        llvm::FunctionType *FT =
            llvm::FunctionType::get(llvm::Type::getInt32Ty(m_context), Doubles, false);

        llvm::Function *F =
            llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "print", m_module.get());

        // Set names for all arguments.
        unsigned Idx = 0;
        for (auto &Arg : F->args())
            Arg.setName("value");
        // --

        m_blockStack->create();

        for (auto& node : nodes)
            generateCode(node);

        m_blockStack->escapeCurrent();
    }

    std::shared_ptr<llvm::Module> CodeGenerator::getModule() {
        return m_module;
    }

    llvm::Value *CodeGenerator::castType(llvm::Value *value, llvm::Type *returnType) {
        if (value->getType()->isIntegerTy() && returnType->isIntegerTy()) {
            auto *ival = (llvm::IntegerType *)value->getType();
            auto *ito  = (llvm::IntegerType *)returnType;

            if(ival->getBitWidth() < ito->getBitWidth())
                return m_builder.CreateZExtOrBitCast(value, returnType);
        }
        else if(value->getType()->isIntegerTy() && returnType->isDoubleTy()) {
            return m_builder.CreateSIToFP(value, returnType);
        }
        else if(value->getType()->isDoubleTy() && returnType->isIntegerTy()) {
            return m_builder.CreateFPToSI(value, returnType);
        }
        else if(returnType->isVoidTy()) {
            return value;
        }

        return m_builder.CreateTruncOrBitCast(value, returnType);
    }
    
    // Definition code from https://llvm.org/docs/tutorial/LangImpl07.html
    llvm::AllocaInst *
    CodeGenerator::createEntryBlockAllocation(llvm::Function *function, const std::string &identifier, llvm::Type *type) {
        llvm::IRBuilder<> TmpB(&function->getEntryBlock(), function->getEntryBlock().begin());

        return TmpB.CreateAlloca(type == nullptr ? llvm::Type::getInt32Ty(m_context) : type, nullptr, identifier);
    }

    GenerationError CodeGenerator::generateCode(std::shared_ptr<Node> node) {
        auto nodePtr = node.get();

        switch (node->getType()) {
            caseNode_Function_Prototype:
                return generateCode((NodeFunction*)nodePtr);
            caseNode_Constant_Double:
                return generateCode((NodeConstantDouble*)nodePtr);
            caseNode_Constant_Integer:
                return generateCode((NodeConstantInteger*)nodePtr);
            caseNode_Variable:
                return generateCode((NodeVariable*)nodePtr);
            caseNode_Operation_Binary:
                return generateCode((NodeOperationBinary*)nodePtr);
            caseNode_Statement_FunctionCall:
                return generateCode((NodeStatementFunctionCall*)nodePtr);
            caseNode_Statement_If:
                return generateCode((NodeStatementIf*)nodePtr);
            caseNode_Statement_Return:
                return generateCode((NodeStatementReturn*)nodePtr);
            caseNode_Statement_Block:
                return generateCode((NodeStatementBlock*)nodePtr);
            caseNode_Statement_VariableDeclaration:
                return generateCode((NodeStatementVariableDeclaration*)nodePtr);

            default:
                return GenerationError();
        }
    }

    // TODO: Return + Types
    GenerationError CodeGenerator::generateCode(NodeFunction* node) {
        llvm::Function *function = m_module->getFunction(node->identifier);

        if (!function) {
            std::vector<llvm::Type*> arguments;
            llvm::Type* functionTypeValue;

            for (const auto &argument : node->arguments) {
                arguments.emplace_back(llvm::Type::getInt32Ty(m_context));
            }

            functionTypeValue = llvm::Type::getInt32Ty(m_context);

            llvm::FunctionType *functionType = llvm::FunctionType::get(functionTypeValue, arguments, false);

            function = llvm::Function::Create(
                    functionType, llvm::Function::ExternalLinkage, node->identifier, m_module.get());

            unsigned i = 0;

            for (auto &Arg : function->args())
                Arg.setName(node->arguments[i++].identifier);
        }

        m_blockStack->create();

        llvm::BasicBlock *basicBlock = llvm::BasicBlock::Create(m_context, "entry", function);

        m_builder.SetInsertPoint(basicBlock);

        for (auto &argument : function->args()) {
            llvm::AllocaInst *allocationInstance = createEntryBlockAllocation(function, argument.getName(), argument.getType());

            m_builder.CreateStore(&argument, allocationInstance);

            m_blockStack->current->variables[argument.getName()] = allocationInstance;
        }

        for (auto& statement : node->body) {
            auto error = generateCode(statement);

            if (!error.success) {
                function->eraseFromParent();

                LOG_ERROR("cannot generate the body of a function: ", node->identifier);

                return GenerationError();
            }
        }

        m_blockStack->escapeCurrent();

        verifyFunction(*function);

        return GenerationError(true, function);
    }

    GenerationError CodeGenerator::generateCode(NodeOperationBinary* node) {
        if (node->operationType == ParserToken_OperatorEqual) {
            auto lhs = std::static_pointer_cast<NodeVariable>(node->lhs);

            if (!lhs) {
                LOG_ERROR("cannot generate a binary operation: lhs: expecting a variable");

                return GenerationError();
            }

            auto rhsValue = generateCode(node->rhs);

            if (!rhsValue.success) {
                LOG_ERROR("cannot generate a binary operation: rhs: generation failed");

                return GenerationError();
            }

            llvm::Value *variable = m_blockStack->findVariable(lhs->identifier);

            if (!variable) {
                LOG_ERROR("cannot generate a binary operation: lhs: unknown variable '" + lhs->identifier + "'");

                return GenerationError();
            }

            m_builder.CreateStore(rhsValue.value, variable);

            return GenerationError(true, rhsValue.value);
        }

        auto lhsValue = generateCode(node->lhs);
        auto rhsValue = generateCode(node->rhs);

        if (!lhsValue.success || !rhsValue.success)
            return GenerationError();

        llvm::Value *value = nullptr;

        // TODO: CODEGENERATOR: Only support integers for now
        // TODO: CODEGENERATOR: Only support signed numbers for now
        switch (node->operationType) {
            case ParserToken_OperatorArithmeticPlus:
                value = m_builder.CreateAdd(lhsValue.value, rhsValue.value, "add");
                break;
            case ParserToken_OperatorArithmeticMinus:
                value = m_builder.CreateSub(lhsValue.value, rhsValue.value, "sub");
                break;
            case ParserToken_OperatorArithmeticMultiply:
                value = m_builder.CreateMul(lhsValue.value, rhsValue.value, "mul");
                break;
            case ParserToken_OperatorArithmeticDivide:
                value = m_builder.CreateSDiv(lhsValue.value, rhsValue.value, "sdiv");
                break;
            case ParserToken_OperatorComparisonLessThan:
                value = m_builder.CreateICmpULT(lhsValue.value, rhsValue.value, "icmpult");
                break;
            case ParserToken_OperatorComparisonLessThanOrEqual:
                value = m_builder.CreateICmpULE(lhsValue.value, rhsValue.value, "icmpule");
                break;
            case ParserToken_OperatorComparisonGreaterThan:
                value = m_builder.CreateICmpUGT(lhsValue.value, rhsValue.value, "icmpugt");
                break;
            case ParserToken_OperatorComparisonGreaterThanOrEqual:
                value = m_builder.CreateICmpUGE(lhsValue.value, rhsValue.value, "icmpuge");
                break;
            case ParserToken_OperatorComparisonEqual:
                value = m_builder.CreateICmpEQ(lhsValue.value, rhsValue.value, "icmpeq");
                break;
            case ParserToken_OperatorComparisonNotEqual:
                value = m_builder.CreateICmpNE(lhsValue.value, rhsValue.value, "icmpeq");
                break;
            default: {
                LOG_ERROR("unknown binary operator: ", Parser::getTokenName(node->operationType));

                return GenerationError();
            }
        }

        return GenerationError(true, value);
    }

    GenerationError CodeGenerator::generateCode(NodeConstantDouble* node) {
        return GenerationError(true, llvm::ConstantFP::get(m_context, llvm::APFloat(node->value)));
    }

    GenerationError CodeGenerator::generateCode(NodeConstantInteger* node) {
        return GenerationError(true, llvm::ConstantInt::get(m_context, llvm::APInt(32, node->value)));
    }

    GenerationError CodeGenerator::generateCode(NodeVariable* node) {
        llvm::Value *value = m_blockStack->findVariable(node->identifier);

        if (!value) {
            LOG_ERROR("unknown variable: ", node->identifier);
        }

        return GenerationError(true, m_builder.CreateLoad(value, node->identifier.c_str()));
    }


    GenerationError CodeGenerator::generateCode(NodeStatementFunctionCall* node) {
        llvm::Function *calledFunction = m_module->getFunction(node->identifier);

        if (!calledFunction) {
            LOG_ERROR("unknown function referenced: ", node->identifier);

            return GenerationError();
        }

        if (calledFunction->arg_size() != node->arguments.size()) {
            LOG_ERROR("incorrect number of argument passed: ",
                      node->arguments.size(), " while expecting ", calledFunction->arg_size());

            return GenerationError();
        }

        std::vector<llvm::Value*> argumentsValues;

        for (unsigned i = 0, e = static_cast<unsigned int>(node->arguments.size()); i != e; ++i) {
            auto error = generateCode(node->arguments[i]);

            if (!error.success)
                return GenerationError();

            argumentsValues.push_back(error.value);
        }

        return GenerationError(true, m_builder.CreateCall(calledFunction, argumentsValues, "calltmp"));
    }

    // TODO: CODEGENERATOR: Support for no "Else" statement
    GenerationError CodeGenerator::generateCode(NodeStatementIf* node) {
        auto conditionValue = generateCode(node->condition);

        if (!conditionValue.success)
            return GenerationError();

        llvm::Function *function = m_builder.GetInsertBlock()->getParent();

        llvm::BasicBlock *ifBasicBlock = llvm::BasicBlock::Create(m_context, "if", function);
        llvm::BasicBlock *elseBasicBlock = llvm::BasicBlock::Create(m_context, "else", function);
        llvm::BasicBlock *mergeBasicBlock = llvm::BasicBlock::Create(m_context, "merge", function);

        m_builder.CreateCondBr(conditionValue.value, ifBasicBlock, elseBasicBlock);
        m_builder.SetInsertPoint(ifBasicBlock);
        m_blockStack->create();

        for (auto &statement : node->ifBody) {
            auto generatedCode = generateCode(statement);

            if (!generatedCode.success)
                return GenerationError();
        }

        m_blockStack->escapeCurrent();
        m_builder.CreateBr(mergeBasicBlock);
        m_builder.SetInsertPoint(elseBasicBlock);

        if (!node->elseBody.empty()) {
            m_blockStack->create();

            for (auto &statement : node->elseBody) {
                auto generatedCode = generateCode(statement);

                if (!generatedCode.success)
                    return GenerationError();
            }

            m_blockStack->escapeCurrent();
        }

        m_builder.CreateBr(mergeBasicBlock);
        m_builder.SetInsertPoint(mergeBasicBlock);

        return GenerationError(true);
    }

    GenerationError CodeGenerator::generateCode(NodeStatementReturn* node) {
        auto error = generateCode(node->expression);

        if (!error.success) {
            LOG_ERROR("cannot generate return value");

            return GenerationError();
        }

        m_builder.CreateRet(error.value);

        return GenerationError(true, error.value);
    }

    GenerationError CodeGenerator::generateCode(NodeStatementBlock *node) {
        m_blockStack->create();

        for (auto &statement : node->body) {
            auto error = generateCode(statement);

            if (!error.success) {
                LOG_ERROR("cannot generate the body of a block in function");
            }
        }

        m_blockStack->escapeCurrent();

        return GenerationError(true);
    }

    // TODO: CODEGENERATOR: Support other variable types (for now only int32).
    GenerationError CodeGenerator::generateCode(NodeStatementVariableDeclaration *node) {
        llvm::Function *function = m_builder.GetInsertBlock()->getParent();

        for (auto &variable : node->variables) {
            GenerationError error;

            if (variable->expression) {
                error = generateCode(variable->expression);

                if (!error.success) {
                    LOG_ERROR("cannot generate declaration of a variable");

                    return GenerationError();
                }
            }
            else {
                error.value = llvm::ConstantInt::get(m_context, llvm::APInt(32, 0));
            }

            llvm::AllocaInst *allocationInstance = createEntryBlockAllocation(function, variable->identifier);

            m_builder.CreateStore(error.value, allocationInstance);

            m_blockStack->current->variables[variable->identifier] = allocationInstance;
        }

        return GenerationError(true);
    }
    
    CodeGenerator &getCodeGenerator() {
        return CodeGenerator::getInstance();
    }

} /* Namespace Aryiele. */