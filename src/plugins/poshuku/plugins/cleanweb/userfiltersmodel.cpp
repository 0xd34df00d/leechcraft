/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "userfiltersmodel.h"
#include <algorithm>
#include <QSettings>
#include <QCoreApplication>
#include <QString>
#include <QRegExp>
#include <QAction>
#include <QMessageBox>
#include <qgraphicswebview.h>
#include <qwebframe.h>
#include <qwebelement.h>
#include <QtDebug>
#include <util/util.h>
#include "ruleoptiondialog.h"
#include "lineparser.h"
#include "core.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace CleanWeb
{
	UserFiltersModel::UserFiltersModel (QObject *parent)
	: QAbstractItemModel (parent)
	{
		ReadSettings ();
		Headers_ << tr ("Filter")
			<< tr ("Policy")
			<< tr ("Type")
			<< tr ("Case sensitive")
			<< tr ("Domains");

		qRegisterMetaType<FilterItem> ("LeechCraft::Poshuku::CleanWeb::FilterItem");
		qRegisterMetaType<QList<FilterItem>> ("QList<LeechCraft::Poshuku::CleanWeb::FilterItem>");
		qRegisterMetaTypeStreamOperators<FilterItem> ("LeechCraft::Poshuku::CleanWeb::FilterItem");
		qRegisterMetaTypeStreamOperators<QList<FilterItem>> ("QList<LeechCraft::Poshuku::CleanWeb::FilterItem>");
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
				return item.OrigString_;
			case 1:
				return isException ?
					tr ("Allowed") :
					tr ("Blocked");
			case 2:
				switch (item.Option_.MatchType_)
				{
					case FilterOption::MTWildcard:
					case FilterOption::MTPlain:
					case FilterOption::MTBegin:
					case FilterOption::MTEnd:
						return tr ("Wildcard");
					case FilterOption::MTRegexp:
						return tr ("Regexp");
				}
			case 3:
				return item.Option_.Case_ == Qt::CaseSensitive ?
					tr ("True") :
					tr ("False");
			case 4:
				{
					const auto& options = item.Option_;

					QStringList result;
					Q_FOREACH (QString domain, options.Domains_)
						result += domain.prepend ("+");
					Q_FOREACH (QString domain, options.NotDomains_)
						result += domain.prepend ("-");
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

		return Add (dia);
	}

	bool UserFiltersModel::Add (const RuleOptionDialog& dia)
	{
		const auto& itemRx = dia.GetType () == FilterOption::MTRegexp ?
				QRegExp (dia.GetString (), dia.GetCase (), QRegExp::RegExp) :
				QRegExp ();
		FilterOption fo;
		fo.Case_ = dia.GetCase ();
		fo.MatchType_ = dia.GetType ();
		fo.Domains_ = dia.GetDomains ();
		fo.NotDomains_ = dia.GetNotDomains ();
		const FilterItem item
		{
			dia.GetString (),
			itemRx,
			fo.MatchType_ == FilterOption::MTPlain ?
					QStringMatcher (dia.GetString (), dia.GetCase ()) :
					QStringMatcher (),
			fo
		};

		auto& container = dia.IsException () ? Filter_.Exceptions_ : Filter_.Filters_;
		const int size = dia.IsException () ? Filter_.Exceptions_.size () : rowCount ();
		beginInsertRows (QModelIndex (), size, size);
		container << item;
		endInsertRows ();

		WriteSettings ();

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
		dia.SetString (item.OrigString_);
		const auto& options = item.Option_;
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
		WriteSettings ();
	}

	void UserFiltersModel::AddMultiFilters (QStringList lines)
	{
		std::for_each (lines.begin (), lines.end (),
				[] (QString& str) { str = str.trimmed (); });

		beginResetModel ();
		auto p = std::for_each (lines.begin (), lines.end (),
				LineParser (&Filter_));
		endResetModel ();

		if (p.GetSuccess () <= 0)
			return;

		WriteSettings ();

		emit gotEntity (Util::MakeNotification ("Poshuku CleanWeb",
				tr ("Imported %1 user filters (%2 parsed successfully).")
					.arg (p.GetSuccess ())
					.arg (p.GetTotal ()),
				PInfo_));
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
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_CleanWeb_Subscr");
		Filter_.Exceptions_ = settings.value ("ExceptionItems").value<decltype (Filter_.Exceptions_)> ();
		Filter_.Filters_ = settings.value ("FilterItems").value<decltype (Filter_.Filters_)> ();
	}

	void UserFiltersModel::WriteSettings () const
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_CleanWeb_Subscr");
		settings.clear ();
		settings.setValue ("ExceptionItems", QVariant::fromValue (Filter_.Exceptions_));
		settings.setValue ("FilterItems", QVariant::fromValue (Filter_.Filters_));
	}

	void UserFiltersModel::blockImage ()
	{
		QAction *blocker = qobject_cast<QAction*> (sender ());
		if (!blocker)
		{
			qWarning () << Q_FUNC_INFO
				<< "sender is not a QAction*"
				<< sender ();
			return;
		}

		QUrl blockUrl = blocker->property ("CleanWeb/URL").value<QUrl> ();
		QGraphicsWebView *view = qobject_cast<QGraphicsWebView*> (blocker->
					property ("CleanWeb/View").value<QObject*> ());
		if (InitiateAdd (blockUrl.toString ()) && view)
		{
			QWebFrame *frame = view->page ()->mainFrame ();
			QWebElement elem = frame->findFirstElement ("img[src=\"" + blockUrl.toEncoded () + "\"]");
			if (!elem.isNull ())
				elem.removeFromDocument ();
		}
	}
}
}
}
