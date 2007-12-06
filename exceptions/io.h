#ifndef IO_H
#define IO_H
#include "generic.h"

namespace Exceptions
{
    class IO : public Generic
    {
	public:
		IO (const std::string& reason = std::string ()) throw ();
		virtual ~IO () throw ();
    };
};

#endif

