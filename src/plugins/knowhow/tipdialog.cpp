/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tipdialog.h"
#include <QDomDocument>
#include <util/util.h>
#include <util/sys/resourceloader.h>
#include <interfaces/core/iiconthememanager.h>
#include "xmlsettingsmanager.h"

namespace LC
{
namespace KnowHow
{
	TipDialog::TipDialog (ICoreProxy_ptr proxy, QWidget *parent)
	: QDialog (parent)
	, Proxy_ (proxy)
	{
		Util::ResourceLoader rl ("knowhow/tips/");
		rl.AddGlobalPrefix ();
		rl.AddLocalPrefix ();

		const QString pre { "tips_" };
		const QStringList vars
		{
			pre + Util::GetLocaleName () + ".xml",
			pre + Util::GetLocaleName ().left (2) + ".xml",
			pre + "en.xml"
		};

		const QString& path = rl.GetPath (vars);
		if (path.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to load tips for"
					<< vars;
			deleteLater ();
			return;
		}

		QFile file (path);
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< file.fileName ()
					<< file.errorString ();
			deleteLater ();
			return;
		}

		Doc_ = std::make_shared<QDomDocument> ();
		if (!Doc_->setContent (&file))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to parse XML from file"
					<< path;
			deleteLater ();
			return;
		}

		const int idx = XmlSettingsManager::Instance ()
				.Property ("StdTipIndex", -1).toInt () + 1;

		Ui_.setupUi (this);

		const auto mgr = Proxy_->GetIconThemeManager ();
		Ui_.Forward_->setIcon (mgr->GetIcon ("go-next"));
		Ui_.Backward_->setIcon (mgr->GetIcon ("go-previous"));

		ShowForIdx (idx);

		setAttribute (Qt::WA_DeleteOnClose, true);
		show ();
	}

	void TipDialog::ShowForIdx (int idx)
	{
		auto tip = GetTipByID (idx);
		if (tip.isEmpty () && !idx)
		{
			qWarning () << Q_FUNC_INFO
					<< "empty tip right for the first one!";
			return;
		}
		else if (tip.isEmpty ())
		{
			ShowForIdx (0);
			return;
		}

		XmlSettingsManager::Instance ().setProperty ("StdTipIndex", idx);

		tip.replace ('\n', "<br/>");
		Ui_.TipEdit_->setHtml (tip);
	}

	QString TipDialog::GetTipByID (int idx)
	{
		const auto& tips = Doc_->firstChildElement ().elementsByTagName ("tip");
		const auto count = tips.count ();

		auto tipIdx = idx % count;
		if (tipIdx < 0)
			tipIdx += count;

		const auto& elem = tips.at (tipIdx).toElement ();

		if (elem.isNull ())
			return {};

		return elem.text ().trimmed ();
	}

	void TipDialog::on_Forward__released ()
	{
		const int idx = XmlSettingsManager::Instance ()
				.Property ("StdTipIndex", -1).toInt () + 1;
		ShowForIdx (idx);
	}

	void TipDialog::on_Backward__released ()
	{
		const int idx = XmlSettingsManager::Instance ()
				.Property ("StdTipIndex", 1).toInt () - 1;
		ShowForIdx (idx);
	}

	void TipDialog::on_DontShow__stateChanged ()
	{
		const bool show = Ui_.DontShow_->checkState () == Qt::Unchecked;
		XmlSettingsManager::Instance ().setProperty ("ShowTips", show);
	}
}
}
