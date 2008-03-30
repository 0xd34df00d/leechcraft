#include "io.h"

Exceptions::IO::IO (const std::string& reason) throw ()
: Generic ("IO", reason)
{
}

Exceptions::IO::~IO () throw ()
{
}

