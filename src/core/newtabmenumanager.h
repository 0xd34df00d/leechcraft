/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QMap>
#include <QString>
#include <QSet>

class QMenu;
class QAction;

class ITabWidget;

namespace LC
{
	class NewTabMenuManager : public QObject
	{
		Q_OBJECT

		QMenu *NewTabMenu_;
		QMenu *AdditionalTabMenu_;
		QList<QObject*> RegisteredMultiTabs_;
		QSet<QChar> UsedAccelerators_;
		QMap<QObject*, QMap<QString, QAction*>> HiddenActions_;
	public:
		explicit NewTabMenuManager (QObject* = nullptr);

		void AddObject (QObject*);
		void SetToolbarActions (QList<QList<QAction*>>);
		void SingleRemoved (ITabWidget*);

		QMenu* GetNewTabMenu () const;
		QMenu* GetAdditionalMenu ();

		void ToggleHide (ITabWidget*, bool hide);
		void HideAction (ITabWidget *itw);
	private:
		QString AccelerateName (QString);
		void ToggleHide (QObject*, const QByteArray&, bool);
		void OpenTab (QAction*);
		void InsertAction (QAction*);
		void InsertActionWParent (QAction*, QObject*, bool sub);
	signals:
		void restoreTabActionAdded (QAction*);
	};
}

