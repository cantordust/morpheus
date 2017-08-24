#include "Config.hpp"
#include "ui_Config.h"

namespace Morpheus
{

	QSize Config::main_window_size;
	QPoint Config::main_window_pos;

	bool Config::confirm_on_exit;
	bool Config::console_output;

	uint Config::max_lines;
	uint Config::lines_to_process_together;

	/// Preprocessing options
	bool Config::proc_lowercase;
	bool Config::proc_remove_all_spaces;
	bool Config::proc_remove_non_alnum;
	bool Config::proc_remove_punctuation;
	bool Config::proc_collapse_multiple_spaces;
	bool Config::proc_keep_apostrophes;

	/// Options to save the processed data into various files
	bool Config::remember_last_open_dir;
	QString Config::last_open_dir;
	bool Config::autosave_stats;
	bool Config::seg_method_ps_count;
	bool Config::seg_method_ps_entropy;
	bool Config::seg_method_character_frequencies;

	/// Semantics
	uint Config::hidden_layer_size;

	Config::Config(QWidget* _parent,
				   Qt::WindowFlags _flags)
		:
		  QDialog(_parent),
		  config(std::make_unique<Ui::Config>())
	{

		config->setupUi(this);
		load();
		connect(config->btnOK,SIGNAL(clicked()),this,SLOT(close()));
		connect(config->lstConfigList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));
		connect(config->chkProcRemoveSpaces, SIGNAL(toggled(bool)), this, SLOT(toggle_remove_all_spaces(bool)));
		connect(config->chkProcRemoveNonAlnum, SIGNAL(toggled(bool)), this, SLOT(toggle_remove_non_alnum(bool)));
		connect(config->chkProcRemovePunct, SIGNAL(toggled(bool)), this, SLOT(toggle_remove_punctuation(bool)));

		toggle_remove_all_spaces(config->chkProcRemoveSpaces->isChecked());
		toggle_remove_punctuation(config->chkProcRemovePunct->isChecked());
		toggle_remove_non_alnum(config->chkProcRemoveNonAlnum->isChecked());

		config->stackSettings->setCurrentIndex(0);
	}

	Config::~Config()
	{

	}

	void Config::closeEvent(QCloseEvent* _event)
	{
		save();
	}

	void Config::save()
	{
		QSettings s;

		/////////////////////
		/// Window parameters
		/////////////////////
		s.beginGroup("windows");

		s.setValue("main/size", main_window_size);
		s.setValue("main/position", main_window_pos);
		s.setValue("config/position", this->pos());

		s.endGroup();

		/////////
		/// Files
		/////////

		s.beginGroup("file/open");
		remember_last_open_dir = config->chkRememberLastOpenDir->isChecked();
		s.setValue("remember_last_open_dir", remember_last_open_dir);

		if (!remember_last_open_dir)
		{
			last_open_dir = "./";
		}
		s.setValue("last_open_dir", last_open_dir);

		s.endGroup();

		//////////////
		/// Morphology
		//////////////

		/// Preprocessing
		s.beginGroup("morphology/preprocessing");

		max_lines = config->sboxMaxLines->value();
		s.setValue("max_lines_to_process", max_lines);

		proc_remove_all_spaces = config->chkProcRemoveSpaces->isChecked();
		s.setValue("remove_all_spaces", proc_remove_all_spaces);

		proc_collapse_multiple_spaces = config->chkProcCollapseMultipleSpaces->isChecked();
		s.setValue("collapse_multiple_spaces", proc_collapse_multiple_spaces);

		proc_remove_non_alnum = config->chkProcRemoveNonAlnum->isChecked();
		s.setValue("remove_non_alnum", proc_remove_non_alnum);

		proc_remove_punctuation = config->chkProcRemovePunct->isChecked();
		s.setValue("remove_all_punctuation", proc_remove_punctuation);

		proc_lowercase = config->chkProcLowercase->isChecked();
		s.setValue("lowercase_everything", proc_lowercase);

		proc_keep_apostrophes = config->chkProcKeepApostrophes->isChecked();
		s.setValue("keep_apostrophes", proc_keep_apostrophes);
		s.endGroup();

		/// Statistics
		s.beginGroup("morphology/statistics");
		seg_method_ps_count = config->rdPSCount->isChecked();
		s.setValue("segmentation_method/ps_count", seg_method_ps_count);

		seg_method_ps_entropy = config->rdPSEntropy->isChecked();
		s.setValue("segmentation_method/ps_entropy", seg_method_ps_entropy);

		seg_method_character_frequencies = config->rdCharacterFrequencies->isChecked();
		s.setValue("segmentation_method/character_frequencies", seg_method_character_frequencies);
		s.endGroup();

		/////////////////
		/// Semantics ///
		/////////////////

		s.beginGroup("semantics/SENSE");

		hidden_layer_size = config->sboxHiddenLayerSize->value();
		s.setValue("hidden_layer_size", hidden_layer_size);

		s.endGroup();

		///////////////////////////////
		/// Program layout and settings
		///////////////////////////////

		s.beginGroup("program");

		confirm_on_exit = config->chkConfirmOnExit->isChecked();
		s.setValue("confirm_on_exit", confirm_on_exit);

		console_output = config->chkConsoleOutput->isChecked();
		s.setValue("console_output", console_output);

		s.endGroup();

	}

	void Config::load()
	{
		QSettings s;
		/// Settings window position (the size is fixed)
		s.beginGroup("windows");
		main_window_size = s.value("main/size", QSize(400, 400)).toSize();
		main_window_pos = s.value("main/position", QPoint(200, 200)).toPoint();
		move(s.value("config/position", QPoint(200, 200)).toPoint());
		s.endGroup();

		/////////
		/// Files
		/////////

		s.beginGroup("file/open");
		remember_last_open_dir = s.value("remember_last_open_dir", false).toBool();
		config->chkRememberLastOpenDir->setChecked(remember_last_open_dir);

		if (remember_last_open_dir)
		{
			last_open_dir = s.value("last_open_dir").toString();
		}
		else
		{
			last_open_dir = "./";
		}
		s.endGroup();

		//////////////
		/// Morphology
		//////////////

		/// Preprocessing
		s.beginGroup("morphology/preprocessing");
		max_lines = s.value("max_lines_to_process", 1).toInt();
		config->sboxMaxLines->setValue(max_lines);

		proc_remove_all_spaces = s.value("remove_all_spaces", false).toBool();
		config->chkProcRemoveSpaces->setChecked(proc_remove_all_spaces);

		proc_collapse_multiple_spaces = s.value("collapse_multiple_spaces", false).toBool();
		config->chkProcCollapseMultipleSpaces->setChecked(proc_collapse_multiple_spaces);

		proc_remove_non_alnum = s.value("remove_non_alnum", false).toBool();
		config->chkProcRemoveNonAlnum->setChecked(proc_remove_non_alnum);

		proc_remove_punctuation = s.value("remove_all_punctuation", false).toBool();
		config->chkProcRemovePunct->setChecked(proc_remove_punctuation);

		proc_lowercase = s.value("lowercase_everything", false).toBool();
		config->chkProcLowercase->setChecked(proc_lowercase);

		proc_keep_apostrophes = s.value("keep_apostrophes", false).toBool();
		config->chkProcKeepApostrophes->setChecked(proc_keep_apostrophes);

		s.endGroup();

		/// Statistics
		s.beginGroup("morphology/statistics");
		seg_method_ps_count = s.value("segmentation_method/ps_count", true).toBool();
		config->rdPSCount->setChecked(seg_method_ps_count);

		seg_method_ps_entropy = s.value("segmentation_method/ps_entropy", false).toBool();
		config->rdPSEntropy->setChecked(seg_method_ps_entropy);

		seg_method_character_frequencies = s.value("segmentation_method/character_frequencies", false).toBool();
		config->rdCharacterFrequencies->setChecked(seg_method_character_frequencies);
		s.endGroup();

		/////////////
		/// Semantics
		/////////////

		s.beginGroup("semantics/SENSE");

		hidden_layer_size = s.value("hidden_layer_size", 300).toUInt();
		config->sboxHiddenLayerSize->setValue(hidden_layer_size);

		s.endGroup();

		///////////////////////////////
		/// Program layout and settings
		///////////////////////////////

		s.beginGroup("program");

		confirm_on_exit = s.value("confirm_on_exit",false).toBool();
		config->chkConfirmOnExit->setChecked(confirm_on_exit);

		console_output = s.value("console_output",false).toBool();
		config->chkConsoleOutput->setChecked(console_output);

		s.endGroup();

	}

	void Config::changePage(QListWidgetItem* _current,
							QListWidgetItem* _previous)
	{
		if (!_current)
		{
			_current = _previous;
		}

		config->stackSettings->setCurrentIndex(config->lstConfigList->row(_current));
	}

	void Config::toggle_remove_all_spaces(bool _checked)
	{
		config->chkProcCollapseMultipleSpaces->setChecked(!_checked);
		config->chkProcCollapseMultipleSpaces->setEnabled(!_checked);
	}

	void Config::toggle_remove_punctuation(bool _checked)
	{
		config->chkProcKeepApostrophes->setChecked(_checked);
		config->chkProcKeepApostrophes->setEnabled(_checked);
	}

	void Config::toggle_remove_non_alnum(bool _checked)
	{
		config->chkProcRemovePunct->setChecked(!_checked);
		config->chkProcKeepApostrophes->setChecked(!_checked);
		config->chkProcRemovePunct->setEnabled(!_checked);
		config->chkProcKeepApostrophes->setEnabled(!_checked);
	}
}
