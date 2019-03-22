#include <Aryiele/CodeGenerator/BlockStack.h>

namespace Aryiele
{
    std::shared_ptr<Block> BlockStack::Create(bool setAsCurrent)
    {
        auto block = std::make_shared<Block>();

        if (Current)
        {
            block->Parent = Current;

            Current->Children.emplace_back(block);
        }

        if (setAsCurrent)
            Current = block;

        return Current;
    }

    std::shared_ptr<Block> BlockStack::EscapeCurrent()
    {
        if (Current->Parent)
        {
            Current = Current->Parent;

            return Current;
        }

        return nullptr;
    }

    std::shared_ptr<Variable> BlockStack::FindVariable(const std::string& identifier)
    {
        auto block = Current;
        std::shared_ptr<Variable> variable = nullptr;

        if (block->Variables.find(identifier) != block->Variables.end())
            variable = Current->Variables[identifier];

        while (!variable)
        {
            block = block->Parent;

            if (!block)
                break;

            if (block->Variables.find(identifier) != block->Variables.end())
                variable = Current->Variables[identifier];
        }

        return variable;
    }

} /* Namespace Aryiele. */