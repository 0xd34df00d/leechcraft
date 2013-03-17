/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#pragma once

#include <QObject>
#include <interfaces/iquarkcomponentprovider.h>
#include <interfaces/core/icoreproxy.h>

class QDockWidget;
class QStandardItemModel;
class QStandardItem;

namespace LeechCraft
{
enum class ActionsEmbedPlace;

namespace SB2
{
	class ActionImageProvider;
	class SBView;

	class TrayComponent : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		QStandardItemModel *Model_;
		QuarkComponent Component_;

		ActionImageProvider *ImageProv_;

		int NextActionId_;

		enum class ActionPos
		{
			Beginning,
			End
		};

		SBView *View_;
	public:
		TrayComponent (ICoreProxy_ptr, SBView*, QObject* parent = 0);

		QuarkComponent GetComponent () const;
		void HandleDockAction (QDockWidget*, bool);
	private:
		QStandardItem* FindItem (QAction*) const;
		void AddActions (const QList<QAction*>&, ActionPos);
		void RemoveAction (QAction*);
	public slots:
		void handlePluginsAvailable ();
	private slots:
		void handleGotActions (const QList<QAction*>&, LeechCraft::ActionsEmbedPlace);
		void handleActionDestroyed ();
		void handleActionChanged ();
	};
}
}
