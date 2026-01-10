/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iplugin2.h>
#include <interfaces/lmp/ilmpplugin.h>
#include <interfaces/lmp/isyncplugin.h>

class IRemovableDevManager;
class QConcatenateTablesProxyModel;
class QSortFilterProxyModel;

namespace LC::LMP::DumbSync
{
	class Plugin : public QObject
				 , public IInfo
				 , public IHaveSettings
				 , public IPlugin2
				 , public ILMPPlugin
				 , public ISyncPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IHaveSettings
				IPlugin2
				LC::LMP::ILMPPlugin
				LC::LMP::ISyncPlugin)

		LC_PLUGIN_METADATA ("org.LeechCraft.LMP.DumbSync")

		Util::XmlSettingsDialog_ptr XSD_;
		ILMPProxy_ptr LMPProxy_;

		QConcatenateTablesProxyModel *DevModelsMerger_;
		QSortFilterProxyModel *SyncTargets_;

		QHash<const QAbstractItemModel*, IRemovableDevManager*> Model2DevManager_;
	public:
		void Init (ICoreProxy_ptr proxy) override;
		void SecondInit () override;
		void Release () override;
		QByteArray GetUniqueID () const override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;

		QSet<QByteArray> GetPluginClasses () const override;

		void SetLMPProxy (ILMPProxy_ptr) override;

		QObject* GetQObject () override;
		QString GetSyncSystemName () const override;
		QAbstractItemModel& GetSyncTargetsModel () override;
		void RefreshSyncTargets () override;
		ISyncPluginConfigWidget_ptr MakeConfigWidget (const QModelIndex&) override;
		Util::ContextTask<UploadResult> Upload (UploadJob) override;
	private:
		Util::ContextTask<Util::Either<UploadFailure, QString>> EnsureMounted (QPersistentModelIndex);
	};
}
