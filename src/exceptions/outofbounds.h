#ifndef OUTOFBOUNDS_H
#define OUTOFBOUNDS_H
#include "logic.h"
#include "config.h"

namespace Exceptions
{
    /** @class OutOfBounds outofbounds.h
     * @author 0xd34df00d
     * @ingroup MEx
     * @brief Out Of Bounds exception.
     * Should be thrown when provided index, iterator etc. is out of
     * bounds.
     *
     * @sa Logic, InvalidParameter
     */
    class LEECHCRAFT_API OutOfBounds : public Logic
    {
    public:
		/** @brief Default Constructor.
		 * Constructs an exception with given parameters and
		 * overriden name "Out of bounds".
		 *
		 * @param reason why exception has been raised.
		 * @param index the index or similar entity that caused the
		 * out of bounds error.
		 * @param leftbound the smallest allowed index or similar
		 * entity.
		 * @param rightbound the biggest allowed index or similar
		 * entity.
		 * @sa OverrideName()
		 */
		OutOfBounds (const std::string& reason = std::string ()) throw ();
		/** Destructor */
		virtual ~OutOfBounds () throw ();
	};
};


#endif

