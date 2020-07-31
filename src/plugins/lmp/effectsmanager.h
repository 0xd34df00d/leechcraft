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
#include <QVariantList>
#include <QModelIndexList>
#include <QIcon>
#include "interfaces/lmp/ifilterelement.h"
#include "interfaces/lmp/ifilterplugin.h"

class QStandardItemModel;
class QAbstractItemModel;
class QByteArray;

namespace LC
{
namespace LMP
{
	class IPath;
	class Path;
	class RGFilterController;

	struct SavedFilterInfo
	{
		QByteArray FilterId_;
		QByteArray InstanceId_;
	};

	QDataStream& operator<< (QDataStream&, const SavedFilterInfo&);
	QDataStream& operator>> (QDataStream&, SavedFilterInfo&);

	class EffectsManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel * const Model_;

		Path * const Path_;

		std::shared_ptr<RGFilterController> RGFilter_;

		QList<EffectInfo> RegisteredEffects_;
		QList<IFilterElement*> Filters_;
	public:
		EffectsManager (Path*, QObject* = 0);

		QAbstractItemModel* GetEffectsModel () const;

		void RegisterEffect (const EffectInfo&);

		void RegisteringFinished ();
	private:
		IFilterElement* RestoreFilter (const QList<EffectInfo>::const_iterator, const QByteArray&);
		void UpdateHeaders ();
		void SaveFilters () const;

		void ReemitEffectsList ();
	public slots:
		void showEffectConfig (int);

		void addRequested (const QString&, const QVariantList&);
		void removeRequested (const QString&, const QModelIndexList&);

		void customButtonPressed (const QString&, const QByteArray&, int);
	signals:
		void effectsListChanged (const QStringList&);
	};
}
}

Q_DECLARE_METATYPE (LC::LMP::SavedFilterInfo)
Q_DECLARE_METATYPE (QList<LC::LMP::SavedFilterInfo>)
