#ifndef CHARACTERTRANITIONSMODEL_HPP
#define CHARACTERTRANITIONSMODEL_HPP
#include "Globals.hpp"
#include "CharacterOccurrenceModel.hpp"
#include "MorphemeExtractor.hpp"
#include <QAbstractTableModel>

namespace Morpheus
{
	class CharacterTransitionModel : public QAbstractTableModel
	{
		private:
			MorphemeExtractor* me;

		public:
			CharacterTransitionModel(QObject* _parent = 0)
				:
				  QAbstractTableModel(_parent)
			{

			}

			CharacterTransitionModel(MorphemeExtractor* _me,
									 QObject* _parent = 0)
				:
				  QAbstractTableModel(_parent),
				  me(_me)
			{

			}

			~CharacterTransitionModel()
			{

			}

			inline int rowCount(const QModelIndex& _parent = QModelIndex()) const
			{
				return me->get_character_occurrences().size();;
			}

			inline int columnCount(const QModelIndex& _parent = QModelIndex()) const
			{
				return me->get_character_occurrences().size();;
			}

			inline QVariant data(const QModelIndex& _index,
								 int _role = Qt::DisplayRole) const
			{
				if (!_index.isValid())
				{
					return QVariant();
				}
				if (_role == Qt::DisplayRole)
				{
					if (Config::transition_table_orientation == TransitionTableOrientation::SuccessorsOnTop)
					{
						return value_at(_index.row(), _index.column());
					}
					else
					{
						return value_at(_index.column(), _index.row());
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
				else if (_orientation == Qt::Horizontal ||
						 _orientation == Qt::Vertical)
				{
					return (me->get_character_occurrences().begin() + _section).key();
				}
				return QVariant();
			}


			inline QChar key_at_row(int _row) const
			{
				return (me->get_character_occurrences().begin() + _row).key();
			}

			inline QChar key_at_col(int _column) const
			{
				return (me->get_character_occurrences().begin() + _column).key();
			}

			inline Real value_at(int _row, int _column) const
			{
				QChar row_char = key_at_row(_row);
				QChar col_char = key_at_col(_column);

				if (me->get_raw_character_transitions().contains(row_char))
				{
					if (me->get_raw_character_transitions()[row_char].contains(col_char))
					{
						if (Config::transition_type == TransitionTableType::Raw)
						{
							return static_cast<Real>(me->get_raw_character_transitions()[row_char][col_char]);
						}
						else
						{
							return static_cast<Real>(me->get_normalised_character_transitions()[row_char][col_char]);
						}
					}
					return 0.0;
				}
				return 0.0;

			}
	};
}
#endif // CHARACTERTRANITIONSMODEL_HPP
