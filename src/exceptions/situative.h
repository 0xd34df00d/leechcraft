#ifndef SITUATIVE_H
#define SITUATIVE_H
#include "logic.h"

namespace Exceptions
{
	class Situative : public Logic
	{
	public:
		Situative (const std::string& reason = std::string ()) throw ();
		virtual ~Situative () throw ();
	};
};

#endif

