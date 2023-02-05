/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 * Copyright (C) 2006-2014  Georg Rudoy
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

namespace LC::Glance
{
	class Plugin : public QObject
				 , public IInfo
				 , public IActionsExporter
				 , public IHaveShortcuts
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IActionsExporter IHaveShortcuts)

		LC_PLUGIN_METADATA ("org.LeechCraft.Glance")

		QAction *ActionGlance_;
	public:
		void Init (ICoreProxy_ptr proxy) override;
		void SecondInit () override;
		void Release () override;
		QByteArray GetUniqueID () const override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QList<QAction*> GetActions (ActionsEmbedPlace) const override;

		QMap<QByteArray, ActionInfo> GetActionInfo () const override;
		void SetShortcut (const QByteArray&, const QKeySequences_t&) override;
	private:
		void ShowGlance ();
	signals:
		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace) override;
	};
}
