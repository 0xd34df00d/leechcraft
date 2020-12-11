/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "msgtemplatesmanager.h"
#include <functional>
#include <QLocale>
#include <QtDebug>
#include <interfaces/itexteditor.h>
#include <util/sll/either.h>
#include <util/sll/functor.h>
#include <util/sll/applicative.h>
#include <util/sll/monadplus.h>
#include <util/sll/curry.h>
#include <util/sys/paths.h>
#include <util/util.h>
#include "account.h"
#include "structures.h"
#include "templatesstorage.h"
#include "templatepattern.h"
#include "messagebodies.h"

namespace LC
{
namespace Snails
{
	MsgTemplatesManager::MsgTemplatesManager (QObject *parent)
	: QObject { parent }
	, Storage_ { std::make_shared<TemplatesStorage> () }
	{
	}

	auto MsgTemplatesManager::GetTemplate (ContentType contentType,
			MsgType msgType, const Account *account) const -> LoadResult_t
	{
		static const auto defaults = GetDefaults ();

		return [&] (const auto& maybeResult)
					{ return maybeResult ? *maybeResult : defaults [contentType] [msgType]; } *
				(
					Util::Pure<TemplatesStorage::LoadResult_t> (Util::Mplus) *
							Storage_->LoadTemplate (contentType, msgType, account) *
							Storage_->LoadTemplate (contentType, msgType, nullptr)
				);
	}

	namespace
	{
		static const QString OpenMarker = "${";
		static const QString CloseMarker = "}";

		QString PerformSubstitutions (const Account *acc,
				const MessageInfo& info, const MessageBodies& bodies, ContentType type, QString text)
		{
			const auto functions = GetKnownPatternsHash ();

			const auto openSize = OpenMarker.size ();

			auto pos = 0;
			while ((pos = text.indexOf (OpenMarker, pos)) != -1)
			{
				const auto closing = text.indexOf (CloseMarker, pos);
				if (closing == -1)
					break;

				const auto& variable = text.mid (pos + openSize, closing - pos - openSize);

				if (functions.contains (variable))
				{
					const auto& subst = functions [variable] (acc, info, bodies, type);
					text.replace (pos, closing - pos + 1, subst);
					pos += subst.size ();
				}
				else
					pos = closing + 1;
			}

			return text;
		}
	}

	QString MsgTemplatesManager::GetTemplatedText (ContentType type,
			MsgType msgType, const Account *acc, const MessageInfo& info, const MessageBodies& bodies) const
	{
		return Util::RightOr (Util::Curry (&PerformSubstitutions) (acc) (info) (bodies) (type) *
					GetTemplate (type, msgType, acc),
				QString {});
	}

	auto MsgTemplatesManager::SaveTemplate (ContentType type,
			MsgType msgType, const Account *acc, const QString& tpl) -> SaveResult_t
	{
		return Storage_->SaveTemplate (type, msgType, acc, tpl);
	}

	namespace
	{
		QString LoadTemplate (ContentType ct, MsgType msgType)
		{
			const auto& basename = GetBasename (msgType);
			const auto& extension = GetExtension (ct);
			const auto& mkFilename = [&] (const QString& locale)
			{
				return basename + (locale.isEmpty () ? "" : ("_" + locale)) + "." + extension;
			};

			const auto& localeName = Util::GetLocaleName ();

			auto str = Util::GetSysPath (Util::SysPath::Share,
					"snails/templates", mkFilename (localeName.left (2)));
			if (str.isEmpty ())
				str = Util::GetSysPath (Util::SysPath::Share,
						"snails/templates", mkFilename ({}));
			if (str.isEmpty ())
				return {};

			QFile file { str };
			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to open file"
						<< file.fileName ()
						<< file.errorString ();
				return {};
			}

			auto data = file.readAll ();
			if (ct == ContentType::HTML)
				data.replace ('\n', "");
			return QString::fromUtf8 (data);
		}
	}

	QMap<ContentType, QMap<MsgType, QString>> MsgTemplatesManager::GetDefaults ()
	{
		QMap<ContentType, QMap<MsgType, QString>> result;
		for (auto ct : { ContentType::PlainText, ContentType::HTML })
			for (auto msgType : { MsgType::New, MsgType::Reply, MsgType::Forward })
				result [ct] [msgType] = LoadTemplate (ct, msgType);
		return result;
	}
}
}
