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

#pragma once

#include <memory>
#include <QObject>
#include <QUrl>
#include <QHash>
#include <QSet>
#include <interfaces/core/icoreproxy.h>

class QMainWindow;
class QDir;
class QStandardItemModel;
class QToolBar;

namespace LeechCraft
{
struct QuarkComponent;

namespace Util
{
	class ShortcutManager;
}

namespace SB2
{
	class SBView;
	class QuarkManager;

	typedef std::shared_ptr<QuarkManager> QuarkManager_ptr;

	class ViewManager : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		QStandardItemModel *ViewItemsModel_;
		SBView *View_;
		QToolBar *Toolbar_;

		QMainWindow *Window_;

		QHash<QUrl, QuarkManager_ptr> Quark2Manager_;
		QSet<QString> RemovedIDs_;
		QStringList PreviousQuarkOrder_;

		QList<QuarkComponent> InternalComponents_;
	public:
		ViewManager (ICoreProxy_ptr, Util::ShortcutManager*, QMainWindow*, QObject* = 0);

		SBView* GetView () const;
		QToolBar* GetToolbar () const;
		QMainWindow* GetManagedWindow () const;

		void SecondInit ();
		void RegisterInternalComponent (const QuarkComponent&);

		void ShowSettings (const QUrl&);

		void RemoveQuark (const QUrl& loadedUrl);
		void RemoveQuark (const QString& id);
		void UnhideQuark (const QuarkComponent&, QuarkManager_ptr);
		void MoveQuark (int from, int to);

		QList<QuarkComponent> FindAllQuarks () const;
		QList<QUrl> GetAddedQuarks () const;
		QuarkManager_ptr GetAddedQuarkManager (const QUrl&) const;
	private:
		void AddComponent (const QuarkComponent&);
		void AddComponent (const QuarkComponent&, QuarkManager_ptr);
		QList<QuarkComponent> ScanRootDir (const QDir&) const;

		void AddToRemoved (const QString&);
		void RemoveFromRemoved (const QString&);

		void SaveRemovedList () const;
		void LoadRemovedList ();

		void SaveQuarkOrder ();
		void LoadQuarkOrder ();

		int GetWindowIndex () const;
	public slots:
		void setOrientation (Qt::Orientation);
		void handleToolbarTopLevel (bool);
	};
}
}
