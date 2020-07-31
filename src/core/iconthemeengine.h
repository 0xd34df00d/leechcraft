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
#include <QString>
#include <QDir>
#include <QHash>
#include <QReadWriteLock>
#include <QIcon>
#include "../interfaces/core/iiconthememanager.h"

class QIcon;
class QAction;
class QPushButton;
class QTabWidget;
class QToolButton;
class QFile;

namespace LC
{
	namespace Util
	{
		class ResourceLoader;
	}

	class IconThemeEngine : public QObject
	{
		Q_OBJECT

		QString OldIconSet_;
		QStringList IconSets_;

		QReadWriteLock IconCacheLock_;
		QHash<QPair<QString, QString>, QIcon> IconCache_;

		QHash<QString, QIcon> PluginIconCache_;
		std::shared_ptr<Util::ResourceLoader> PluginIconLoader_;

		QList<std::function<void ()>> Handlers_;

		IconThemeEngine ();
	public:
		static IconThemeEngine& Instance ();

		QIcon GetIcon (const QString&, const QString& = {});
		void UpdateIconset (const QList<QAction*>&);
		void UpdateIconset (const QList<QPushButton*>&);
		void UpdateIconset (const QList<QToolButton*>&);

		void ManageWidget (QWidget*);

		void RegisterChangeHandler (const std::function<void ()>&);

		QStringList ListIcons () const;

		QIcon GetPluginIcon (const QString& basename);
	protected:
		bool eventFilter (QObject*, QEvent*) override;
	private:
		template<typename T>
		void SetIcon (T);
		void FindIconSets ();
		void FindIcons ();
	private slots:
		void flushCaches ();
	};
};
