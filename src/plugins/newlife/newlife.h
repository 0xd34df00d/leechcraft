/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QStringList>
#include <interfaces/iinfo.h>
#include <interfaces/iactionsexporter.h>

class QTranslator;

namespace LC
{
namespace NewLife
{
	class Plugin : public QObject
					, public IInfo
					, public IActionsExporter
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IActionsExporter)

		LC_PLUGIN_METADATA ("org.LeechCraft.NewLife")

		ICoreProxy_ptr Proxy_;

		QAction *ImporterAction_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QList<QAction*> GetActions (ActionsEmbedPlace) const;
	private slots:
		void runWizard ();
	signals:
		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace);
	};
}
}
