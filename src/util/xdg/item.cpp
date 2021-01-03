/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "item.h"
#include <stdexcept>
#include <QFile>
#include <QUrl>
#include <QProcess>
#include <util/sll/qtutil.h>
#include <util/xpc/util.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/iiconthememanager.h>
#include "desktopparser.h"
#include "xdg.h"

namespace LC::Util::XDG
{
	bool operator== (const Item& left, const Item& right)
	{
		return left.IsHidden_ == right.IsHidden_ &&
				left.Type_ == right.Type_ &&
				left.Name_ == right.Name_ &&
				left.GenericName_ == right.GenericName_ &&
				left.Comments_ == right.Comments_ &&
				left.Categories_ == right.Categories_ &&
				left.Command_ == right.Command_ &&
				left.WD_ == right.WD_ &&
				left.IconName_ == right.IconName_;
	}

	bool operator!= (const Item& left, const Item& right)
	{
		return !(left == right);
	}

	bool Item::IsValid () const
	{
		return !Name_.isEmpty ();
	}

	bool Item::IsHidden () const
	{
		return IsHidden_;
	}

	void Item::Execute (ICoreProxy_ptr proxy) const
	{
		auto command = GetCommand ();

		if (GetType () == Type::Application)
		{
			command.remove ("%c");
			command.remove ("%f");
			command.remove ("%F");
			command.remove ("%u");
			command.remove ("%U");
			command.remove ("%i");
			auto items = command.split (' ', Qt::SkipEmptyParts);
			auto removePred = [] (const QString& str)
				{ return str.size () == 2 && str.at (0) == '%'; };
			items.erase (std::remove_if (items.begin (), items.end (), removePred),
					items.end ());
			if (items.isEmpty ())
				return;

			QProcess::startDetached (items.at (0), items.mid (1), GetWorkingDirectory ());
		}
		else if (GetType () == Type::URL)
		{
			const auto& e = Util::MakeEntity (QUrl (command),
					QString (),
					FromUserInitiated | OnlyHandle);
			proxy->GetEntityManager ()->HandleEntity (e);
		}
		else
		{
			qWarning () << Q_FUNC_INFO
					<< "don't know how to execute this type of app";
		}
	}

	namespace
	{
		QString ByLang (const QHash<QString, QString>& cont, const QString& lang)
		{
			return cont.value (cont.contains (lang) ? lang : QString ());
		}
	}

	QString Item::GetName (const QString& lang) const
	{
		return ByLang (Name_, lang);
	}

	QString Item::GetGenericName (const QString& lang) const
	{
		return ByLang (GenericName_, lang);
	}

	QString Item::GetComment (const QString& lang) const
	{
		return ByLang (Comments_, lang);
	}

	QString Item::GetIconName () const
	{
		return IconName_;
	}

	QStringList Item::GetCategories () const
	{
		return Categories_;
	}

	Type Item::GetType () const
	{
		return Type_;
	}

	QString Item::GetCommand () const
	{
		return Command_;
	}

	QString Item::GetWorkingDirectory () const
	{
		return WD_;
	}

	QString Item::GetPermanentID () const
	{
		return GetCommand ();
	}

	namespace
	{
		QIcon GetIconDevice (const ICoreProxy_ptr& proxy, QString name)
		{
			if (name.isEmpty ())
				return QIcon ();

			if (name.endsWith (".png") || name.endsWith (".svg"))
				name.chop (4);

			auto result = proxy->GetIconThemeManager ()->GetIcon (name);
			if (!result.isNull ())
				return result;

			result = GetAppIcon (name);
			if (!result.isNull ())
				return result;

			qDebug () << Q_FUNC_INFO << name << "not found";

			return result;
		}
	}

	QIcon Item::GetIcon (const ICoreProxy_ptr& proxy) const
	{
		if (!Icon_)
			Icon_ = GetIconDevice (proxy, GetIconName ());

		return *Icon_;
	}

	QDebug Item::DebugPrint (QDebug dbg) const
	{
		dbg.nospace () << "DesktopItem\n{\n\tNames: " << Name_
				<< "\n\tGenericNames: " << GenericName_
				<< "\n\tComments: " << Comments_
				<< "\n\tCategories: " << Categories_
				<< "\n\tCommand: " << Command_
				<< "\n\tWorkingDir: " << WD_
				<< "\n\tIconName: " << IconName_
				<< "\n\tHidden: " << IsHidden_
				<< "\n}\n";
		return dbg.space ();
	}

	namespace
	{
		QHash<QString, QString> FirstValues (const QHash<QString, QStringList>& hash)
		{
			QHash<QString, QString> result;
			for (const auto& [key, values] : Util::Stlize (hash))
				result [key] = values.value (0);
			return result;
		}
	}

	Item_ptr Item::FromDesktopFile (const QString& filename)
	{
		QFile file (filename);
		if (!file.open (QIODevice::ReadOnly))
			throw std::runtime_error ("Unable to open file");

		const auto& result = Util::XDG::DesktopParser {} (file.readAll ());
		const auto& group = result ["Desktop Entry"];

		const auto& item = std::make_shared<Item> ();
		item->Name_ = FirstValues (group ["Name"]);
		item->GenericName_ = FirstValues (group ["GenericName"]);
		item->Comments_ = FirstValues (group ["Comment"]);

		item->Categories_ = group ["Categories"] [{}];

		auto getSingle = [&group] (const QString& name) { return group [name] [{}].value (0); };

		item->IconName_ = getSingle ("Icon");

		const auto& typeStr = getSingle ("Type");
		if (typeStr == "Application")
		{
			item->Type_ = Type::Application;
			item->Command_ = getSingle ("Exec");
			item->WD_ = getSingle ("Path");
		}
		else if (typeStr == "URL")
		{
			item->Type_ = Type::URL;
			item->Command_ = getSingle ("URL");
		}
		else if (typeStr == "Directory")
			item->Type_ = Type::Dir;
		else
			item->Type_ = Type::Other;

		item->IsHidden_ = getSingle ("NoDisplay").toLower () == "true";

		return item;
	}

	QDebug operator<< (QDebug dbg, const Item& item)
	{
		return item.DebugPrint (std::move (dbg));
	}
}
