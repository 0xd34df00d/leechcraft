/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 * Copyright (C) 2011 ForNeVeR
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_CHOROID_CHOROID_H
#define PLUGINS_CHOROID_CHOROID_H
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>

namespace LC
{
namespace Choroid
{
	class Plugin : public QObject
				 , public IInfo
				 , public IHaveTabs
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveTabs)

		LC_PLUGIN_METADATA ("org.LeechCraft.Choroid")

		ICoreProxy_ptr Proxy_;
		TabClassInfo TabInfo_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);
	signals:
		void addNewTab (const QString&, QWidget*);
		void removeTab (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void statusBarChanged (QWidget*, const QString&);
		void raiseTab (QWidget*);
	};
}
}

#endif
