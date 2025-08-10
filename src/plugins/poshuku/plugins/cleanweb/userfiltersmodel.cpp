/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "userfiltersmodel.h"
#include <algorithm>
#include <QSettings>
#include <QCoreApplication>
#include <QString>
#include <QAction>
#include <QMessageBox>
#include <QtDebug>
#include <util/xpc/util.h>
#include <util/sll/prelude.h>
#include <util/sll/qstringwrappers.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/poshuku/iwebview.h>
#include "ruleoptiondialog.h"
#include "lineparser.h"
#include "core.h"

namespace LC
{
namespace Poshuku
{
namespace CleanWeb
{
	namespace
	{
		QStringList MakeHeaders ()
		{
			return
			{
				UserFiltersModel::tr ("Filter"),
				UserFiltersModel::tr ("Policy"),
				UserFiltersModel::tr ("Type"),
				UserFiltersModel::tr ("Case sensitive"),
				UserFiltersModel::tr ("Domains")
			};
		}
	}

	UserFiltersModel::UserFiltersModel (const ICoreProxy_ptr& proxy, QObject *parent)
	: QAbstractItemModel { parent }
	, Proxy_ { proxy }
	, Headers_ { MakeHeaders () }
	{
		qRegisterMetaType<FilterItem> ("LC::Poshuku::CleanWeb::FilterItem");
		qRegisterMetaType<QList<FilterItem>> ("QList<LC::Poshuku::CleanWeb::FilterItem>");

		ReadSettings ();
	}

	int UserFiltersModel::columnCount (const QModelIndex&) const
	{
		return Headers_.size ();
	}

	QVariant UserFiltersModel::data (const QModelIndex& index, int role) const
	{
		if (role != Qt::DisplayRole ||
				!index.isValid ())
			return QVariant ();

		int row = index.row ();
		bool isException = true;
		SplitRow (&row, &isException);

		const auto& list = isException ?
			Filter_.Exceptions_ :
			Filter_.Filters_;

		const auto& item = list.at (row);

		switch (index.column ())
		{
		case 0:
			return QString::fromUtf8 (item->PlainMatcher_);
		case 1:
			return isException ?
				tr ("Allowed") :
				tr ("Blocked");
		case 2:
			return item->Option_.MatchType_ == FilterOption::MatchType::Regexp ?
				tr ("Regexp") :
				tr ("Wildcard");
		case 3:
			return item->Option_.Case_ == Qt::CaseSensitive ?
				tr ("True") :
				tr ("False");
		case 4:
		{
			const auto& options = item->Option_;

			QStringList result;
			for (const auto& domain : options.Domains_)
				result += "+" + domain;
			for (const auto& domain : options.NotDomains_)
				result += "-" + domain;
			return result.join ("; ");
		}
		default:
			return QVariant ();
		}
	}

	QVariant UserFiltersModel::headerData (int section,
			Qt::Orientation orient, int role) const
	{
		if (orient != Qt::Horizontal ||
				role != Qt::DisplayRole)
			return QVariant ();

		return Headers_.at (section);
	}

	QModelIndex UserFiltersModel::index (int row, int column,
			const QModelIndex& parent) const
	{
		if (!hasIndex (row, column, parent))
			return QModelIndex ();

		return createIndex (row, column);
	}

	QModelIndex UserFiltersModel::parent (const QModelIndex&) const
	{
		return QModelIndex ();
	}

	int UserFiltersModel::rowCount (const QModelIndex& index) const
	{
		return index.isValid () ? 0 :
			Filter_.Exceptions_.size () + Filter_.Filters_.size ();
	}

	const Filter& UserFiltersModel::GetFilter () const
	{
		return Filter_;
	}

	bool UserFiltersModel::InitiateAdd (const QString& suggested)
	{
		RuleOptionDialog dia;
		dia.SetString (suggested);
		dia.setWindowTitle (tr ("Add a filter"));
		if (dia.exec () != QDialog::Accepted)
			return false;

		const bool isException = Add (dia);
		WriteSettings ();
		return isException;
	}

	bool UserFiltersModel::Add (const RuleOptionDialog& dia)
	{
		const auto& itemRx = dia.GetType () == FilterOption::MatchType::Regexp ?
				Util::RegExp (dia.GetString (), dia.GetCase ()) :
				Util::RegExp ();
		FilterOption fo;
		fo.Case_ = dia.GetCase ();
		fo.MatchType_ = dia.GetType ();
		fo.Domains_ = dia.GetDomains ();
		fo.NotDomains_ = dia.GetNotDomains ();
		const auto item = std::make_shared<FilterItem> (FilterItem {
					itemRx,
					dia.GetString ().toUtf8 (),
					fo
				});

		auto& container = dia.IsException () ? Filter_.Exceptions_ : Filter_.Filters_;
		const int size = dia.IsException () ? Filter_.Exceptions_.size () : rowCount ();
		beginInsertRows (QModelIndex (), size, size);
		container << item;
		endInsertRows ();

		return !dia.IsException ();
	}

	void UserFiltersModel::Modify (int index)
	{
		int pos = index;
		bool isException;
		SplitRow (&pos, &isException);

		const auto& item = isException ? Filter_.Exceptions_.at (pos) : Filter_.Filters_.at (pos);

		RuleOptionDialog dia;
		dia.SetException (isException);
		dia.SetString (QString::fromUtf8 (item->PlainMatcher_));
		const auto& options = item->Option_;
		dia.SetType (options.MatchType_);
		dia.SetCase (options.Case_);
		dia.SetDomains (options.Domains_);
		dia.SetNotDomains (options.NotDomains_);

		dia.setWindowTitle (tr ("Modify filter"));
		if (dia.exec () != QDialog::Accepted)
			return;

		Remove (index);
		Add (dia);
	}

	void UserFiltersModel::Remove (int index)
	{
		int pos = index;
		bool isException = false;
		SplitRow (&pos, &isException);
		beginRemoveRows (QModelIndex (), index, index);
		if (isException)
			Filter_.Exceptions_.removeAt (pos);
		else
			Filter_.Filters_.removeAt (pos);
		endRemoveRows ();
	}

	void UserFiltersModel::AddMultiFilters (QStringList lines)
	{
		lines = Util::Map (lines, Util::QStringTrimmed {});

		beginResetModel ();
		auto p = std::for_each (lines.begin (), lines.end (),
				LineParser (&Filter_));
		endResetModel ();

		if (p.GetSuccess () <= 0)
			return;

		Proxy_->GetEntityManager ()->HandleEntity (Util::MakeNotification ("Poshuku CleanWeb",
				tr ("Imported %1 user filters (%2 parsed successfully).")
					.arg (p.GetSuccess ())
					.arg (p.GetTotal ()),
				Priority::Info));
	}

	void UserFiltersModel::SplitRow (int *row, bool *isException) const
	{
		if (*row >= Filter_.Exceptions_.size ())
		{
			*isException = false;
			*row = *row - Filter_.Exceptions_.size ();
		}
		else
			*isException = true;
	}

	void UserFiltersModel::ReadSettings ()
	{
		beginResetModel ();

		Filter_.Exceptions_.clear ();
		Filter_.Filters_.clear ();

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_CleanWeb_Subscr");

		auto readItems = [&settings] (const QString& name, QList<FilterItem_ptr>& to)
		{
			for (const auto& item : settings.value (name).value<QList<FilterItem>> ())
				to << std::make_shared<FilterItem> (item);
		};
		readItems ("ExceptionItems", Filter_.Exceptions_);
		readItems ("FilterItems", Filter_.Filters_);

		endResetModel ();
	}

	void UserFiltersModel::WriteSettings ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_CleanWeb_Subscr");
		settings.clear ();

		auto writeItems = [&settings] (const QString& name, const QList<FilterItem_ptr>& from)
		{
			QList<FilterItem> saved;
			saved.reserve (from.size ());
			for (const auto& item : from)
				saved << *item;
			settings.setValue (name, QVariant::fromValue (saved));
		};
		writeItems ("ExceptionItems", Filter_.Exceptions_);
		writeItems ("FilterItems", Filter_.Filters_);

		emit filtersChanged ();
	}

	void UserFiltersModel::BlockImage (const QUrl& blockUrl, IWebView *view)
	{
		if (!InitiateAdd (blockUrl.toString ()))
			return;

		QString js;
		js += "var elems = document.querySelectorAll(\"img[src='" + blockUrl.toEncoded () + "']\");";
		js += "for (var i = 0; i < elems.length; ++i) elems[i].remove();";
		view->EvaluateJS (js);
	}
}
}
}
