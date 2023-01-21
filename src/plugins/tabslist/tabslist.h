/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QAction>
#include <interfaces/iinfo.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/ihaveshortcuts.h>

class ICoreTabWidget;

namespace LC
{
namespace TabsList
{
	class Plugin : public QObject
				 , public IInfo
				 , public IActionsExporter
				 , public IHaveShortcuts
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IActionsExporter IHaveShortcuts)

		LC_PLUGIN_METADATA ("org.LeechCraft.TabsList")

		ICoreProxy_ptr Proxy_;
		QAction *ShowList_ = nullptr;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QList<QAction*> GetActions (ActionsEmbedPlace) const;

		QMap<QByteArray, ActionInfo> GetActionInfo () const;
		void SetShortcut (const QByteArray&, const QKeySequences_t&);

		void RemoveTab (ICoreTabWidget*, int);
	private slots:
		void handleShowList ();
		void navigateToTab ();
	signals:
		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace);
	};
}
}
