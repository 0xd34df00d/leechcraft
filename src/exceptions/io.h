#ifndef IO_H
#define IO_H
#include "generic.h"
#include "config.h"

namespace Exceptions
{
    class LEECHCRAFT_API IO : public Generic
    {
    public:
        LEECHCRAFT_API IO (const std::string& reason = std::string ()) throw ();
        LEECHCRAFT_API virtual ~IO () throw ();
    };
};

#endif

