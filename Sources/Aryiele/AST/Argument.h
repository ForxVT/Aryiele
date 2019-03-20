#ifndef ARYIELE_ARGUMENT_H
#define ARYIELE_ARGUMENT_H

#include <Aryiele/Core/Includes.h>

namespace Aryiele
{
    struct Argument
    {
        Argument(const std::string& name, const std::string& type);

        std::string Name;
        std::string Type;
    };

} /* Namespace Aryiele. */

#endif /* ARYIELE_ARGUMENT_H. */
