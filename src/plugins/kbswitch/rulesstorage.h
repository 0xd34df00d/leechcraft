/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include <QMap>
#include <QStringList>

typedef struct _XDisplay Display;

namespace LC
{
namespace KBSwitch
{
	class RulesStorage : public QObject
	{
		Display * const Display_;
		const QString X11Dir_;

		QHash<QString, QString> LayName2Desc_;
		QHash<QString, QString> LayDesc2Name_;

		QHash<QString, QStringList> LayName2Variants_;
		QHash<QString, QPair<QString, QString>> VarLayHR2NameVarPair_;

		QHash<QString, QString> KBModels_;
		QHash<QString, QString> KBModelString2Code_;
		QStringList KBModelsStrings_;

		QMap<QString, QMap<QString, QString>> Options_;
	public:
		RulesStorage (Display*, QObject* = 0);

		const QHash<QString, QString>& GetLayoutsD2N () const;
		const QHash<QString, QString>& GetLayoutsN2D () const;

		const QHash<QString, QPair<QString, QString>>& GetVariantsD2Layouts () const;

		QStringList GetLayoutVariants (const QString&) const;

		const QHash<QString, QString>& GetKBModels () const;
		const QStringList& GetKBModelsStrings () const;
		QString GetKBModelCode (const QString&) const;

		const QMap<QString, QMap<QString, QString>>& GetOptions () const;
	};
}
}
