#ifndef GENERIC_H
#define GENERIC_H
#include <string>
#include <exception>

namespace Exceptions
{
    /**
     * @defgroup MEx My Exception Hierarchy
     * @author 0xd34df00d
     * 
     * Trying to create full but extensible and handy exception
     * hierarchy.
     */
    /** @class Generic generic.h
     * @author 0xd34df00d
     * @ingroup MEx
     * @brief Generic exception.
     * This is a base class for all other classes in my exception
     * minilibrary, you should avoid throwing it directly, it is
     * better to subclass and throw that class.
     *
     * @sa NotImplemented, Logic, OutOfBounds, InvalidParameter
     */
    class Generic : public std::exception
    {
    /** Name of the exception */ 
    std::string Name_;
    /** Reason of the exception */
    std::string Reason_;
    public:
    Generic (const std::string& name = std::string (), const std::string& reason = std::string ()) throw ();
    virtual ~Generic () throw ();

    virtual const std::string& GetName () const throw ();
    virtual const std::string& GetReason () const throw ();
    virtual const char* what () const throw ();
    protected:
    virtual Generic& OverrideName (const std::string& name) throw ();
    };
};

#define MESSAGE(a) QString (QString(Q_FUNC_INFO) + QString (" ") + QString (a)).toStdString ()

#endif

