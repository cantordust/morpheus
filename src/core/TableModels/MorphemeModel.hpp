#ifndef MORPHEMEMODEL_HPP
#define MORPHEMEMODEL_HPP
#include "Globals.hpp"
#include <QAbstractTableModel>
#include "MorphemeExtractor.hpp"

namespace Morpheus
{
	class MorphemeModel : public QAbstractTableModel
	{
			Q_OBJECT
		private:
			MorphemeExtractor* me;

		public:
			MorphemeModel(QObject* _parent = 0)
				:
				  QAbstractTableModel(_parent)
			{

			}

			MorphemeModel(MorphemeExtractor* _me,
						  QObject* _parent = 0)
				:
				  QAbstractTableModel(_parent),
				  me(_me)
			{

			}

			~MorphemeModel()
			{

			}

			inline int rowCount(const QModelIndex& _parent = QModelIndex()) const
			{
				if (me != nullptr)
				{
					return me->get_dictionary_size();
				}
				return 0;
			}

			inline int columnCount(const QModelIndex& _parent = QModelIndex()) const
			{
				return 2;
			}

			inline QVariant data(const QModelIndex& _index, int _role = Qt::DisplayRole) const
			{
				if (!_index.isValid())
				{
					return QVariant();
				}
				if (_role == Qt::DisplayRole)
				{
					switch (_index.column())
					{
						case 0:
							return me->dictionary_vector[_index.row()].first;
							break;

						case 1:
							return me->dictionary_vector[_index.row()].second;
							break;
					}
				}
				return QVariant();
			}

			inline Qt::ItemFlags flags(const QModelIndex & _index) const
			{
				return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
			}

			inline QVariant headerData(int _section, Qt::Orientation _orientation, int _role) const
			{
				if (_role != Qt::DisplayRole)
				{
					return QVariant();
				}
				else if (_orientation == Qt::Horizontal)
				{
					switch (_section)
					{
						case 0:
							return QString("Morpheme");
						case 1:
							return QString("Count");
					}
				}
				return QVariant();
			}
	};
}
#endif // MORPHEMEMODEL_HPP
