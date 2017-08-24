#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "Globals.hpp"
#include "Config.hpp"
#include "MorphemeExtractor.hpp"
#include "CharacterModel.hpp"
#include "MorphemeModel.hpp"
#include "Sense.hpp"
#include <QMainWindow>

namespace Morpheus
{
	class MainWindow : public QMainWindow
	{
			Q_OBJECT

		private:

			/// The main program window
			uptr<Ui::MainWindow> main_window;

			/// The configuration window
			uptr<Config> config;

			virtual void closeEvent(QCloseEvent* _event);

			uptr<MorphemeExtractor> morpheme_extractor;

			/// The input file for processing (the corpus)
			/// TODO: Multiple files
			/// TODO: Buffering of large / multiple files
			QFile input_file;

			/// The processed file
			QFile processed_file;

			/// The processed file (file.ext.seg, etc.)
			/// TODO: Multiple files
			QFile segmented_file;

			/// The number of lines in the corpus
			uint lines;

			/// Statistics about characters and morphemes
			QFile stats_file;

			/// Character occurrences
			uptr<CharacterModel> character_model;
			uptr<QSortFilterProxyModel> character_model_filter;

			/// Morpheme occurrences
			uptr<MorphemeModel> morpheme_model;
			uptr<QSortFilterProxyModel> morpheme_model_filter;

			/// SENSE instance
			uptr<Sense> sense;

			/// Status bar label
			QString status;
			QLabel status_label;

			/// Initalises connections between signals and slots
			void setup_connections();

			/// Preprocess the current line
			/// (remove spaces, punctuation, etc.)
			QString process_line(const QString&& _line);

			/// Set the file names for the processed and segmented
			/// versions of the corpus
			void set_filenames(const QString& _file_name);

		public:
			explicit MainWindow(QWidget* _parent = 0);

			virtual ~MainWindow();

		public slots:

			/// Opens the settins dialog
			inline void open_settings_window()
			{
				if (config != nullptr)
				{
					config = std::make_unique<Config>();
				}
				config->show();
			}

			inline void open_file_dialog()
			{
				QDir dir(Config::last_open_dir);
				QString file_name = QFileDialog::getOpenFileName(this,
																 tr("Load corpus"),
																 (dir.exists() ? Config::last_open_dir : "./"),
																 tr("Text files (*.txt)"));
				if (!file_name.isEmpty())
				{
					set_filenames(file_name);
				}
			}

			inline void clear_filenames()
			{
				input_file.setFileName("");
				processed_file.setFileName("");
				segmented_file.setFileName("");
				stats_file.setFileName("");
			}

			void load_corpus();

			/// Processing routines
			void extract_morphemes();
			void resegment_morphemes();

			/// Display the tables
			void show_character_table();
			void show_morpheme_table();

			/// Display options
			void set_transition_table_properties();
			void reset_tables();
			void clear_morphemes();

			inline void set_status_text(const QString& _text)
			{
				status_label.setText(_text);
			}

			/// Self-explanatory
			void save_stats();

			/// Update the log
			void update_log(const QString& _message);

			/// Clear the log
			void clear_log();

	};
}

#endif // MAINWINDOW_HPP
