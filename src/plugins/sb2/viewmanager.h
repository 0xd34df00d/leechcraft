/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QUrl>
#include <QHash>
#include <QSet>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/iquarkcomponentprovider.h>

class QSettings;
class QMainWindow;
class QDir;
class QStandardItemModel;
class QToolBar;

namespace LC
{
class QuarkComponent;

namespace Util
{
	class ShortcutManager;
}

namespace SB2
{
	class ViewSettingsManager;
	class SBView;
	class QuarkManager;
	class ViewGeometryManager;

	typedef std::shared_ptr<QuarkManager> QuarkManager_ptr;

	class ViewManager : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		QStandardItemModel *ViewItemsModel_;
		SBView *View_;
		QToolBar *Toolbar_;

		QMainWindow *Window_;
		const bool IsDesktopMode_;
		const int OnloadWindowIndex_;

		QHash<QUrl, QuarkManager_ptr> Quark2Manager_;
		QSet<QString> RemovedIDs_;
		QStringList PreviousQuarkOrder_;

		QuarkComponents_t InternalComponents_;

		ViewSettingsManager *SettingsManager_;

		ViewGeometryManager *GeomManager_;
	public:
		ViewManager (const ICoreProxy_ptr&, Util::ShortcutManager*, QMainWindow*, QObject* = nullptr);

		SBView* GetView () const;
		QToolBar* GetToolbar () const;
		QMainWindow* GetManagedWindow () const;
		QRect GetFreeCoords () const;
		ViewSettingsManager* GetViewSettingsManager () const;

		bool IsDesktopMode () const;

		void SecondInit ();
		void RegisterInternalComponent (const QuarkComponent_ptr&);

		void RemoveQuark (const QUrl& loadedUrl);
		void RemoveQuark (const QString& id);
		void UnhideQuark (const QuarkComponent_ptr&, const QuarkManager_ptr&);
		void MoveQuark (int from, int to);

		void MovePanel (Qt::ToolBarArea);

		QuarkComponents_t FindAllQuarks () const;
		QList<QUrl> GetAddedQuarks () const;
		QuarkManager_ptr GetAddedQuarkManager (const QUrl&) const;

		int GetWindowIndex () const;

		std::shared_ptr<QSettings> GetSettings () const;
	private:
		template<int Role, typename T>
		void RemoveQuarkBy (const T&);

		void AddComponent (const QuarkComponent_ptr&, bool forceAdd);
		void AddComponent (const QuarkComponent_ptr&, const QuarkManager_ptr&, bool forceAdd);

		void AddToRemoved (const QString&);
		void RemoveFromRemoved (const QString&);

		void SaveRemovedList () const;
		void LoadRemovedList ();

		void SaveQuarkOrder ();
		void LoadQuarkOrder ();

		void HandleQuarksAdded (const QList<QUrl>&);
		void HandleQuarksRemoved (const QList<QUrl>&);
	};
}
}
