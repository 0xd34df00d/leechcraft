/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "syncevents.h"
#include "ui_devicesbrowserwidget.h"

class QConcatenateTablesProxyModel;

class IRemovableDevManager;

namespace LC
{
namespace LMP
{
	class ISyncPlugin;
	class ISyncPluginConfigWidget;
	class UploadModel;

	class DevicesBrowserWidget : public QWidget
	{
		Q_OBJECT

		Ui::DevicesBrowserWidget Ui_;
		UploadModel *DevUploadModel_;

		std::unique_ptr<QConcatenateTablesProxyModel> Merger_;
		QHash<const QAbstractItemModel*, ISyncPlugin*> Model2Syncer_;

		std::unique_ptr<ISyncPluginConfigWidget> SyncerConfigWidget_;

		QMap<QString, TranscodingParams> Device2Params_;
	public:
		explicit DevicesBrowserWidget (QWidget* = nullptr);
		~DevicesBrowserWidget () override;

		void InitializeUploaders ();
	private:
		void UpdateGuiForSyncer (int);
		QModelIndex GetSourceIndex (int) const;
		ISyncPlugin* GetSyncerForIndex (int) const;

		void LoadLastParams ();
		void SaveLastParams () const;

		void HandleSyncEvent (const SyncEvents::Event&);
		static QString ToString (const SyncEvents::Event&);
	private slots:
		void on_UploadButton__released ();
		void on_RefreshButton__released ();
	};
}
}
