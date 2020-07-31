/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QVariantMap>
#include <QtDBus/QDBusObjectPath>

typedef QMap<QString, QVariantMap> VariantMapMap_t;
Q_DECLARE_METATYPE (VariantMapMap_t)

typedef QMap<QDBusObjectPath, VariantMapMap_t> EnumerationResult_t;
Q_DECLARE_METATYPE (EnumerationResult_t)

typedef QList<QByteArray> ByteArrayList_t;
Q_DECLARE_METATYPE (ByteArrayList_t)
