/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>
#include <QSet>
#include <QDateTime>
#include <interfaces/iscriptloader.h>
#include "structures.h"

namespace LC
{
namespace XProxy
{
	struct HostInfo
	{
		QString Host_;
		int Port_;
		QString Scheme_;
	};

	class UrlListScript : public QObject
	{
		Q_OBJECT

		const IScript_ptr Script_;

		QString ListName_;
		QSet<HostInfo> Hosts_;

		bool IsEnabled_ = false;
		QDateTime LastUpdate_;
	public:
		UrlListScript (const IScript_ptr& script, QObject* = nullptr);

		QByteArray GetListId () const;
		QString GetListName () const;

		void SetEnabled (bool);

		bool Accepts (const QString& host, int port, const QString& proto);

		Q_INVOKABLE void setUrls (const QStringList&);
	private:
		void SetUrlsImpl (const QStringList&);
	public slots:
		void refresh ();
	};

	using ScriptEntry_t = QPair<QByteArray, Proxy>;
}
}

Q_DECLARE_METATYPE (LC::XProxy::ScriptEntry_t)
Q_DECLARE_METATYPE (QList<LC::XProxy::ScriptEntry_t>)
