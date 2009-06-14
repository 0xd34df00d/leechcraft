#include "trackerschanger.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			TrackersChanger::TrackersChanger (QWidget *parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);
			}
			
			void TrackersChanger::SetTrackers (const QStringList& trackers)
			{
				QString result = trackers.join ("\n");
				Ui_.TrackersEdit_->setText (result);
			}
			
			QStringList TrackersChanger::GetTrackers () const
			{
				QString raw = Ui_.TrackersEdit_->toPlainText ();
				QStringList result = raw.split ('\n');
				for (int i = 0; i < result.size (); ++i)
					result [i]  = result.at (i).trimmed ();
				return result;
			}
			
		};
	};
};

