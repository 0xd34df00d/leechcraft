#include "generic.h"

/** Default constructor.
 *
 * @param name name of the exception (i.e. Logic, Not Implemented).
 * @param reason why the exception has been raised.
 */
Exceptions::Generic::Generic (const std::string& name, const std::string& reason) throw ()
{
    try
    {
   Name_ = name;
   Reason_ = reason;
    }
    catch (...) {}
}

/** Destructor */
Exceptions::Generic::~Generic () throw ()
{
}

/** @brief Returns the name.
 * Returns name of the excepion, possibly with some modifications.
 *
 * @return name name of the exception.
 */
const std::string& Exceptions::Generic::GetName () const throw ()
{
    return Name_;
}

/** @brief Returns reason.
 * Returns reason of the exception. Similar to std::exception::what ().
 *
 * @return reason why the exception has been rised.
 * @sa what() 
 */
const std::string& Exceptions::Generic::GetReason () const throw ()
{
    return Reason_;
}

/** @brief Returns reason in C-string.
 * Returns C-string with a bit modified content of GetReason () 's
 * one. Provided for compatibility with std::exception.
 *
 * @return reason of the exception in C-string format.
 * @sa GetReason()
 */
const char* Exceptions::Generic::what () const throw ()
{
    return (GetReason () + "\n\t\tPlease tell author what you've been doing.").c_str ();
}

/** @brief Overrides the default name.
 *
 * Allows to change name outside of this class after exception has
 * been constructed
 *
 * @param name the new name.
 * @return reference to itself.
 */
Exceptions::Generic& Exceptions::Generic::OverrideName (const std::string& name) throw ()
{
    try
    {
   Name_ = name;
    }
    catch (...) {}

    return *this;
}

