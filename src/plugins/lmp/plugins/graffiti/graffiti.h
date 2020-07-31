/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QMenu>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ijobholder.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/lmp/ilmpplugin.h>

namespace LC
{
namespace LMP
{
struct MediaInfo;

namespace Graffiti
{
	class ProgressManager;
	class GraffitiTab;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveTabs
				 , public IJobHolder
				 , public ILMPPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IPlugin2
				IHaveTabs
				IJobHolder
				LC::LMP::ILMPPlugin)

		LC_PLUGIN_METADATA ("org.LeechCraft.LMP.Graffiti")

		ICoreProxy_ptr CoreProxy_;
		ILMPProxy_ptr LMPProxy_;

		TabClassInfo TaggerTC_;

		ProgressManager *ProgressMgr_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray& tabClass);

		QAbstractItemModel* GetRepresentation () const;

		void SetLMPProxy (ILMPProxy_ptr);
	private:
		GraffitiTab* MakeTab ();
	public slots:
		void hookPlaylistContextMenuRequested (LC::IHookProxy_ptr,
				QMenu*,
				const LC::LMP::MediaInfo&);
		void hookCollectionContextMenuRequested (LC::IHookProxy_ptr,
				QMenu*,
				const LC::LMP::MediaInfo&);
	private slots:
		void handleOpenTabFromContextMenu ();
	signals:
		void addNewTab (const QString&, QWidget*);
		void removeTab (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void raiseTab (QWidget*);
		void statusBarChanged (QWidget*, const QString&);
	};
}
}
}
