#ifndef MORPHEMEEXTRACTOR_HPP
#define MORPHEMEEXTRACTOR_HPP
#include "Globals.hpp"
#include "Config.hpp"
#include "SuffixArray.hpp"

namespace Morpheus
{
	class MorphemeExtractor : public QObject
	{
			Q_OBJECT
		private:

			/// Total number of characters
			uint total_char_count;

			/// The number of character transitions
			uint char_transition_count;

			/// Entropy of the entire alphabet
			real alphabet_ent;

			/// Normalised entropy of the entire alphabet
			real alphabet_norm_ent;

			/// Corpus length (incremental)
			uint corpus_length;

			/// Dictionary cost
			real dict_cost;

			/// Corpus entropy
			real dict_entropy;

			/// Average occurrence probability for any character
			real avg_char_prob;

			/// Sum of all occurrences of all morphemes
			uint total_morpheme_count;

			/// Individual character counts
			QHash<QChar, uint> alphabet;

			/// All extracted morphemes
			QHash<QString, uint> dictionary;

			/// Entropy of each character
			QHash<QChar, real> char_code_length;

			/// Morpheme cost in terms of character entropy
			QHash<QString, real> morpheme_cost;

			/// Character transitions
			QHash<QChar, QHash<QChar, uint>> character_transitions;

			/// Number of predecessors
			QHash<QString, uint> p_cache;

			/// Number of successors
			QHash<QString, uint> s_cache;

			/// Entropy of predecessors
			QHash<QString, real> p_ent_cache;

			/// Entropy of successors
			QHash<QString, real> s_ent_cache;

			/// Temporary dictionary of string occurrences
			QHash<QString, uint> string_cache;

			/// Suffix array instance for searching
			uptr<SuffixArray> sa;

			///
			/// \brief Find the total number of predecessors of the	search string
			/// \param _string
			/// \return
			///
			inline uint get_total_predecessor_count(const QString& _string)
			{
				return sa->get_total_predecessor_count(std::move(_string));
			}

			///
			/// \brief Find the total number of successors of the search string
			/// \param _string
			/// \return
			///
			inline uint get_total_successor_count(const QString& _string)
			{
				return sa->get_total_successor_count(std::move(_string));
			}

			///
			/// \brief Find the number of distinct predecessors of the search string
			/// \param _string
			/// \return
			///
			inline uint get_distinct_predecessor_count(const QString& _string)
			{
				if (_string.size() == 0)
				{
					return alphabet.size();
				}
				else if (!p_cache.contains(_string))
				{
					p_cache[_string] = sa->get_distinct_predecessor_count(std::move(_string));
				}
				return p_cache[_string];
			}

			///
			/// \brief Find the number of distinct successors of the search string
			/// \param _string
			/// \return
			///
			inline uint get_distinct_successor_count(const QString& _string)
			{
				if (_string.size() == 0)
				{
					return alphabet.size();
				}
				else if (!s_cache.contains(_string))
				{
					s_cache[_string] = sa->get_distinct_successor_count(std::move(_string));
				}
				return s_cache[_string];
			}

			///
			/// \brief Compute the entropy of the predecessors of the search string
			/// \param _string
			/// \param _ch: predecessor
			/// \return
			///
			real get_predecessor_entropy(const QString& _string,
												const QString& _ch = "");

			///
			/// \brief Compute the entropy of the successors of the search string
			/// \param _string
			/// \param _ch: successor
			/// \return
			///
			real get_successor_entropy(const QString& _string,
											  const QString& _ch = "");

			///
			/// \brief Compute the *normalised* entropy of the predecessors of the search string
			/// \param _string
			/// \param _ch: predecessor
			/// \return
			///
			inline real get_norm_predecessor_entropy(const QString& _string,
													 const QString& _ch = "")
			{
				if ((_ch + _string).size() == 0
					|| get_total_predecessor_count(_string) == 0)
				{
					return alphabet_norm_ent;
				}
				real p_total(get_total_predecessor_count(_string));
				return (get_predecessor_entropy(_string, _ch) / (p_total > 1 ? static_cast<real>(std::log2(p_total) ) : 1.0 ) );
			}

			///
			/// \brief Compute the *normalised* entropy of the successors of the search string
			/// \param _string
			/// \param _ch: successor
			/// \return
			///
			inline real get_norm_successor_entropy(const QString& _string,
												   const QString& _ch = "")
			{
				if ((_string + _ch).size() == 0
					|| get_total_successor_count(_string) == 0)
				{
					return alphabet_norm_ent;
				}
				real p_total(get_total_successor_count(_string));
				return (get_successor_entropy(_string, _ch) / (p_total > 1 ? static_cast<real>(std::log2(p_total) ) : 1.0 ) );
			}

			///
			/// \brief Find occurrences of the current string in the processed text
			/// \param _string
			/// \return
			///
			inline uint get_occurrences(const QString& _string)
			{
				if (!string_cache.contains(_string))
				{
					string_cache[_string] = sa->get_occurrences(std::move(_string));
				}
				return string_cache[_string];
			}

			///
			/// \brief get_description_length
			/// \param _string
			/// \return
			///
			inline real get_description_length(const QString& _string)
			{
				if (!dictionary.contains(_string))
				{
					uint norm(0);
					for (const QChar& ch : alphabet.keys())
					{
						norm += alphabet[ch];
					}
					return -std::log(norm / static_cast<real>(total_char_count));
				}
				return -std::log(dictionary[_string] / static_cast<real>(dictionary.size()));
			}

			///
			/// \brief Returns the percentage of the corpus attributable to
			/// _string in terms of number of characters
			/// \param _string
			/// \return
			///
			inline real get_corpus_coverage(const QString& _string)
			{
				if (_string.size() > 0)
				{
					return (_string.size() * get_occurrences(_string)) / static_cast<real>(total_char_count);
				}
				return 0.0;
			}

			///
			/// \brief Extract morphemes based on predecessor / successor counts and/or entropy.
			///	ps = predecessors & successors
			/// \param _line
			/// \return
			///
			QString extract_morphemes_ps(const QString&& _line,
										 const bool _reseg = false);

			///
			/// \brief Extract morphemes based on character frequencies
			/// \param _line
			/// \return
			///
			QString extract_morphemes_character_frequencies(const QString&& _line);

			///
			/// \brief Computes the new dictionary cost for a potential split
			/// \param _word
			/// \param _left
			/// \param _right
			/// \return
			///
			real get_dict_cost_split(const QString& _word,
									 const QString& _left,
									 const QString& _right);

			///
			/// \brief Computes the new corpus entropy for a potential split
			/// \param _word
			/// \param _left
			/// \param _right
			/// \return
			///
			real get_dict_entropy_split(const QString& _word,
										  const QString& _left,
										  const QString& _right);


			///
			/// \brief Computes the new dictionary cost for a potential merge
			/// \param _word
			/// \param _left
			/// \param _right
			/// \return
			///
			real get_dict_cost_merge(const QString& _word,
									 const QString& _left,
									 const QString& _right);

			///
			/// \brief Computes the new corpus entropy for a potential merge
			/// \param _word
			/// \param _left
			/// \param _right
			/// \return
			///
			real get_corpus_entropy_merge(const QString& _word,
										  const QString& _left,
										  const QString& _right);

		public:

			MorphemeExtractor()
			{

			}

			~MorphemeExtractor()
			{

			}

			///
			/// \brief Return the total number of characters in the corpus
			/// \return
			///
			inline uint get_total_character_count() const
			{
				return total_char_count;
			}

			///
			/// \brief Return the total number of morphemes in the corpus
			/// \return
			///
			inline uint get_total_morpheme_count() const
			{
				return total_morpheme_count;
			}

			///
			/// \brief Return the number of character types (not the total character count)
			/// \return
			///
			inline uint get_alphabet_size() const
			{
				return static_cast<uint>(alphabet.size());
			}

			///
			/// \brief Return the number of distinct morphemes in the dictionary
			/// \return
			///
			inline uint get_dictionary_size	() const
			{
				return static_cast<uint>(dictionary.size());
			}

			///
			/// \brief Return the alphabet
			/// (distinct characters with their counts)
			/// \return
			///
			inline QHash<QChar,uint>& get_alphabet()
			{
				return alphabet;
			}

			///
			/// \brief Return the dictionary
			/// (distinct morphemes with their counts)
			/// \return
			///
			inline QHash<QString,uint>& get_dictionary()
			{
				return dictionary;
			}

			///
			/// \brief Vector holding the alphabet.
			/// Used for fast viewing and sorting in tables.
			///
			QVector<QPair<QChar,uint>> alphabet_vector;

			///
			/// \brief Vector holding the dictionary.
			/// Used for fast viewing and sorting in tables.
			///
			QVector<QPair<QString,uint>> dictionary_vector;

			///
			/// \brief Extract the characters from the preprocessed corpus.
			/// \param _processed_file
			///
			void extract_characters(const QString& _processed_file);

			///
			/// \brief Extract morphemes - interface function
			/// \param _line
			/// \return
			///
			QString extract_morphemes(const QString&& _line)
			{
				if (Config::seg_method_ps_count ||
					Config::seg_method_ps_entropy)
				{
					return extract_morphemes_ps(std::move(_line));
				}
				else if (Config::seg_method_character_frequencies)
				{
					return extract_morphemes_character_frequencies(std::move(_line));
				}
				return QString();
			}

			///
			/// \brief Optimises the current segmentation.
			///
			QString resegment_morphemes(const QString& _line);

			///
			/// \brief Computes the initial dictionary cost and entropy
			///
			void init_entropy();

		signals:

			void characters_extracted();
			void morphemes_extracted();
			void morphemes_resegmented();
			void morpheme_extractor_reset();
			void morpheme_extractor_cleared();
			void update_log(const QString& _message);

		public slots:

			///
			/// \brief Reset all private variables
			/// except the character and morpheme count
			/// and the collected characters.
			/// See reset() below.
			/// \param _clear_morphemes
			///
			inline void clear(const bool _clear_morphemes = false)
			{
				p_cache.clear();
				s_cache.clear();
				p_ent_cache.clear();
				s_ent_cache.clear();
				string_cache.clear();

				if (_clear_morphemes)
				{
					dictionary_vector.clear();
					dictionary.clear();
					total_morpheme_count = 0;
				}

				emit morpheme_extractor_cleared();
			}

			///
			/// \brief Reset all private variables
			///
			inline void reset()
			{
				clear(true);

				total_char_count = 0;
				total_morpheme_count = 0;
				alphabet.clear();

				emit morpheme_extractor_reset();
			}

			///
			/// \brief Initialise the morpheme extractor
			/// \param _processed_file
			///
			inline void init(const QString& _processed_file)
			{
				reset();
				sa = std::make_unique<SuffixArray>();
				sa->set_filenames(_processed_file);
				extract_characters(_processed_file);
			}

	};
}

#endif // MORPHEMEEXTRACTOR_HPP
