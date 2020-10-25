/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "manifest.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QtDebug>
#include <util/sll/parsejson.h>

namespace LC
{
namespace SB2
{
	const int IconSize = 32;

	Manifest::Manifest (const QString& path)
	: QuarkPath_ { path }
	, ID_ { QFileInfo { path }.baseName () }
	, Name_ { ID_ }
	, Icon_ { QIcon::fromTheme ("applications-science") }
	{
		QFile file { path + ".manifest" };
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open manifest file"
					<< file.errorString ()
					<< "for quark"
					<< path;
			return;
		}

		const auto& varMap = Util::ParseJson (&file, Q_FUNC_INFO).toMap ();
		if (varMap.isEmpty ())
			return;

		Name_ = varMap ["quarkName"].toString ();
		Areas_ = varMap ["areas"].toStringList ();

		Description_ = varMap ["description"].toString ();

		const auto& idVar = varMap ["quarkID"];
		if (!idVar.isNull ())
			ID_ = idVar.toString ();

		IsHiddenByDefault_ = !varMap.value ("defaultVisibility", true).toBool ();

		const auto& iconVar = varMap ["icon"];
		if (!iconVar.isNull ())
		{
			const auto& iconName = iconVar.toString ();
			TryFullImage (iconName) || TryTheme (iconName) || TryLC (iconName);
		}
	}

	QString Manifest::GetID () const
	{
		return ID_;
	}

	QString Manifest::GetName () const
	{
		return Name_;
	}

	QIcon Manifest::GetIcon () const
	{
		return Icon_;
	}

	QString Manifest::GetDescription () const
	{
		return Description_;
	}

	QStringList Manifest::GetAreas () const
	{
		return Areas_;
	}

	bool Manifest::IsHiddenByDefault () const
	{
		return IsHiddenByDefault_;
	}

	bool Manifest::TryFullImage (const QString& iconName)
	{
		const auto& dirName = QFileInfo { QuarkPath_ }.absoluteDir ().path ();
		const auto& fullName = dirName + '/' + iconName;

		const QPixmap px (fullName);
		if (px.isNull ())
			return false;

		Icon_ = QIcon {};
		Icon_.addPixmap (px);
		return true;
	}

	bool Manifest::TryTheme (const QString& iconName)
	{
		const auto& icon = QIcon::fromTheme (iconName);
		const auto& px = icon.pixmap (IconSize, IconSize);
		if (px.isNull ())
			return false;

		Icon_ = icon;
		return true;
	}

	bool Manifest::TryLC (const QString& iconName)
	{
		if (iconName != "leechcraft")
			return false;

		Icon_ = QIcon {};
		Icon_.addFile ("lcicons:/resources/images/leechcraft.svg");
		return true;
	}
}
}
