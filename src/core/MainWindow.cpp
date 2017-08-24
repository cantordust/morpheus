#include "MainWindow.hpp"
#include "ui_MainWindow.h"

namespace Morpheus
{
	MainWindow::MainWindow(QWidget* _parent)
		:
		  QMainWindow(_parent),
		  main_window(std::make_unique<Ui::MainWindow>())
	{
		QCoreApplication::setOrganizationName("Cantordust");
		QCoreApplication::setApplicationName("Morpheus");
		main_window->setupUi(this);
		main_window->tabWidget->setCurrentWidget(main_window->tabStatistics);

		/// Morpheme extractor
		morpheme_extractor = std::make_unique<MorphemeExtractor>();

		character_model = std::make_unique<CharacterModel>(morpheme_extractor.get());
		character_model_filter = std::make_unique<QSortFilterProxyModel>();

		morpheme_model = std::make_unique<MorphemeModel>(morpheme_extractor.get());
		morpheme_model_filter = std::make_unique<QSortFilterProxyModel>();

		/// SENSE
		sense = std::make_unique<Sense>();

		/// Configuration
		config = std::make_unique<Config>();
		config->load();
		resize(Config::main_window_size);
		move(Config::main_window_pos);

		/// Status label
		main_window->statusBar->addWidget(&status_label);
		set_status_text("");

		/// Connect signals and slots
		setup_connections();
	}

	MainWindow::~MainWindow()
	{
		if (Config::autosave_stats)
		{
			save_stats();
		}
	}

	void MainWindow::closeEvent(QCloseEvent* _event)
	{
		Config::main_window_pos = this->pos();
		Config::main_window_size =  this->size();
		config->save();
		if (Config::confirm_on_exit)
		{
			QMessageBox::StandardButton confirm;
			confirm = QMessageBox::question(this, "Morpheus", "Do you really want to exit?",
											QMessageBox::Yes|QMessageBox::No);
			{
				if (confirm == QMessageBox::Yes)
				{
					_event->accept();
				}
				else
				{
					_event->ignore();
				}
			}
		}
		else
		{
			_event->accept();
		}
	}

	void MainWindow::set_filenames(const QString& _file_name)
	{
		input_file.setFileName(_file_name);
		if (input_file.exists())
		{
			QFileInfo fi(input_file);
			if (Config::remember_last_open_dir)
			{
				Config::last_open_dir = fi.absoluteDir().absolutePath();
			}
			config->save();
			QDir dir(fi.absoluteDir());
			QString base(fi.baseName());
			QString ext(fi.completeSuffix());
			processed_file.setFileName(dir.absolutePath() + "/" + base + "_proc." + ext);
			segmented_file.setFileName(dir.absolutePath() + "/" + base + "_seg." + ext);
			stats_file.setFileName(dir.absolutePath() + "/" + base + "_stats.xml");
			load_corpus();
		}
		else
		{
			main_window->actionExtractMorphemes->setEnabled(false);
			main_window->actionResegmentMorphemes->setEnabled(false);
			main_window->actionRunSENSE->setEnabled(false);
			clear_filenames();
		}
	}

	void MainWindow::show_character_table()
	{

		morpheme_extractor->alphabet_vector.clear();
		QHash<QChar, uint>::const_iterator ch = morpheme_extractor->get_alphabet().constBegin();
		while (ch != morpheme_extractor->get_alphabet().constEnd())
		{
			morpheme_extractor->alphabet_vector.push_back(QPair<QChar,uint>(ch.key(),ch.value()));
			++ch;
		}

		character_model_filter->setSourceModel(character_model.get());
		main_window->tblCharacters->setModel(character_model_filter.get());
		main_window->tblCharacters->setAlternatingRowColors(true);
		main_window->tblCharacters->horizontalHeader()->show();
		character_model_filter->sort(1, Qt::DescendingOrder);
		main_window->tblCharacters->show();

		main_window->actionLoadCorpus->setEnabled(true);

		main_window->tabWidget->setCurrentWidget(main_window->tabStatistics);
		status = "Total characters: " + QString::number(morpheme_extractor->get_total_character_count())
				 + ", distinct characters: " + QString::number(morpheme_extractor->get_alphabet_size());
		set_status_text(status);
	}

	void MainWindow::show_morpheme_table()
	{
		morpheme_extractor->dictionary_vector.clear();
		QHash<QString, uint>::const_iterator morph = morpheme_extractor->get_dictionary().constBegin();
		while (morph != morpheme_extractor->get_dictionary().constEnd())
		{
			morpheme_extractor->dictionary_vector.push_back(QPair<QString,uint>(morph.key(),morph.value()));
			++morph;
		}

		morpheme_model_filter->setSourceModel(morpheme_model.get());
		main_window->tblMorphemes->setModel(morpheme_model_filter.get());
		main_window->tblMorphemes->setAlternatingRowColors(true);
		main_window->tblMorphemes->horizontalHeader()->show();
		morpheme_model_filter->sort(1, Qt::DescendingOrder);
		main_window->tblMorphemes->show();
		main_window->actionLoadCorpus->setEnabled(true);
		status = "Total characters: " + QString::number(morpheme_extractor->get_total_character_count())
				 + ", distinct characters: " + QString::number(morpheme_extractor->get_alphabet_size())
				 + ", total morphemes: " + QString::number(morpheme_extractor->get_total_morpheme_count())
				 + ", distinct morphemes: " + QString::number(morpheme_extractor->get_dictionary_size());
		set_status_text(status);
	}

	void MainWindow::set_transition_table_properties()
	{

	}

	void MainWindow::reset_tables()
	{
		show_character_table();
		show_morpheme_table();
	}

	void MainWindow::clear_morphemes()
	{
		show_morpheme_table();
	}

	void MainWindow::save_stats()
	{
		if (stats_file.open(QFile::WriteOnly | QFile::Truncate))
		{
			stats_file.close();
			pugi::xml_document stats;
			pugi::xml_parse_result result = stats.load_file(stats_file.fileName().toUtf8().constData());
			if (result)
			{
				pugi::xml_node root = stats.root();
				root.set_name("Statistics");
				pugi::xml_node parameters = root.append_child("Parameters");
				std::stringstream ss;

				/// Dump the parameters which were used to extract these stats
				ss << Config::max_lines;
				pugi::xml_node parameter = parameters.append_child("max_lines");
				parameter.set_value(ss.str().c_str());
				ss.clear();

				ss << Config::proc_lowercase;
				parameter = parameters.append_child("process_lowercase");
				parameter.set_value(ss.str().c_str());
				ss.clear();

				ss << Config::proc_remove_all_spaces;
				parameter = parameters.append_child("process_remove_spaces");
				parameter.set_value(ss.str().c_str());
				ss.clear();

				ss << Config::proc_remove_punctuation;
				parameter = parameters.append_child("process_remove_punctuation");
				parameter.set_value(ss.str().c_str());
				ss.clear();

				ss << Config::proc_collapse_multiple_spaces;
				parameter = parameters.append_child("process_replace_multiple_spaces");
				parameter.set_value(ss.str().c_str());
				ss.clear();

				ss << Config::remember_last_open_dir;
				parameter = parameters.append_child("remember_last_input_file");
				parameter.set_value(ss.str().c_str());
				parameter.append_attribute("file_name").set_value(Config::last_open_dir.toUtf8().constData());
				ss.clear();

				if (morpheme_extractor->get_alphabet().size() > 0)
				{
					pugi::xml_node character_root = root.append_child("characters");
					pugi::xml_node character_node;

					QHash<QChar, uint>& characters(morpheme_extractor->get_alphabet());

					for (const QChar& character : characters.keys())
					{
						character_node = character_root.append_child("char");
						character_node.append_attribute("occurrences").set_value(characters[character]);
						character_node.set_value(QString(character).toUtf8().constData());
					}
				}

				if (morpheme_extractor->get_dictionary().size() > 0)
				{
					pugi::xml_node morpheme_root = root.append_child("morphemes");
					pugi::xml_node morpheme_node;
					QHash<QString, uint>& morphemes(morpheme_extractor->get_dictionary());
					for (const QString& morpheme : morphemes.keys())
					{
						morpheme_node = morpheme_root.append_child("morpheme");
						morpheme_node.append_attribute("occurrences").set_value(morphemes[morpheme]);
						morpheme_node.set_value(morpheme.toUtf8().constData());
					}
				}
			}
			stats.save_file(stats_file.fileName().toUtf8().constData());
		}
	}

	void MainWindow::update_log(const QString& _message)
	{
		main_window->ptxtLog->appendPlainText(_message);
	}

	void MainWindow::clear_log()
	{
		main_window->ptxtLog->clear();
	}

	void MainWindow::setup_connections()
	{
		main_window->statusBar->show();

		connect(main_window->actionSettings,SIGNAL(triggered()),this,SLOT(open_settings_window()));
		connect(main_window->actionLoadCorpus,SIGNAL(triggered()),this,SLOT(open_file_dialog()));
		//		connect(main_window->actionSaveCharacters, SIGNAL(triggered()), morpheme_extractor.get(), SLOT(save_characters()));
		//		connect(main_window->actionSaveMorphemes, SIGNAL(triggered()), morpheme_extractor.get(), SLOT(save_morphemes()));

		connect(main_window->actionExtractMorphemes,SIGNAL(triggered()),this,SLOT(extract_morphemes()));
		connect(main_window->actionResegmentMorphemes,SIGNAL(triggered()),this,SLOT(resegment_morphemes()));
		connect(main_window->actionRunSENSE,SIGNAL(triggered()),sense.get(),SLOT(test()));

		connect(main_window->actionQuit,SIGNAL(triggered()),this,SLOT(close()));
		connect(morpheme_extractor.get(), SIGNAL(characters_extracted()),this,SLOT(show_character_table()));
		connect(morpheme_extractor.get(), SIGNAL(morphemes_resegmented()),this,SLOT(show_morpheme_table()));
		connect(morpheme_extractor.get(), SIGNAL(morpheme_extractor_reset()), this, SLOT(reset_tables()));
		connect(morpheme_extractor.get(), SIGNAL(morpheme_extractor_cleared()), this, SLOT(clear_morphemes()));

		connect(morpheme_extractor.get(), SIGNAL(update_log(const QString&)), this, SLOT(update_log(const QString&)));
	}

	QString MainWindow::process_line(const QString&& _line)
	{
		QString processed_line;
		processed_line.reserve(_line.size());

		for (int i = 0; i < _line.size(); ++i)
		{
			if ((_line.at(i).isSpace()
				 && Config::proc_remove_all_spaces
				 )
				|| (!_line.at(i).isLetterOrNumber()
					&& Config::proc_remove_non_alnum
					)
				|| (_line.at(i).isPunct()
					&& Config::proc_remove_punctuation
					)
				)
			{
				if (_line.at(i) == '\''
					&& Config::proc_keep_apostrophes)
				{
					processed_line.push_back('\'');
				}
				else
				{
					continue;
				}
			}

			processed_line.push_back((Config::proc_lowercase) ? _line.at(i).toLower() : _line.at(i));
		}

		if (Config::proc_collapse_multiple_spaces)
		{
			processed_line = processed_line.simplified();
			if (Config::proc_remove_all_spaces)
			{
				processed_line.replace(" ","");
			}
		}

		return processed_line;
	}

	void MainWindow::load_corpus()
	{
		main_window->lnedtFileName->setText(input_file.fileName());

		uint l(0);
		if (input_file.open(QFile::ReadOnly))
		{
			QTextStream input_qts(&input_file);
			input_qts.autoDetectUnicode();
			QString ln;
			while (input_qts.readLineInto(&ln))
			{
				++l;
			}
			input_file.close();
		}

		if (input_file.open(QFile::ReadOnly))
		{
			QProgressDialog pd;
			pd.setMinimum(0);
			pd.setWindowModality(Qt::WindowModal);
			pd.setCancelButton(0);
			lines = 0;
			QTextStream input_qts(&input_file);
			input_qts.autoDetectUnicode();
			if (processed_file.open(QFile::WriteOnly | QFile::Truncate))
			{
				pd.setMaximum(l);
				pd.setLabelText("Extracting characters...");
				pd.open();
				QTextStream processed_qts(&processed_file);
				processed_qts.autoDetectUnicode();
				QString line;
				QString processed_line;
				while (input_qts.readLineInto(&line))
				{
					processed_line = process_line(std::move(line));
					if (processed_line.size() > 0)
					{
						processed_qts << processed_line << '\n';
						++lines;
						if (lines % 1000 == 0)
						{
							processed_qts.flush();
							pd.setValue(lines);
							QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
						}
					}
				}

				processed_file.close();
				pd.close();

				/// Initialise the morpheme extractor, which includes
				///	creating the suffix array.

				morpheme_extractor->init(processed_file.fileName());

				main_window->actionExtractMorphemes->setEnabled(true);
			}

			input_file.close();
		}
	}

	void MainWindow::extract_morphemes()
	{
		morpheme_extractor->clear(true);
		main_window->actionLoadCorpus->setEnabled(false);
		main_window->actionSaveCharacters->setEnabled(false);
		main_window->actionSaveMorphemes->setEnabled(false);
		connect(morpheme_extractor.get(), SIGNAL(morphemes_extracted()), this, SLOT(show_morpheme_table()));

		if (processed_file.open(QFile::ReadOnly))
		{
			if (segmented_file.open(QFile::ReadWrite | QFile::Truncate | QFile::Text))
			{
				QTextStream processed_qts(&processed_file);
				QString line;
				uint line_count(0);
				QProgressDialog pd;
				pd.setMinimum(0);
				pd.setWindowModality(Qt::WindowModal);
				pd.setCancelButton(0);
				pd.open();
				QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

				QTextStream segmented_qts(&segmented_file);
				if (Config::max_lines == 0)
				{
					pd.setMaximum(lines);
				}
				else
				{
					pd.setMaximum(Config::max_lines);
				}
				pd.setLabelText("Processing line " + QString::number(line_count + 1) + "/" + QString::number(pd.maximum()));
				pd.setValue(0);
				QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

				while (processed_qts.readLineInto(&line)
					   && (Config::max_lines == 0
						   || (Config::max_lines > 0
							   && line_count < Config::max_lines
							   )
						   )
					   )
				{
					++line_count;

					line.push_back('\n');
					segmented_qts << morpheme_extractor->extract_morphemes(std::move(line)) << endl;
					//					morpheme_extractor->extract_morphemes(line);

					/// Update the progress bar
					pd.setLabelText("Processing line " + QString::number(line_count + 1)
									+ "/" + QString::number(pd.maximum())
									+ "\nTotal morpheme count: " + QString::number(morpheme_extractor->get_total_morpheme_count())
									+ "\nDistinct morphemes: " + QString::number(morpheme_extractor->get_dictionary_size()));

					pd.setValue(line_count);
					QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
				}

				segmented_qts.seek(0);
				main_window->ptxtSegmentedCorpus->setPlainText(segmented_qts.readAll());

				segmented_file.close();
				pd.close();
			}
			processed_file.close();

			morpheme_extractor->clear();

			main_window->actionResegmentMorphemes->setEnabled(true);
			//			main_window->actionRunSENSE->setEnabled(true);

			show_morpheme_table();
		}
	}

	void MainWindow::resegment_morphemes()
	{
		if (segmented_file.open(QFile::ReadOnly))
		{
			/// Compute the initial dictionary cost and ntropy
			morpheme_extractor->init_entropy();

			QStringList reseg_list;
			QTextStream segmented_qts(&segmented_file);
			QString line;
			uint line_count(0);
			QProgressDialog pd;
			pd.setMinimum(0);
			pd.setWindowModality(Qt::WindowModal);
			pd.setCancelButton(0);
			pd.open();
			QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

			if (Config::max_lines == 0)
			{
				pd.setMaximum(lines);
			}
			else
			{
				pd.setMaximum(Config::max_lines);
			}
			pd.setLabelText("Processing line " + QString::number(line_count + 1) + "/" + QString::number(pd.maximum()));
			pd.setValue(0);
			QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

			while (segmented_qts.readLineInto(&line))
			{
				++line_count;

				reseg_list.append(morpheme_extractor->resegment_morphemes(std::move(line)) + '\n');
				//					morpheme_extractor->extract_morphemes(line);

				/// Update the progress bar
				pd.setLabelText("Processing line " + QString::number(line_count + 1)
								+ "/" + QString::number(pd.maximum())
								+ "\nTotal morpheme count: " + QString::number(morpheme_extractor->get_total_morpheme_count())
								+ "\nDistinct morphemes: " + QString::number(morpheme_extractor->get_dictionary_size()));

				pd.setValue(line_count);
				QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
			}

			segmented_file.close();

			if (segmented_file.open(QFile::ReadWrite | QFile::Truncate))
			{
				segmented_qts.setDevice(&segmented_file);
				for (const QString& str : reseg_list)
				{
					segmented_qts << str;
				}
			}

			segmented_qts.seek(0);
			main_window->ptxtSegmentedCorpus->setPlainText(segmented_qts.readAll());

			segmented_file.close();

			pd.close();

			show_morpheme_table();
		}
	}
}
