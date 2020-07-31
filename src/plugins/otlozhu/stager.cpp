/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "stager.h"
#include <sstream>
#include <vector>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/variant.hpp>
#include <QDateTime>
#include <QtDebug>
#include <laretz/operation.h>
#include <laretz/opsummer.h>
#include <util/util.h>
#include <util/sys/paths.h>

namespace LC
{
namespace Util
{
namespace Sync
{
	Laretz::Field_t ToField (const QString& str)
	{
		return std::string (str.toUtf8 ().constData ());
	}

	Laretz::Field_t ToField (const QStringList& strList)
	{
		std::vector<std::string> result;
		result.reserve (strList.size ());
		for (const auto& str : strList)
			result.push_back (str.toUtf8 ().constData ());
		return result;
	}

	Laretz::Field_t ToField (const QDateTime& dt)
	{
		return static_cast<int64_t> (dt.toSecsSinceEpoch ());
	}

	Laretz::Field_t ToField (const QVariant& var)
	{
		switch (var.type ())
		{
		case QVariant::DateTime:
			return ToField (var.toDateTime ());
		case QVariant::String:
			return ToField (var.toString ());
		case QVariant::StringList:
			return ToField (var.toStringList ());
		case QVariant::UInt:
		case QVariant::ULongLong:
		case QVariant::Int:
		case QVariant::LongLong:
			return ToField (static_cast<int64_t> (var.toLongLong ()));
		default:
			qWarning () << Q_FUNC_INFO
					<< "unhandled variant type"
					<< var;
			return {};
		}
	}

	void FillItem (Laretz::Item& item, const QVariantMap& map)
	{
		for (auto i = map.begin (); i != map.end (); ++i)
			item [i.key ().toUtf8 ().constData ()] = ToField (i.value ());
	}

	namespace
	{
		struct ToVariant : boost::static_visitor<QVariant>
		{
			QVariant operator() (const std::vector<char>& data) const
			{
				QByteArray result;
				result.reserve (data.size ());
				std::copy (data.begin (), data.end (), std::back_inserter (result));
				return result;
			}

			QVariant operator() (const std::string& str) const
			{
				return QString::fromUtf8 (str.c_str ());
			}

			QVariant operator() (const std::vector<std::string>& strList) const
			{
				QStringList result;
				result.reserve (strList.size ());
				for (const auto& str : strList)
					result << QString::fromUtf8 (str.c_str ());
				return result;
			}

			QVariant operator() (int64_t num) const
			{
				return static_cast<qlonglong> (num);
			}

			QVariant operator() (double num) const
			{
				return num;
			}
		};
	}

	QVariantMap ItemToMap (const Laretz::Item& item)
	{
		QVariantMap result;
		for (const auto& pair : item)
			result [pair.first.c_str ()] = boost::apply_visitor (ToVariant (), pair.second);
		return result;
	}

	Stager::Stager (const QString& areaId, QObject *parent)
	: QObject (parent)
	, StagingDir_ (Util::CreateIfNotExists ("sync/staging/" + areaId))
	, Summer_ (new Laretz::OpSummer)
	, IsEnabled_ (StagingDir_.exists ("enabled"))
	{
		QFile compFile (StagingDir_.absoluteFilePath ("compressed"));
		if (compFile.exists () && compFile.open (QIODevice::ReadOnly))
		{
			const auto& data = compFile.readAll ();
			std::istringstream arIstr (data.constData ());
			boost::archive::text_iarchive iars (arIstr);
			std::vector<Laretz::Operation> ops;
			iars >> ops;

			for (const auto& op : ops)
				(*Summer_) += op;
		}
	}

	void Stager::Enable ()
	{
		if (IsEnabled_)
			return;

		IsEnabled_ = true;

		QFile file (StagingDir_.absoluteFilePath ("enabled"));
		file.open (QIODevice::WriteOnly);
		file.write ("1");
	}

	bool Stager::IsEnabled () const
	{
		return IsEnabled_;
	}

	auto Stager::EnterMergeMode () -> MergeGuard_t
	{
		const bool was = IsEnabled_;
		IsEnabled_ = false;
		return MergeGuard_t (nullptr, [this, was] (void*) { IsEnabled_ = was; });
	}

	void Stager::Add (const std::vector<Laretz::Operation>& ops)
	{
		if (!IsEnabled_)
			return;

		for (const auto& op : ops)
			(*Summer_) += op;

		QFile file (StagingDir_.absoluteFilePath ("compressed"));
		if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot open staging file"
					<< file.fileName ()
					<< file.errorString ();
			throw std::runtime_error ("cannot open staging file");
		}

		std::ostringstream arOstr;
		boost::archive::text_oarchive oars (arOstr);
		oars << Summer_->getOps ();
		const auto& opsStr = arOstr.str ();

		file.write (opsStr.c_str ());

		return;
	}

	QList<Laretz::Operation> Stager::GetStagedOps () const
	{
		const auto& vec = Summer_->getOps ();

		QList<Laretz::Operation> result;
		result.reserve (vec.size ());
		std::copy (vec.begin (), vec.end (), std::back_inserter (result));
		return result;
	}
}
}
}
