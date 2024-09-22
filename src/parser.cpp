/**
 * @file: parser.cpp
 * @date: 2024-09-22
 *
 * @telegram: @forman
 * @author: Yuriy Lobarev
 * @phone: +7 (910) 983-95-90
 * @email: forman@anyks.com
 * @site: https://anyks.com
 *
 * @copyright: Copyright © 2024
 */

// Подключаем заголовочный файл
#include <parser.hpp>

/**
 * clearPatterns Метод сброса списка добавленных шаблонов GROK
 */
void anyks::Parser::clearPatterns() noexcept {
	// Выполняем блокировку потока
	const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
	// Выполняем удаление списка добавленных шаблонов GROK
	this->_grok.clearPatterns();
}
/**
 * pattern Метод добавления шаблона GROK
 * @param key название переменной
 * @param val регуляреное выражение соответствующее переменной
 */
void anyks::Parser::pattern(const string & key, const string & val) noexcept {
	// Выполняем блокировку потока
	const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
	// Выполняем добавление шаблона GROK
	this->_grok.pattern(key, val);
}
/**
 * yaml Метод конвертации текста в формате YAML в объект JSON
 * @param text текст для конвертации
 * @return     объект в формате JSON
 */
nlohmann::json anyks::Parser::yaml(const string & text) noexcept {
	// Результат работы функции
	nlohmann::json result = nlohmann::json::object();
	// Если текст передан
	if(!text.empty()){
		// Выполняем блокировку потока
		const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			/**
			 * parseFn Прототип функции парсинга XML документа
			 * @param таблица результатов
			 * @param название ключа куда добавляется содержимое ноды
			 * @param объект текущей ноды
			 */
			function <void (nlohmann::json &, const nlohmann::json, const YAML::Node &)> parseFn;
			/**
			 * parseFn Функция парсинга XML документа
			 * @param root корень объекта для записи результата
			 * @param name название ключа куда добавляется содержимое ноды
			 * @param node объект текущей ноды
			 */
			parseFn = [&parseFn, this](nlohmann::json & root, const nlohmann::json name, const YAML::Node & node) -> void {
				// Определяем тип полученной ноды
				switch(node.Type()){
					// Если объект определён как не существующий
					case YAML::NodeType::Null: {
						// Если если корень раздела является объектом
						if(root.is_object())
							// Устанавливаем пустое значение
							root.emplace(name, nullptr);
						// Если если корень раздела является массивом
						else if(root.is_array())
							// Добавляем пустое значение в массив
							root[name.get <size_t> ()] = nullptr;
					} break;
					// Если объект является скалярным выражением
					case YAML::NodeType::Scalar: {
						// Если полученное значение является числом
						if(this->_fmk->is(node.as <string> (), fmk_t::check_t::NUMBER)){
							// Получаем значение числа
							const long long number = node.as <long long> ();
							// Если число является отрицательным
							if(number < 0){
								// Если если корень раздела является объектом
								if(root.is_object())
									// Устанавливаем числовое значение
									root.emplace(name, number);
								// Если если корень раздела является массивом
								else if(root.is_array())
									// Добавляем полученное число в массив
									root[name.get <size_t> ()] = number;
							// Выводим числовое значение
							} else {
								// Если если корень раздела является объектом
								if(root.is_object())
									// Устанавливаем числовое значение
									root.emplace(name, node.as <unsigned long long> ());
								// Если если корень раздела является массивом
								else if(root.is_array())
									// Добавляем числовое значение в массив
									root[name.get <size_t> ()] = node.as <unsigned long long> ();
							}
						// Если полученное значение является числом с плавающей точкой
						} else if(this->_fmk->is(node.as <string> (), fmk_t::check_t::DECIMAL)) {
							// Если если корень раздела является объектом
							if(root.is_object())
								// Устанавливаем числовое значение с плавающей точкой
								root.emplace(name, node.as <double> ());
							// Если если корень раздела является массивом
							else if(root.is_array())
								// Добавляем числовое значение с плавающей точкой в массив
								root[name.get <size_t> ()] = node.as <double> ();
						// Если полученное значение является строкой
						} else {
							// Флаг истинного значения
							bool mode = false;
							// Если значение является булевым
							if((mode = this->_fmk->compare(node.as <string> (), "true")) || this->_fmk->compare(node.as <string> (), "false")){
								// Если если корень раздела является объектом
								if(root.is_object())
									// Устанавливаем булевое значение
									root.emplace(name, mode);
								// Если если корень раздела является массивом
								else if(root.is_array())
									// Добавляем булевое значение в массив
									root[name.get <size_t> ()] = mode;
							// Если значение является просто строкой
							} else {
								// Если если корень раздела является объектом
								if(root.is_object())
									// Устанавливаем значение строки как она есть
									root.emplace(name, node.as <string> ());
								// Если если корень раздела является массивом
								else if(root.is_array())
									// Добавляем значение строки как она есть в массив
									root[name.get <size_t> ()] = node.as <string> ();
							}
						}
					} break;
					// Если объект является последовательностью
					case YAML::NodeType::Sequence: {
						// Создаём новый массив данных
						nlohmann::json item = nlohmann::json::array();
						// Выполняем перебор всей последовательности
						for(size_t i = 0; i < node.size(); i++)
							// Выполняем добавление последовательности
							parseFn(item, i, node[i]);
						// Если если корень раздела является объектом
						if(root.is_object())
							// Устанавливаем массив в карту объекта
							root.emplace(name, item);
						// Если если корень раздела является массивом
						else if(root.is_array())
							// Добавляем массив в карту объекта в массив
							root[name.get <size_t> ()] = item;
					} break;
					// Если объект является картой
					case YAML::NodeType::Map: {
						// Создаём новый объект данных
						nlohmann::json item = nlohmann::json::object();
						// Выполняем перебор всего списка полученных данных
						for(auto i = node.begin(); i != node.end(); ++i){
							// Если ключ является строковым значением
							if(i->first.IsScalar())
								// Выполняем добавление последовательности
								parseFn(item, i->first.as <string> (), i->second);
						}
						// Если если корень раздела является объектом
						if(root.is_object())
							// Устанавливаем массив в карту объекта
							root.emplace(name, item);
						// Если если корень раздела является массивом
						else if(root.is_array())
							// Добавляем массив в карту объекта в массив
							root[name.get <size_t> ()] = item;
					} break;
					// Если объект не определён
					case YAML::NodeType::Undefined:
						// Выводим сообщение об ошибке
						this->_log->print("Node in \"Parser:yaml\" method is undefined", log_t::flag_t::WARNING);
					break;
				}
			};
			// Выполняем загрузку текстовых данных YAML
			YAML::Node node = YAML::Load(text);
			// Определяем тип полученной ноды
			switch(node.Type()){
				// Если объект определён как не существующий
				case YAML::NodeType::Null:
					// Выводим результат
					return result;
				// Если объект является скалярным выражением
				case YAML::NodeType::Scalar: {
					// Если полученное значение является числом
					if(this->_fmk->is(node.as <string> (), fmk_t::check_t::NUMBER)){
						// Получаем значение числа
						const long long number = node.as <long long> ();
						// Если число является отрицательным
						if(number < 0)
							// Выводим числовое значение
							result = number;
						// Выводим числовое значение
						else result = node.as <unsigned long long> ();
					// Если полученное значение является числом с плавающей точкой
					} else if(this->_fmk->is(node.as <string> (), fmk_t::check_t::DECIMAL))
						// Выводим числовое значение с плавающей точкой
						result = node.as <double> ();
					// Если полученное значение является строкой
					else {
						// Флаг истинного значения
						bool mode = false;
						// Если значение является булевым
						if((mode = this->_fmk->compare(node.as <string> (), "true")) || this->_fmk->compare(node.as <string> (), "false"))
							// Выводим булевое значение
							result = mode;
						// Если значение является просто строкой, выводим как оно есть
						else result = node.as <string> ();
					}
				} break;
				// Если объект является последовательностью
				case YAML::NodeType::Sequence: {
					// Выполняем создание результата в виде массива
					result = nlohmann::json::array();
					// Выполняем перебор всей последовательности
					for(size_t i = 0; i < node.size(); i++)
						// Выполняем добавление последовательности
						parseFn(result, i, node[i]);
				} break;
				// Если объект является картой
				case YAML::NodeType::Map: {
					// Выполняем перебор всего списка полученных данных
					for(auto i = node.begin(); i != node.end(); ++i){
						// Если ключ является строковым значением
						if(i->first.IsScalar())
							// Выполняем добавление последовательности
							parseFn(result, i->first.as <string> (), i->second);
					}
				} break;
				// Если объект не определён
				case YAML::NodeType::Undefined:
					// Выводим сообщение об ошибке
					this->_log->print("Node in \"Parser:yaml\" method is undefined", log_t::flag_t::WARNING);
				break;
			}
		/**
		 * Если возникает ошибка
		 */
		} catch(const YAML::ParserException & error) {
			// Выводим переданный лог
			this->_log->print("Parser: \"%s\" - %s [%s]", log_t::flag_t::CRITICAL, "YAML", error.what(), text.c_str());
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			// Выводим переданный лог
			this->_log->print("Parser: \"%s\" - %s [%s]", log_t::flag_t::CRITICAL, "YAML", error.what(), text.c_str());
		}
	}
	// Выводим результат работы функции
	return result;
}
/**
 * yaml Метод конвертации объекта JSON в текст в формате YAML
 * @param data данные в объекте JSON
 * @return     текст после конвертации
 */
string anyks::Parser::yaml(const nlohmann::json & data) noexcept {
	// Результат работы функции
	string result = "";
	// Если данные переданы
	if((data.is_object() || data.is_array()) && !data.empty()){
		// Выполняем блокировку потока
		const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			/**
			 * parseFn Прототип функции парсинга XML документа
			 * @param таблица результатов
			 * @param название ключа куда добавляется содержимое ноды
			 * @param объект текущей ноды
			 */
			function <void (YAML::Node &, nlohmann::json, const nlohmann::json &)> parseFn;
			/**
			 * parseFn Функция парсинга XML документа
			 * @param node  корень объекта для записи результата
			 * @param name  название ключа куда добавляется содержимое ноды
			 * @param value объект текущей ноды
			 */
			parseFn = [&parseFn, this](YAML::Node & node, nlohmann::json name, const nlohmann::json & value) -> void {
				// Если значение является числом
				if(value.is_number()){
					// Временное значение переменной
					double intpart = 0;
					// Выполняем извлечение числа
					const double number = value.get <double> ();
					// Выполняем проверку есть ли дробная часть у числа
					if(::modf(number, &intpart) == 0){
						// Если число является положительным
						if(number > 0.){
							// Если название ячейки является числом
							if(name.is_number())
								// Выполняем добавление значения в ноду
								node[name.get <uint32_t> ()] = value.get <uint64_t> ();
							// Если название ячейки является строкой
							else if(name.is_string())
								// Выполняем добавление значения в ноду
								node[name.get <string> ()] = value.get <uint64_t> ();
						// Если число является отрицательным
						} else {
							// Если название ячейки является числом
							if(name.is_number())
								// Выполняем добавление значения в ноду
								node[name.get <uint32_t> ()] = value.get <int64_t> ();
							// Если название ячейки является строкой
							else if(name.is_string())
								// Выполняем добавление значения в ноду
								node[name.get <string> ()] = value.get <int64_t> ();
						}
					// Если у числа имеется дробная часть
					} else {
						// Если название ячейки является числом
						if(name.is_number())
							// Выполняем добавление значения в ноду
							node[name.get <uint32_t> ()] = number;
						// Если название ячейки является строкой
						else if(name.is_string())
							// Выполняем добавление значения в ноду
							node[name.get <string> ()] = number;
					}
				// Если значение является булевым
				} else if(value.is_boolean()) {
					// Если название ячейки является числом
					if(name.is_number())
						// Выполняем добавление значения в ноду
						node[name.get <uint32_t> ()] = value.get <bool> ();
					// Если название ячейки является строкой
					else if(name.is_string())
						// Выполняем добавление значения в ноду
						node[name.get <string> ()] = value.get <bool> ();
				// Если значение является строкой
				} else if(value.is_string()) {
					// Если название ячейки является числом
					if(name.is_number())
						// Выполняем добавление значения в ноду
						node[name.get <uint32_t> ()] = value.get <string> ();
					// Если название ячейки является строкой
					else if(name.is_string())
						// Выполняем добавление значения в ноду
						node[name.get <string> ()] = value.get <string> ();
				// Если значение является массивом
				} else if(value.is_array()) {
					// Создаём объект дочерней ноды
					YAML::Node child;
					// Выполняем перебор всего списка
					for(size_t i = 0; i < value.size(); i++)
						// Выполняем добавление в массив полученных значений
						parseFn(child, i, value[i]);
					// Если название ячейки является числом
					if(name.is_number())
						// Выполняем добавление значения в ноду
						node[name.get <uint32_t> ()] = child;
					// Если название ячейки является строкой
					else if(name.is_string())
						// Выполняем добавление значения в ноду
						node[name.get <string> ()] = child;
				// Если значение является объектом
				} else if(value.is_object()) {
					// Создаём объект дочерней ноды
					YAML::Node child;
					// Выполняем перебор всего списка
					for(auto & el : value.items())
						// Выполняем добавление в объект полученных значений
						parseFn(child, el.key(), el.value());
					// Если название ячейки является числом
					if(name.is_number())
						// Выполняем добавление значения в ноду
						node[name.get <uint32_t> ()] = child;
					// Если название ячейки является строкой
					else if(name.is_string())
						// Выполняем добавление значения в ноду
						node[name.get <string> ()] = child;
				}
			};
			// Объект ноды для формирования результата
			YAML::Node node;
			// Если значение является массивом
			if(data.is_array()){
				// Выполняем перебор всего списка
				for(size_t i = 0; i < data.size(); i++)
					// Выполняем добавление в массив полученных значений
					parseFn(node, i, data[i]);
			// Если значение является объектом
			} else if(data.is_object()) {
				// Выполняем перебор всего списка
				for(auto & el : data.items())
					// Выполняем добавление в объект полученных значений
					parseFn(node, el.key(), el.value());
			}
			// Создаём поток для конвертации ноды YAML
			std::stringstream stream;
			// Записываем ноду YAML в поток
			stream << node;
			// Выполняем формирование результата
			result = stream.str();
		/**
		 * Если возникает ошибка
		 */
		} catch(const YAML::ParserException & error) {
			// Выводим переданный лог
			this->_log->print("Parser: \"%s\" - %s [%s]", log_t::flag_t::CRITICAL, "YAML", error.what(), data.dump(4).c_str());
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			// Выводим переданный лог
			this->_log->print("Parser: \"%s\" - %s [%s]", log_t::flag_t::CRITICAL, "YAML", error.what(), data.dump(4).c_str());
		}
	}
	// Выводим результат работы функции
	return result;
}
/**
 * ini Метод конвертации текста в формате INI в объект JSON
 * @param text текст для конвертации
 * @return     объект в формате JSON
 */
nlohmann::json anyks::Parser::ini(const string & text) noexcept {
	// Результат работы функции
	nlohmann::json result = nlohmann::json::object();
	// Если текст передан
	if(!text.empty()){
		// Выполняем блокировку потока
		const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем загрузку INI-конфига
			ini::File file = ini::load(text);
			// Выполняем перебор всего INI-файла
			for(auto i = file.begin(); i != file.end(); ++i){
				// Выполняем установку нового раздела
				result.emplace(i->first, nlohmann::json::object());
				// Выполняем перебор всех элементов секции
				for(auto j = i->second.begin(); j != i->second.end(); ++j){
					// Если полученное значение является числом
					if(this->_fmk->is(j->second, fmk_t::check_t::NUMBER)){
						// Получаем значение числа
						const long long number = ::stoll(j->second);
						// Если число является отрицательным
						if(number < 0)
							// Выполняем установку полученного значения
							result[i->first].emplace(j->first, number);
						// Если число является положительным
						else result[i->first].emplace(j->first, ::stoull(j->second));
					// Если полученное значение является числом с плавающей точкой
					} else if(this->_fmk->is(j->second, fmk_t::check_t::DECIMAL))
						// Выполняем установку полученного значения
						result[i->first].emplace(j->first, ::stod(j->second));
					// Если полученное значение является строкой
					else {
						// Флаг истинного значения
						bool mode = false;
						// Если значение является булевым
						if((mode = this->_fmk->compare(j->second, "true")) || this->_fmk->compare(j->second, "false"))
							// Выполняем установку полученного значения
							result[i->first].emplace(j->first, mode);
						// Если значение является строковым
						else result[i->first].emplace(j->first, j->second);
					}
				}
			}
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			// Выводим переданный лог
			this->_log->print("Parser: \"%s\" - %s [%s]", log_t::flag_t::CRITICAL, "INI", error.what(), text.c_str());
		}
	}
	// Выводим результат работы функции
	return result;
}
/**
 * ini Метод конвертации объекта JSON в текст в формате INI
 * @param data данные в объекте JSON
 * @return     текст после конвертации
 */
string anyks::Parser::ini(const nlohmann::json & data) noexcept {
	// Результат работы функции
	string result = "";
	// Если данные переданы
	if(data.is_object() && !data.empty()){
		// Выполняем блокировку потока
		const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем загрузку INI-конфига
			ini::File file = ini::load("");
			// Выполняем перебор всего списка
			for(auto & el : data.items()){
				// Выполняем создание секции
				file.add_section(el.key());
				// Если значения существуют верные
				if(el.value().is_object()){
					// Выполняем переход по всем параметрам секции
					for(auto & item : el.value().items()){
						// Если значение является числом
						if(item.value().is_number())
							// Устанавливаем значение в виде числа
							file[el.key()].set <double> (item.key(), item.value());
						// Если значение является булевым значением
						else if(item.value().is_boolean())
							// Устанавливаем значение в виде булевого значения
							file[el.key()].set <bool> (item.key(), item.value());
						// Если значение является строковым
						else if(item.value().is_string())
							// Устанавливаем значение в виде строки
							file[el.key()].set <string> (item.key(), item.value());
					}
				}
			}
			// Выполняем запись результата
			file.write(result);
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			// Выводим переданный лог
			this->_log->print("Parser: \"%s\" - %s [%s]", log_t::flag_t::CRITICAL, "INI", error.what(), data.dump(4).c_str());
		}
	}
	// Выводим результат работы функции
	return result;
}
/**
 * syslog Метод конвертации текста в формате SysLog в объект JSON
 * @param text текст для конвертации
 * @return     объект в формате JSON
 */
nlohmann::json anyks::Parser::syslog(const string & text) noexcept {
	// Если текст передан
	if(!text.empty()){
		// Выполняем блокировку потока
		const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем очистку контейнера
			this->_syslog.clear();
			// Выполняем парсинг данных
			this->_syslog = text;
			// Выполняем дамп данных в формате JSON
			return this->_syslog.dump();
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			// Выводим переданный лог
			this->_log->print("Parser: \"%s\" - %s [%s]", log_t::flag_t::CRITICAL, "SysLog", error.what(), text.c_str());
		}
	}
	// Выводим результат по умолчанию
	return nlohmann::json::object();
}
/**
 * syslog Метод конвертации объекта JSON в текст в формате SysLog
 * @param data данные в объекте JSON
 * @return     текст после конвертации
 */
string anyks::Parser::syslog(const nlohmann::json & data) noexcept {
	// Если данные переданы
	if(data.is_object() && !data.empty()){
		// Выполняем блокировку потока
		const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем очистку контейнера
			this->_syslog.clear();
			// Устанавливаем полученные данные
			this->_syslog.dump(data);
			// Выполняем формирование SysLog
			return this->_syslog.syslog();
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			// Выводим переданный лог
			this->_log->print("Parser: \"%s\" - %s [%s]", log_t::flag_t::CRITICAL, "SysLog", error.what(), data.dump(4).c_str());
		}
	}
	// Выводим результат по умолчанию
	return "";
}
/**
 * grok Метод конвертации текста в формате GROK в объект JSON
 * @param text    текст для конвертации
 * @param pattern регулярное выражение в формате GROK
 * @return        объект в формате JSON
 */
nlohmann::json anyks::Parser::grok(const string & text, const string & pattern) noexcept {
	// Если текст и шаблон переданы
	if(!text.empty() && !pattern.empty()){
		// Выполняем блокировку потока
		const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем сброс собранных данных
			this->_grok.reset();
			// Выполняем получение регулярного выражения
			string express = pattern;
			// Выполняем парсинг данных
			this->_grok.parse(text, this->_grok.build(express));
			// Выводим полученные данные
			return this->_grok.dump();
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			// Выводим переданный лог
			this->_log->print("Parser: \"%s\" - %s [%s]", log_t::flag_t::CRITICAL, "GROK", error.what(), text.c_str());
		}
	}
	// Выводим результат по умолчанию
	return nlohmann::json::object();
}
/**
 * csv Метод конвертации объекта JSON в текст в формате CSV
 * @param data  данные в объекте JSON
 * @param delim используемый разделитель
 * @return      текст после конвертации
 */
string anyks::Parser::csv(const nlohmann::json & data, const char delim) noexcept {
	// Результат работы функции
	string result = "";
	// Если данные переданы
	if(data.is_object() && !data.empty()){
		// Выполняем блокировку потока
		const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем очистку контейнера
			this->_csv.clear();
			// Выполняем загрузку данных
			this->_csv.dump(data);
			// Выполняем перебор полученного количества строк
			for(size_t i = 0; i < this->_csv.rows(); i++){
				// Получаем данные для добавления в файл
				result.append(this->_csv.row(i, delim));
				// Добавляем разделитель строки
				result.append("\r\n");
			}
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			// Выводим переданный лог
			this->_log->print("Parser: \"%s\" - %s [%s]", log_t::flag_t::CRITICAL, "CSV", error.what(), data.dump(4).c_str());
		}
	}
	// Выводим результат по умолчанию
	return result;
}
/**
 * csv Метод конвертации текста в формате CSV в объект JSON
 * @param text   текст для конвертации
 * @param header флаг формирования заголовков
 * @return       объект в формате JSON
 */
nlohmann::json anyks::Parser::csv(const string & text, const bool header) noexcept {
	// Если текст передан
	if(!text.empty()){
		// Выполняем блокировку потока
		const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем очистку контейнера
			this->_csv.clear();
			// Выполняем парсинг данных
			this->_csv = text;
			// Работаем с заголовком
			this->_csv.header(header);
			// Выполняем дамп данных в формате JSON
			return this->_csv.dump();
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			// Выводим переданный лог
			this->_log->print("Parser: \"%s\" - %s [%s]", log_t::flag_t::CRITICAL, "CSV", error.what(), text.c_str());
		}
	}
	// Выводим результат по умолчанию
	return nlohmann::json::object();
}
/**
 * xml Метод конвертации текста в формате XML в объект JSON
 * @param text текст для конвертации
 * @return     объект в формате JSON
 */
nlohmann::json anyks::Parser::xml(const string & text) noexcept {
	// Результат работы функции
	nlohmann::json result = nlohmann::json::object();
	// Если текст передан
	if(!text.empty()){
		// Выполняем блокировку потока
		const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем поиск первого символа
			const size_t pos1 = text.find("<");
			// Выполняем поиск последнего символа
			const size_t pos2 = text.rfind(">");
			// Если первый и последний символ получены
			if((pos1 != string::npos) && (pos2 != string::npos)){
				// Выполняем парсинг XML скрипта
				xmlDocPtr doc = xmlParseDoc(reinterpret_cast <const xmlChar *> (text.substr(pos1, (pos2 + 1) - pos1).c_str()));
				// Если парсинг не выполнен
				if(doc == nullptr){
					// Выводим переданный лог
					this->_log->print("document not parsed successfully", log_t::flag_t::CRITICAL);
					// Выводим результат
					return result;
				}
				// Выполняем получение корневой ноды
				xmlNodePtr node = xmlDocGetRootElement(doc);
				// Если корневая нода получена
				if(node != nullptr){
					/**
					 * parseFn Прототип функции парсинга XML документа
					 * @param таблица результатов
					 * @param объект текущей ноды
					 */
					function <void (nlohmann::json &, xmlNodePtr)> parseFn;
					/**
					 * parseFn Функция парсинга XML документа
					 * @param root корень объекта для записи результата
					 * @param node объект текущей ноды
					 */
					parseFn = [&parseFn, this](nlohmann::json & root, xmlNodePtr node) -> void {
						// Если переданная нода существует
						while(node != nullptr){
							// Если нода передана не системная
							if((node->type == XML_ELEMENT_NODE) && !xmlIsBlankNode(node)){
								// Если доступны дочерние ноды
								if(node->xmlChildrenNode != nullptr){
									// Если есть дочерние элементы у ноды
									if(xmlChildElementCount(node) > 0){
										// Если у ноды есть параметры
										if(node->properties == nullptr)
											// Выполняем создание нового объекта
											root[reinterpret_cast <const char *> (node->name)] = json::object();
										// Выполняем парсинг ноды дальше
										parseFn(root[reinterpret_cast <const char *> (node->name)], node->xmlChildrenNode);
									// Если дочерних элементов нет
									} else {
										// Если корневой элемент не является объектом
										if(!root.is_object())
											// Создаём объект
											root = nlohmann::json::object();
										// Если корень объекта уже содержит данные
										if(root[reinterpret_cast <const char *> (node->name)].is_object()){
											// Поулчаем текущие данные объекта
											nlohmann::json value = root[reinterpret_cast <const char *> (node->name)];
											// Создаём новый массив
											root[reinterpret_cast <const char *> (node->name)] = nlohmann::json::array();
											// Выполняем установку полученного значения
											root[reinterpret_cast <const char *> (node->name)].push_back(std::move(value));
											// Выполняем создание объекта внутри массива
											root[reinterpret_cast <const char *> (node->name)].push_back(nlohmann::json::object());
										// Если полученный объект не является массивом
										} else if(!root[reinterpret_cast <const char *> (node->name)].is_array())
											// Выполняем создание нового объекта	
											root[reinterpret_cast <const char *> (node->name)] = nlohmann::json::object();
										// Иначе добавляем в указанный индекс массива
										else if(root[reinterpret_cast <const char *> (node->name)].is_array())
											// Выполняем создание объекта внутри массива
											root[reinterpret_cast <const char *> (node->name)].push_back(nlohmann::json::object());
										// Если у ноды есть параметры
										if(node->properties != nullptr){
											// Получаем список атрибутов
											xmlAttr * attribute = node->properties;
											// Выполняем перебор всего списка атрибутов
											while((attribute != nullptr) && (attribute->name != nullptr) && (attribute->children != nullptr)){
												// Выполняем получение значения
												xmlChar * value = xmlNodeListGetString(node->doc, attribute->children, 1);
												// Если полученное значение является числом
												if(this->_fmk->is(reinterpret_cast <const char *> (value), fmk_t::check_t::NUMBER)){
													// Получаем значение числа
													const long long number = ::stoll(reinterpret_cast <const char *> (value));
													// Если число является отрицательным
													if(number < 0){
														// Если элемент не является массивом
														if(root[reinterpret_cast <const char *> (node->name)].is_object())
															// Выполняем формирования списка параметров
															root[reinterpret_cast <const char *> (node->name)][reinterpret_cast <const char *> (attribute->name)] = number;
														// Иначе добавляем в указанный индекс массива
														else if(root[reinterpret_cast <const char *> (node->name)].is_array())
															// Выполняем добавление названия атрибута
															root[reinterpret_cast <const char *> (node->name)].back().emplace(reinterpret_cast <const char *> (attribute->name), number);
													// Если число является положительным
													} else {
														// Если элемент не является массивом
														if(root[reinterpret_cast <const char *> (node->name)].is_object())
															// Выполняем формирования списка параметров
															root[reinterpret_cast <const char *> (node->name)][reinterpret_cast <const char *> (attribute->name)] = ::stoull(reinterpret_cast <const char *> (value));
														// Иначе добавляем в указанный индекс массива
														else if(root[reinterpret_cast <const char *> (node->name)].is_array())
															// Выполняем добавление названия атрибута
															root[reinterpret_cast <const char *> (node->name)].back().emplace(reinterpret_cast <const char *> (attribute->name), ::stoull(reinterpret_cast <const char *> (value)));
													}
												// Если полученное значение является числом с плавающей точкой
												} else if(this->_fmk->is(reinterpret_cast <const char *> (value), fmk_t::check_t::DECIMAL)) {
													// Если элемент не является массивом
													if(root[reinterpret_cast <const char *> (node->name)].is_object())
														// Выполняем формирования списка параметров
														root[reinterpret_cast <const char *> (node->name)][reinterpret_cast <const char *> (attribute->name)] = ::stod(reinterpret_cast <const char *> (value));
													// Иначе добавляем в указанный индекс массива
													else if(root[reinterpret_cast <const char *> (node->name)].is_array())
														// Выполняем добавление названия атрибута
														root[reinterpret_cast <const char *> (node->name)].back().emplace(reinterpret_cast <const char *> (attribute->name), ::stod(reinterpret_cast <const char *> (value)));
												// Если полученное значения является строкой
												} else {
													// Флаг булевого значения
													bool isTrue = false;
													// Если трока является булевым значением
													if((isTrue = this->_fmk->compare("true", reinterpret_cast <const char *> (value))) || this->_fmk->compare("false", reinterpret_cast <const char *> (value))){
														// Если элемент не является массивом
														if(root[reinterpret_cast <const char *> (node->name)].is_object())
															// Выполняем формирования списка параметров
															root[reinterpret_cast <const char *> (node->name)][reinterpret_cast <const char *> (attribute->name)] = (isTrue ? true : false);
														// Иначе добавляем в указанный индекс массива
														else if(root[reinterpret_cast <const char *> (node->name)].is_array())
															// Выполняем добавление названия атрибута
															root[reinterpret_cast <const char *> (node->name)].back().emplace(reinterpret_cast <const char *> (attribute->name), (isTrue ? true : false));
													// Если значение является обычной строкой
													} else {
														// Если элемент не является массивом
														if(root[reinterpret_cast <const char *> (node->name)].is_object())
															// Выполняем формирования списка параметров
															root[reinterpret_cast <const char *> (node->name)][reinterpret_cast <const char *> (attribute->name)] = reinterpret_cast <const char *> (value);
														// Иначе добавляем в указанный индекс массива
														else if(root[reinterpret_cast <const char *> (node->name)].is_array())
															// Выполняем добавление названия атрибута
															root[reinterpret_cast <const char *> (node->name)].back().emplace(reinterpret_cast <const char *> (attribute->name), reinterpret_cast <const char *> (value));
													}
												}
												// Выполняем итерацию по аттрибутам
												attribute = attribute->next;
												// Выполняем освобождение памяти выделенной под значение
												xmlFree(value);
											}
										}
										// Получаем ключ записи
										string key("value");
										// Если элемент не является массивом
										if(root[reinterpret_cast <const char *> (node->name)].is_object()){
											// Если такой ключ уже существует в объекте
											while(root[reinterpret_cast <const char *> (node->name)].contains(key))
												// Выполняем изменение ключа
												key.insert(key.begin(), '_');
										// Иначе добавляем в указанный индекс массива
										} else if(root[reinterpret_cast <const char *> (node->name)].is_array()) {
											// Если такой ключ уже существует в объекте
											while(root[reinterpret_cast <const char *> (node->name)].back().contains(key))
												// Выполняем изменение ключа
												key.insert(key.begin(), '_');
										}
										// Выполняем получение значения
										xmlChar * value = xmlNodeListGetString(node->doc, node->xmlChildrenNode, 1);
										// Если полученное значение является числом
										if(this->_fmk->is(reinterpret_cast <const char *> (value), fmk_t::check_t::NUMBER)){
											// Получаем значение числа
											const long long number = ::stoll(reinterpret_cast <const char *> (value));
											// Если число является отрицательным
											if(number < 0){
												// Если элемент не является массивом
												if(root[reinterpret_cast <const char *> (node->name)].is_object()){
													// Если у ноды есть параметры
													if(node->properties != nullptr)
														// Выполняем формирования списка параметров
														root[reinterpret_cast <const char *> (node->name)][key] = number;
													// Выполняем установку полученного значения
													else root[reinterpret_cast <const char *> (node->name)] = number;
												// Иначе добавляем в указанный индекс массива
												} else if(root[reinterpret_cast <const char *> (node->name)].is_array())
													// Выполняем добавление названия атрибута
													root[reinterpret_cast <const char *> (node->name)].back().emplace(key, number);
											// Если число является положительным
											} else {
												// Если элемент не является массивом
												if(root[reinterpret_cast <const char *> (node->name)].is_object()){
													// Если у ноды есть параметры
													if(node->properties != nullptr)
														// Выполняем формирования списка параметров
														root[reinterpret_cast <const char *> (node->name)][key] = ::stoull(reinterpret_cast <const char *> (value));
													// Выполняем установку полученного значения
													else root[reinterpret_cast <const char *> (node->name)] = ::stoull(reinterpret_cast <const char *> (value));
												// Иначе добавляем в указанный индекс массива
												} else if(root[reinterpret_cast <const char *> (node->name)].is_array())
													// Выполняем добавление названия атрибута
													root[reinterpret_cast <const char *> (node->name)].back().emplace(key, ::stoull(reinterpret_cast <const char *> (value)));
											}
										// Если полученное значение является числом с плавающей точкой
										} else if(this->_fmk->is(reinterpret_cast <const char *> (value), fmk_t::check_t::DECIMAL)) {
											// Если элемент не является массивом
											if(root[reinterpret_cast <const char *> (node->name)].is_object()){
												// Если у ноды есть параметры
												if(node->properties != nullptr)
													// Выполняем формирования списка параметров
													root[reinterpret_cast <const char *> (node->name)][key] = ::stod(reinterpret_cast <const char *> (value));
												// Выполняем установку полученного значения
												else root[reinterpret_cast <const char *> (node->name)] = ::stod(reinterpret_cast <const char *> (value));
											// Иначе добавляем в указанный индекс массива
											} else if(root[reinterpret_cast <const char *> (node->name)].is_array())
												// Выполняем добавление названия атрибута
												root[reinterpret_cast <const char *> (node->name)].back().emplace(key, ::stod(reinterpret_cast <const char *> (value)));
										// Если значение не является числом
										} else {
											// Флаг булевого значения
											bool isTrue = false;
											// Если трока является булевым значением
											if((isTrue = this->_fmk->compare("true", reinterpret_cast <const char *> (value))) || this->_fmk->compare("false", reinterpret_cast <const char *> (value))){
												// Если элемент не является массивом
												if(root[reinterpret_cast <const char *> (node->name)].is_object()){
													// Если у ноды есть параметры
													if(node->properties != nullptr)
														// Выполняем формирования списка параметров
														root[reinterpret_cast <const char *> (node->name)][key] = (isTrue ? true : false);
													// Выполняем установку полученного значения
													else root[reinterpret_cast <const char *> (node->name)] = (isTrue ? true : false);
												// Иначе добавляем в указанный индекс массива
												} else if(root[reinterpret_cast <const char *> (node->name)].is_array())
													// Выполняем добавление названия атрибута
													root[reinterpret_cast <const char *> (node->name)].back().emplace(key, (isTrue ? true : false));
											// Если значение является обычной строкой
											} else {
												// Если элемент не является массивом
												if(root[reinterpret_cast <const char *> (node->name)].is_object()){
													// Если у ноды есть параметры
													if(node->properties != nullptr)
														// Выполняем формирования списка параметров
														root[reinterpret_cast <const char *> (node->name)][key] = reinterpret_cast <const char *> (value);
													// Выполняем установку полученного значения
													else root[reinterpret_cast <const char *> (node->name)] = reinterpret_cast <const char *> (value);
												// Иначе добавляем в указанный индекс массива
												} else if(root[reinterpret_cast <const char *> (node->name)].is_array())
													// Выполняем добавление названия атрибута
													root[reinterpret_cast <const char *> (node->name)].back().emplace(key, reinterpret_cast <const char *> (value));
											}
										}
										// Выполняем освобождение памяти выделенной под значение
										xmlFree(value);
									}
								// Если дочерних элементов не обнаружено
								} else {
									// Выполняем создание нового объекта
									root[reinterpret_cast <const char *> (node->name)] = nlohmann::json::object();
									// Если у ноды есть параметры
									if(node->properties != nullptr){
										// Получаем список атрибутов
										xmlAttr * attribute = node->properties;
										// Выполняем перебор всего списка атрибутов
										while((attribute != nullptr) && (attribute->name != nullptr) && (attribute->children != nullptr)){
											// Выполняем получение значения
											xmlChar * value = xmlNodeListGetString(node->doc, attribute->children, 1);
											// Если полученное значение является числом
											if(this->_fmk->is(reinterpret_cast <const char *> (value), fmk_t::check_t::NUMBER)){
												// Получаем значение числа
												const long long number = ::stoll(reinterpret_cast <const char *> (value));
												// Если число является отрицательным
												if(number < 0)
													// Выполняем формирования списка параметров
													root[reinterpret_cast <const char *> (node->name)][reinterpret_cast <const char *> (attribute->name)] = number;
												// Выполняем формирования списка параметров
												else root[reinterpret_cast <const char *> (node->name)][reinterpret_cast <const char *> (attribute->name)] = ::stoull(reinterpret_cast <const char *> (value));
											// Если полученное значение является числом с плавающей точкой
											} else if(this->_fmk->is(reinterpret_cast <const char *> (value), fmk_t::check_t::DECIMAL))
												// Выполняем формирования списка параметров
												root[reinterpret_cast <const char *> (node->name)][reinterpret_cast <const char *> (attribute->name)] = ::stod(reinterpret_cast <const char *> (value));
											// Если значение является строковым
											else {
												// Флаг булевого значения
												bool isTrue = false;
												// Если трока является булевым значением
												if((isTrue = this->_fmk->compare("true", reinterpret_cast <const char *> (value))) || this->_fmk->compare("false", reinterpret_cast <const char *> (value)))
													// Выполняем формирования списка параметров
													root[reinterpret_cast <const char *> (node->name)][reinterpret_cast <const char *> (attribute->name)] = (isTrue ? true : false);
												// Выполняем формирования списка параметров
												else root[reinterpret_cast <const char *> (node->name)][reinterpret_cast <const char *> (attribute->name)] = reinterpret_cast <const char *> (value);
											}
											// Выполняем итерацию по аттрибутам
											attribute = attribute->next;
											// Выполняем освобождение памяти выделенной под значение
											xmlFree(value);
										}
									// Устанавливаем значение как булевое
									} else root[reinterpret_cast <const char *> (node->name)] = true;
								}
							}
							// Выполняем итерацию ноды
							node = node->next;
						}
					};
					// Устанавливаем в корневой объект наш первый параметр
					result.emplace(reinterpret_cast <const char *> (node->name), nlohmann::json::object());
					// Если у ноды есть параметры
					if(node->properties != nullptr){
						// Получаем список атрибутов
						xmlAttr * attribute = node->properties;
						// Выполняем создание нового объекта
						result[reinterpret_cast <const char *> (node->name)] = nlohmann::json::object();
						// Выполняем перебор всего списка атрибутов
						while((attribute != nullptr) && (attribute->name != nullptr) && (attribute->children != nullptr)){
							// Выполняем получение значения
							xmlChar * value = xmlNodeListGetString(node->doc, attribute->children, 1);
							// Если полученное значение является числом
							if(this->_fmk->is(reinterpret_cast <const char *> (value), fmk_t::check_t::NUMBER)){
								// Получаем значение числа
								const long long number = ::stoll(reinterpret_cast <const char *> (value));
								// Если число является отрицательным
								if(number < 0)
									// Выполняем формирования списка параметров
									result[reinterpret_cast <const char *> (node->name)][reinterpret_cast <const char *> (attribute->name)] = number;
								// Выполняем формирования списка параметров
								else result[reinterpret_cast <const char *> (node->name)][reinterpret_cast <const char *> (attribute->name)] = ::stoull(reinterpret_cast <const char *> (value));
							// Если полученное значение является числом с плавающей точкой
							} else if(this->_fmk->is(reinterpret_cast <const char *> (value), fmk_t::check_t::DECIMAL))
								// Выполняем формирования списка параметров
								result[reinterpret_cast <const char *> (node->name)][reinterpret_cast <const char *> (attribute->name)] = ::stod(reinterpret_cast <const char *> (value));
							// Если значение является строковым
							else {
								// Флаг булевого значения
								bool isTrue = false;
								// Если трока является булевым значением
								if((isTrue = this->_fmk->compare("true", reinterpret_cast <const char *> (value))) || this->_fmk->compare("false", reinterpret_cast <const char *> (value)))
									// Выполняем формирования списка параметров
									result[reinterpret_cast <const char *> (node->name)][reinterpret_cast <const char *> (attribute->name)] = (isTrue ? true : false);
								// Выполняем формирования списка параметров
								else result[reinterpret_cast <const char *> (node->name)][reinterpret_cast <const char *> (attribute->name)] = reinterpret_cast <const char *> (value);
							}
							// Выполняем итерацию по аттрибутам
							attribute = attribute->next;
							// Выполняем освобождение памяти выделенной под значение
							xmlFree(value);
						}
					}
					// Выполняем парсинг всего XML объекта
					parseFn(result[reinterpret_cast <const char *> (node->name)], node->xmlChildrenNode);
				// Сообщаем, что переданные данные не соответствуют ожидаемым
				} else this->_log->print("Method: \"%s\" - data received is not as expected", log_t::flag_t::WARNING, "PARSER:xml");
				// Выполняем очистку выделенной памяти под ноду
				// xmlFreeNode(node);
				// Выполняем очистку выделенных данных парсера
				xmlFreeDoc(doc);
				// Выполняем очистку глобальных параметров парсера
				xmlCleanupParser();
			// Сообщаем, что переданные данные не соответствуют ожидаемым
			} else this->_log->print("Method: \"%s\" - data received is not as expected", log_t::flag_t::WARNING, "Parser:xml");
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			// Выводим переданный лог
			this->_log->print("Parser: \"%s\" - %s [%s]", log_t::flag_t::CRITICAL, "XML", error.what(), text.c_str());
		}
	}
	// Выводим результат по умолчанию
	return result;
}
/**
 * xml Метод конвертации объекта JSON в текст в формате XML
 * @param data   данные в объекте JSON
 * @param pretty флаг генерации читаемого формата
 * @return       текст после конвертации
 */
string anyks::Parser::xml(const nlohmann::json & data, const bool pretty) noexcept {
	// Результат работы функции
	string result = "";
	// Если данные переданы
	if((data.is_object() || data.is_array()) && !data.empty()){
		// Выполняем блокировку потока
		const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Результат работы функции
			result = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>";
			// Если разрешено выполнять разложение XML-объекта
			if(pretty)
				// Выполняем добавление переноса строк
				result.append(1, '\n');
			/**
			 * parseFn Прототип функции парсинга XML документа
			 * @param таблица результатов
			 * @param объект текущей ноды
			 * @param количество отступов
			 */
			function <void (string &, const nlohmann::json &, const uint16_t)> parseFn;
			/**
			 * parseFn Функция парсинга XML документа
			 * @param root корень объекта для записи результата
			 * @param node объект текущей ноды
			 * @param tabs количество отступов
			 */
			parseFn = [pretty, &parseFn, this](string & root, const nlohmann::json & node, const uint16_t tabs) -> void {
				// Создаём объект результата
				string result = "";
				// Выполняем перебор всего объекта
				for(auto & el : node.items()){
					// Если значение является числом
					if(el.value().is_number()){
						// Если количество отступов больше нуля
						if(pretty && (tabs > 0)){
							// Выполняем установку количества отступов
							for(uint16_t i = 0; i < tabs; i++)
								// Выполняем добавление отступов
								result.append(1, '\t');
						}
						// Выполняем открытие тега
						result.append(1, '<');
						// Выполняем формирование результата
						result.append(el.key());
						// Выполняем закрытие тега
						result.append(1, '>');
						// Временное значение переменной
						double intpart = 0;
						// Выполняем проверку есть ли дробная часть у числа
						if(::modf(el.value().get <double> (), &intpart) == 0){
							// Получаем целочисленные данные
							const long long number = el.value().get <long long> ();
							// Если число отрицательное
							if(number < 0)
								// Выполняем получение числа с учётом знака
								result.append(std::to_string(number));
							// Выводим беззнаковое число
							else result.append(std::to_string(el.value().get <unsigned long long> ()));
						// Если у числа имеется дробная часть
						} else result.append(std::to_string(el.value().get <double> ()));
						// Выполняем закрытие тега
						result.append("</");
						// Выполняем установку тега
						result.append(el.key());
						// Выполняем закрытие тега
						result.append(1, '>');
						// Если разрешено выполнять разложение XML-объекта
						if(pretty)
							// Выполняем добавление переноса строк
							result.append(1, '\n');
					// Если значение является строкой
					} else if(el.value().is_string()) {
						// Если количество отступов больше нуля
						if(pretty && (tabs > 0)){
							// Выполняем установку количества отступов
							for(uint16_t i = 0; i < tabs; i++)
								// Выполняем добавление отступов
								result.append(1, '\t');
						}
						// Выполняем открытие тега
						result.append(1, '<');
						// Выполняем формирование результата
						result.append(el.key());
						// Выполняем закрытие тега
						result.append(1, '>');
						// Выполняем установку строки
						result.append(el.value().get <string> ());
						// Выполняем закрытие тега
						result.append("</");
						// Выполняем установку тега
						result.append(el.key());
						// Выполняем закрытие тега
						result.append(1, '>');
						// Если разрешено выполнять разложение XML-объекта
						if(pretty)
							// Выполняем добавление переноса строк
							result.append(1, '\n');
					// Если значение является булевым значением
					} else if(el.value().is_boolean()) {
						// Если количество отступов больше нуля
						if(pretty && (tabs > 0)){
							// Выполняем установку количества отступов
							for(uint16_t i = 0; i < tabs; i++)
								// Выполняем добавление отступов
								result.append(1, '\t');
						}
						// Если значение истинное
						if(el.value().get <bool> ()){
							// Выполняем открытие тега
							result.append(1, '<');
							// Выполняем формирование результата
							result.append(el.key());
							// Выполняем закрытие тега
							result.append("/>");
						// Если значение ложное
						} else if(!el.value().get <bool> ()) {
							// Выполняем открытие тега
							result.append(1, '<');
							// Выполняем формирование результата
							result.append(el.key());
							// Выполняем закрытие тега
							result.append(1, '>');
							// Выполняем установку строки
							result.append("False");
							// Выполняем закрытие тега
							result.append("</");
							// Выполняем установку тега
							result.append(el.key());
							// Выполняем закрытие тега
							result.append(1, '>');
						}
						// Если разрешено выполнять разложение XML-объекта
						if(pretty)
							// Выполняем добавление переноса строк
							result.append(1, '\n');
					// Если значение является пустым значением
					} else if(el.value().is_null()) {
						// Если количество отступов больше нуля
						if(pretty && (tabs > 0)){
							// Выполняем установку количества отступов
							for(uint16_t i = 0; i < tabs; i++)
								// Выполняем добавление отступов
								result.append(1, '\t');
						}
						// Выполняем открытие тега
						result.append(1, '<');
						// Выполняем формирование результата
						result.append(el.key());
						// Выполняем закрытие тега
						result.append(1, '>');
						// Выполняем установку строки
						result.append("Null");
						// Выполняем закрытие тега
						result.append("</");
						// Выполняем установку тега
						result.append(el.key());
						// Выполняем закрытие тега
						result.append(1, '>');
						// Если разрешено выполнять разложение XML-объекта
						if(pretty)
							// Выполняем добавление переноса строк
							result.append(1, '\n');
					// Если значение является объектом
					} else if(el.value().is_object()) {
						// Если количество отступов больше нуля
						if(pretty && (tabs > 0)){
							// Выполняем установку количества отступов
							for(uint16_t i = 0; i < tabs; i++)
								// Выполняем добавление отступов
								result.append(1, '\t');
						}
						// Выполняем открытие тега
						result.append(1, '<');
						// Выполняем формирование результата
						result.append(el.key());
						// Флаг формирования сложного тега
						bool difficult = false;
						// Выполняем поиск вложенных объектов и массивов
						for(auto & item : el.value().items()){
							// Если найден массив или объект
							if((difficult = (item.value().is_object() || item.value().is_array())))
								// Выходим из цикла
								break;
						}
						// Если нужно сформировать простой тег
						if(!difficult){
							// Значение записи тега
							string value = "";
							// Позиция ключа поиска
							size_t pos = string::npos, count = 0;
							// Выполняем перебор всех значений объекта
							for(auto & item : el.value().items()){
								// Если ключом является устанавливаемое значение
								if((pos = item.key().rfind("value")) != string::npos){
									// Если значение ещё не установлено
									if(value.empty() || (pos > count)){
										// Если значение уже установлено и количество подчеркиваний больше чем было
										if(!value.empty() && (pos > count)){
											// Выполняем добавление разделителя параметра
											result.append(1, ' ');
											// Выполняем добавление ключа записи
											result.append("value");
											// Выполняем добавление знака присвоения
											result.append("=\"");
											// Выполняем добавление установленного значения
											result.append(value);
											// Выполняем добавление экранирование параметра
											result.append(1, '"');
										}
										// Запоминаем количество найденных подчеркиваний
										count = pos;
										// Если значение является числом
										if(item.value().is_number()){
											// Временное значение переменной
											double intpart = 0;
											// Выполняем проверку есть ли дробная часть у числа
											if(::modf(item.value().get <double> (), &intpart) == 0){
												// Получаем целочисленные данные
												const long long number = item.value().get <long long> ();
												// Если число отрицательное
												if(number < 0)
													// Выполняем получение числа с учётом знака
													value = std::to_string(number);
												// Выводим беззнаковое число
												else value = std::to_string(item.value().get <unsigned long long> ());
											// Если у числа имеется дробная часть
											} else value = std::to_string(item.value().get <double> ());
											// Продолжаем дальше
											continue;
										// Если значение является строкой
										} else if(item.value().is_string()) {
											// Выполняем установку полученного значения
											value = item.value().get <string> ();
											// Продолжаем дальше
											continue;
										// Если значение является булевым значением
										} else if(item.value().is_boolean()) {
											// Выполняем установку полученного значения
											value = (item.value().get <bool> () ? "True" : "False");
											// Продолжаем дальше
											continue;
										// Если значение является пустым значением
										} else if(item.value().is_null()) {
											// Выполняем установку полученного значения
											value = "Null";
											// Продолжаем дальше
											continue;
										}
									}
								}
								// Если значение является числом
								if(item.value().is_number()){
									// Выполняем добавление разделителя параметра
									result.append(1, ' ');
									// Выполняем добавление ключа записи
									result.append(item.key());
									// Выполняем добавление знака присвоения
									result.append("=\"");
									// Временное значение переменной
									double intpart = 0;
									// Выполняем проверку есть ли дробная часть у числа
									if(::modf(item.value().get <double> (), &intpart) == 0){
										// Получаем целочисленные данные
										const long long number = item.value().get <long long> ();
										// Если число отрицательное
										if(number < 0)
											// Выполняем получение числа с учётом знака
											result.append(std::to_string(number));
										// Выводим беззнаковое число
										else result.append(std::to_string(item.value().get <unsigned long long> ()));
									// Если у числа имеется дробная часть
									} else result.append(std::to_string(item.value().get <double> ()));
									// Выполняем добавление экранирование параметра
									result.append(1, '"');
								// Если значение является строкой
								} else if(item.value().is_string()) {
									// Выполняем добавление разделителя параметра
									result.append(1, ' ');
									// Выполняем добавление ключа записи
									result.append(item.key());
									// Выполняем добавление знака присвоения
									result.append("=\"");
									// Выполняем добавление значения
									result.append(item.value().get <string> ());
									// Выполняем добавление экранирование параметра
									result.append(1, '"');
								// Если значение является булевым значением
								} else if(item.value().is_boolean()) {
									// Выполняем добавление разделителя параметра
									result.append(1, ' ');
									// Выполняем добавление ключа записи
									result.append(item.key());
									// Выполняем добавление знака присвоения
									result.append("=\"");
									// Выполняем добавление значения
									result.append(item.value().get <bool> () ? "True" : "False");
									// Выполняем добавление экранирование параметра
									result.append(1, '"');
								// Если значение является пустым значением
								} else if(item.value().is_null()) {
									// Выполняем добавление разделителя параметра
									result.append(1, ' ');
									// Выполняем добавление ключа записи
									result.append(item.key());
									// Выполняем добавление знака присвоения
									result.append("=\"");
									// Выполняем добавление значения
									result.append("Null");
									// Выполняем добавление экранирование параметра
									result.append(1, '"');
								}
							}
							// Если значение не получено
							if(value.empty())
								// Выполняем установку закрывающего тега
								result.append("/>");
							// Если значение получено
							else {
								// Выполняем закрытие тега
								result.append(1, '>');
								// Выполняем добавление установленного значения
								result.append(value);
								// Выполняем закрытие тега
								result.append("</");
								// Выполняем установку тега
								result.append(el.key());
								// Выполняем закрытие тега
								result.append(1, '>');
							}
						// Если необходимо сформировать расширенный XML
						} else {
							// Выполняем закрытие тега
							result.append(1, '>');
							// Если разрешено выполнять разложение XML-объекта
							if(pretty)
								// Выполняем добавление переноса строк
								result.append(1, '\n');
							// Выполняем извлечение данных объекта
							parseFn(result, el.value(), tabs + 1);
							// Если количество отступов больше нуля
							if(pretty && (tabs > 0)){
								// Выполняем установку количества отступов
								for(uint16_t i = 0; i < tabs; i++)
									// Выполняем добавление отступов
									result.append(1, '\t');
							}
							// Выполняем закрытие тега
							result.append("</");
							// Выполняем установку тега
							result.append(el.key());
							// Выполняем закрытие тега
							result.append(1, '>');
						}
						// Если разрешено выполнять разложение XML-объекта
						if(pretty)
							// Выполняем добавление переноса строк
							result.append(1, '\n');
					// Если значение является массивом
					} else if(el.value().is_array()) {
						// Выполняем переход по всему массиву
						for(auto & item : el.value()){
							// Если значение является числом
							if(item.is_number()){
								// Если количество отступов больше нуля
								if(pretty && (tabs > 0)){
									// Выполняем установку количества отступов
									for(uint16_t i = 0; i < tabs; i++)
										// Выполняем добавление отступов
										result.append(1, '\t');
								}
								// Выполняем открытие тега
								result.append(1, '<');
								// Выполняем формирование результата
								result.append(el.key());
								// Выполняем закрытие тега
								result.append(1, '>');
								// Временное значение переменной
								double intpart = 0;
								// Выполняем проверку есть ли дробная часть у числа
								if(::modf(item.get <double> (), &intpart) == 0){
									// Получаем целочисленные данные
									const long long number = item.get <long long> ();
									// Если число отрицательное
									if(number < 0)
										// Выполняем получение числа с учётом знака
										result.append(std::to_string(number));
									// Выводим беззнаковое число
									else result.append(std::to_string(item.get <unsigned long long> ()));
								// Если у числа имеется дробная часть
								} else result.append(std::to_string(item.get <double> ()));
								// Выполняем закрытие тега
								result.append("</");
								// Выполняем установку тега
								result.append(el.key());
								// Выполняем закрытие тега
								result.append(1, '>');
								// Если разрешено выполнять разложение XML-объекта
								if(pretty)
									// Выполняем добавление переноса строк
									result.append(1, '\n');
							// Если значение является строкой
							} else if(item.is_string()) {
								// Если количество отступов больше нуля
								if(pretty && (tabs > 0)){
									// Выполняем установку количества отступов
									for(uint16_t i = 0; i < tabs; i++)
										// Выполняем добавление отступов
										result.append(1, '\t');
								}
								// Выполняем открытие тега
								result.append(1, '<');
								// Выполняем формирование результата
								result.append(el.key());
								// Выполняем закрытие тега
								result.append(1, '>');
								// Выполняем установку строки
								result.append(item.get <string> ());
								// Выполняем закрытие тега
								result.append("</");
								// Выполняем установку тега
								result.append(el.key());
								// Выполняем закрытие тега
								result.append(1, '>');
								// Если разрешено выполнять разложение XML-объекта
								if(pretty)
									// Выполняем добавление переноса строк
									result.append(1, '\n');
							// Если значение является булевым значением
							} else if(item.is_boolean()) {
								// Если количество отступов больше нуля
								if(pretty && (tabs > 0)){
									// Выполняем установку количества отступов
									for(uint16_t i = 0; i < tabs; i++)
										// Выполняем добавление отступов
										result.append(1, '\t');
								}
								// Выполняем открытие тега
								result.append(1, '<');
								// Выполняем формирование результата
								result.append(el.key());
								// Выполняем закрытие тега
								result.append(1, '>');
								// Выполняем установку строки
								result.append(item.get <bool> () ? "True" : "False");
								// Выполняем закрытие тега
								result.append("</");
								// Выполняем установку тега
								result.append(el.key());
								// Выполняем закрытие тега
								result.append(1, '>');
								// Если разрешено выполнять разложение XML-объекта
								if(pretty)
									// Выполняем добавление переноса строк
									result.append(1, '\n');
							// Если значение является пустым значением
							} else if(item.is_null()) {
								// Если количество отступов больше нуля
								if(pretty && (tabs > 0)){
									// Выполняем установку количества отступов
									for(uint16_t i = 0; i < tabs; i++)
										// Выполняем добавление отступов
										result.append(1, '\t');
								}
								// Выполняем открытие тега
								result.append(1, '<');
								// Выполняем формирование результата
								result.append(el.key());
								// Выполняем закрытие тега
								result.append(1, '>');
								// Выполняем установку строки
								result.append("Null");
								// Выполняем закрытие тега
								result.append("</");
								// Выполняем установку тега
								result.append(el.key());
								// Выполняем закрытие тега
								result.append(1, '>');
								// Если разрешено выполнять разложение XML-объекта
								if(pretty)
									// Выполняем добавление переноса строк
									result.append(1, '\n');
							// Если значение является объектом
							} else if(item.is_object()) {
								// Если количество отступов больше нуля
								if(pretty && (tabs > 0)){
									// Выполняем установку количества отступов
									for(uint16_t i = 0; i < tabs; i++)
										// Выполняем добавление отступов
										result.append(1, '\t');
								}
								// Выполняем открытие тега
								result.append(1, '<');
								// Выполняем формирование результата
								result.append(el.key());
								// Флаг формирования сложного тега
								bool difficult = false;
								// Выполняем поиск вложенных объектов и массивов
								for(auto & el : item.items()){
									// Если найден массив или объект
									if((difficult = (el.value().is_object() || el.value().is_array())))
										// Выходим из цикла
										break;
								}
								// Если нужно сформировать простой тег
								if(!difficult){
									// Значение записи тега
									string value = "";
									// Позиция ключа поиска
									size_t pos = string::npos, count = 0;
									// Выполняем перебор всех значений объекта
									for(auto & el : item.items()){
										// Если ключом является устанавливаемое значение
										if((pos = el.key().rfind("value")) != string::npos){
											// Если значение ещё не установлено
											if(value.empty() || (pos > count)){
												// Если значение уже установлено и количество подчеркиваний больше чем было
												if(!value.empty() && (pos > count)){
													// Выполняем добавление разделителя параметра
													result.append(1, ' ');
													// Выполняем добавление ключа записи
													result.append("value");
													// Выполняем добавление знака присвоения
													result.append("=\"");
													// Выполняем добавление установленного значения
													result.append(value);
													// Выполняем добавление экранирование параметра
													result.append(1, '"');
												}
												// Запоминаем количество найденных подчеркиваний
												count = pos;
												// Если значение является числом
												if(el.value().is_number()){
													// Временное значение переменной
													double intpart = 0;
													// Выполняем проверку есть ли дробная часть у числа
													if(::modf(el.value().get <double> (), &intpart) == 0){
														// Получаем целочисленные данные
														const long long number = el.value().get <long long> ();
														// Если число отрицательное
														if(number < 0)
															// Выполняем получение числа с учётом знака
															value = std::to_string(number);
														// Выводим беззнаковое число
														else value = std::to_string(el.value().get <unsigned long long> ());
													// Если у числа имеется дробная часть
													} else value = std::to_string(el.value().get <double> ());
													// Продолжаем дальше
													continue;
												// Если значение является строкой
												} else if(el.value().is_string()) {
													// Выполняем установку полученного значения
													value = el.value().get <string> ();
													// Продолжаем дальше
													continue;
												// Если значение является булевым значением
												} else if(el.value().is_boolean()) {
													// Выполняем установку полученного значения
													value = (el.value().get <bool> () ? "True" : "False");
													// Продолжаем дальше
													continue;
												// Если значение является пустым значением
												} else if(el.value().is_null()) {
													// Выполняем установку полученного значения
													value = "Null";
													// Продолжаем дальше
													continue;
												}
											}
										}
										// Если значение является числом
										if(el.value().is_number()){
											// Выполняем добавление разделителя параметра
											result.append(1, ' ');
											// Выполняем добавление ключа записи
											result.append(el.key());
											// Выполняем добавление знака присвоения
											result.append("=\"");
											// Временное значение переменной
											double intpart = 0;
											// Выполняем проверку есть ли дробная часть у числа
											if(::modf(el.value().get <double> (), &intpart) == 0){
												// Получаем целочисленные данные
												const long long number = el.value().get <long long> ();
												// Если число отрицательное
												if(number < 0)
													// Выполняем получение числа с учётом знака
													result.append(std::to_string(number));
												// Выводим беззнаковое число
												else result.append(std::to_string(el.value().get <unsigned long long> ()));
											// Если у числа имеется дробная часть
											} else result.append(std::to_string(el.value().get <double> ()));
											// Выполняем добавление экранирование параметра
											result.append(1, '"');
										// Если значение является строкой
										} else if(el.value().is_string()) {
											// Выполняем добавление разделителя параметра
											result.append(1, ' ');
											// Выполняем добавление ключа записи
											result.append(el.key());
											// Выполняем добавление знака присвоения
											result.append("=\"");
											// Выполняем добавление значения
											result.append(el.value().get <string> ());
											// Выполняем добавление экранирование параметра
											result.append(1, '"');
										// Если значение является булевым значением
										} else if(el.value().is_boolean()) {
											// Выполняем добавление разделителя параметра
											result.append(1, ' ');
											// Выполняем добавление ключа записи
											result.append(el.key());
											// Выполняем добавление знака присвоения
											result.append("=\"");
											// Выполняем добавление значения
											result.append(el.value().get <bool> () ? "True" : "False");
											// Выполняем добавление экранирование параметра
											result.append(1, '"');
										// Если значение является пустым значением
										} else if(el.value().is_null()) {
											// Выполняем добавление разделителя параметра
											result.append(1, ' ');
											// Выполняем добавление ключа записи
											result.append(el.key());
											// Выполняем добавление знака присвоения
											result.append("=\"");
											// Выполняем добавление значения
											result.append("Null");
											// Выполняем добавление экранирование параметра
											result.append(1, '"');
										}
									}
									// Если значение не получено
									if(value.empty())
										// Выполняем установку закрывающего тега
										result.append("/>");
									// Если значение получено
									else {
										// Выполняем закрытие тега
										result.append(1, '>');
										// Выполняем добавление установленного значения
										result.append(value);
										// Выполняем закрытие тега
										result.append("</");
										// Выполняем установку тега
										result.append(el.key());
										// Выполняем закрытие тега
										result.append(1, '>');
									}
								// Если необходимо сформировать расширенный XML
								} else {
									// Выполняем закрытие тега
									result.append(1, '>');
									// Если разрешено выполнять разложение XML-объекта
									if(pretty)
										// Выполняем добавление переноса строк
										result.append(1, '\n');
									// Выполняем извлечение данных объекта
									parseFn(result, item, tabs + 1);
									// Если количество отступов больше нуля
									if(pretty && (tabs > 0)){
										// Выполняем установку количества отступов
										for(uint16_t i = 0; i < tabs; i++)
											// Выполняем добавление отступов
											result.append(1, '\t');
									}
									// Выполняем закрытие тега
									result.append("</");
									// Выполняем установку тега
									result.append(el.key());
									// Выполняем закрытие тега
									result.append(1, '>');
								}
								// Если разрешено выполнять разложение XML-объекта
								if(pretty)
									// Выполняем добавление переноса строк
									result.append(1, '\n');
							// Если значение является массивом
							} else if(item.is_array()) {
								// Если количество отступов больше нуля
								if(pretty && (tabs > 0)){
									// Выполняем установку количества отступов
									for(uint16_t i = 0; i < tabs; i++)
										// Выполняем добавление отступов
										result.append(1, '\t');
								}
								// Формируем объект объекта
								nlohmann::json obj = nlohmann::json::object();
								// Добавляем полученный объекта
								obj.emplace(el.key(), item);
								// Выполняем извлечение данных объекта
								parseFn(result, obj, tabs + 1);
								// Если разрешено выполнять разложение XML-объекта
								if(pretty)
									// Выполняем добавление переноса строк
									result.append(1, '\n');
							}
						}
					}
				}
				// Если результат сформирован
				if(!result.empty())
					// Формируем объект ключа
					root.append(result);
			};
			// Выполняем парсинг объекта XML
			parseFn(result, data, 0);
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			// Выводим переданный лог
			this->_log->print("Parser: \"%s\" - %s [%s]", log_t::flag_t::CRITICAL, "XML", error.what(), data.dump(4).c_str());
		}
	}
	// Выводим результат по умолчанию
	return result;
}
/**
 * json Метод конвертации текста в формате JSON в объект JSON
 * @param text текст для конвертации
 * @return     объект в формате JSON
 */
nlohmann::json anyks::Parser::json(const string & text) noexcept {
	// Если текст передан
	if(!text.empty()){
		// Выполняем блокировку потока
		const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем создание объекта JSON
			return nlohmann::json::parse(text);
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			// Выводим переданный лог
			this->_log->print("Parser: \"%s\" - %s [%s]", log_t::flag_t::CRITICAL, "JSON", error.what(), text.c_str());
		}
	}
	// Выводим результат по умолчанию
	return nlohmann::json::object();
}
/**
 * json Метод конвертации объекта JSON в текст в формате JSON
 * @param data   данные в объекте JSON
 * @param pretty флаг генерации читаемого формата
 * @return       текст после конвертации
 */
string anyks::Parser::json(const nlohmann::json & data, const bool pretty) noexcept {
	// Если данные переданы
	if((data.is_object() || data.is_array()) && !data.empty()){
		// Выполняем блокировку потока
		const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выводим полученный результат
			return (pretty ? data.dump(4) : data.dump());
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			// Выводим переданный лог
			this->_log->print("Parser: \"%s\" - %s [%s]", log_t::flag_t::CRITICAL, "JSON", error.what(), data.dump(4).c_str());
		}
	}
	// Выводим результат по умолчанию
	return "";
}
/**
 * cef Метод конвертации текста в формате CEF в объект JSON
 * @param text текст для конвертации
 * @param mode режим парсинга
 * @return     объект в формате JSON
 */
nlohmann::json anyks::Parser::cef(const string & text, const cef_t::mode_t mode) noexcept {
	// Если текст передан
	if(!text.empty()){
		// Выполняем блокировку потока
		const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем очистку объекта
			this->_cef.clear();
			// Устанавливаем строгий режим работы
			this->_cef.mode(mode);
			// Выполняем парсинг текста
			this->_cef = text;
			// Выводим полученный результат
			return this->_cef.dump();
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			// Выводим переданный лог
			this->_log->print("Parser: \"%s\" - %s [%s]", log_t::flag_t::CRITICAL, "CEF", error.what(), text.c_str());
		}
	}
	// Выводим результат по умолчанию
	return nlohmann::json::object();
}
/**
 * cef Метод конвертации объекта JSON в текст в формате CEF
 * @param data данные в объекте JSON
 * @param mode режим парсинга
 * @return     текст после конвертации
 */
string anyks::Parser::cef(const nlohmann::json & data, const cef_t::mode_t mode) noexcept {
	// Если данные переданы
	if(data.is_object() && !data.empty()){
		// Выполняем блокировку потока
		const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем очистку объекта
			this->_cef.clear();
			// Устанавливаем строгий режим работы
			this->_cef.mode(mode);
			// Устанавливаем полученные данные обратно
			this->_cef.dump(data);
			// Выводим полученный результат
			return this->_cef.cef();
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			// Выводим переданный лог
			this->_log->print("Parser: \"%s\" - %s [%s]", log_t::flag_t::CRITICAL, "CEF", error.what(), data.dump(4).c_str());
		}
	}
	// Выводим результат по умолчанию
	return "";
}
/**
 * Parser Конструктор
 * @param fmk объект фреймворка
 * @param log объект для работы с логами
 */
anyks::Parser::Parser(const fmk_t * fmk, const log_t * log) noexcept :
 _cef(fmk, log), _csv(fmk, log), _grok(fmk, log), _syslog(fmk, log), _fmk(fmk), _log(log) {
	// Выполняем инициализацию Grok
	this->_grok.init();
}
