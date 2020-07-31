/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>
#include <QList>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihavesettings.h>

namespace LC
{
namespace Poleemery
{
	class Plugin : public QObject
				 , public IInfo
				 , public IHaveTabs
				 , public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveTabs IHaveSettings)

		LC_PLUGIN_METADATA ("org.LeechCraft.Poleemery")

		QList<QPair<TabClassInfo, std::function<void (TabClassInfo)>>> TabClasses_;
		Util::XmlSettingsDialog_ptr XSD_;
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

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
	private:
		template<typename T>
		void MakeTab (const TabClassInfo&);
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
