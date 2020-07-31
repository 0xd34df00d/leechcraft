/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPointer>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ijobholder.h>
#include <interfaces/lmp/ilmpplugin.h>

class QWidget;

namespace LC
{
namespace LMP
{
namespace BrainSlugz
{
	class CheckTab;
	class ProgressModelManager;

	class Plugin : public QObject
				 , public IInfo
				 , public IHaveTabs
				 , public IPlugin2
				 , public IJobHolder
				 , public ILMPPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveTabs IPlugin2 IJobHolder LC::LMP::ILMPPlugin)

		LC_PLUGIN_METADATA ("org.LeechCraft.LMP.BrainSlugz")

		TabClassInfo CheckTC_;

		ICoreProxy_ptr CoreProxy_;
		ILMPProxy_ptr LmpProxy_;

		QPointer<CheckTab> OpenedTab_;

		ProgressModelManager *ProgressModelManager_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);

		QSet<QByteArray> GetPluginClasses () const;

		QAbstractItemModel* GetRepresentation () const;

		void SetLMPProxy (ILMPProxy_ptr);
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
}
