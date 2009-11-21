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

#include "requestnormalizer.h"
#include <stdexcept>
#include <QtDebug>
#include <interfaces/ifinder.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Summary
		{
			RequestNormalizer::RequestHolder::RequestHolder ()
			: Op_ (OperationalModel::OpNull)
			{
			}

			RequestNormalizer::RequestHolder::~RequestHolder ()
			{
				if (Op_ != OperationalModel::OpNull)
				{
					Merger_->RemoveModel (Left_->Merger_.get ());
					Merger_->RemoveModel (Right_->Merger_.get ());
				}
			}

			RequestNormalizer::RequestNormalizer (const boost::shared_ptr<Util::MergeModel>& merge,
					QObject *parent)
			: QObject (parent)
			, MergeModel_ (merge)
			, Root_ (new Util::MergeModel (QStringList (tr ("Name"))
						<< tr ("Status")
						<< tr ("State")))
			, Parser_ (new RequestParser)
			{
				setObjectName ("RequestNormalizer");
				Root_->setProperty ("__LeechCraft_own_core_model", true);
			}

			void RequestNormalizer::SetRequest (const QString& req)
			{
				try
				{
					Validate (req);
				}
				catch (const std::runtime_error& e)
				{
					qWarning () << Q_FUNC_INFO << e.what ();
					emit error (tr ("Request validation error: %1").arg (e.what ()));
				}

				try
				{
					RequestHolder_ptr c = Parse (req);
					SetMerger (c);

					Root_->AddModel (c->Merger_.get ());
					if (Current_)
						Root_->RemoveModel (Current_->Merger_.get ());
					Current_ = c;
				}
				catch (const std::runtime_error& e)
				{
					qWarning () << Q_FUNC_INFO << e.what ();
					emit error (tr ("Request parsing error: %1").arg (e.what ()));
				}
			}

			QAbstractItemModel* RequestNormalizer::GetModel () const
			{
				return Root_.get ();
			}

			void RequestNormalizer::Validate (const QString& req) const
			{
				if (req.size () < 2)
					return;

				// Check for braces
				int openedBraces = req.startsWith ('(');
				for (int i = 0; i < req.size () - 1; ++i)
				{
					if (req.mid (i, 2) == " (")
						++openedBraces;
					else if (req.mid (i, 2) == ") ")
						--openedBraces;
				}
				if (openedBraces)
					throw std::runtime_error (qPrintable (tr ("Open/close braces mismatch: %1")
								.arg (openedBraces)));
			}

			namespace
			{
				int FindWB (const QString& text, const QString& string)
				{
					if (string.size () < 2)
						return -1;

					int openedBraces = 0;
					int i = 0;
					for ( ; i <= string.size (); ++i)
					{
						if (i == string.size ())
						{
							i = -1;
							break;
						}

						if (string.mid (i, 2) == " (" ||
								(!i && string.at (i) == '('))
							++openedBraces;
						else if (string.mid (i, 2) == ") ")
							--openedBraces;

						if (!openedBraces &&
								string.mid (i, text.size ()) == text)
							break;
					}
					return i;
				}
			};

			RequestNormalizer::RequestHolder_ptr RequestNormalizer::Parse (QString req) const
			{
				req = req.simplified ();
				if (req.size () > 1 &&
						req.at (0) == '(' &&
						req.at (req.size () - 1) == ')')
					req = req.mid (1, req.size () - 2);

				RequestHolder_ptr node (new RequestHolder ());

				int pos = 0;
				if ((pos = FindWB (" OR ", req)) != -1)
				{
					node->Op_ = OperationalModel::OpOr;
					node->Left_ = Parse (req.left (pos));
					node->Right_ = Parse (req.mid (pos + sizeof (" OR ") - 1));
				}
				else if ((pos = FindWB (" AND ", req)) != -1)
				{
					node->Op_ = OperationalModel::OpAnd;
					node->Left_ = Parse (req.left (pos));
					node->Right_ = Parse (req.mid (pos + sizeof (" AND ") - 1));
				}
				// If there are no OR/ANDs out of braces, but there is an opening
				// brace, than there is some text before it. The same with the
				// closing brace.
				else if ((pos = req.indexOf ('(')) > 0)
				{
					QString add = req.left (pos).trimmed ();
					add.append (' ');

					int rightPos = req.lastIndexOf (')');
					QString subBraces = req.mid (pos + 1, rightPos - pos - 1);
					subBraces.prepend (add);
					subBraces.replace (" AND ", QString (" AND %1").arg (add));
					subBraces.replace (" OR ", QString (" OR %1").arg (add));
					req.replace (pos + 1, rightPos - pos - 1, subBraces);
					req = req.mid (pos);
					node = Parse (req);
				}
				else if ((pos = req.lastIndexOf (')')) > 0)
				{
					QString add = req.mid (pos + 1).trimmed ();
					add.prepend (' ');

					int leftPos = req.indexOf ('(');
					QString subBraces = req.mid (leftPos + 1, pos - leftPos - 1);
					subBraces.append (add);
					subBraces.replace (" AND ", QString ("%1 AND ").arg (add));
					subBraces.replace (" OR ", QString ("%1 OR ").arg (add));
					req.replace (leftPos + 1, pos - leftPos - 1, subBraces);
					req = req.left (req.size () - add.size ());
					node = Parse (req);
				}
				else
				{
					Parser_->Parse (req);
					node->Req_.reset (new Request (Parser_->GetRequest ()));
				}

				return node;
			}

			void RequestNormalizer::SetMerger (RequestHolder_ptr holder)
			{
				if (holder->Req_)
				{
					CategoryMerger *merger = new CategoryMerger (*holder->Req_,
							MergeModel_);
					holder->Merger_.reset (merger);
				}
				else
				{
					SetMerger (holder->Left_);
					SetMerger (holder->Right_);

					OperationalModel *oper = new OperationalModel;
					oper->SetOperation (holder->Op_);
					oper->AddModel (holder->Left_->Merger_.get ());
					oper->AddModel (holder->Right_->Merger_.get ());
					oper->setObjectName (holder->Left_->Merger_->objectName () +
							" &&& " +
							holder->Right_->Merger_->objectName ());
					holder->Merger_.reset (oper);
				}
			}
		};
	};
};

