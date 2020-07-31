/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "otzerkalu.h"
#include <QIcon>
#include <QUrl>
#include <interfaces/entitytesthandleresult.h>
#include "otzerkaludialog.h"
#include <interfaces/core/icoreproxy.h>

namespace LC
{
namespace Otzerkalu
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
		RepresentationModel_ = new QStandardItemModel (this);
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Otzerkalu";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Otzerkalu";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Otzerkalu allows one to recursively download a web site.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& entity) const
	{
		const bool can = !entity.Entity_.toUrl ().isEmpty () &&
				(entity.Parameters_ & FromUserInitiated) &&
				entity.Additional_.value ("AllowedSemantics").toStringList ().contains ("save");
		return can ?
				EntityTestHandleResult (EntityTestHandleResult::PHigh) :
				EntityTestHandleResult ();
	}

	void Plugin::Handle (Entity entity)
	{
		QUrl dUrl = entity.Entity_.toUrl ();
		if (!dUrl.isValid ())
			return;

		OtzerkaluDialog dialog;
		if (dialog.exec () != QDialog::Accepted)
			return;

		const QList<QStandardItem*> row
		{
			new QStandardItem (tr ("Mirroring %1...").arg (dUrl.toString ())),
			new QStandardItem ("0/0"),
			new QStandardItem ()
		};
		auto progressItem = row [1];
		RepresentationModel_->appendRow (row);

		const auto dl = new OtzerkaluDownloader ({
					.DownloadUrl_ = dUrl,
					.DestDir_ = dialog.GetDir (),
					.RecLevel_ = dialog.GetRecursionLevel (),
					.FromOtherSite_ = dialog.FetchFromExternalHosts ()
				},
				Proxy_,
				this);

		connect (dl,
				&OtzerkaluDownloader::fileDownloaded,
				this,
				[progressItem] (int count) { progressItem->setText (QString ("%1/%2").arg (count).arg ("unknown")); });
		connect (dl,
				&OtzerkaluDownloader::mirroringFinished,
				this,
				[this, progressItem] { RepresentationModel_->removeRow (progressItem->row ()); });

		dl->Begin ();
	}

	QAbstractItemModel* Plugin::GetRepresentation () const
	{
		return RepresentationModel_;
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_otzerkalu, LC::Otzerkalu::Plugin);
