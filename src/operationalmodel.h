#ifndef OPERATIONALMODEL_H
#define OPERATIONALMODEL_H
#include <plugininterface/mergemodel.h>

namespace LeechCraft
{
	class OperationalModel : public Util::MergeModel
	{
		Q_OBJECT
	public:
		enum Operation
		{
			OpNull,
			OpAnd,
			OpOr
		};
	private:
		Operation Op_;
	public:
		OperationalModel (QObject* = 0);
		void SetOperation (Operation);
	protected:
		bool AcceptsRow (QAbstractItemModel*, int) const;
	};
};

#endif

