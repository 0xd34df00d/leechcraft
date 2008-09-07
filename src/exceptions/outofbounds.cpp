#include "outofbounds.h"

Exceptions::OutOfBounds::OutOfBounds (const std::string& reason) throw ()
: Logic (reason)
{
	OverrideName ("Out of bounds");
}

Exceptions::OutOfBounds::~OutOfBounds () throw ()
{
}

