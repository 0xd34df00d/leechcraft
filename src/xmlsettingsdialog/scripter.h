/*
    Copyright (c) 2008 by Rudoy Georg <0xd34df00d@gmail.com>

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*/
#ifndef SCRIPTER_H
#define SCRIPTER_H
#include <memory>
#include <QVariant>
#include <QDomElement>

class QScriptEngine;

namespace LeechCraft
{
	class Scripter
	{
		std::auto_ptr<QScriptEngine> Engine_;
		QDomElement Container_;
	public:
		Scripter (const QDomElement&);

		QStringList GetOptions ();
		QString HumanReadableOption (const QString&);
	private:
		QString GetScript (const QDomElement&) const;
		void FeedRequiredClasses () const;
		void Reset ();
	};
};

#endif

