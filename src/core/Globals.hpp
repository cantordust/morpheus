#ifndef GLOBALS_HPP
#define GLOBALS_HPP

/// STL
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <memory>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>
#include <utility>
#include <cmath>

/// Qt
#include <QApplication>
#include <QObject>
#include <QSettings>
#include <QCloseEvent>

#include <QSet>
#include <QHash>
#include <QMap>
#include <QMultiMap>
#include <QPair>

#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QTextStream>

#include <QString>

#include <QLabel>
#include <QProgressDialog>
#include <QMessageBox>
#include <QDialog>
#include <QListWidgetItem>

#include <QSortFilterProxyModel>

/// Dependencies
#include "pugixml.hpp"

namespace Ui
{
	class MainWindow;
	class Config;
}

namespace Morpheus
{
	template <typename T>
	using uptr = std::unique_ptr<T>;

	template <typename T>
	using sptr = std::shared_ptr<T>;

	template <typename T>
	using heap = std::priority_queue<T>;

	/// Benchmarking
	using std::chrono::duration_cast;
	using std::chrono::nanoseconds;

	template<typename T1, typename T2>
	using hashmap = std::unordered_map<T1,T2>;

	template<typename T>
	using hashset = std::unordered_set<T>;

	typedef double real;
	typedef unsigned char uchar;
	typedef unsigned int uint;
	typedef unsigned long int ulong;
	typedef unsigned long long int ullong;

	inline void pause()
	{
		static char ch('a');
		if (ch != 'c')
		{
			std::cout << " ('x' to exit, 'c' to disable pausing)" << std::flush;
			std::cin.get(ch);
			if (ch == 'x')
			{
				exit(1);
			}
		}
	}

}

#endif // GLOBALS_HPP
