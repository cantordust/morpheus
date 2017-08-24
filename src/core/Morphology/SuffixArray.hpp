#ifndef SUFFIXARRAY_HPP
#define SUFFIXARRAY_HPP

#include "Globals.hpp"

namespace Morpheus
{

	struct Context
	{
			/// distance, char, occurrences
			hashmap<uint, QHash<QChar,uint>> predecessor_positions;
			hashmap<uint, QHash<QChar,uint>> successor_positions;
	};

	class SuffixArray : public QObject
	{
			Q_OBJECT

			typedef std::vector<uint>::const_iterator vcit;
			typedef std::pair<vcit, vcit> vrange;

		private:

			/// A generic progress value
			uint progress;

			QFile input_file;
			QFile sa_file;

			/// The input file as a continuous string
			static std::vector<uchar> input_string;

			/// Array holding conversion indices
			/// from the suffix array to the original string
			std::vector<uint> char_index;

			/// Character counts
			hashmap<uchar, uint> char_counts;

			/// S-suffix count
			uint s_count;

			/// L-suffix count
			uint l_count;

			/// S- of L-type suffixes to be sorted
			std::map<uchar, std::vector<uint>> suffixes_to_sort;

			/// Sorted S- or L-type suffixe
			std::map<uchar, std::vector<uint>> sorted_suffixes;

			/// Suffixes which are to be arranged after the complementary ones are sorted
			hashset<uint> suffixes_to_arrange;

			/// The final suffix array
			std::map<uchar, std::vector<uint>> SA;
			hashmap<uchar, std::pair<uint, uint>> bucket_pos;

			QHash<QChar, uint> predecessors;
			QHash<QChar, uint> successors;

			/// map<pattern length, map<# of occurrences, string>>
			QHash<QString, uint> pattern_occurrences;

			static struct cmp
			{
					uint depth;

					bool operator()(const uint _pos,
									const uchar _key_ch)
					{
						if (_pos + depth < input_string.size())
						{
							return input_string[_pos + depth] < _key_ch;
						}
						return true;
					}

					bool operator()(const uchar _key_ch,
									const uint _pos)
					{
						if (_pos + depth < input_string.size())
						{
							return _key_ch < input_string[_pos + depth];
						}
						return false;
					}

			} char_cmp;

			void sort_suffixes(const std::vector<uint>&& _positions,
							   const uint _depth);

			void load_corpus();

			std::vector<uchar> str_to_vec(const QString& _str);

			void make_index(const std::vector<uchar>& _input,
							std::vector<uint>& _index);

			std::map<uchar, std::vector<uint>> create_SA(const std::vector<uchar>& _input);

			void save_SA();

			void put_chars_in_buckets(const std::vector<uchar>& _input,
									  hashmap<uchar, uint>& _char_counts,
									  const bool _progress = false);

			inline void sort(const uint _last_pos,
							 const bool _progress = false);

			void compile_suffix_array(std::map<uchar, std::vector<uint> >& _SA,
									  const hashmap<uchar,uint>& _char_counts,
									  const bool _progress = false);

			vrange get_equal_range(std::map<uchar,std::vector<uint>>& _SA,
								   const std::vector<uchar>& _key);

			vrange get_equal_range(std::map<uchar,std::vector<uint>>& _SA,
								   const QString& _qstr);

		public:

			SuffixArray(){}

			~SuffixArray(){}

			/// Get the total number of occurrences of a string
			inline uint get_occurrences(const QString& _qstr)
			{
				vrange range(get_equal_range(SA, std::move(_qstr)));
				if (std::distance(range.first, range.second) <= 0)
				{
					return 0;
				}
				return std::distance(range.first, range.second);
			}

			/// Get the total number of occurrences of a string
			/// represented as a vector<uchar>
			inline uint get_occurrences(const std::vector<uchar>& _vec)
			{
				vrange range(get_equal_range(SA, std::move(_vec)));
				if (std::distance(range.first, range.second) <= 0)
				{
					return 0;
				}
				return std::distance(range.first, range.second);
			}

			/// Count the total number of predecessors (as Unicode characters, not as chars)
			inline uint get_total_predecessor_count(const QString& _key)
			{
				uint total(0);
				vrange range(get_equal_range(SA, std::move(_key)));
				while (range.first != range.second)
				{
					if (*range.first > 0)
					{
						++total;
					}
					++range.first;
				}
				return total;
			}

			/// Count the total number of successors (as Unicode characters, not as chars)
			inline uint get_total_successor_count(const QString& _key)
			{
				uint total(0);
				vrange range(get_equal_range(SA, std::move(_key)));
				while (range.first != range.second)
				{
					if (input_string.size() - 1 > *range.first + _key.size())
					{
						++total;
					}
					++range.first;
				}
				return total;
			}

			/// Count the number of distinct predecessors (as Unicode characters, not as chars)
			inline uint get_distinct_predecessor_count(const QString& _key)
			{
				return get_predecessors(std::move(_key)).size();
			}

			/// Count the number of distinct successors (as Unicode characters, not as chars)
			inline uint get_distinct_successor_count(const QString& _key)
			{
				return get_successors(std::move(_key)).size();
			}

			QHash<QChar, uint> get_predecessors(const QString& _key);

			QHash<QChar, uint> get_successors(const QString& _key);

			hashset<uchar> get_successors(const std::vector<uchar>& _key);

			hashset<uchar> get_successors(vrange& _range,
										  uint _depth,
										  const std::vector<uchar>& _input);

			/// Turn an array of uchars into a QString
			QString vec_to_qstr(std::vector<uchar> _str);


		public slots:

			void set_filenames(const QString& _file_name);

		signals:

			void set_progress_text(const QString& _text);
			void set_progress_max(const uint _max);
			void set_progress_value(const uint _progress);

	};
}
#endif // SUFFIXARRAY_HPP
