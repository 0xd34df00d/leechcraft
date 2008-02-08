#include "invalidparameter.h"

/** @brief Default constructor.
 *
 * @param reason why exception has been raised.
 * @sa OverrideName()
 */
Exceptions::InvalidParameter::InvalidParameter (const std::string& reason) throw ()
: Logic (reason)
{
    OverrideName ("Invalid Parameter");
}

/** Destructor */
Exceptions::InvalidParameter::~InvalidParameter () throw ()
{
}

