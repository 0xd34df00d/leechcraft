#include "logic.h"

/** @brief Default constructor.
 * Constructs exception with a given reason and a "Logic" name.
 *
 * @param reason why exception has been raised.
 * @sa Generic
 */
Exceptions::Logic::Logic (const std::string& reason) throw ()
: Generic ("Logic", reason) 
{
    try
    {
    Corrections_ = "[undefined]";
    }
    catch (...) {}
}

/** Destructor */
Exceptions::Logic::~Logic () throw ()
{
}

/** @brief Sets the message with possible correction variants.
 * If we know what to do to avoid this error we can call this
 * function with a needed message in code that throws the exception
 * and, for example, show it to the user in handling code.
 *
 * @param cor message with corrections.
 */
Exceptions::Logic& Exceptions::Logic::SetCorrections (const std::string& cor) throw ()
{
    try
    {
    Corrections_ = cor;
    }
    catch (...) {}
    return *this;
}

/** @brief Gets the message with possible correction variants.
 * We can call this function to get the possible correction variants
 * to show it to user, for example.
 *
 * @return message with corrections.
 */
const std::string& Exceptions::Logic::GetCorrections () const throw ()
{
    return Corrections_;
}

