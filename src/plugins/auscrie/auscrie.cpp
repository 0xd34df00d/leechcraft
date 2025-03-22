/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "auscrie.h"
#include <QMainWindow>
#include <QIcon>
#include <QTimer>
#include <QDir>
#include <QFileDialog>
#include <QPixmap>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include <util/xpc/util.h>
#include <util/sll/unreachable.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/ientityhandler.h>
#include "platform.h"
#include "shooterdialog.h"
#include "util.h"

namespace LC::Auscrie
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Dialog_ = new ShooterDialog;

		ShotAction_ = new QAction (GetIcon (),
				tr ("Make a screenshot"),
				this);
		connect (ShotAction_,
				&QAction::triggered,
				this,
				[this] { MakeScreenshot (0); });
		connect (Dialog_,
				&ShooterDialog::screenshotRequested,
				this,
				[this] { MakeScreenshot (Dialog_->GetTimeout () * 1000); },
				Qt::QueuedConnection);
		connect (Dialog_,
				&QDialog::accepted,
				this,
				&Plugin::PerformAction);
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Auscrie";
	}

	QString Plugin::GetName () const
	{
		return QStringLiteral ("Auscrie");
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Simple auto screenshotter.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace place) const
	{
		QList<QAction*> result;

		if (place == ActionsEmbedPlace::CommonContextMenu)
			result << ShotAction_;

		return result;
	}

	void Plugin::PerformAction ()
	{
		const auto& pm = Dialog_->GetScreenshot ();
		if (pm.isNull ())
			return;

		auto mw = GetProxyHolder ()->GetRootWindowsManager ()->GetPreferredWindow ();
		const int quality = Dialog_->GetQuality ();

		switch (Dialog_->GetAction ())
		{
		case ShooterDialog::Action::Save:
		{
			const auto sm = GetProxyHolder ()->GetSettingsManager ();
			const auto settingsKey = "PluginsStorage/Auscrie/SavePath";
			const auto& defaultPath = QDir::currentPath () + "01." + Dialog_->GetFormat ();
			const auto& path = sm->Property (settingsKey, defaultPath).toString ();

			const auto& filename = QFileDialog::getSaveFileName (mw,
					tr ("Save as"),
					path,
					tr ("%1 files (*.%1);;All files (*.*)")
						.arg (Dialog_->GetFormat ()));

			if (!filename.isEmpty ())
			{
				pm.save (filename,
						qPrintable (Dialog_->GetFormat ()),
						quality);
				sm->setProperty (settingsKey, filename);
			}
			break;
		}
		case ShooterDialog::Action::Upload:
		{
			const auto& info = Dialog_->GetDFInfo ();
			if (!info.Object_)
			{
				qWarning () << Q_FUNC_INFO
						<< "no object set";
				break;
			}

			auto e = Util::MakeEntity (pm.toImage (),
					{}, {}, QStringLiteral ("x-leechcraft/data-filter-request"));
			e.Additional_ [QStringLiteral ("Format")] = Dialog_->GetFormat ();
			e.Additional_ [QStringLiteral ("Quality")] = quality;
			e.Additional_ [QStringLiteral ("DataFilter")] = info.Variant_;

			auto ieh = qobject_cast<IEntityHandler*> (info.Object_);
			ieh->Handle (e);

			const auto iinfo = qobject_cast<IInfo*> (info.Object_);
			SaveFilterState ({ iinfo->GetUniqueID (), info.Variant_ });

			break;
		}
		}
	}

	void Plugin::MakeScreenshot (int timeout)
	{
		Dialog_->setVisible (!Dialog_->ShouldHide ());

		ShotAction_->setEnabled (false);
		const auto minTimeout = 200;
		QTimer::singleShot (std::max (timeout, minTimeout),
				this,
				[this]
				{
					ShotAction_->setEnabled (true);

					const auto& pm = GetPixmap (Dialog_->GetMode ());
					Dialog_->show ();
					Dialog_->SetScreenshot (pm);
				});
	}
}

LC_EXPORT_PLUGIN (leechcraft_auscrie, LC::Auscrie::Plugin);
