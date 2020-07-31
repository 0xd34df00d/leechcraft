/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QtPlugin>
#include <QFileInfo>
#include <QMap>
#include <QVariant>

class QPixmap;

namespace LC
{
namespace LMP
{
	class ILocalCollection;
	class ITagResolver;
	class ILMPUtilProxy;
	class ILMPGuiProxy;

	class ILMPProxy
	{
	public:
		virtual ~ILMPProxy () {}

		virtual ILocalCollection* GetLocalCollection () const = 0;

		virtual ITagResolver* GetTagResolver () const = 0;

		virtual const ILMPUtilProxy* GetUtilProxy () const = 0;

		virtual const ILMPGuiProxy* GetGuiProxy () const = 0;

		virtual void PreviewRelease (const QString& artist, const QString& release,
				const QList<QPair<QString, int>>& tracks) const = 0;
	};

	using ILMPProxy_ptr = ILMPProxy*;
}
}

Q_DECLARE_INTERFACE (LC::LMP::ILMPProxy, "org.LeechCraft.LMP.ILMPProxy/1.0")
