/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlerpath.h"
#include <QDir>
#include <QStandardPaths>
#include <QtDebug>
#include <util/sys/paths.h>
#include <util/sll/qtutil.h>
#include "../widgets/filepicker.h"

namespace LC
{
	namespace
	{
		FilePicker::Type GetPickerType (const QDomElement& item)
		{
			const auto& typeAttr = item.attribute ("pickerType"_qs);
			if (typeAttr.isEmpty ())
				return FilePicker::Type::ExistingDirectory;
			if (typeAttr == "openFileName"_ql)
				return FilePicker::Type::OpenFileName;
			if (typeAttr == "saveFileName"_ql)
				return FilePicker::Type::SaveFileName;

			qWarning () << "unknown picker type" << typeAttr;

			return FilePicker::Type::ExistingDirectory;
		}

		QString GetDefaultPath (const QDomElement& item, QString def)
		{
			if (item.attribute ("defaultHomePath"_qs) == "true"_ql)
				return QDir::homePath ();

			if (def.isEmpty ())
				return {};

			static const QVector<QPair<QString, QString>> str2loc
			{
				{ "DOCUMENTS", QStandardPaths::writableLocation (QStandardPaths::DocumentsLocation) },
				{ "DESKTOP", QStandardPaths::writableLocation (QStandardPaths::DesktopLocation) },
				{ "MUSIC", QStandardPaths::writableLocation (QStandardPaths::MusicLocation) },
				{ "MOVIES", QStandardPaths::writableLocation (QStandardPaths::MoviesLocation) },
				{ "LCDIR", Util::GetUserDir (Util::UserDir::LC, {}).absolutePath () },
				{ "CACHEDIR", Util::GetUserDir (Util::UserDir::Cache, {}).absolutePath () }
			};

			for (const auto& [pattern, path] : str2loc)
				if (def.startsWith ("{" + pattern + "}"))
				{
					def.replace (0, pattern.length () + 2, path);
					break;
				}

			return def;
		}
	}

	ItemRepresentation HandlePath (const ItemContext& ctx)
	{
		const auto& item = ctx.Elem_;

		const auto picker = new FilePicker { GetPickerType (item) };
		if (item.attribute ("onCancel"_qs) == "clear"_ql)
			picker->SetClearOnCancel (true);
		if (item.hasAttribute ("filter"_qs))
			picker->SetFilter (item.attribute ("filter"_qs));

		SetChangedSignal (ctx, picker, &FilePicker::textChanged);

		return
		{
			.Widget_ = picker,

			.DefaultValue_ = GetDefaultPath (item, ctx.Default_),
			.Getter_ = [picker] { return picker->GetText (); },
			.Setter_ = [picker] (const QVariant& value) { picker->SetText (value.toString ()); },
		};
	}
}
