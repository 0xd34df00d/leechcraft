/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_LEGACYFORMBUILDER_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_LEGACYFORMBUILDER_H
#include <boost/function.hpp>
#include <QObject>
#include <QHash>
#include <QXmppElement.h>

class QWidget;

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class LegacyFormBuilder : public QObject
	{
		QWidget *Widget_;

		typedef boost::function<void (QWidget*, const QXmppElement&)> ElementActor_t;
		QHash<QString, ElementActor_t> Tag2Actor_;
	public:
		LegacyFormBuilder ();
		
		QWidget* CreateForm (const QXmppElement&, QWidget* = 0);
		QList<QXmppElement> GetFilledChildren () const;
		
		QString GetUsername () const;
		QString GetPassword () const;
	};
}
}
}

#endif
