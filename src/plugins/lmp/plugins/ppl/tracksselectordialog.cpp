/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tracksselectordialog.h"
#include <numeric>
#include <QAbstractItemModel>
#include <QApplication>
#include <QDesktopWidget>
#include <QShortcut>
#include <util/sll/prelude.h>
#include <util/sll/views.h>
#include <util/sll/util.h>

namespace LC
{
namespace LMP
{
namespace PPL
{
	class TracksSelectorDialog::TracksModel : public QAbstractItemModel
	{
		const QStringList HeaderLabels_;

		enum Header : uint8_t
		{
			ScrobbleSummary,
			Artist,
			Album,
			Track,
			Date
		};
		static constexpr uint8_t MaxPredefinedHeader = Header::Date;

		const Media::IAudioScrobbler::BackdatedTracks_t Tracks_;

		QVector<QVector<bool>> Scrobble_;
	public:
		TracksModel (const Media::IAudioScrobbler::BackdatedTracks_t&,
				const QList<Media::IAudioScrobbler*>&, QObject* = nullptr);

		static constexpr uint8_t ColumnSelectAll = Header::ScrobbleSummary;

		QModelIndex index (int, int, const QModelIndex& = {}) const override;
		QModelIndex parent (const QModelIndex&) const override;
		int rowCount (const QModelIndex& = {}) const override;
		int columnCount (const QModelIndex&) const override;
		QVariant data (const QModelIndex&, int) const override;

		QVariant headerData (int, Qt::Orientation, int) const override;

		Qt::ItemFlags flags (const QModelIndex& index) const override;
		bool setData (const QModelIndex& index, const QVariant& value, int role) override;

		QList<TracksSelectorDialog::SelectedTrack> GetSelectedTracks () const;

		void MarkAll ();
		void UnmarkAll ();
		void SetMarked (const QList<QModelIndex>&, bool);

		void UnmarkRepeats ();
	private:
		template<typename Summary, typename Specific>
		auto WithCheckableColumns (const QModelIndex& index, Summary&& summary, Specific&& specific) const
		{
			switch (index.column ())
			{
			case Header::Artist:
			case Header::Album:
			case Header::Track:
			case Header::Date:
				using ResultType_t = std::result_of_t<Summary (int)>;
				if constexpr (std::is_same_v<ResultType_t, void>)
					return;
				else
					return ResultType_t {};
			case Header::ScrobbleSummary:
				return summary (index.row ());
			}

			return specific (index.row (), index.column () - (MaxPredefinedHeader + 1));
		}

		void MarkRow (const QModelIndex&, bool);
	};

	TracksSelectorDialog::TracksModel::TracksModel (const Media::IAudioScrobbler::BackdatedTracks_t& tracks,
			const QList<Media::IAudioScrobbler*>& scrobblers, QObject *parent)
	: QAbstractItemModel { parent }
	, HeaderLabels_
	{
		[&scrobblers]
		{
			const QStringList predefined
			{
				{},
				tr ("Artist"),
				tr ("Album"),
				tr ("Title"),
				tr ("Date")
			};
			const auto& scrobbleNames = Util::Map (scrobblers, &Media::IAudioScrobbler::GetServiceName);
			return predefined + scrobbleNames;
		} ()
	}
	, Tracks_ { tracks }
	, Scrobble_ { tracks.size (), QVector<bool> (scrobblers.size (), true) }
	{
	}

	QModelIndex TracksSelectorDialog::TracksModel::index (int row, int column, const QModelIndex& parent) const
	{
		return parent.isValid () ?
				QModelIndex {} :
				createIndex (row, column);
	}

	QModelIndex TracksSelectorDialog::TracksModel::parent (const QModelIndex&) const
	{
		return {};
	}

	int TracksSelectorDialog::TracksModel::rowCount (const QModelIndex& parent) const
	{
		return parent.isValid () ?
				0 :
				Tracks_.size () + 1;
	}

	int TracksSelectorDialog::TracksModel::columnCount (const QModelIndex& parent) const
	{
		return parent.isValid () ?
				0 :
				HeaderLabels_.size ();
	}

	namespace
	{
		template<typename H, typename D>
		auto WithIndex (const QModelIndex& index, H&& header, D&& data)
		{
			if (!index.row ())
				return header (index);
			else
				return data (index.sibling (index.row () - 1, index.column ()));
		}

		QVariant PartialCheck (int enabled, int total)
		{
			if (!enabled)
				return Qt::Unchecked;
			else if (enabled == total)
				return Qt::Checked;
			else
				return Qt::PartiallyChecked;
		}
	}

	QVariant TracksSelectorDialog::TracksModel::data (const QModelIndex& srcIdx, int role) const
	{
		return WithIndex (srcIdx,
				[&] (const QModelIndex& index) -> QVariant
				{
					if (role != Qt::CheckStateRole)
						return {};

					return WithCheckableColumns (index,
							[this] (int)
							{
								const auto enabled = std::accumulate (Scrobble_.begin (), Scrobble_.end (), 0,
										[] (int val, const auto& subvec)
										{
											return std::accumulate (subvec.begin (), subvec.end (), val);
										});
								const auto total = Scrobble_.size () * Scrobble_ [0].size ();
								return PartialCheck (enabled, total);
							},
							[this] (int, int column)
							{
								const auto enabled = std::accumulate (Scrobble_.begin (), Scrobble_.end (), 0,
										[column] (int val, const auto& subvec) { return val + subvec.at (column); });
								return PartialCheck (enabled, Scrobble_.size ());
							});
				},
				[&] (const QModelIndex& index) -> QVariant
				{
					switch (role)
					{
					case Qt::DisplayRole:
					{
						const auto& record = Tracks_.value (index.row ());

						switch (index.column ())
						{
						case Header::ScrobbleSummary:
							return {};
						case Header::Artist:
							return record.first.Artist_;
						case Header::Album:
							return record.first.Album_;
						case Header::Track:
							return record.first.Title_;
						case Header::Date:
							return record.second.toString ();
						}

						return {};
					}
					case Qt::CheckStateRole:
					{
						return WithCheckableColumns (index,
								[&] (int row)
								{
									const auto& flags = Scrobble_.value (row);
									const auto enabled = std::accumulate (flags.begin (), flags.end (), 0);
									return PartialCheck (enabled, flags.size ());
								},
								[&] (int row, int column) -> QVariant
								{
									return Scrobble_.value (row).value (column) ?
											Qt::Checked :
											Qt::Unchecked;
								});
					}
					default:
						return {};
					}
				});
	}

	QVariant TracksSelectorDialog::TracksModel::headerData (int section, Qt::Orientation orientation, int role) const
	{
		if (role != Qt::DisplayRole)
			return {};

		switch (orientation)
		{
		case Qt::Horizontal:
			return HeaderLabels_.value (section);
		case Qt::Vertical:
			return section ?
					QString::number (section) :
					tr ("All");
		default:
			return {};
		}
	}

	Qt::ItemFlags TracksSelectorDialog::TracksModel::flags (const QModelIndex& index) const
	{
		switch (index.column ())
		{
		case Header::Artist:
		case Header::Album:
		case Header::Track:
		case Header::Date:
			return QAbstractItemModel::flags (index);
		default:
			return Qt::ItemIsSelectable |
					Qt::ItemIsEnabled |
					Qt::ItemIsUserCheckable;
		}
	}

	bool TracksSelectorDialog::TracksModel::setData (const QModelIndex& srcIdx, const QVariant& value, int role)
	{
		if (role != Qt::CheckStateRole)
			return false;

		MarkRow (srcIdx, value.toInt () == Qt::Checked);
		return true;
	}

	QList<TracksSelectorDialog::SelectedTrack> TracksSelectorDialog::TracksModel::GetSelectedTracks () const
	{
		QList<TracksSelectorDialog::SelectedTrack> result;
		for (const auto& [track, scrobbles] : Util::Views::Zip<std::pair> (Tracks_, Scrobble_))
			if (std::ranges::any_of (scrobbles, Util::Id))
				result.push_back ({ track, scrobbles });
		return result;
	}

	void TracksSelectorDialog::TracksModel::MarkAll ()
	{
		for (int i = 0; i < rowCount (); ++i)
			MarkRow (index (i, Header::ScrobbleSummary), true);
	}

	void TracksSelectorDialog::TracksModel::UnmarkAll ()
	{
		for (int i = 0; i < rowCount (); ++i)
			MarkRow (index (i, Header::ScrobbleSummary), false);
	}

	void TracksSelectorDialog::TracksModel::SetMarked (const QList<QModelIndex>& indices, bool shouldScrobble)
	{
		for (const auto& idx : indices)
			MarkRow (idx, shouldScrobble);
	}

	void TracksSelectorDialog::TracksModel::UnmarkRepeats ()
	{
		const auto asTuple = [] (const auto& pair)
		{
			const auto& media = pair.first;
			return std::tie (media.Artist_, media.Album_, media.Title_);
		};

		for (auto pos = Tracks_.begin (); pos != Tracks_.end (); )
		{
			pos = std::adjacent_find (pos, Tracks_.end (), Util::EqualityBy (asTuple));
			if (pos == Tracks_.end ())
				break;

			const auto& referenceInfo = asTuple (*pos);
			while (++pos != Tracks_.end () && asTuple (*pos) == referenceInfo)
				MarkRow (index (std::distance (Tracks_.begin (), pos) + 1, Header::ScrobbleSummary), false);
		}
	}

	void TracksSelectorDialog::TracksModel::MarkRow (const QModelIndex& srcIdx, bool shouldScrobble)
	{
		WithIndex (srcIdx,
				[&] (const QModelIndex& index)
				{
					WithCheckableColumns (index,
							[&] (int)
							{
								for (auto& subvec : Scrobble_)
									std::ranges::fill (subvec, shouldScrobble);
							},
							[&] (int, int column)
							{
								for (auto& subvec : Scrobble_)
									subvec [column] = shouldScrobble;
							});

					const auto lastRow = rowCount (index.parent ()) - 1;
					const auto lastColumn = columnCount (index.parent ()) - 1;
					emit dataChanged (createIndex (0, 0), createIndex (lastRow, lastColumn));
				},
				[&] (const QModelIndex& index)
				{
					auto& scrobbles = Scrobble_ [index.row ()];

					WithCheckableColumns (index,
							[&] (int) { std::ranges::fill (scrobbles, shouldScrobble); },
							[&] (int, int column) { scrobbles [column] = shouldScrobble; });

					const auto lastColumn = columnCount (index.parent ()) - 1;
					emit dataChanged (createIndex (0, 0), createIndex (0, lastColumn));
					emit dataChanged (index.sibling (index.row (), 0), index.sibling (index.row (), lastColumn));
				});
	}

	TracksSelectorDialog::TracksSelectorDialog (const Media::IAudioScrobbler::BackdatedTracks_t& tracks,
			const QList<Media::IAudioScrobbler*>& scrobblers,
			QWidget *parent)
	: QDialog { parent }
	, Model_ { new TracksModel { tracks, scrobblers, this } }
	{
		Ui_.setupUi (this);
		Ui_.Tracks_->setModel (Model_);

		FixSize ();

		auto withSelected = [this] (bool shouldScrobble)
		{
			return [this, shouldScrobble]
			{
				auto indices = Ui_.Tracks_->selectionModel ()->selectedIndexes ();
				if (indices.isEmpty ())
					return;

				const auto notCheckable = [] (const auto& idx) { return !(idx.flags () & Qt::ItemIsUserCheckable); };

				if (notCheckable (indices.value (0)))
				{
					const auto column = indices.value (0).column ();
					if (std::ranges::all_of (indices,
							[&column] (const auto& idx) { return idx.column () == column; }))
						for (auto& idx : indices)
							idx = idx.sibling (idx.row (), TracksModel::ColumnSelectAll);
				}

				indices.erase (std::remove_if (indices.begin (), indices.end (), notCheckable), indices.end ());

				Model_->SetMarked (indices, shouldScrobble);
			};
		};

		connect (new QShortcut { QString { "Alt+S" }, this },
				&QShortcut::activated,
				Ui_.MarkSelected_,
				&QPushButton::click);
		connect (new QShortcut { QString { "Alt+U" }, this },
				&QShortcut::activated,
				Ui_.UnmarkSelected_,
				&QPushButton::click);

		connect (Ui_.MarkAll_,
				&QPushButton::clicked,
				[this] { Model_->MarkAll (); });
		connect (Ui_.UnmarkAll_,
				&QPushButton::clicked,
				[this] { Model_->UnmarkAll (); });
		connect (Ui_.UnmarkRepeats_,
				&QPushButton::clicked,
				[this] { Model_->UnmarkRepeats (); });
		connect (Ui_.MarkSelected_,
				&QPushButton::clicked,
				withSelected (true));
		connect (Ui_.UnmarkSelected_,
				&QPushButton::clicked,
				withSelected (false));
	}

	void TracksSelectorDialog::FixSize ()
	{
		Ui_.Tracks_->resizeColumnsToContents ();

		const auto showGuard = Util::MakeScopeGuard ([this] { show (); });

		const auto Margin = 50;

		int totalWidth = Margin + Ui_.Tracks_->verticalHeader ()->width ();

		const auto header = Ui_.Tracks_->horizontalHeader ();
		for (int j = 0; j < Model_->columnCount ({}); ++j)
			totalWidth += std::max (header->sectionSize (j),
					Ui_.Tracks_->sizeHintForIndex (Model_->index (0, j, {})).width ());

		totalWidth += Ui_.ButtonsLayout_->sizeHint ().width ();

		if (totalWidth < size ().width ())
			return;

		const auto desktop = qApp->desktop ();
		const auto& availableGeometry = desktop->availableGeometry (this);
		if (totalWidth > availableGeometry.width ())
			return;

		setGeometry (QStyle::alignedRect (Qt::LeftToRight,
				Qt::AlignCenter,
				{ totalWidth, height () },
				availableGeometry));
	}

	QList<TracksSelectorDialog::SelectedTrack> TracksSelectorDialog::GetSelectedTracks () const
	{
		return Model_->GetSelectedTracks ();
	}
}
}
}
