#ifndef SITUATIVE_H
#define SITUATIVE_H
#include "logic.h"
#include "config.h"

namespace Exceptions
{
    class LEECHCRAFT_API Situative : public Logic
    {
    public:
        LEECHCRAFT_API Situative (const std::string& reason = std::string ()) throw ();
        LEECHCRAFT_API virtual ~Situative () throw ();
    };
};

#endif

