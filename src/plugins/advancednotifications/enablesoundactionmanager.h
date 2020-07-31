/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_ADVANCEDNOTIFICATIONS_ENABLESOUNDACTIONMANAGER_H
#define PLUGINS_ADVANCEDNOTIFICATIONS_ENABLESOUNDACTIONMANAGER_H
#include <QObject>
#include <interfaces/iactionsexporter.h>

class QAction;

namespace LC
{
namespace AdvancedNotifications
{
	class EnableSoundActionManager : public QObject
	{
		Q_OBJECT

		QAction *EnableAction_;
	public:
		EnableSoundActionManager (QObject* = 0);

		QAction* GetAction () const;

		QList<QAction*> GetActions (ActionsEmbedPlace) const;
	private slots:
		void xsdPropChanged ();
		void enableSounds (bool);
	};
}
}

#endif
