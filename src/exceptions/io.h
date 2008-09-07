#ifndef IO_H
#define IO_H
#include "generic.h"
#include "config.h"

namespace Exceptions
{
    class LEECHCRAFT_API IO : public Generic
    {
    public:
        IO (const std::string& reason = std::string ()) throw ();
        virtual ~IO () throw ();
    };
};

#endif

