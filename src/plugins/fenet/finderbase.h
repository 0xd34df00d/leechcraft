/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QDir>
#include <QStandardItemModel>
#include <QtDebug>
#include <util/sys/paths.h>
#include <util/sll/parsejson.h>

class QAbstractItemModel;
class QStandardItemModel;

namespace LC
{
namespace Fenet
{
	template<typename InfoT>
	class FinderBase : public QObject
	{
	protected:
		const QStringList Path_;

		QList<InfoT> Known_;
		QList<InfoT> Found_;

		QStandardItemModel * const FoundModel_;
	public:
		FinderBase (QObject *parent = 0)
		: QObject (parent)
		, Path_ (Util::GetSystemPaths ())
		, FoundModel_ (new QStandardItemModel (this))
		{
		}

		const QList<InfoT>& GetFound () const
		{
			return Found_;
		}

		QAbstractItemModel* GetFoundModel () const
		{
			return FoundModel_;
		}
	protected:
		void Find (const QString& subpath)
		{
			qDebug () << Q_FUNC_INFO << "searching for WMs...";

			const auto& cands = Util::GetPathCandidates (Util::SysPath::Share, "fenet/" + subpath);
			for (const auto& cand : cands)
				for (const auto& entry : QDir (cand).entryInfoList ({ "*.json" }))
					HandleDescr (entry.absoluteFilePath ());

			qDebug () << Known_.size () << "known WMs;"
					<< Found_.size () << "found WMs";
		}

		void HandleDescr (const QString& filePath)
		{
			QFile file (filePath);
			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot open file"
						<< file.fileName ()
						<< file.errorString ();
				return;
			}

			const auto& parsedVar = Util::ParseJson (&file, Q_FUNC_INFO);
			if (parsedVar.isNull ())
				return;

			const auto& varmap = parsedVar.toMap ();

			QStringList execNames;
			for (const auto& var : varmap ["execNames"].toList ())
				execNames << var.toString ();

			const auto& info = GetInfo (filePath, execNames, varmap);
			Known_ << info;

			if (std::any_of (execNames.begin (), execNames.end (),
					[this] (const QString& name) { return this->IsAvailable (name); }))
			{
				qDebug () << Q_FUNC_INFO << info.Name_ << "available";
				Found_ << info;

				auto item = new QStandardItem (info.Name_);
				item->setEditable (false);
				item->setToolTip (info.Comment_);
				FoundModel_->appendRow (item);
			}
		}

		virtual InfoT GetInfo (const QString& filePath,
				const QStringList& execNames, const QVariantMap& varmap) const = 0;

		bool IsAvailable (const QString& executable) const
		{
			if (QFileInfo (executable).isExecutable ())
				return true;

			return !Util::FindInSystemPath (executable, Path_,
					[] (const QFileInfo& fi) { return fi.isExecutable (); }).isEmpty ();
		}
	};
}
}
