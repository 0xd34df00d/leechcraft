/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <util/sll/eitherfwd.h>
#include <util/sll/void.h>

template<typename, typename>
class QMap;

namespace LC
{
enum class ContentType;

namespace Snails
{
	class Account;
	class TemplatesStorage;
	struct MessageInfo;
	struct MessageBodies;

	enum class MsgType;

	class MsgTemplatesManager : public QObject
	{
		const std::shared_ptr<TemplatesStorage> Storage_;
	public:
		MsgTemplatesManager (QObject* = nullptr);

		using LoadResult_t = Util::Either<std::runtime_error, QString>;

		LoadResult_t GetTemplate (ContentType, MsgType, const Account*) const;
		QString GetTemplatedText (ContentType, MsgType, const Account*, const MessageInfo&, const MessageBodies&) const;

		using SaveResult_t = Util::Either<std::runtime_error, Util::Void>;
		SaveResult_t SaveTemplate (ContentType, MsgType, const Account*, const QString&);
	private:
		static QMap<ContentType, QMap<MsgType, QString>> GetDefaults ();
	};
}
}
