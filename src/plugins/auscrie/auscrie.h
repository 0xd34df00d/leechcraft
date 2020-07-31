/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
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

namespace LC
{
namespace Auscrie
{
	class ShooterDialog;

	class Plugin : public QObject
					, public IInfo
					, public IActionsExporter
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IActionsExporter)

		LC_PLUGIN_METADATA ("org.LeechCraft.Auscrie")

		ICoreProxy_ptr Proxy_;
		QAction *ShotAction_;
		ShooterDialog *Dialog_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QList<QAction*> GetActions (ActionsEmbedPlace) const;
	private:
		void PerformAction ();
		void MakeScreenshot (int);
		QPixmap GetPixmap () const;
	signals:
		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace);
	};
}
}
