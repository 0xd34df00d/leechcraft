/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_GLANCE_GLANCE_H
#define PLUGINS_GLANCE_GLANCE_H
#include <QObject>
#include <QAction>
#include <interfaces/iinfo.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/ihaveshortcuts.h>

namespace LC
{
namespace Plugins
{
namespace Glance
{
	class GlanceShower;

	class Plugin : public QObject
				 , public IInfo
				 , public IActionsExporter
				 , public IHaveShortcuts
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IActionsExporter IHaveShortcuts)

		LC_PLUGIN_METADATA ("org.LeechCraft.Glance")

		ICoreProxy_ptr Proxy_;
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

		QMap<QByteArray, ActionInfo> GetActionInfo () const;
		void SetShortcut (const QByteArray&, const QKeySequences_t&);
	public slots:
		void on_ActionGlance__triggered ();
	signals:
		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace);
	};
};
};
};

#endif // PLUGINS_GLANCE_GLANCE_H
