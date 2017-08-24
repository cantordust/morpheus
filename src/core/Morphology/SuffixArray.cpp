#include "SuffixArray.hpp"

namespace Morpheus
{

	std::vector<uchar> SuffixArray::input_string;
	SuffixArray::cmp SuffixArray::char_cmp;

	void SuffixArray::set_filenames(const QString& _file_name)
	{
		input_file.setFileName(_file_name);
		QFileInfo fi(input_file);
		QDir dir(fi.absoluteDir());
		QString base(fi.baseName());
		QString ext(fi.completeSuffix());
		sa_file.setFileName(dir.absolutePath() + "/" + base + ".sa");
		load_corpus();
	}

	void SuffixArray::sort_suffixes(const std::vector<uint>&& _positions, const uint _depth)
	{
		if (_positions.size() > 1)
		{
			std::map<uchar, std::vector<uint>> new_map;
			for (const uint pos : _positions)
			{
				new_map[input_string[pos + _depth]].push_back(pos);
			}
			for (const std::pair<uchar, std::vector<uint>>& new_ch : new_map)
			{
				sort_suffixes(std::move(new_ch.second), _depth + 1);
			}
		}
		else
		{
			sorted_suffixes[input_string[_positions[0]]].push_back(_positions[0]);
		}
	}

	void SuffixArray::load_corpus()
	{
		input_string.clear();
		SA.clear();
		suffixes_to_sort.clear();
		char_counts.clear();

		std::ifstream input_stream;
		input_stream.open(input_file.fileName().toUtf8().constData());
		if (input_stream.is_open())
		{
			char ch;
			while (input_stream.get(ch))
			{
				input_string.push_back(static_cast<uchar>(ch));
			}
			input_stream.close();
		}
		input_string.push_back('\0');

		if (input_string.size() > 0)
		{
			make_index(input_string, char_index);
			put_chars_in_buckets(input_string, char_counts, true);
			sort(input_string.size() - 1, true);
			compile_suffix_array(SA, char_counts, true);
		}
	}

	std::vector<uchar> SuffixArray::str_to_vec(const QString& _str)
	{
		std::vector<uchar> input;
		for (const char ch : _str.toStdString())
		{
			input.push_back(static_cast<uchar>(ch));
		}
		input.push_back('\0');
		return input;
	}

	void SuffixArray::make_index(const std::vector<uchar>& _input, std::vector<uint>& _index)
	{
		_index.resize(_input.size());
		uint idx(0);
		for (const uchar uch : _input)
		{
			if (uch <= 0x7F ||
				uch >= 0xC0)
			{
				_index.push_back(idx++);
			}
			else
			{
				_index.push_back(idx);
			}
		}
	}

	std::map<uchar, std::vector<uint> > SuffixArray::create_SA(const std::vector<uchar>& _input)
	{
		std::map<uchar, std::vector<uint>> suffix_array;
		hashmap<uchar,uint> char_counts_tmp;
		put_chars_in_buckets(_input, char_counts_tmp);
		sort(_input.size() - 1);
		compile_suffix_array(suffix_array, char_counts_tmp);
		return suffix_array;
	}

	void SuffixArray::save_SA()
	{
		std::ofstream sa_ofstream(sa_file.fileName().toUtf8().constData(), std::ios::binary | std::ios::trunc);
		sa_ofstream.write(reinterpret_cast<const char*>(&SA[0]), SA.size()*sizeof(uint));
		sa_ofstream.flush();
		sa_ofstream.close();
	}

	void SuffixArray::put_chars_in_buckets(const std::vector<uchar>& _input,
										   hashmap<uchar,uint>& _char_counts,
										   const bool _progress)
	{
		/// Progress bar
		QProgressDialog pd;

		if (_progress)
		{
			pd.setLabelText("Identifying and bucketing suffixes...");
			pd.setMinimum(0);
			pd.setMaximum(0);
			pd.setValue(0);
			pd.setAutoClose(true);
			pd.setWindowModality(Qt::WindowModal);
			pd.setCancelButton(0);
			pd.open();
			progress = 0;
			QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
		}

		//////////////////////////////////////////////////
		/// Count the number of S-type and L-type suffixes
		//////////////////////////////////////////////////

		/// A string that holds identical characters until
		/// we can decide whether they are of type S or L
		std::string s_or_l;
		uint char_count(0);
		if (_input.size() > 0)
		{
			_char_counts['\0'] = 1;
			s_count = 1; /// \0 is lexicographically smaller than any other character
			l_count	= 0;
			while (char_count <= _input.size() - 2)
			{
				if (_input[char_count] < _input[char_count + 1])
				{
					/// This is an S-type suffix
					++s_count;
					if (s_or_l.size() > 0)
					{
						s_count += s_or_l.size();
						s_or_l.clear();
					}
				}
				else if (_input[char_count] > _input[char_count + 1])
				{
					/// This is an L-type suffix
					++l_count;
					if (s_or_l.size() > 0)
					{
						l_count += s_or_l.size();
						s_or_l.clear();
					}
				}
				else
				{
					/// Store for later
					s_or_l.push_back(_input[char_count]);
				}
				++_char_counts[input_string[char_count]];
				++char_count;
			}
		}

		////////////////////////////////////////
		/// Extract all characters and put them
		/// into their respective buckets.
		////////////////////////////////////////
		suffixes_to_sort.clear();
		suffixes_to_arrange.clear();
		if (_input.size() > 0)
		{
			uchar ch;
			s_or_l.clear();
			char_count = 0;
			while (char_count <= _input.size() - 2)
			{
				ch = _input[char_count];

				if ((ch < _input[char_count + 1] && s_count <= l_count) ||
					(ch > _input[char_count + 1] && s_count > l_count))

				{
					suffixes_to_sort[ch].push_back(char_count);
					if (s_or_l.size() > 0)
					{
						uint i = s_or_l.size();
						for (const uchar sl_char : s_or_l)
						{
							suffixes_to_sort[sl_char].push_back(char_count - i);
							--i;
						}
						s_or_l.clear();
					}
				}
				else if ((ch < _input[char_count + 1] && s_count > l_count) ||
						 (ch > _input[char_count + 1] && s_count <= l_count))
				{
					suffixes_to_arrange.insert(char_count);
					if (s_or_l.size() > 0)
					{
						for (uint i = 1; i <= s_or_l.size(); ++i)
						{
							suffixes_to_arrange.insert(char_count - i);
						}
						s_or_l.clear();
					}
				}
				else if (ch == _input[char_count + 1])
				{
					s_or_l.push_back(ch);
				}
				++char_count;
			}
		}
	}

	void SuffixArray::sort(const uint _last_pos, const bool _progress)
	{
		///////////////////////////////////////////
		/// Sort the smaller collection of suffixes
		///////////////////////////////////////////

		/// Progress bar
		QProgressDialog pd;

		if (_progress)
		{
			pd.setLabelText("Sorting suffixes...");
			pd.setMinimum(0);
			pd.setMaximum(suffixes_to_sort.size());
			pd.setValue(0);
			pd.setAutoClose(true);
			pd.setWindowModality(Qt::WindowModal);
			pd.setCancelButton(0);
			pd.open();
			progress = 0;
			QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
		}

		sorted_suffixes.clear();
		for (const std::pair<uchar, std::vector<uint>>& ch : suffixes_to_sort)
		{
			sort_suffixes(std::move(ch.second), 1);
			if (_progress)
			{
				pd.setValue(++progress);
				QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
			}
		}
		suffixes_to_sort.clear();
		sorted_suffixes['\0'].push_back(_last_pos);
	}

	void SuffixArray::compile_suffix_array(std::map<uchar, std::vector<uint>>& _SA,
										   const hashmap<uchar, uint>& _char_counts,
										   const bool _progress)
	{
		/// Progress bar
		QProgressDialog pd;

		if (_progress)
		{
			pd.setLabelText("Populating sorted suffixes...");
			pd.setMinimum(0);
			pd.setMaximum(sorted_suffixes.size());
			pd.setValue(0);
			pd.setAutoClose(true);
			pd.setWindowModality(Qt::WindowModal);
			pd.setCancelButton(0);
			pd.open();
			progress = 0;
			QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
		}

		////////////////////////////
		/// Compile the suffix array
		////////////////////////////

		/// Reserve the right amount of space for each bucket
		/// to avoid the overhead of dynamic resizing of the vectors
		_SA.clear();
		for (const std::pair<uchar, uint>& ch_count : _char_counts)
		{
			_SA[ch_count.first].resize(ch_count.second, 0);
			bucket_pos[ch_count.first].first = 0;
			bucket_pos[ch_count.first].second = ch_count.second - 1;
		}

		/// Insert the sorted suffixes in their correct positions
		if (s_count <= l_count)
		{
			/// Scan the sorted suffixes from right to left.
			/// S-type suffixes go at the end of their bucket.
			for (const std::pair<uchar, std::vector<uint>>& ch : sorted_suffixes)
			{
				std::vector<uint>::const_reverse_iterator pos = ch.second.rbegin();
				while (pos != ch.second.rend())
				{
					_SA[ch.first][bucket_pos[ch.first].second] = *pos;
					--bucket_pos[ch.first].second;
					++pos;
				}
				if (_progress)
				{
					pd.setValue(++progress);
					QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
				}
			}
		}
		else
		{
			/// Scan the sorted suffixes from left to right.
			/// L-type suffixes go at the front of their bucket.
			for (const std::pair<uchar, std::vector<uint>>& ch : sorted_suffixes)
			{
				std::vector<uint>::const_iterator pos = ch.second.begin();
				while (pos != ch.second.end())
				{
					_SA[ch.first][bucket_pos[ch.first].first] = *pos;
					++bucket_pos[ch.first].first;
					++pos;
				}
				if (_progress)
				{
					pd.setValue(++progress);
					QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
				}
			}
		}

		/// Free the memory occupied by the sorted suffixes
		sorted_suffixes.clear();

		if (_progress)
		{
			pd.setLabelText("Filling gaps...");
			pd.setMaximum(SA.size());
			pd.setValue(progress = 0);
			QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
		}
		/// Scan all the suffixes and populate the empty slots in each bucket
		if (s_count <= l_count)
		{
			/// Scan from left to right
			std::map<uchar, std::vector<uint>>::const_iterator map_it = _SA.begin();
			while (map_it != _SA.end())
			{
				std::vector<uint>::const_iterator vec_it = map_it->second.begin();
				while (vec_it != map_it->second.end())
				{
					if (*vec_it > 0 &&
						suffixes_to_arrange.count(*vec_it - 1) > 0)
					{
						_SA[input_string[*vec_it - 1]][bucket_pos[input_string[*vec_it - 1]].first] = *vec_it - 1;
						++bucket_pos[input_string[*vec_it - 1]].first;
					}
					++vec_it;
				}
				++map_it;
				if (_progress)
				{
					pd.setValue(++progress);
					QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
				}
			}
		}
		else
		{
			/// Scan from right to left
			std::map<uchar, std::vector<uint>>::const_reverse_iterator map_rit = _SA.rbegin();
			while (map_rit != _SA.rend())
			{
				std::vector<uint>::const_reverse_iterator vec_rit = map_rit->second.rbegin();
				while (vec_rit != map_rit->second.rend())
				{
					if (*vec_rit > 0 &&
						suffixes_to_arrange.count(*vec_rit - 1) > 0)
					{
						_SA[input_string[*vec_rit - 1]][bucket_pos[input_string[*vec_rit - 1]].second] = *vec_rit - 1;
						--bucket_pos[input_string[*vec_rit - 1]].second;
					}
					++vec_rit;
				}
				++map_rit;
				if (_progress)
				{
					pd.setValue(++progress);
					QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
				}
			}
		}
	}

	SuffixArray::vrange SuffixArray::get_equal_range(std::map<uchar, std::vector<uint> >& _SA, const std::vector<uchar>& _key)
	{
		vrange range;

		if (_key.size() > 0)
		{
			/// Binary search
			if (_SA.find(_key[0]) != _SA.end())
			{
				range.first = _SA[_key[0]].cbegin();
				range.second = _SA[_key[0]].cend();

				for (uint i = 1; i < _key.size(); ++i)
				{
					char_cmp.depth = i;
					range = std::equal_range(range.first, range.second, _key[i], SuffixArray::char_cmp);
					if (std::distance(range.first,range.second) <= 0)
					{
						break;
					}
				}
			}
		}
		return range;
	}

	SuffixArray::vrange SuffixArray::get_equal_range(std::map<uchar, std::vector<uint> >& _SA, const QString& _qstr)
	{
		std::string key_str(_qstr.toStdString());
		std::vector<uchar> key;
		key.reserve(key_str.size());

		if (key_str.size() > 0)
		{
			for (const char ch : key_str)
			{
				key.push_back(static_cast<uchar>(ch));
			}
			return get_equal_range(_SA,key);
		}
		return vrange();
	}

	QHash<QChar, uint> SuffixArray::get_predecessors(const QString& _key)
	{
		predecessors.clear();
		std::string utf8_str;
		vrange range(get_equal_range(SA, std::move(_key)));
		while (range.first != range.second)
		{
			uint offset(*range.first);
			if (offset > 0)
			{
				utf8_str.clear();
				utf8_str.push_back(static_cast<char>(input_string[--offset]));
				if (input_string[offset] > 0x7F)
				{
					do
					{
						utf8_str.push_back(static_cast<char>(input_string[--offset]));
					} while (input_string[offset] < 0xC0 &&
							 offset > 0);
					std::reverse(utf8_str.begin(),utf8_str.end());
				}
				++predecessors[QString::fromStdString(utf8_str).at(0)];
			}
			++range.first;
		}
		return predecessors;
	}

	QHash<QChar, uint> SuffixArray::get_successors(const QString& _key)
	{
		successors.clear();
		uint key_length(_key.toStdString().size());
		std::string utf8_str;
		vrange range(get_equal_range(SA, std::move(_key)));
		while (range.first != range.second)
		{
			uint offset(*range.first + key_length);
			if (input_string.size() - 1 > offset)
			{
				utf8_str.clear();
				utf8_str.push_back(static_cast<char>(input_string[offset]));
				if (input_string[offset] > 0x7F)
				{
					do
					{
						utf8_str.push_back(static_cast<char>(input_string[++offset]));
					} while (input_string[offset] < 0xC0 &&
							 input_string[offset] > 0x7F &&
							 offset < input_string.size() - 1);
				}
				++successors[QString::fromStdString(utf8_str).at(0)];
			}
			++range.first;
		}
		return successors;
	}

	hashset<uchar> SuffixArray::get_successors(const std::vector<uchar>& _key)
	{
		hashset<uchar> successors;
		vrange range(get_equal_range(SA, std::move(_key)));
		while (range.first != range.second)
		{
			uint offset(*range.first + _key.size());
			if (offset < input_string.size() - 1)
			{
				successors.insert(input_string.at(offset));
			}
			++range.first;
		}
		return successors;
	}

	hashset<uchar> SuffixArray::get_successors(SuffixArray::vrange& _range,
											   const uint _depth,
											   const std::vector<uchar>& _input)
	{
		hashset<uchar> successors;
		while (_range.first != _range.second)
		{
			if (*_range.first + _depth < _input.size() - 1)
			{
				successors.insert(_input.at(*_range.first + _depth));
			}
			++_range.first;
		}
		return successors;
	}
}
