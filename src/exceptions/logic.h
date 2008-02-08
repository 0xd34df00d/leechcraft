#ifndef LOGIC_H
#define LOGIC_H
#include "generic.h"

namespace Exceptions
{
    /** @class Logic logic.h
     * @author 0xd34df00d
     * @ingroup MEx
     * @brief Logic exception.
     * Should be thrown whenever a logic error occurs. It's hard to
     * explain what we call "logic". In most cases client could do
     * some assertions and checks. Try to avoid throwing this class
     * directly, better subclass and throw that class.
     *
     * @sa Generic, OutOfBounds, InvalidParameter
     */
    class Logic : public Generic
    {
    /** @brief Correction variants  representation.
     * Keeps the message with the correction variants.
     *
     * @sa SetCorrections ), GetCorrections()
     */
    std::string Corrections_;
    public:
    Logic (const std::string& reason = std::string ()) throw ();
    virtual ~Logic () throw ();

    virtual Logic& SetCorrections (const std::string&) throw ();
    virtual const std::string& GetCorrections () const throw ();
    };
};

#endif

