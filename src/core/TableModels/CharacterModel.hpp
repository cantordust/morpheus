#ifndef CHARACTERMODEL_HPP
#define CHARACTERMODEL_HPP
#include "Globals.hpp"
#include "MorphemeExtractor.hpp"
#include <QAbstractTableModel>

namespace Morpheus
{
	class CharacterModel : public QAbstractTableModel
	{
			Q_OBJECT
		private:
			MorphemeExtractor* me;

		public:

			CharacterModel(QObject* _parent = 0)
				:
				  QAbstractTableModel(_parent)
			{

			}

			CharacterModel(MorphemeExtractor* _me,
						   QObject* _parent = 0)
				:
				  QAbstractTableModel(_parent),
				  me(_me)
			{

			}

			~CharacterModel()
			{

			}

			inline int rowCount(const QModelIndex& _parent) const
			{
				if (me != nullptr)
				{
					return me->get_alphabet_size();
				}
				return 0;
			}

			inline int columnCount(const QModelIndex& _parent) const
			{
				return 3;
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
							return me->alphabet_vector[_index.row()].first;
							break;

						case 1:
							return me->alphabet_vector[_index.row()].second;
							break;

						case 2:
							return me->alphabet_vector[_index.row()].second / static_cast<real>(me->get_total_character_count());
							break;
					}
				}
				return QVariant();
			}

			inline Qt::ItemFlags flags(const QModelIndex& _index) const
			{
				return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
			}

			inline QVariant headerData(int _section,
									   Qt::Orientation _orientation,
									   int _role) const
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
							return QString("Character");
						case 1:
							return QString("Raw count");
						case 2:
							return QString("Normalised");
					}
				}
				return QVariant();
			}
	};
}
#endif // CHARACTERMODEL_HPP
