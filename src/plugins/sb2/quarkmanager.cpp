/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "quarkmanager.h"
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QStandardItem>
#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QFile>
#include <QtDebug>
#include <QFileInfo>
#include <QDir>
#include <qjson/parser.h>
#include <interfaces/iquarkcomponentprovider.h>
#include "viewmanager.h"
#include "sbview.h"
#include "quarksettingsmanager.h"

namespace LeechCraft
{
namespace SB2
{
	const int IconSize = 32;

	QuarkManager::QuarkManager (const QuarkComponent& comp,
			ViewManager *manager, ICoreProxy_ptr proxy)
	: QObject (manager)
	, ViewMgr_ (manager)
	, Proxy_ (proxy)
	, URL_ (comp.Url_)
	, SettingsManager_ (0)
	, ID_ (QFileInfo (URL_.path ()).fileName ())
	, Name_ (ID_)
	, Icon_ (proxy->GetIcon ("applications-science"))
	{
		ParseManifest ();

		if (!ViewMgr_)
			return;

		qDebug () << Q_FUNC_INFO << "adding" << comp.Url_;
		auto ctx = manager->GetView ()->rootContext ();
		for (const auto& pair : comp.StaticProps_)
			ctx->setContextProperty (pair.first, pair.second);
		for (const auto& pair : comp.DynamicProps_)
			ctx->setContextProperty (pair.first, pair.second);

		auto engine = manager->GetView ()->engine ();
		for (const auto& pair : comp.ImageProviders_)
			if (!engine->imageProvider (pair.first))
				engine->addImageProvider (pair.first, pair.second);

		CreateSettings ();
	}

	QString QuarkManager::GetID () const
	{
		return ID_;
	}

	QString QuarkManager::GetName () const
	{
		return Name_;
	}

	QIcon QuarkManager::GetIcon () const
	{
		return Icon_;
	}

	QString QuarkManager::GetDescription () const
	{
		return Description_;
	}

	bool QuarkManager::IsValidArea () const
	{
		return Areas_.isEmpty () || Areas_.contains ("panel");
	}

	bool QuarkManager::HasSettings () const
	{
		return SettingsManager_;
	}

	void QuarkManager::ShowSettings ()
	{
		if (!HasSettings ())
			return;

		QDialog dia;
		const auto& settingsTitle = Name_.isEmpty () ?
				tr ("Settings") :
				tr ("Settings for %1").arg (Name_);
		dia.setWindowTitle (settingsTitle);

		dia.setLayout (new QVBoxLayout ());
		dia.layout ()->addWidget (XSD_.get ());

		auto box = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		connect (box,
				SIGNAL (accepted ()),
				&dia,
				SLOT (accept ()));
		connect (box,
				SIGNAL (rejected ()),
				&dia,
				SLOT (reject ()));
		connect (box,
				SIGNAL (accepted ()),
				XSD_.get (),
				SLOT (accept ()));
		connect (box,
				SIGNAL (rejected ()),
				XSD_.get (),
				SLOT (reject ()));
		dia.layout ()->addWidget (box);

		dia.exec ();
		XSD_->setParent (0);
	}

	QString QuarkManager::GetSuffixedName (const QString& suffix) const
	{
		if (!URL_.isLocalFile ())
			return QString ();

		const auto& localName = URL_.toLocalFile ();
		const auto& suffixed = localName + suffix;
		if (!QFile::exists (suffixed))
			return QString ();

		return suffixed;
	}

	void QuarkManager::ParseManifest ()
	{
		const auto& manifestName = GetSuffixedName (".manifest");
		if (manifestName.isEmpty ())
			return;

		QFile file (manifestName);
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open manifest file"
					<< file.errorString ();
			return;
		}

		const auto& varMap = QJson::Parser ().parse (&file).toMap ();
		Name_ = varMap ["quarkName"].toString ();
		Areas_ = varMap ["areas"].toStringList ();

		Description_ = varMap ["description"].toString ();

		if (varMap.contains ("quarkID"))
			ID_ = varMap ["quarkID"].toString ();

		if (varMap.contains ("icon"))
		{
			const auto& iconName = varMap ["icon"].toString ();

			TryFullImage (iconName) || TryTheme (iconName) || TryLC (iconName);
		}
	}

	bool QuarkManager::TryFullImage (const QString& iconName)
	{
		const auto& dirName = QFileInfo (URL_.toLocalFile ()).absoluteDir ().path ();
		const auto& fullName = dirName + '/' + iconName;

		const QPixmap px (fullName);
		if (px.isNull ())
			return false;

		Icon_ = QIcon ();
		Icon_.addPixmap (px);
		return true;
	}

	bool QuarkManager::TryTheme (const QString& iconName)
	{
		const auto& icon = Proxy_->GetIcon (iconName);
		const auto& px = icon.pixmap (IconSize, IconSize);
		if (px.isNull ())
			return false;

		Icon_ = icon;
		return true;
	}

	bool QuarkManager::TryLC (const QString& iconName)
	{
		if (iconName != "leechcraft")
			return false;

		Icon_ = QIcon ();
		Icon_.addFile (":/resources/images/leechcraft.svg");
		return true;
	}

	void QuarkManager::CreateSettings ()
	{
		const auto& settingsName = GetSuffixedName (".settings");
		if (settingsName.isEmpty ())
			return;

		XSD_.reset (new Util::XmlSettingsDialog);
		SettingsManager_ = new QuarkSettingsManager (URL_, ViewMgr_->GetView ()->rootContext ());
		XSD_->RegisterObject (SettingsManager_, settingsName);
	}
}
}
