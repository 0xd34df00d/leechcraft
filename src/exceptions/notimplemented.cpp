#include "notimplemented.h"

/** @brief Default constructor.
 * Constructs exception with a given reason and a "Not implemented"
 * name.
 *
 * @param reason why exception has been raised.
 * @sa Generic
 */
Exceptions::NotImplemented::NotImplemented (const std::string& reason) throw ()
: Generic ("Not implemented", reason)
{}

/** Destructor */
Exceptions::NotImplemented::~NotImplemented () throw ()
{}

