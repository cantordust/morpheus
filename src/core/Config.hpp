#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "Globals.hpp"

namespace Morpheus
{

	class Config : public QDialog
	{
			Q_OBJECT

		private:
			uptr<Ui::Config> config;
			virtual void closeEvent(QCloseEvent* _event);

		public:
			explicit Config(QWidget* _parent = 0,
							Qt::WindowFlags _flags = Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::FramelessWindowHint);
			virtual ~Config();

			static QSize main_window_size;
			static QPoint main_window_pos;

			static bool confirm_on_exit;
			static bool console_output;

			static uint max_lines;
			static uint lines_to_process_together;

			/// Preprocessing options
			static bool proc_lowercase;
			static bool proc_remove_all_spaces;
			static bool proc_remove_non_alnum;
			static bool proc_remove_punctuation;
			static bool proc_collapse_multiple_spaces;
			static bool proc_keep_apostrophes;

			/// Options to dump the processed data into various files
			static bool remember_last_open_dir;
			static QString last_open_dir;
			static bool autosave_stats;
			static bool seg_method_ps_count;
			static bool seg_method_ps_entropy;
			static bool seg_method_character_frequencies;

			/// Semantics
			static uint hidden_layer_size;

			void save();
			void load();

		signals:

		private slots:

			void changePage(QListWidgetItem* _current,
							QListWidgetItem* _previous);

			void toggle_remove_all_spaces(bool _checked);
			void toggle_remove_punctuation(bool _checked);
			void toggle_remove_non_alnum(bool _checked);

	};
}

#endif // CONFIG_HPP
