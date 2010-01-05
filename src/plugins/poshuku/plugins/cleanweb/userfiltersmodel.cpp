/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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
#include <QSettings>
#include <QCoreApplication>
#include <QString>
#include <QRegExp>
#include <QAction>
#include <QtDebug>
#include "ruleoptiondialog.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Plugins
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

						qRegisterMetaType<RegExpsDict_t> ("LeechCraft::Plugins::Poshuku::Plugins::CleanWeb::RegExpsDict_t");
						qRegisterMetaType<OptionsDict_t> ("LeechCraft::Plugins::Poshuku::Plugins::CleanWeb::OptionsDict_t");
						qRegisterMetaTypeStreamOperators<RegExpsDict_t> ("LeechCraft::Plugins::Poshuku::Plugins::CleanWeb::RegExpsDict_t");
						qRegisterMetaTypeStreamOperators<OptionsDict_t> ("LeechCraft::Plugins::Poshuku::Plugins::CleanWeb::OptionsDict_t");
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

						const QStringList& list = isException ?
							Filter_.ExceptionStrings_ :
							Filter_.FilterStrings_;

						QString str = list.at (row);

						switch (index.column ())
						{
							case 0:
								return str;
							case 1:
								return isException ?
									tr ("Allowed") :
									tr ("Blocked");
							case 2:
								switch (Filter_.Options_ [str].MatchType_)
								{
									case FilterOption::MTWildcard:
										return tr ("Wildcard");
									case FilterOption::MTRegexp:
										return tr ("Regexp");
								}
							case 3:
								return Filter_.Options_ [str].Case_ == Qt::CaseSensitive ?
									tr ("True") :
									tr ("False");
							case 4:
								{
									QStringList result;
									Q_FOREACH (QString domain, Filter_.Options_ [str].Domains_)
										result += domain.prepend ("+");
									Q_FOREACH (QString domain, Filter_.Options_ [str].NotDomains_)
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
							Filter_.ExceptionStrings_.size () + Filter_.FilterStrings_.size ();
					}

					const Filter& UserFiltersModel::GetFilter () const
					{
						return Filter_;
					}

					void UserFiltersModel::InitiateAdd (const QString& suggested)
					{
						RuleOptionDialog dia;
						dia.SetString (suggested);
						dia.setWindowTitle (tr ("Add a filter"));
						if (dia.exec () != QDialog::Accepted)
							return;

						Add (dia);
					}
					
					void UserFiltersModel::Add (const RuleOptionDialog& dia)
					{
						QString rule = dia.GetString ();
						int size = 0;
						if (dia.IsException ())
						{
							size = Filter_.ExceptionStrings_.size ();
							Filter_.ExceptionStrings_ << rule;
						}
						else
						{
							size = rowCount ();
							Filter_.FilterStrings_ << rule;
						}

						beginInsertRows (QModelIndex (), size, size);

						if (dia.GetType () == FilterOption::MTRegexp)
							Filter_.RegExps_ [rule] = QRegExp (rule,
									dia.GetCase (), QRegExp::RegExp);
						Filter_.Options_ [rule].MatchType_ = dia.GetType ();
						Filter_.Options_ [rule].Case_ = dia.GetCase ();
						Filter_.Options_ [rule].Domains_ = dia.GetDomains ();
						Filter_.Options_ [rule].NotDomains_ = dia.GetNotDomains ();
						endInsertRows ();

						WriteSettings ();
					}

					void UserFiltersModel::Modify (int index)
					{
						int pos = index;
						bool isException;
						SplitRow (&pos, &isException);

						QString rule;
						if (isException)
							rule = Filter_.ExceptionStrings_.at (pos);
						else
							rule = Filter_.FilterStrings_.at (pos);

						RuleOptionDialog dia;
						dia.SetException (isException);
						dia.SetString (rule);
						dia.SetType (Filter_.Options_ [rule].MatchType_);
						dia.SetCase (Filter_.Options_ [rule].Case_);
						dia.SetDomains (Filter_.Options_ [rule].Domains_);
						dia.SetNotDomains (Filter_.Options_ [rule].NotDomains_);

						dia.setWindowTitle (tr ("Modify filter"));
						if (dia.exec () != QDialog::Accepted)
							return;

						Remove (index);
						Add (dia);
					}

					void UserFiltersModel::Remove (int index)
					{
						int pos = index;
						bool isException;
						SplitRow (&pos, &isException);
						beginRemoveRows (QModelIndex (), index, index);
						if (isException)
							Filter_.ExceptionStrings_.removeAt (pos);
						else
							Filter_.FilterStrings_.removeAt (pos);
						endRemoveRows ();
					}

					void UserFiltersModel::SplitRow (int *row, bool *isException) const
					{
						if (*row >= Filter_.ExceptionStrings_.size ())
						{
							*isException = false;
							*row = *row - Filter_.ExceptionStrings_.size ();
						}
						else
							*isException = true;
					}

					void UserFiltersModel::ReadSettings ()
					{
						QSettings settings (QCoreApplication::organizationName (),
								QCoreApplication::applicationName () + "_CleanWeb_Subscr");
						Filter_.ExceptionStrings_ = settings.value ("Exceptions").toStringList ();
						Filter_.FilterStrings_ = settings.value ("Filters").toStringList ();
						Filter_.RegExps_ = settings.value ("RegExps").value<RegExpsDict_t> ();
						Filter_.Options_ = settings.value ("Options").value<OptionsDict_t> ();
					}

					void UserFiltersModel::WriteSettings () const
					{
						QSettings settings (QCoreApplication::organizationName (),
								QCoreApplication::applicationName () + "_CleanWeb_Subscr");
						settings.setValue ("Exceptions", Filter_.ExceptionStrings_);
						settings.setValue ("Filters", Filter_.FilterStrings_);
						settings.setValue ("RegExps", QVariant::fromValue (Filter_.RegExps_));
						settings.setValue ("Options", QVariant::fromValue (Filter_.Options_));
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

						QUrl blockUrl = blocker->data ().value<QUrl> ();

						InitiateAdd (blockUrl.toString ());
					}
				};
			};
		};
	};
};

