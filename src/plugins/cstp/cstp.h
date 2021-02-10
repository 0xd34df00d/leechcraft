/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QModelIndex>
#include <interfaces/iinfo.h>
#include <interfaces/idownload.h>
#include <interfaces/ijobholder.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/structures.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "core.h"

class QTabWidget;
class QToolBar;

namespace LC
{
namespace CSTP
{
	class Core;

	class CSTP : public QObject
				, public IInfo
				, public IDownload
				, public IJobHolder
				, public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IDownload IJobHolder IHaveSettings)

		LC_PLUGIN_METADATA ("org.LeechCraft.CSTP")

		ICoreProxy_ptr Proxy_;

		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;
		QToolBar *Toolbar_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		void Release () override;
		QByteArray GetUniqueID () const override;
		QString GetName () const override;
		QString GetInfo () const override;
		QStringList Provides () const override;
		QIcon GetIcon () const override;

		qint64 GetDownloadSpeed () const override;
		qint64 GetUploadSpeed () const override;
		EntityTestHandleResult CouldDownload (const LC::Entity&) const override;
		QFuture<Result> AddJob (LC::Entity) override;

		QAbstractItemModel* GetRepresentation () const override;
		IJobHolderRepresentationHandler_ptr CreateRepresentationHandler () override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;
	private:
		void SetupToolbar ();
	private slots:
		void handleFileExists (Core::FileExistsBehaviour*);
		void handleError (const QString&);
	};
}
}
