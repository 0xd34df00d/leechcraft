/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "templatesstorage.h"
#include <QtDebug>
#include <util/sys/paths.h>
#include <util/sll/either.h>
#include <interfaces/itexteditor.h>
#include "account.h"
#include "structures.h"

namespace LC
{
namespace Snails
{
	TemplatesStorage::TemplatesStorage ()
	: RootDir_ { Util::GetUserDir (Util::UserDir::LC, "snails/templates") }
	{
	}

	namespace
	{
		QString GetAccountDirName (const Account *acc)
		{
			return acc->GetID ();
		}

		template<typename F>
		auto WithFile (const QString& filename, QIODevice::OpenMode mode, F&& worker)
		{
			QFile file { filename };
			if (!file.open (QIODevice::ReadOnly))
			{
				const auto modeStr = mode == QIODevice::ReadOnly ? "for reading:" : "for writing:";
				qWarning () << Q_FUNC_INFO
						<< "cannot open file"
						<< file.fileName ()
						<< modeStr
						<< file.errorString ();
				const auto& msg = "Cannot open file " + file.fileName ().toStdString () +
						" " + modeStr + " " + file.errorString ().toStdString ();
				return std::result_of_t<F (QFile&)>::Left (std::runtime_error { msg });
			}

			return worker (file);
		}
	}

	auto TemplatesStorage::LoadTemplate (ContentType contentType,
			MsgType msgType, const Account *account) -> LoadResult_t
	{
		auto dir = RootDir_;
		if (account && !dir.cd (GetAccountDirName (account)))
			return LoadResult_t::Right ({});

		const auto& filename = GetBasename (msgType) + "." + GetExtension (contentType);
		if (!dir.exists (filename))
			return LoadResult_t::Right ({});

		return WithFile (dir.filePath (filename), QIODevice::ReadOnly,
				[] (QFile& file)
				{
					return LoadResult_t::Right (QString::fromUtf8 (file.readAll ()));
				});
	}

	auto TemplatesStorage::SaveTemplate (ContentType contentType,
			MsgType msgType, const Account *account, const QString& tpl) -> SaveResult_t
	{
		auto dir = RootDir_;
		if (account)
		{
			const auto& dirName = GetAccountDirName (account);
			if (!dir.exists (dirName) && !dir.mkdir (dirName))
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot create"
						<< dirName
						<< "in"
						<< dir;
				const auto& msg = "Cannot create " + dir.filePath (dirName).toStdString ();
				return SaveResult_t::Left (std::runtime_error { msg });
			}

			if (!dir.cd (account->GetID ()))
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot cd"
						<< dir
						<< "into"
						<< dirName;
				const auto& msg = "Cannot cd into " + dir.filePath (dirName).toStdString ();
				return SaveResult_t::Left (std::runtime_error { msg });
			}
		}

		return WithFile (dir.filePath (GetBasename (msgType) + "." + GetExtension (contentType)),
				QIODevice::WriteOnly,
				[&tpl] (QFile& file)
				{
					if (file.write (tpl.toUtf8 ()) == -1)
					{
						qWarning () << Q_FUNC_INFO
								<< "cannot save file"
								<< file.fileName ()
								<< ":"
								<< file.errorString ();
						const auto& msg = "Cannot save file " + file.fileName ().toStdString () +
								": " + file.errorString ().toStdString ();
						return SaveResult_t::Left (std::runtime_error { msg });
					}

					return SaveResult_t::Right ({});
				});
	}
}
}
