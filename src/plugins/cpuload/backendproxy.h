/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPointF>

class QStandardItemModel;
class QAbstractItemModel;

namespace LC
{
namespace CpuLoad
{
	class Backend;
	class CpuLoadProxyObj;

	class BackendProxy : public QObject
	{
		Q_OBJECT

		Backend * const Backend_;

		QStandardItemModel * const Model_;
		QList<CpuLoadProxyObj*> ModelPropObjs_;
	public:
		BackendProxy (Backend*);

		QAbstractItemModel* GetModel () const;
	public slots:
		void update ();

		QList<QPointF> sumPoints (QList<QPointF>, const QList<QPointF>&);
		QList<QPointF> enableIf (QList<QPointF>, bool);
	};
}
}
