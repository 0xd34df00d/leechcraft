/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QString>
#include <interfaces/azoth/iauthable.h>

class QXmlStreamWriter;
class QDomElement;

namespace LC
{
namespace Azoth
{
class IProxyObject;

namespace Xoox
{
	class GlooxAccount;

	struct OfflineDataSource
	{
		QString ID_;
		QString Name_;
		QStringList Groups_;
		AuthStatus AuthStatus_;
	};
	using OfflineDataSource_ptr = std::shared_ptr<OfflineDataSource>;

	void Save (OfflineDataSource_ptr, QXmlStreamWriter*, IProxyObject*);
	void Load (OfflineDataSource_ptr, const QDomElement&, IProxyObject*, GlooxAccount*);
}
}
}
