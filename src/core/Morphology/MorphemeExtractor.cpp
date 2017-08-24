#include "MorphemeExtractor.hpp"

namespace Morpheus
{

	void MorphemeExtractor::extract_characters(const QString& _processed_file)
	{
		total_char_count = 0;
		char_transition_count = 0;
		total_morpheme_count = 0;
		alphabet.clear();
		character_transitions.clear();
		char_code_length.clear();
		QFile corpus(_processed_file);
		QTextStream qts(&corpus);
		QString line;
		if (corpus.open(QFile::ReadOnly))
		{
			while(qts.readLineInto(&line))
			{
				uint index = 0;
				for (const QChar& ch : line)
				{
					//		    if (index > 0)
					//		    {
					//			++character_transitions[line.at(index - 1)][line.at(index)];
					//			++char_transition_count;
					//		    }
					++alphabet[ch];
					++total_char_count;
					++index;
				}
			}
		}

		avg_char_prob = 0.0;
		real count_tmp(static_cast<real>(total_char_count));
		for (const QChar& ch : alphabet.keys())
		{
			real ch_p(alphabet[ch] / count_tmp);
			char_code_length[ch] = - std::log(ch_p);
			avg_char_prob += ch_p;
		}
		avg_char_prob /= static_cast<real>(alphabet.size());

		/// Compute the normalised entropy of all characters and store it.
		real total(0);
		alphabet_ent = 0;
		alphabet_norm_ent = 0.0;

		for (const QChar& ch : alphabet.keys())
		{
			total += static_cast<real>(alphabet[ch]);
		}

		for (const QChar& ch : alphabet.keys())
		{
			real prob(alphabet[ch] / total);
			alphabet_ent -= prob * std::log(prob);
		}
		alphabet_norm_ent = alphabet_ent / static_cast<real>(std::log(total_char_count));

		emit characters_extracted();
	}

	real MorphemeExtractor::get_predecessor_entropy(const QString& _string, const QString& _ch)
	{
		if ((_ch + _string).size() == 0
			|| get_distinct_predecessor_count(_string) == 0)
		{
			return alphabet_ent;
		}
		else if (get_distinct_predecessor_count(_string) == 1)
		{
			return 0.0;
		}
		else if (!p_ent_cache.contains(_ch + _string))
		{
			QHash<QChar, uint> predecessors_tmp(sa->get_predecessors(_string));

			if (_ch.size() == 0)
			{
				real prob(0.0);
				real ent(0.0);
				uint total(0.0);
				for (const QChar& ch : predecessors_tmp.keys())
				{
					total += predecessors_tmp[ch];
				}
				for (const QChar& ch : predecessors_tmp.keys())
				{
					prob = predecessors_tmp[ch] / static_cast<real>(total);
					ent -= prob * std::log(prob);
				}
				p_ent_cache[_ch + _string] = ent;
			}
			else
			{
				real prob(predecessors_tmp[_ch.at(0)] / static_cast<real>(get_total_predecessor_count(_string)));
				p_ent_cache[_ch + _string] = -prob * std::log(prob);
			}
		}
		return p_ent_cache[_ch + _string];
	}

	real MorphemeExtractor::get_successor_entropy(const QString& _string, const QString& _ch)
	{
		if ((_string + _ch).size() == 0
			|| get_distinct_successor_count(_string) == 0)
		{
			return alphabet_ent;
		}
		else if (get_distinct_successor_count(_string) == 1)
		{
			return 0.0;
		}
		else if (!s_ent_cache.contains(_string + _ch))
		{
			QHash<QChar, uint> successors_tmp(sa->get_successors(_string));

			if (_ch.size() == 0)
			{
				real prob(0.0);
				real ent(0.0);
				uint total(0.0);
				for (const QChar& ch : successors_tmp.keys())
				{
					total += successors_tmp[ch];
				}
				for (const QChar& ch : successors_tmp.keys())
				{
					prob = successors_tmp[ch] / static_cast<real>(total);
					ent -= prob * std::log(prob);
				}
				s_ent_cache[_string + _ch] = ent;
			}
			else
			{
				real prob(successors_tmp[_ch.at(0)] / static_cast<real>(get_total_successor_count(_string)));
				s_ent_cache[_string + _ch] = -prob * std::log(prob);
			}
		}
		return s_ent_cache[_string + _ch];
	}

	QString MorphemeExtractor::extract_morphemes_ps(const QString&& _line,
													const bool _reseg)
	{

		if (Config::console_output)
		{
			std::cout << "line: " << _line.toUtf8().constData() << std::endl;
		}

		/// A list holding the identified morphemes
		/// in the order in which they appear in the current line
		QStringList tmp_dictionary;

		/// A hapax which holds one or more morphemes
		QString candidate;

		/// The combined length of all morphemes in the line
		uint line_size(_line.size());

		/// A list of individual morphemes in case of resegmentation
		QStringList line_seg;

		/// The next morpheme in the list
		uint next_morph(0);

		if (_reseg)
		{
			line_size -= _line.count(" ");
			line_seg = _line.split(" ");
		}

		/// The morpheme identified in the procedure below
		QString morpheme;

		/// Index along the current line
		uint line_index(0);

		/// The morpheme identified at the preceding step
		QString prev("");

		/// Characters preceding and following the left and right candidates
		QString l_prev;
		QString l_next;
		QString r_prev;
		QString r_next;

		QString l_prev_1;
		QString l_next_1;
		QString r_prev_1;
		QString r_next_1;

		/////////////////////////////
		/// The initial candidate ///
		/////////////////////////////

		QString left("");
		QString right("");

		uint lp_count(0);
		uint ls_count(0);
		uint rp_count(0);
		uint rs_count(0);
		uint total_count(0);

		real lp_ent(0.0);
		real ls_ent(0.0);
		real rp_ent(0.0);
		real rs_ent(0.0);
		real total_ent(0.0);

		///////////////////////////////////////////
		/// Boundary one character to the right ///
		///////////////////////////////////////////

		QString left_1("");
		QString right_1("");

		uint lp_count_1(0);
		uint ls_count_1(0);
		uint rp_count_1(0);
		uint rs_count_1(0);
		uint total_count_1(0);

		real lp_ent_1(0.0);
		real ls_ent_1(0.0);
		real rp_ent_1(0.0);
		real rs_ent_1(0.0);
		real total_ent_1(0.0);

		/// The size of the left candidate
		uint left_size(0);

		while ((!_reseg
				&& line_index < line_size)
			   || (_reseg
				   && next_morph < line_seg.size()))
		{
			if (!_reseg)
			{
				/// Collect characters until the string becomes a hapax legomenon
				right = candidate.right(candidate.size() - left_size);
				do
				{
					candidate.push_back(_line.at(line_index));
					right.push_back(_line.at(line_index));
					++line_index;
				} while (get_occurrences(right) > 1 &&
						 line_index < _line.size());

				/// Chop characters from the back until we have
				/// two or more distinct predecessors
				while (right.size() > 1
					   && (get_distinct_predecessor_count(right) <= 1
						   || get_distinct_successor_count(right) <= 1))
				{
					candidate.chop(1);
					right.chop(1);
					--line_index;
				}

				if (Config::console_output)
				{
					std::cout << "candidate: " << candidate.toUtf8().constData() << std::endl;
					pause();
				}
			}
			else
			{
				candidate.append(line_seg.at(next_morph++));
				if (next_morph == 1
					&& line_seg.size() > 1)
				{
					candidate.append(line_seg.at(next_morph++));
				}

				if (Config::console_output)
				{
					std::cout << "reseg candidate: " << candidate.toUtf8().constData() << std::endl;
					pause();
				}
			}

			/// The size of the left candidate
			left_size = 0;

			/// total_count_1 > total_count
			bool up(false);

			/// total_count_1 < total_count
			bool down(false);

			/// total_count_1 = total_count
			bool plateau(false);

			/// total_count has reached a peak
			bool peak(false);

			/// total_count has reached a valley
			bool valley(false);

			/// Extract morphemes from the candidate
			while (left_size <= candidate.size())
			{
				l_prev = "";
				l_next = "";
				r_prev = "";
				r_next = "";
				l_prev_1 = "";
				l_next_1 = "";
				r_prev_1 = "";
				r_next_1 = "";

				left = candidate.left(left_size);
				right = candidate.right(candidate.size() - left_size);

				left_1 = left + right.left(1);
				right_1 = candidate.right(candidate.size() - left_size - 1);

				lp_count = get_distinct_predecessor_count(prev + left) + get_distinct_predecessor_count(left);
				ls_count = get_distinct_successor_count(prev + left) + get_distinct_successor_count(left);

				if (Config::seg_method_ps_entropy)
				{
					if (line_index - candidate.size() - prev.size() > 0)
					{
						l_prev = _line.mid(line_index - candidate.size() - prev.size() - 1, 1);
						l_prev_1 = l_prev;
					}
					if (right.size() > 0)
					{
						l_next = right.left(1);
					}

					lp_ent = get_norm_predecessor_entropy(prev + left, l_prev);
					ls_ent = get_norm_successor_entropy(prev + left, l_next);

//					lp_ent = get_norm_predecessor_entropy(prev + left);
//					ls_ent = get_norm_successor_entropy(prev + left);
				}

				rp_count = get_distinct_predecessor_count(left + right) + get_distinct_predecessor_count(right);
				rs_count = get_distinct_successor_count(left + right) + get_distinct_successor_count(right);

				if (Config::seg_method_ps_entropy)
				{
					if (left.size() > 0)
					{
						r_prev = left.right(1);
					}
					if (line_index < _line.size() - 1)
					{
						r_next = _line.mid(line_index, 1);
						r_next_1 = r_next;
					}
					rp_ent = get_norm_predecessor_entropy(right, r_prev);
					rs_ent = get_norm_successor_entropy(right, r_next);

//					rp_ent = get_norm_predecessor_entropy(right);
//					rs_ent = get_norm_successor_entropy(right);
				}

				lp_count_1 = get_distinct_predecessor_count(prev + left_1) + get_distinct_predecessor_count(left_1);
				ls_count_1 = get_distinct_successor_count(prev + left_1) + get_distinct_successor_count(left_1);

				if (Config::seg_method_ps_entropy)
				{
					if (right_1.size() > 0)
					{
						l_next_1 = right_1.left(1);
					}

					lp_ent_1 = get_norm_predecessor_entropy(prev + left_1, l_prev_1);
					ls_ent_1 = get_norm_successor_entropy(prev + left_1, l_next_1);

//					lp_ent_1 = get_norm_predecessor_entropy(prev + left_1);
//					ls_ent_1 = get_norm_successor_entropy(prev + left_1);
				}

				rp_count_1 = get_distinct_predecessor_count(left_1 + right_1) + get_distinct_predecessor_count(right_1);
				rs_count_1 = get_distinct_successor_count(left_1 + right_1) + get_distinct_successor_count(right_1);

				if (Config::seg_method_ps_entropy)
				{
					if (left_1.size() > 0)
					{
						r_prev_1 = left_1.right(1);
					}
					rp_ent_1 = get_norm_predecessor_entropy(right_1, r_prev_1);
					rs_ent_1 = get_norm_successor_entropy(right_1, r_next_1);

//					rp_ent_1 = get_norm_predecessor_entropy(right_1);
//					rs_ent_1 = get_norm_successor_entropy(right_1);
				}

				total_count = ls_count * rp_count/* + lp_count * rs_count*/;
				total_count_1 = ls_count_1 * rp_count_1/* + lp_count_1 * rs_count_1*/;

				if (Config::seg_method_ps_entropy)
				{
					total_ent = lp_ent + ls_ent + rp_ent + rs_ent;
					total_ent_1 = lp_ent_1 + ls_ent_1 + rp_ent_1 + rs_ent_1;
				}

				if (Config::console_output)
				{
					std::cout << left_size
							  << "\t" << (prev + "+" + left).toUtf8().constData()
							  << "|" << right.toUtf8().constData();
					if (Config::seg_method_ps_count)
					{
						std::cout << "\t" << lp_count
								  << "\t" << ls_count
								  << "\t" << rp_count
								  << "\t" << rs_count
								  << "\t(" << total_count << ")";
					}

					if (Config::seg_method_ps_entropy)
					{
						std::cout << "\t" << lp_ent
								  << "\t" << ls_ent
								  << "\t" << rp_ent
								  << "\t" << rs_ent
								  << "\t(" << total_ent << ")";
					}
					std::cout << std::endl;

					std::cout << "\t" << (prev + "+" + left_1).toUtf8().constData()
							  << "|" << right_1.toUtf8().constData();

					if (Config::seg_method_ps_count)
					{
						std::cout << "\t" << lp_count_1
								  << "\t" << ls_count_1
								  << "\t" << rp_count_1
								  << "\t" << rs_count_1
								  << "\t(" << total_count_1 << ")";
					}

					if (Config::seg_method_ps_entropy)
					{
						std::cout << "\t" << lp_ent_1
								  << "\t" << ls_ent_1
								  << "\t" << rp_ent_1
								  << "\t" << rs_ent_1
								  << "\t(" << total_ent_1 << ")";
					}

					std::cout << std::endl;

					if (Config::seg_method_ps_entropy)
					{
						std::cout << "l_prev: " << l_prev.toUtf8().constData()
								  << "\tl_next: " << l_next.toUtf8().constData()
								  << "\tr_prev: " << r_prev.toUtf8().constData()
								  << "\tr_next: " << r_next.toUtf8().constData()
								  << "l_prev_1: " << l_prev_1.toUtf8().constData()
								  << "\tl_next_1: " << l_next_1.toUtf8().constData()
								  << "\tr_prev_1: " << r_prev_1.toUtf8().constData()
								  << "\tr_next_1: " << r_next_1.toUtf8().constData()
								  << std::endl

								  << "\t" << (prev + "+" + left_1).toUtf8().constData()
								  << "|" << right_1.toUtf8().constData()

								  << "\t" << lp_count_1
								  << "\t" << ls_count_1
								  << "\t" << rp_count_1
								  << "\t" << rs_count_1
								  << "\t(" << total_count_1 << ")"

								  << std::endl;
					}
				}

				++left_size;

				//////////////////////
				/// Detect valleys ///
				//////////////////////

				/// Detect a second peak
				if (Config::seg_method_ps_count)
				{
					if (total_count_1 < total_count)
					{
						down = true;
						if (up)
						{
							peak = true;
						}
					}
					else if (total_count == total_count_1)
					{
						/// Plateau
						if (prev.size() > 0)
						{
							left_size = 0;
						}
						prev.clear();
						continue;
					}
					else
					{
						up = true;
						if (down)
						{
							valley = true;
							down = false;
						}
					}
				}

				else if (Config::seg_method_ps_entropy)
				{
					if (total_ent_1 < total_ent)
					{
						down = true;
						if (up)
						{
							peak = true;
						}
					}
					else if (total_ent == total_ent_1)
					{
						/// Plateau
						if (prev.size() > 0)
						{
							left_size = 0;
						}
						prev.clear();
						continue;
					}
					else
					{
						up = true;
						if (down)
						{
							valley = true;
							down = false;
						}
					}
				}

				if (Config::console_output)
				{
					std::cout << "\tu: " << up
							  << "\td: " << down
							  << "\tv: " << valley
							  << "\tp: " << peak
							  << std::endl;
				}

				/// We have identified a peak!
				/// There must be a morpheme boundary here.
				if (peak)
				{

					/// The shorter candidate is a morpheme.
					if (dictionary.contains(left))
					{
						morpheme = left;
					}
					else if (dictionary.contains(left_1))
					{
						morpheme = left_1;
					}
					else
					{
						if (peak)
						{
							morpheme = left;
						}
						else
						{
							morpheme = left_1;
						}
					}

					/// The morpheme becomes the previous morpheme
					/// for the next candidate.

					prev = morpheme;

					/// Chop the current candidate at the end of the identified morpheme
					candidate = candidate.right(candidate.size() - morpheme.size());

					/// Append the identified morpheme to the temporary list
					if (morpheme.trimmed().size() > 0)
					{
						tmp_dictionary.append(morpheme.trimmed());
						if (Config::console_output)
						{
							std::cout << "morpheme: '" << morpheme.trimmed().toUtf8().constData() << "'" << std::endl;
						}
					}

					/// Clean up the variables
					left_size = 0;
					up = false;
					down = false;
					plateau = false;
					peak = false;
					valley = false;

					if (candidate.size() == 0
						|| get_occurrences(candidate) > 1)
					{
						break;
					}
				}

				if (Config::console_output)
				{
					pause();
				}

				if (right_1.size() == 0)
				{
					break;
				}
			}
		}
		if (candidate.trimmed().size() > 0)
		{
			/// This makes sure that the candidate at the
			/// end of the line is not omitted from the list
			tmp_dictionary.append(candidate.trimmed());
			if (Config::console_output)
			{
				std::cout << "morpheme: '" << candidate.trimmed().toUtf8().constData() << "'" << std::endl;
			}
		}

		/// Add the morphemes from the temporary list to the dictionary
		for (const QString& m : tmp_dictionary)
		{
			++dictionary[m];
			++total_morpheme_count;
		}

		return tmp_dictionary.join(" ");
	}

	QString MorphemeExtractor::extract_morphemes_character_frequencies(const QString&& _line)
	{
		return QString();
	}

	real MorphemeExtractor::get_dict_cost_split(const QString& _word,
												const QString& _left,
												const QString& _right)
	{
		real dict_cost_new(dict_cost - morpheme_cost[_word]);

		if (!dictionary.contains(_left))
		{
			for (const QChar& ch : _left)
			{
				dict_cost_new += char_code_length[ch];
			}
		}

		if (!dictionary.contains(_right))
		{
			for (const QChar& ch : _right)
			{
				dict_cost_new += char_code_length[ch];
			}
		}

		return dict_cost_new;
	}

	real MorphemeExtractor::get_dict_entropy_split(const QString& _word,
												   const QString& _left,
												   const QString& _right)
	{
		real q((total_morpheme_count + dictionary[_word]) / static_cast<real>(total_morpheme_count));

		/// The occurrences of the left and right candidates
		uint l_occ(0);
		uint r_occ(0);
		if (dictionary.contains(_left))
		{
			l_occ = dictionary[_left];
		}
		else
		{
			l_occ = sa->get_occurrences(_left);
		}

		if (dictionary.contains(_right))
		{
			r_occ = dictionary[_right];
		}
		else
		{
			r_occ = sa->get_occurrences(_right);
		}

		std::cout << "left: " << _left.toUtf8().constData() << ", right: " << _right.toUtf8().constData() << std::endl;

		/// The occurrence probability of the word being split
		real word_prob(dictionary[_word] / static_cast<real>(total_morpheme_count));
		std::cout << "\tword_prob: " << word_prob << std::endl;

		/// The first three elements
		real old_ent_mod(dict_entropy + std::log1p(q) + word_prob * std::log(word_prob));
		std::cout << "\told_ent_mod: " << old_ent_mod << std::endl;

		real log1(l_occ * std::log1p((l_occ + dictionary[_word]) / static_cast<real>(l_occ)));
		std::cout << "\tlog1: " << log1 << std::endl;

		real log2(dictionary[_word] * std::log((l_occ + dictionary[_word]) / static_cast<real>(total_morpheme_count + dictionary[_word])));
		std::cout << "\tlog2: " << log2 << std::endl;

		real log3(r_occ * std::log1p((r_occ + dictionary[_word]) / static_cast<real>(r_occ)));
		std::cout << "\tlog3: " << log3 << std::endl;

		real log4(dictionary[_word] * std::log((r_occ + dictionary[_word]) / static_cast<real>(total_morpheme_count + dictionary[_word])));
		std::cout << "\tlog4: " << log4 << std::endl;

		real sum((log1 + log2 + log3 + log4) / static_cast<real>(total_morpheme_count));
		std::cout << "\tsum: " << sum << std::endl;
		pause();

		return ((old_ent_mod - sum) / q);
	}

	void MorphemeExtractor::init_entropy()
	{
		/// Compute the initial description length.

		dict_cost = 0.0;
		dict_entropy = 0.0;

		/// Used to compute the dictionary entropy
		real morpheme_prob(0.0);

		for (const QString& m : dictionary.keys())
		{
			morpheme_cost[m] = 0.0;
			for (const QChar& ch : m)
			{
				morpheme_cost[m] += char_code_length[ch];
			}
			dict_cost += morpheme_cost[m];

			/// Corpus entropy
			morpheme_prob = dictionary[m] / static_cast<real>(total_morpheme_count);
			dict_entropy += -morpheme_prob * std::log(morpheme_prob);
		}
	}

	QString MorphemeExtractor::resegment_morphemes(const QString& _line)
	{

		QStringList morphemes(_line.split(" "));
		QString new_line("");
		QStringList candidate;
		QString joined;

		real old_entropy(0.0);
		real new_entropy(0.0);
		real old_cost(0.0);
		real new_cost(0.0);
		real p;

		do
		{
			for (const QString& str : morphemes)
			{
				p = dictionary[str] / total_morpheme_count;
				old_entropy -= p * std::log(p);
			}

			uint i(0);

			do
			{
				if (candidate.empty())
				{
					candidate.append(morphemes.at(i));
				}

				joined = candidate.join("");

				std::cout << "Considering " << joined.toUtf8().constData() << std::endl;

				if (i < morphemes.size() - 1)
				{
					if (dictionary.contains(joined + morphemes.at(i + 1))
						&& dictionary[joined + morphemes.at(i + 1)] > dictionary[joined])
					{
						candidate.append(morphemes.at(i + 1));
					}
					else
					{
						new_line.append(candidate.join("") + " ");

						++dictionary[candidate.join("")];
						for (const QString& m : candidate)
						{
							--dictionary[m];
							if (dictionary[m] == 0)
							{
								dictionary.remove(m);
							}
						}
						candidate.clear();
					}
				}

				++i;

			} while (i < morphemes.size());

			new_line.append(candidate.join(""));
			morphemes.clear();
			morphemes = new_line.split(" ");

			extract_morphemes_ps(std::move(new_line), true);

			for (const QString& str : morphemes)
			{
				p = dictionary[str] / total_morpheme_count;
				new_entropy -= p * std::log(p);
			}

			if (Config::console_output)
			{
				std::cout << "Old entropy: " << old_entropy << ", new entropy: " << new_entropy << std::endl;
				pause();
			}

		} while (new_entropy > old_entropy);


		/////////////////////////////////////////////

		//		QMultiMap<uint, QString> morphemes_local;

		//		for (const QString& str	: dictionary.keys())
		//		{
		//			morphemes_local.insert(dictionary[str], str);
		//		}

		//		/// Holds the current best segmentation.
		//		/// The first element is the size of the left segment,
		//		/// and the second holds a pair of the dictionary cost
		//		/// and the dictionary entropy.
		//		std::pair<int, std::pair<real, real>> best_segmentation(0, std::pair<real, real>(dict_cost, dict_entropy));

		//		QVector<QString> matches_left;
		//		QVector<QString> matches_right;

		//		QMultiMap<uint, QString>::const_iterator mit(morphemes_local.cend());

		//		while (mit != morphemes_local.cbegin())
		//		{
		//			const QString& pattern(mit.value());

		//			QString left;
		//			QString right;

		//			for (const QString& target : dictionary.keys())
		//			{
		//				if (dictionary[target] != 0
		//					&& target.size() > pattern.size())
		//				{
		//					if (target.left(pattern.size()) == pattern)
		//					{
		//						matches_left.push_back(target);
		//					}
		//					else if (target.right(pattern.size()) == pattern)
		//					{
		//						matches_right.push_back(target);
		//					}
		//				}
		//			}

		//			if (best_segmentation.first != 0)
		//			{
		//				/// Insert the new entries into the dictionary
		//				left = pattern.left(best_segmentation.first);
		//				right = pattern.right(pattern.size() - left.size());

		//				if (Config::console_output)
		//				{
		//					std::cout << "\t\tSegmenting '" << pattern.toUtf8().constData()
		//							  << "'' into '" << left.toUtf8().constData()
		//							  << "' and '" << right.toUtf8().constData() << "'" << std::endl;

		//					pause();
		//				}

		//				dictionary[left] += dictionary[pattern];
		//				dictionary[right] += dictionary[pattern];

		//				/// Update the morpheme count
		//				total_morpheme_count += dictionary[pattern];

		//				/// Update the dictionary cost, corpus entropy and effort
		//				dict_cost = best_segmentation.second.first;
		//				dict_entropy = best_segmentation.second.second;

		//				/// Remove the current morpheme from the dictionary
		//				dictionary.remove(mit.value());

		//				/// Reset the best segmentation
		//				best_segmentation.first = 0;
		//			}
		//			--mit;
		//		}

		//		if (Config::console_output)
		//		{
		//			std::cout << std::endl << "Done!" << std::endl;
		//		}
		emit morphemes_resegmented();

		return new_line;

	}
}
