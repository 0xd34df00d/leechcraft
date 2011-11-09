/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_SNAILS_PROGRESSLISTENER_H
#define PLUGINS_SNAILS_PROGRESSLISTENER_H
#include <QMetaType>
#include <QPointer>
#include <QObject>
#include <vmime/utility/progressListener.hpp>

namespace LeechCraft
{
namespace Snails
{
	class ProgressListener : public QObject
						   , public vmime::utility::progressListener
	{
		Q_OBJECT

		QString Context_;
	public:
		ProgressListener (const QString&, QObject* = 0);

		QString GetContext () const;

		bool cancel () const;
	signals:
		void start (const int);
		void progress (const int, const int);
		void stop (const int);
	};

	typedef QPointer<ProgressListener> ProgressListener_g_ptr;
}
}

Q_DECLARE_METATYPE (LeechCraft::Snails::ProgressListener_g_ptr);

#endif
