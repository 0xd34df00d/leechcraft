/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef INTERFACES_IDIAGINFO_H
#define INTERFACES_IDIAGINFO_H
#include <QtPlugin>

class QString;

/** @brief Interface for plugins having human-readable diagnostic data.
 *
 * Library versions, the compiled-in features, etc., count as diagnostic
 * info. The info aggregated from all plugins will be sent to the issue
 * tracker with bug reports, for example, so returning proper data can
 * aid in debugging.
 */
class Q_DECL_EXPORT IHaveDiagInfo
{
public:
	virtual ~IHaveDiagInfo () {}

	/** @brief Returns the human-readable diagnostic info.
	 *
	 * @return The human-readable diagnostic info.
	 */
	virtual QString GetDiagInfoString () const = 0;
};

Q_DECLARE_INTERFACE (IHaveDiagInfo, "org.Deviant.LeechCraft.IHaveDiagInfo/1.0")

#endif
