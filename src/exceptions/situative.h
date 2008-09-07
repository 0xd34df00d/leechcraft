#ifndef SITUATIVE_H
#define SITUATIVE_H
#include "logic.h"
#include "config.h"

namespace Exceptions
{
    class LEECHCRAFT_API Situative : public Logic
    {
    public:
        Situative (const std::string& reason = std::string ()) throw ();
        virtual ~Situative () throw ();
    };
};

#endif

