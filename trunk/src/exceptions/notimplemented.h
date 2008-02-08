#ifndef NOTIMPLEMENTED_H
#define NOTIMPLEMENTED_H
#include "generic.h"

namespace Exceptions
{
    /** @class NotImplemented
     * @author 0xd34df00d
     * @ingroup MEx
     * @brief Not Implemented exception.
     * Should be thrown when some requested functionality is not
     * available yet.
     *
     * @sa Generic
     */
    class NotImplemented : public Generic
    {
    public:
 NotImplemented (const std::string& reason = std::string ()) throw ();
 virtual ~NotImplemented () throw ();
    };
};

#endif

