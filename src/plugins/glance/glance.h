/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#ifndef PLUGINS_GLANCE_GLANCE_H
#define PLUGINS_GLANCE_GLANCE_H
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iactionsexporter.h>

class QAction;

namespace LeechCraft
{
namespace Plugins
{
namespace Glance
{
	class GlanceShower;

	class Plugin : public QObject
				 , public IInfo
				 , public IActionsExporter
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IActionsExporter)

		QAction *ActionGlance_;
		GlanceShower *Glance_;
	public:
		void Init (ICoreProxy_ptr proxy);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
		
		QList<QAction*> GetActions (ActionsEmbedPlace) const;
	public slots:
		void on_ActionGlance__triggered ();
	};
};
};
};

#endif // PLUGINS_GLANCE_GLANCE_H
