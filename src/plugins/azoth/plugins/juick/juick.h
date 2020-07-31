/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QRegExp>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/azoth/imessage.h>

namespace LC
{
namespace Azoth
{
namespace Juick
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.Juick")

		QRegExp UserRX_;
		QRegExp PostRX_;
		QRegExp IdRX_;
		QRegExp UnsubRX_;
		QRegExp ReplyRX_;
		QRegExp AvatarRX_;
		QRegExp TagRX_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;
	private:
		QString FormatBody (QString);
		void InsertAvatars (QString& body);
		void InsertNickLinks (QString& body);
		bool ShouldHandle (QObject* msgObj,
				IMessage::Direction direction, IMessage::Type type);
		bool IsBehind (const QString& text, int index, const QString& pattern) const;
	public slots:
		void hookFormatBodyEnd (LC::IHookProxy_ptr proxy,
				QObject *message);
		void hookMessageWillCreated (LC::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *entry,
				int type,
				QString variant);
	};
}
}
}
