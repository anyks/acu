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
 * patterns Метод добавления списка поддерживаемых шаблонов
 * @param patterns список поддерживаемых шаблонов
 */
void anyks::Parser::patterns(const Document & patterns) noexcept {
	// Выполняем блокировку потока
	const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
	// Выполняем добавление списка шаблонов
	this->_grok.patterns(patterns);
}
/**
 * yaml Метод конвертации текста в формате YAML в объект JSON
 * @param text текст для конвертации
 * @return     объект в формате JSON
 */
Document anyks::Parser::yaml(const string & text) noexcept {
	// Результат работы функции
	Document result(kObjectType);
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
			function <void (Document &, const Value &, const YAML::Node &)> parseFn;
			/**
			 * parseFn Функция парсинга XML документа
			 * @param root корень объекта для записи результата
			 * @param name название ключа куда добавляется содержимое ноды
			 * @param node объект текущей ноды
			 */
			parseFn = [&parseFn, this](Document & root, const Value & name, const YAML::Node & node) -> void {
				// Определяем тип полученной ноды
				switch(node.Type()){
					// Если объект определён как не существующий
					case YAML::NodeType::Null: {
						// Если если корень раздела является объектом
						if(root.IsObject())
							// Устанавливаем пустое значение
							root.AddMember(Value(name, root.GetAllocator()).Move(), Value(0).Move(), root.GetAllocator());
						// Если если корень раздела является массивом
						else if(root.IsArray())
							// Добавляем пустое значение в массив
							root.PushBack(Value(0).Move(), root.GetAllocator());
					} break;
					// Если объект является скалярным выражением
					case YAML::NodeType::Scalar: {
						// Если полученное значение является числом
						if(this->_fmk->is(node.as <string> (), fmk_t::check_t::NUMBER)){
							// Получаем значение числа
							const int64_t number = node.as <int64_t> ();
							// Если число является отрицательным
							if(number < 0){
								// Если если корень раздела является объектом
								if(root.IsObject())
									// Устанавливаем числовое значение
									root.AddMember(Value(name, root.GetAllocator()).Move(), Value(number).Move(), root.GetAllocator());
								// Если если корень раздела является массивом
								else if(root.IsArray())
									// Добавляем полученное число в массив
									root.PushBack(Value(number).Move(), root.GetAllocator());
							// Выводим числовое значение
							} else {
								// Если если корень раздела является объектом
								if(root.IsObject())
									// Устанавливаем числовое значение
									root.AddMember(Value(name, root.GetAllocator()).Move(), Value(node.as <uint64_t> ()).Move(), root.GetAllocator());
								// Если если корень раздела является массивом
								else if(root.IsArray())
									// Добавляем числовое значение в массив
									root.PushBack(Value(node.as <uint64_t> ()).Move(), root.GetAllocator());
							}
						// Если полученное значение является числом с плавающей точкой
						} else if(this->_fmk->is(node.as <string> (), fmk_t::check_t::DECIMAL)) {
							// Если если корень раздела является объектом
							if(root.IsObject())
								// Устанавливаем числовое значение с плавающей точкой
								root.AddMember(Value(name, root.GetAllocator()).Move(), Value(node.as <double> ()).Move(), root.GetAllocator());
							// Если если корень раздела является массивом
							else if(root.IsArray())
								// Добавляем числовое значение с плавающей точкой двойной точности в массив
								root.PushBack(Value(node.as <double> ()).Move(), root.GetAllocator());
						// Если полученное значение является строкой
						} else {
							// Флаг истинного значения
							bool mode = false;
							// Если значение является булевым
							if((mode = this->_fmk->compare(node.as <string> (), "true")) || this->_fmk->compare(node.as <string> (), "false")){
								// Если если корень раздела является объектом
								if(root.IsObject())
									// Устанавливаем булевое значение
									root.AddMember(Value(name, root.GetAllocator()).Move(), Value(mode).Move(), root.GetAllocator());
								// Если если корень раздела является массивом
								else if(root.IsArray())
									// Добавляем булевое значение в массив
									root.PushBack(Value(mode).Move(), root.GetAllocator());
							// Если значение является просто строкой
							} else {
								// Получаем значение для добавления
								const string & value = node.as <string> ();
								// Если если корень раздела является объектом
								if(root.IsObject())
									// Устанавливаем значение строки как она есть
									root.AddMember(Value(name, root.GetAllocator()).Move(), Value(value.c_str(), value.length(), root.GetAllocator()).Move(), root.GetAllocator());
								// Если если корень раздела является массивом
								else if(root.IsArray())
									// Добавляем значение строки как она есть в массив
									root.PushBack(Value(value.c_str(), value.length(), root.GetAllocator()).Move(), root.GetAllocator());
							}
						}
					} break;
					// Если объект является последовательностью
					case YAML::NodeType::Sequence: {
						// Создаём новый массив данных
						Document item(kArrayType);
						// Выполняем перебор всей последовательности
						for(size_t i = 0; i < node.size(); i++)
							// Выполняем добавление последовательности
							parseFn(item, Value(static_cast <uint64_t> (i)).Move(), node[i]);
						// Создаём объект значения
						Value value;
						// Копируем полученный результат
						value.CopyFrom(item, item.GetAllocator());
						// Если если корень раздела является объектом
						if(root.IsObject())
							// Устанавливаем массив в карту объекта
							root.AddMember(Value(name, root.GetAllocator()).Move(), Value(value, root.GetAllocator()).Move(), root.GetAllocator());
						// Если если корень раздела является массивом
						else if(root.IsArray())
							// Добавляем массив в карту объекта в массив
							root.PushBack(Value(value, root.GetAllocator()).Move(), root.GetAllocator());
					} break;
					// Если объект является картой
					case YAML::NodeType::Map: {
						// Создаём новый массив данных
						Document item(kObjectType);
						// Выполняем перебор всего списка полученных данных
						for(auto i = node.begin(); i != node.end(); ++i){
							// Если ключ является строковым значением
							if(i->first.IsScalar()){
								// Получаем название ключа
								const string & value = i->first.as <string> ();
								// Выполняем добавление последовательности
								parseFn(item, Value(value.c_str(), value.length(), root.GetAllocator()).Move(), i->second);
							}
						}
						// Создаём объект значения
						Value value;
						// Копируем полученный результат
						value.CopyFrom(item, item.GetAllocator());
						// Если если корень раздела является объектом
						if(root.IsObject())
							// Устанавливаем массив в карту объекта
							root.AddMember(Value(name, root.GetAllocator()).Move(), Value(value, root.GetAllocator()).Move(), root.GetAllocator());
						// Если если корень раздела является массивом
						else if(root.IsArray())
							// Добавляем массив в карту объекта в массив
							root.PushBack(Value(value, root.GetAllocator()).Move(), root.GetAllocator());
					} break;
					// Если объект не определён
					case YAML::NodeType::Undefined: {
						/**
						 * Если включён режим отладки
						 */
						#if defined(DEBUG_MODE)
							// Выводим сообщение об ошибке
							this->_log->debug("%s", __PRETTY_FUNCTION__, {}, log_t::flag_t::CRITICAL, "Node method is undefined");
						/**
						* Если режим отладки не включён
						*/
						#else
							// Выводим сообщение об ошибке
							this->_log->print("%s", log_t::flag_t::CRITICAL, "Node method is undefined");
						#endif
					} break;
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
					// Получаем значение в виде строки
					const string & value = node.as <string> ();
					// Если полученное значение является числом
					if(this->_fmk->is(value, fmk_t::check_t::NUMBER)){
						// Получаем значение числа
						const int64_t number = node.as <int64_t> ();
						// Если число является отрицательным
						if(number < 0)
							// Выводим числовое значение
							result.SetInt64(number);
						// Выводим числовое значение
						else result.SetUint64(node.as <uint64_t> ());
					// Если полученное значение является числом с плавающей точкой
					} else if(this->_fmk->is(value, fmk_t::check_t::DECIMAL))
						// Выводим числовое значение с плавающей точкой двойной точности
						result.SetDouble(node.as <double> ());
					// Если полученное значение является строкой
					else {
						// Флаг истинного значения
						bool mode = false;
						// Если значение является булевым
						if((mode = this->_fmk->compare(value, "true")) || this->_fmk->compare(value, "false"))
							// Выводим булевое значение
							result.SetBool(mode);
						// Если значение является просто строкой, выводим как оно есть
						else result.SetString(value.c_str(), value.length());
					}
				} break;
				// Если объект является последовательностью
				case YAML::NodeType::Sequence: {
					// Выполняем создание результата в виде массива
					result.SetArray();
					// Выполняем перебор всей последовательности
					for(size_t i = 0; i < node.size(); i++)
						// Выполняем добавление последовательности
						parseFn(result, Value(static_cast <uint64_t> (i)).Move(), node[i]);
				} break;
				// Если объект является картой
				case YAML::NodeType::Map: {
					// Выполняем перебор всего списка полученных данных
					for(auto i = node.begin(); i != node.end(); ++i){
						// Если ключ является строковым значением
						if(i->first.IsScalar()){
							// Получаем название ключа
							const string & value = i->first.as <string> ();
							// Выполняем добавление последовательности
							parseFn(result, Value(value.c_str(), value.length(), result.GetAllocator()).Move(), i->second);
						}
					}
				} break;
				// Если объект не определён
				case YAML::NodeType::Undefined: {
					/**
					 * Если включён режим отладки
					 */
					#if defined(DEBUG_MODE)
						// Выводим сообщение об ошибке
						this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(text), log_t::flag_t::WARNING, "Method is undefined");
					/**
					* Если режим отладки не включён
					*/
					#else
						// Выводим сообщение об ошибке
						this->_log->print("%s", log_t::flag_t::WARNING, "Method is undefined");
					#endif
				} break;
			}
		/**
		 * Если возникает ошибка
		 */
		} catch(const YAML::ParserException & error) {
			/**
			 * Если включён режим отладки
			 */
			#if defined(DEBUG_MODE)
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(text), log_t::flag_t::CRITICAL, error.what());
			/**
			* Если режим отладки не включён
			*/
			#else
				// Выводим сообщение об ошибке
				this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
			#endif
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			/**
			 * Если включён режим отладки
			 */
			#if defined(DEBUG_MODE)
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(text), log_t::flag_t::CRITICAL, error.what());
			/**
			* Если режим отладки не включён
			*/
			#else
				// Выводим сообщение об ошибке
				this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
			#endif
		}
	}
	// Выводим результат
	return result;
}
/**
 * yaml Метод конвертации объекта JSON в текст в формате YAML
 * @param data данные в объекте JSON
 * @return     текст после конвертации
 */
string anyks::Parser::yaml(const Document & data) noexcept {
	// Результат работы функции
	string result = "";
	// Если данные переданы
	if((data.IsObject() && !data.ObjectEmpty()) || (data.IsArray() && !data.Empty())){
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
			function <void (YAML::Node &, const Value &, const Value &)> parseFn;
			/**
			 * parseFn Функция парсинга XML документа
			 * @param node  корень объекта для записи результата
			 * @param name  название ключа куда добавляется содержимое ноды
			 * @param value объект текущей ноды
			 */
			parseFn = [&parseFn, this](YAML::Node & node, const Value & name, const Value & value) -> void {
				// Если значение является отрицательным 32-х битным числом
				if(value.IsInt()){
					// Если название ячейки является числом
					if(name.IsUint64())
						// Выполняем добавление значения в ноду
						node[name.GetUint64()] = value.GetInt();
					// Если название ячейки является строкой
					else if(name.IsString())
						// Выполняем добавление значения в ноду
						node[name.GetString()] = value.GetInt();
				// Если значение является положительным 32-х битным числом
				} else if(value.IsUint()) {
					// Если название ячейки является числом
					if(name.IsUint64())
						// Выполняем добавление значения в ноду
						node[name.GetUint64()] = value.GetUint();
					// Если название ячейки является строкой
					else if(name.IsString())
						// Выполняем добавление значения в ноду
						node[name.GetString()] = value.GetUint();
				// Если значение является отрицательным 64-х битным числом
				} else if(value.IsUint64()) {
					// Если название ячейки является числом
					if(name.IsUint64())
						// Выполняем добавление значения в ноду
						node[name.GetUint64()] = value.GetInt64();
					// Если название ячейки является строкой
					else if(name.IsString())
						// Выполняем добавление значения в ноду
						node[name.GetString()] = value.GetInt64();
				// Если значение является положительным 64-х битным числом
				} else if(value.IsUint64()) {
					// Если название ячейки является числом
					if(name.IsUint64())
						// Выполняем добавление значения в ноду
						node[name.GetUint64()] = value.GetUint64();
					// Если название ячейки является строкой
					else if(name.IsString())
						// Выполняем добавление значения в ноду
						node[name.GetString()] = value.GetUint64();
				// Если значение является числом с плавающей точкой
				} else if(value.IsFloat()) {
					// Если название ячейки является числом
					if(name.IsUint64())
						// Выполняем добавление значения в ноду
						node[name.GetUint64()] = value.GetFloat();
					// Если название ячейки является строкой
					else if(name.IsString())
						// Выполняем добавление значения в ноду
						node[name.GetString()] = value.GetFloat();
				// Если значение является числом с плавающей точкой двойной точности
				} else if(value.IsDouble()) {
					// Если название ячейки является числом
					if(name.IsUint64())
						// Выполняем добавление значения в ноду
						node[name.GetUint64()] = value.GetDouble();
					// Если название ячейки является строкой
					else if(name.IsString())
						// Выполняем добавление значения в ноду
						node[name.GetString()] = value.GetDouble();
				// Если значение является булевым
				} else if(value.IsBool()) {
					// Если название ячейки является числом
					if(name.IsUint64())
						// Выполняем добавление значения в ноду
						node[name.GetUint64()] = value.GetBool();
					// Если название ячейки является строкой
					else if(name.IsString())
						// Выполняем добавление значения в ноду
						node[name.GetString()] = value.GetBool();
				// Если значение является строкой
				} else if(value.IsString()) {
					// Если название ячейки является числом
					if(name.IsUint64())
						// Выполняем добавление значения в ноду
						node[name.GetUint64()] = value.GetString();
					// Если название ячейки является строкой
					else if(name.IsString())
						// Выполняем добавление значения в ноду
						node[name.GetString()] = value.GetString();
				// Если значение является массивом
				} else if(value.IsArray()) {
					// Создаём объект дочерней ноды
					YAML::Node child;
					// Выполняем перебор всего списка
					for(size_t i = 0; i < value.Size(); i++)
						// Выполняем добавление в массив полученных значений
						parseFn(child, Value(static_cast <uint64_t> (i)).Move(), value[i]);
					// Если название ячейки является числом
					if(name.IsUint64())
						// Выполняем добавление значения в ноду
						node[name.GetUint64()] = child;
					// Если название ячейки является строкой
					else if(name.IsString())
						// Выполняем добавление значения в ноду
						node[name.GetString()] = child;
				// Если значение является объектом
				} else if(value.IsObject()) {
					// Создаём объект дочерней ноды
					YAML::Node child;
					// Выполняем перебор всего списка
					for(auto & m : value.GetObj())
						// Выполняем добавление в объект полученных значений
						parseFn(child, m.name, m.value);
					// Если название ячейки является числом
					if(name.IsUint64())
						// Выполняем добавление значения в ноду
						node[name.GetUint64()] = child;
					// Если название ячейки является строкой
					else if(name.IsString())
						// Выполняем добавление значения в ноду
						node[name.GetString()] = child;
				}
			};
			// Объект ноды для формирования результата
			YAML::Node node;
			// Если значение является массивом
			if(data.IsArray()){
				// Выполняем перебор всего списка
				for(size_t i = 0; i < data.Size(); i++)
					// Выполняем добавление в массив полученных значений
					parseFn(node, Value(static_cast <uint64_t> (i)).Move(), data[i]);
			// Если значение является объектом
			} else if(data.IsObject()) {
				// Выполняем перебор всего списка
				for(auto & m : data.GetObj())
					// Выполняем добавление в объект полученных значений
					parseFn(node, m.name, m.value);
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
			/**
			 * Если включён режим отладки
			 */
			#if defined(DEBUG_MODE)
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, {}, log_t::flag_t::CRITICAL, error.what());
			/**
			* Если режим отладки не включён
			*/
			#else
				// Выводим сообщение об ошибке
				this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
			#endif
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			/**
			 * Если включён режим отладки
			 */
			#if defined(DEBUG_MODE)
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, {}, log_t::flag_t::CRITICAL, error.what());
			/**
			* Если режим отладки не включён
			*/
			#else
				// Выводим сообщение об ошибке
				this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
			#endif
		}
	}
	// Выводим результат
	return result;
}
/**
 * ini Метод конвертации текста в формате INI в объект JSON
 * @param text текст для конвертации
 * @return     объект в формате JSON
 */
Document anyks::Parser::ini(const string & text) noexcept {
	// Результат работы функции
	Document result(kObjectType);
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
				result.AddMember(Value(i->first.c_str(), i->first.length(), result.GetAllocator()).Move(), Value(kObjectType).Move(), result.GetAllocator());
				// Выполняем перебор всех элементов секции
				for(auto j = i->second.begin(); j != i->second.end(); ++j){
					// Если полученное значение является числом
					if(this->_fmk->is(j->second, fmk_t::check_t::NUMBER)){
						/**
						 * Выполняем отлов ошибок
						 */
						try {
							// Если число является отрицательным
							if(j->second.front() == '-')
								// Выполняем установку полученного значения
								result[i->first.c_str()].AddMember(Value(j->first.c_str(), j->first.length(), result.GetAllocator()).Move(), Value(static_cast <int64_t> (::stoll(j->second))).Move(), result.GetAllocator());
							// Если число является положительным
							else result[i->first.c_str()].AddMember(Value(j->first.c_str(), j->first.length(), result.GetAllocator()).Move(), Value(static_cast <uint64_t> (::stoull(j->second))).Move(), result.GetAllocator());
						/**
						 * Если возникает ошибка
						 */
						} catch(const std::exception &) {
							// Выполняем установку полученного значения
							result[i->first.c_str()].AddMember(Value(j->first.c_str(), j->first.length(), result.GetAllocator()).Move(), Value(j->second.c_str(), j->second.length(), result.GetAllocator()).Move(), result.GetAllocator());
						}
					// Если полученное значение является числом с плавающей точкой
					} else if(this->_fmk->is(j->second, fmk_t::check_t::DECIMAL)) {
						/**
						 * Выполняем отлов ошибок
						 */
						try {
							// Выполняем установку полученного значения
							result[i->first.c_str()].AddMember(Value(j->first.c_str(), j->first.length(), result.GetAllocator()).Move(), Value(::stod(j->second)).Move(), result.GetAllocator());
						/**
						 * Если возникает ошибка
						 */
						} catch(const std::exception &) {
							// Выполняем установку полученного значения
							result[i->first.c_str()].AddMember(Value(j->first.c_str(), j->first.length(), result.GetAllocator()).Move(), Value(j->second.c_str(), j->second.length(), result.GetAllocator()).Move(), result.GetAllocator());
						}
					// Если полученное значение является строкой
					} else {
						// Флаг истинного значения
						bool mode = false;
						// Если значение является булевым
						if((mode = this->_fmk->compare(j->second, "true")) || this->_fmk->compare(j->second, "false"))
							// Выполняем установку полученного значения
							result[i->first.c_str()].AddMember(Value(j->first.c_str(), j->first.length(), result.GetAllocator()).Move(), Value(mode).Move(), result.GetAllocator());
						// Если значение является строковым
						else result[i->first.c_str()].AddMember(Value(j->first.c_str(), j->first.length(), result.GetAllocator()).Move(), Value(j->second.c_str(), j->second.length(), result.GetAllocator()).Move(), result.GetAllocator());
					}
				}
			}
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			/**
			 * Если включён режим отладки
			 */
			#if defined(DEBUG_MODE)
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(text), log_t::flag_t::CRITICAL, error.what());
			/**
			* Если режим отладки не включён
			*/
			#else
				// Выводим сообщение об ошибке
				this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
			#endif
		}
	}
	// Выводим результат
	return result;
}
/**
 * ini Метод конвертации объекта JSON в текст в формате INI
 * @param data данные в объекте JSON
 * @return     текст после конвертации
 */
string anyks::Parser::ini(const Document & data) noexcept {
	// Результат работы функции
	string result = "";
	// Если данные переданы
	if(data.IsObject()){
		// Выполняем блокировку потока
		const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем загрузку INI-конфига
			ini::File file = ini::load("");
			// Выполняем перебор всего списка
			for(auto & m : data.GetObj()){
				// Если ключ является строкой
				if(m.name.IsString()){
					// Выполняем создание секции
					file.add_section(m.name.GetString());
					// Если значения существуют верные
					if(m.value.IsObject()){
						// Выполняем переход по всем параметрам секции
						for(auto & i : m.value.GetObj()){
							// Если значение является отрицательным 32-х битным числом
							if(i.value.IsInt())
								// Устанавливаем значение в виде числа
								file[m.name.GetString()].set <int32_t> (i.name.GetString(), i.value.GetInt());
							// Если значение является положительным 32-х битным числом
							else if(i.value.IsUint())
								// Устанавливаем значение в виде числа
								file[m.name.GetString()].set <uint32_t> (i.name.GetString(), i.value.GetUint());
							// Если значение является отрицательным 64-х битным числом
							else if(i.value.IsInt64())
								// Устанавливаем значение в виде числа
								file[m.name.GetString()].set <int64_t> (i.name.GetString(), i.value.GetInt64());
							// Если значение является положительным 64-х битным числом
							else if(i.value.IsUint64())
								// Устанавливаем значение в виде числа
								file[m.name.GetString()].set <uint64_t> (i.name.GetString(), i.value.GetUint64());
							// Если значение является числом с плавающей точкой
							else if(i.value.IsFloat())
								// Устанавливаем значение в виде числа
								file[m.name.GetString()].set <float> (i.name.GetString(), i.value.GetFloat());
							// Если значение является числом с плавающей точкой двойной точности
							else if(i.value.IsDouble())
								// Устанавливаем значение в виде числа
								file[m.name.GetString()].set <double> (i.name.GetString(), i.value.GetDouble());
							// Если значение является булевым значением
							else if(i.value.IsBool())
								// Устанавливаем значение в виде булевого значения
								file[m.name.GetString()].set <bool> (i.name.GetString(), i.value.GetBool());
							// Если значение является строковым
							else if(i.value.IsString())
								// Устанавливаем значение в виде строки
								file[m.name.GetString()].set <string> (i.name.GetString(), i.value.GetString());
						}
					}
				}
			}
			// Выполняем запись результата
			file.write(result);
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			/**
			 * Если включён режим отладки
			 */
			#if defined(DEBUG_MODE)
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, {}, log_t::flag_t::CRITICAL, error.what());
			/**
			* Если режим отладки не включён
			*/
			#else
				// Выводим сообщение об ошибке
				this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
			#endif
		}
	}
	// Выводим результат
	return result;
}
/**
 * syslog Метод конвертации текста в формате SysLog в объект JSON
 * @param text текст для конвертации
 * @return     объект в формате JSON
 */
Document anyks::Parser::syslog(const string & text) noexcept {
	// Результат работы функции
	Document result(kObjectType);
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
			/**
			 * Если включён режим отладки
			 */
			#if defined(DEBUG_MODE)
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(text), log_t::flag_t::CRITICAL, error.what());
			/**
			* Если режим отладки не включён
			*/
			#else
				// Выводим сообщение об ошибке
				this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
			#endif
		}
	}
	// Выводим результат
	return result;
}
/**
 * syslog Метод конвертации объекта JSON в текст в формате SysLog
 * @param data данные в объекте JSON
 * @return     текст после конвертации
 */
string anyks::Parser::syslog(const Document & data) noexcept {
	// Если данные переданы
	if(data.IsObject()){
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
			/**
			 * Если включён режим отладки
			 */
			#if defined(DEBUG_MODE)
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, {}, log_t::flag_t::CRITICAL, error.what());
			/**
			* Если режим отладки не включён
			*/
			#else
				// Выводим сообщение об ошибке
				this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
			#endif
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
Document anyks::Parser::grok(const string & text, const string & pattern) noexcept {
	// Результат работы функции
	Document result(kObjectType);
	// Если текст и шаблон переданы
	if(!text.empty() && !pattern.empty()){
		// Выполняем блокировку потока
		const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем получение регулярного выражения
			string express = pattern;
			// Выполняем сборку регулярного выражения
			const size_t cid = this->_grok.build(express);
			// Выполняем парсинг данных
			this->_grok.parse(text, cid);
			// Выводим полученные данные
			result = this->_grok.dump(cid);
			// Выполняем сброс собранных данных
			this->_grok.reset(cid);
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			/**
			 * Если включён режим отладки
			 */
			#if defined(DEBUG_MODE)
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(text, pattern), log_t::flag_t::CRITICAL, error.what());
			/**
			* Если режим отладки не включён
			*/
			#else
				// Выводим сообщение об ошибке
				this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
			#endif
		}
	}
	// Выводим результат
	return result;
}
/**
 * csv Метод конвертации текста в формате CSV в объект JSON
 * @param text   текст для конвертации
 * @param header флаг формирования заголовков
 * @return       объект в формате JSON
 */
Document anyks::Parser::csv(const string & text, const bool header) noexcept {
	// Результат работы функции
	Document result(kObjectType);
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
			/**
			 * Если включён режим отладки
			 */
			#if defined(DEBUG_MODE)
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(text, header), log_t::flag_t::CRITICAL, error.what());
			/**
			* Если режим отладки не включён
			*/
			#else
				// Выводим сообщение об ошибке
				this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
			#endif
		}
	}
	// Выводим результат
	return result;
}
/**
 * csv Метод конвертации объекта JSON в текст в формате CSV
 * @param data   данные в объекте JSON
 * @param header флаг формирования заголовков
 * @param delim  используемый разделитель
 * @return       текст после конвертации
 */
string anyks::Parser::csv(const Document & data, const bool header, const char delim) noexcept {
	// Результат работы функции
	string result = "";
	// Если данные переданы
	if((data.IsObject() && !data.ObjectEmpty()) || (data.IsArray() && !data.Empty())){
		// Выполняем блокировку потока
		const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем очистку контейнера
			this->_csv.clear();
			// Работаем с заголовком
			this->_csv.header(header);
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
			/**
			 * Если включён режим отладки
			 */
			#if defined(DEBUG_MODE)
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, {}, log_t::flag_t::CRITICAL, error.what());
			/**
			* Если режим отладки не включён
			*/
			#else
				// Выводим сообщение об ошибке
				this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
			#endif
		}
	}
	// Выводим результат
	return result;
}
/**
 * xml Метод конвертации текста в формате XML в объект JSON
 * @param text текст для конвертации
 * @return     объект в формате JSON
 */
Document anyks::Parser::xml(const string & text) noexcept {
	// Результат работы функции
	Document result(kObjectType);
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
					/**
					 * Если включён режим отладки
					 */
					#if defined(DEBUG_MODE)
						// Выводим сообщение об ошибке
						this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(text), log_t::flag_t::CRITICAL, "Document not parsed successfully");
					/**
					* Если режим отладки не включён
					*/
					#else
						// Выводим сообщение об ошибке
						this->_log->print("%s", log_t::flag_t::CRITICAL, "Document not parsed successfully");
					#endif
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
					 * @param аллокатор для копирования
					 */
					function <void (Value &, xmlNodePtr, Document::AllocatorType &)> parseFn;
					/**
					 * parseFn Функция парсинга XML документа
					 * @param root      корень объекта для записи результата
					 * @param node      объект текущей ноды
					 * @param allocator аллокатор для копирования
					 */
					parseFn = [&parseFn, this](Value & root, xmlNodePtr node, Document::AllocatorType & allocator) -> void {
						// Если переданная нода существует
						while(node != nullptr){
							// Если нода передана не системная
							if((node->type == XML_ELEMENT_NODE) && !xmlIsBlankNode(node)){
								// Если доступны дочерние ноды
								if(node->xmlChildrenNode != nullptr){
									// Если есть дочерние элементы у ноды
									if(xmlChildElementCount(node) > 0){
										// Если такого ключа ещё не существует в списке
										if(!root.HasMember(reinterpret_cast <const char *> (node->name))){
											// Документ для извлечения данных
											Document result(kObjectType);
											// Выполняем парсинг ноды дальше
											parseFn(result, node->xmlChildrenNode, result.GetAllocator());
											// Создаём объект контейнера для помещения в него данных XML
											root.AddMember(Value(reinterpret_cast <const char *> (node->name), allocator).Move(), Value(kObjectType).Move(), allocator);
											// Извлекаем полученные данные объекта
											root[reinterpret_cast <const char *> (node->name)].CopyFrom(result, allocator);
										// Если текущий ключ не является массивом
										} else if(!root[reinterpret_cast <const char *> (node->name)].IsArray()) {
											// Документ для извлечения данных
											Document result(kArrayType);
											// Добавляем в массив полученное значение
											result.PushBack(Value(root[reinterpret_cast <const char *> (node->name)], result.GetAllocator()), result.GetAllocator());
											// Добавляем пустой объект в список
											result.PushBack(Value(kObjectType).Move(), result.GetAllocator());
											// Выполняем парсинг ноды дальше
											parseFn(result[result.Size() - 1], node->xmlChildrenNode, result.GetAllocator());
											// Создаём из текущего объекта массив
											root[reinterpret_cast <const char *> (node->name)].SetArray();
											// Копируем полученный результат
											root[reinterpret_cast <const char *> (node->name)].CopyFrom(result, allocator);
										// Если текущий ключ уже является массивом
										} else {
											// Получаем количество элементов в массиве
											const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
											// Выполняем создание объекта внутри массива
											root[reinterpret_cast <const char *> (node->name)].PushBack(Value(kObjectType).Move(), allocator);
											// Выполняем парсинг ноды дальше
											parseFn(root[reinterpret_cast <const char *> (node->name)][size], node->xmlChildrenNode, allocator);
										}
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
													/**
													 * Выполняем отлов ошибок
													 */
													try {
														// Если число является отрицательным
														if(reinterpret_cast <const char *> (value)[0] == '-'){
															// Если элемент не является массивом
															if(root[reinterpret_cast <const char *> (node->name)].IsObject())
																// Выполняем формирования списка параметров
																root[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(static_cast <int64_t> (::stoll(reinterpret_cast <const char *> (value)))).Move(), allocator);
															// Иначе добавляем в указанный индекс массива
															else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
																// Получаем количество элементов в массиве
																const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
																// Выполняем добавление названия атрибута
																root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(static_cast <int64_t> (::stoll(reinterpret_cast <const char *> (value)))).Move(), allocator);
															}
														// Если число является положительным
														} else {
															// Если элемент не является массивом
															if(root[reinterpret_cast <const char *> (node->name)].IsObject())
																// Выполняем формирования списка параметров
																root[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(static_cast <uint64_t> (::stoull(reinterpret_cast <const char *> (value)))).Move(), allocator);
															// Иначе добавляем в указанный индекс массива
															else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
																// Получаем количество элементов в массиве
																const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
																// Выполняем добавление названия атрибута
																root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(static_cast <uint64_t> (::stoull(reinterpret_cast <const char *> (value)))).Move(), allocator);
															}
														}
													/**
													 * Если возникает ошибка
													 */
													} catch(const std::exception &) {
														// Если элемент не является массивом
														if(root[reinterpret_cast <const char *> (node->name)].IsObject())
															// Выполняем формирования списка параметров
															root[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(reinterpret_cast <const char *> (value), allocator).Move(), allocator);
														// Иначе добавляем в указанный индекс массива
														else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
															// Получаем количество элементов в массиве
															const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
															// Выполняем добавление названия атрибута
															root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(reinterpret_cast <const char *> (value), allocator).Move(), allocator);
														}
													}
												// Если полученное значение является числом с плавающей точкой
												} else if(this->_fmk->is(reinterpret_cast <const char *> (value), fmk_t::check_t::DECIMAL)) {
													/**
													 * Выполняем отлов ошибок
													 */
													try {
														// Если элемент не является массивом
														if(root[reinterpret_cast <const char *> (node->name)].IsObject())
															// Выполняем формирования списка параметров
															root[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(::stod(reinterpret_cast <const char *> (value))).Move(), allocator);
														// Иначе добавляем в указанный индекс массива
														else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
															// Получаем количество элементов в массиве
															const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
															// Выполняем добавление названия атрибута
															root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(::stod(reinterpret_cast <const char *> (value))).Move(), allocator);
														}
													/**
													 * Если возникает ошибка
													 */
													} catch(const std::exception &) {
														// Если элемент не является массивом
														if(root[reinterpret_cast <const char *> (node->name)].IsObject())
															// Выполняем формирования списка параметров
															root[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(reinterpret_cast <const char *> (value), allocator).Move(), allocator);
														// Иначе добавляем в указанный индекс массива
														else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
															// Получаем количество элементов в массиве
															const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
															// Выполняем добавление названия атрибута
															root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(reinterpret_cast <const char *> (value), allocator).Move(), allocator);
														}
													}
												// Если полученное значения является строкой
												} else {
													// Флаг булевого значения
													bool flag = false;
													// Если строка является булевым значением
													if((flag = this->_fmk->compare("true", reinterpret_cast <const char *> (value))) || this->_fmk->compare("false", reinterpret_cast <const char *> (value))){
														// Если элемент не является массивом
														if(root[reinterpret_cast <const char *> (node->name)].IsObject())
															// Выполняем формирования списка параметров
															root[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(flag).Move(), allocator);
														// Иначе добавляем в указанный индекс массива
														else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
															// Получаем количество элементов в массиве
															const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
															// Выполняем добавление названия атрибута
															root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(flag).Move(), allocator);
														}
													// Если значение является обычной строкой
													} else {
														// Если элемент не является массивом
														if(root[reinterpret_cast <const char *> (node->name)].IsObject())
															// Выполняем формирования списка параметров
															root[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(reinterpret_cast <const char *> (value), allocator).Move(), allocator);
														// Иначе добавляем в указанный индекс массива
														else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
															// Получаем количество элементов в массиве
															const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
															// Выполняем добавление названия атрибута
															root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(reinterpret_cast <const char *> (value), allocator).Move(), allocator);
														}
													}
												}
												// Выполняем итерацию по аттрибутам
												attribute = attribute->next;
												// Выполняем освобождение памяти выделенной под значение
												xmlFree(value);
											}
										}
									// Если дочерних элементов нет
									} else {
										// Если корневой элемент не является объектом
										if(!root.IsObject())
											// Устанавливаем тип JSON как объект
											root.SetObject();
										// Если такой ключ уже существует
										if(root.HasMember(reinterpret_cast <const char *> (node->name))){
											// Если ключ не является массивом
											if(!root[reinterpret_cast <const char *> (node->name)].IsArray()){
												// Документ для извлечения данных
												Document result(kArrayType);
												// Добавляем в массив полученное значение
												result.PushBack(Value(root[reinterpret_cast <const char *> (node->name)], result.GetAllocator()), result.GetAllocator());
												// Добавляем пустой объект в список
												result.PushBack(Value(kObjectType).Move(), result.GetAllocator());
												// Создаём из текущего объекта массив
												root[reinterpret_cast <const char *> (node->name)].SetArray();
												// Копируем полученный результат
												root[reinterpret_cast <const char *> (node->name)].CopyFrom(result, result.GetAllocator());
											// Выполняем создание объекта внутри массива
											} else root[reinterpret_cast <const char *> (node->name)].PushBack(Value(kObjectType).Move(), allocator);
										// Если такой ключ ещё не существует
										} else root.AddMember(Value(reinterpret_cast <const char *> (node->name), allocator).Move(), Value(kObjectType).Move(), allocator);
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
													/**
													 * Выполняем отлов ошибок
													 */
													try {
														// Если число является отрицательным
														if(reinterpret_cast <const char *> (value)[0] == '-'){
															// Если элемент не является массивом
															if(root[reinterpret_cast <const char *> (node->name)].IsObject())
																// Выполняем формирования списка параметров
																root[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(static_cast <int64_t> (::stoll(reinterpret_cast <const char *> (value)))).Move(), allocator);
															// Иначе добавляем в указанный индекс массива
															else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
																// Получаем количество элементов в массиве
																const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
																// Выполняем добавление названия атрибута
																root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(static_cast <int64_t> (::stoll(reinterpret_cast <const char *> (value)))).Move(), allocator);
															}
														// Если число является положительным
														} else {
															// Если элемент не является массивом
															if(root[reinterpret_cast <const char *> (node->name)].IsObject())
																// Выполняем формирования списка параметров
																root[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(static_cast <uint64_t> (::stoull(reinterpret_cast <const char *> (value)))).Move(), allocator);
															// Иначе добавляем в указанный индекс массива
															else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
																// Получаем количество элементов в массиве
																const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
																// Выполняем добавление названия атрибута
																root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(static_cast <uint64_t> (::stoull(reinterpret_cast <const char *> (value)))).Move(), allocator);
															}
														}
													/**
													 * Если возникает ошибка
													 */
													} catch(const std::exception &) {
														// Если элемент не является массивом
														if(root[reinterpret_cast <const char *> (node->name)].IsObject())
															// Выполняем формирования списка параметров
															root[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(reinterpret_cast <const char *> (value), allocator).Move(), allocator);
														// Иначе добавляем в указанный индекс массива
														else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
															// Получаем количество элементов в массиве
															const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
															// Выполняем добавление названия атрибута
															root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(reinterpret_cast <const char *> (value), allocator).Move(), allocator);
														}
													}
												// Если полученное значение является числом с плавающей точкой
												} else if(this->_fmk->is(reinterpret_cast <const char *> (value), fmk_t::check_t::DECIMAL)) {
													/**
													 * Выполняем отлов ошибок
													 */
													try {
														// Если элемент не является массивом
														if(root[reinterpret_cast <const char *> (node->name)].IsObject())
															// Выполняем формирования списка параметров
															root[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(::stod(reinterpret_cast <const char *> (value))).Move(), allocator);
														// Иначе добавляем в указанный индекс массива
														else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
															// Получаем количество элементов в массиве
															const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
															// Выполняем добавление названия атрибута
															root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(::stod(reinterpret_cast <const char *> (value))).Move(), allocator);
														}
													/**
													 * Если возникает ошибка
													 */
													} catch(const std::exception &) {
														// Если элемент не является массивом
														if(root[reinterpret_cast <const char *> (node->name)].IsObject())
															// Выполняем формирования списка параметров
															root[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(reinterpret_cast <const char *> (value), allocator).Move(), allocator);
														// Иначе добавляем в указанный индекс массива
														else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
															// Получаем количество элементов в массиве
															const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
															// Выполняем добавление названия атрибута
															root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(reinterpret_cast <const char *> (value), allocator).Move(), allocator);
														}
													}
												// Если полученное значения является строкой
												} else {
													// Флаг булевого значения
													bool flag = false;
													// Если строка является булевым значением
													if((flag = this->_fmk->compare("true", reinterpret_cast <const char *> (value))) || this->_fmk->compare("false", reinterpret_cast <const char *> (value))){
														// Если элемент не является массивом
														if(root[reinterpret_cast <const char *> (node->name)].IsObject())
															// Выполняем формирования списка параметров
															root[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(flag).Move(), allocator);
														// Иначе добавляем в указанный индекс массива
														else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
															// Получаем количество элементов в массиве
															const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
															// Выполняем добавление названия атрибута
															root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(flag).Move(), allocator);
														}
													// Если значение является обычной строкой
													} else {
														// Если элемент не является массивом
														if(root[reinterpret_cast <const char *> (node->name)].IsObject())
															// Выполняем формирования списка параметров
															root[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(reinterpret_cast <const char *> (value), allocator).Move(), allocator);
														// Иначе добавляем в указанный индекс массива
														else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
															// Получаем количество элементов в массиве
															const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
															// Выполняем добавление названия атрибута
															root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(reinterpret_cast <const char *> (value), allocator).Move(), allocator);
														}
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
										if(root[reinterpret_cast <const char *> (node->name)].IsObject()){
											// Если такой ключ уже существует в объекте
											while(root[reinterpret_cast <const char *> (node->name)].HasMember(key.c_str()))
												// Выполняем изменение ключа
												key.insert(key.begin(), '_');
										// Иначе добавляем в указанный индекс массива
										} else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
											// Получаем количество элементов в массиве
											const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
											// Если такой ключ уже существует в объекте
											while(root[reinterpret_cast <const char *> (node->name)][size - 1].HasMember(key.c_str()))
												// Выполняем изменение ключа
												key.insert(key.begin(), '_');
										}
										// Выполняем получение значения
										xmlChar * value = xmlNodeListGetString(node->doc, node->xmlChildrenNode, 1);
										// Если полученное значение является числом
										if(this->_fmk->is(reinterpret_cast <const char *> (value), fmk_t::check_t::NUMBER)){
											/**
											 * Выполняем отлов ошибок
											 */
											try {
												// Если число является отрицательным
												if(reinterpret_cast <const char *> (value)[0] == '-'){
													// Если элемент не является массивом
													if(root[reinterpret_cast <const char *> (node->name)].IsObject()){
														// Если у ноды есть параметры
														if(node->properties != nullptr)
															// Выполняем формирования списка параметров
															root[reinterpret_cast <const char *> (node->name)].AddMember(Value(key.c_str(), key.length(), allocator).Move(), Value(static_cast <int64_t> (::stoll(reinterpret_cast <const char *> (value)))).Move(), allocator);
														// Выполняем установку полученного значения
														else root[reinterpret_cast <const char *> (node->name)].SetInt64(static_cast <int64_t> (::stoll(reinterpret_cast <const char *> (value))));
													// Иначе добавляем в указанный индекс массива
													} else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
														// Получаем количество элементов в массиве
														const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
														// Если у ноды есть параметры
														if(node->properties != nullptr)
															// Выполняем добавление названия атрибута
															root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(key.c_str(), key.length(), allocator).Move(), Value(static_cast <int64_t> (::stoll(reinterpret_cast <const char *> (value)))).Move(), allocator);
														// Выполняем установку полученного значения
														else root[reinterpret_cast <const char *> (node->name)][size - 1].SetInt64(static_cast <int64_t> (::stoll(reinterpret_cast <const char *> (value))));
													}
												// Если число является положительным
												} else {
													// Если элемент не является массивом
													if(root[reinterpret_cast <const char *> (node->name)].IsObject()){
														// Если у ноды есть параметры
														if(node->properties != nullptr)
															// Выполняем формирования списка параметров
															root[reinterpret_cast <const char *> (node->name)].AddMember(Value(key.c_str(), key.length(), allocator).Move(), Value(static_cast <uint64_t> (::stoull(reinterpret_cast <const char *> (value)))).Move(), allocator);
														// Выполняем установку полученного значения
														else root[reinterpret_cast <const char *> (node->name)].SetUint64(static_cast <uint64_t> (::stoull(reinterpret_cast <const char *> (value))));
													// Иначе добавляем в указанный индекс массива
													} else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
														// Получаем количество элементов в массиве
														const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
														// Если у ноды есть параметры
														if(node->properties != nullptr)
															// Выполняем добавление названия атрибута
															root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(key.c_str(), key.length(), allocator).Move(), Value(static_cast <uint64_t> (::stoull(reinterpret_cast <const char *> (value)))).Move(), allocator);
														// Выполняем установку полученного значения
														else root[reinterpret_cast <const char *> (node->name)][size - 1].SetUint64(static_cast <uint64_t> (::stoull(reinterpret_cast <const char *> (value))));
													}
												}
											/**
											 * Если возникает ошибка
											 */
											} catch(const std::exception &) {
												// Если элемент не является массивом
												if(root[reinterpret_cast <const char *> (node->name)].IsObject()){
													// Если у ноды есть параметры
													if(node->properties != nullptr)
														// Выполняем формирования списка параметров
														root[reinterpret_cast <const char *> (node->name)].AddMember(Value(key.c_str(), key.length(), allocator).Move(), Value(reinterpret_cast <const char *> (value), allocator).Move(), allocator);
													// Выполняем установку полученного значения
													else root[reinterpret_cast <const char *> (node->name)].SetString(reinterpret_cast <const char *> (value), allocator);
												// Иначе добавляем в указанный индекс массива
												} else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
													// Получаем количество элементов в массиве
													const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
													// Если у ноды есть параметры
													if(node->properties != nullptr)
														// Выполняем добавление названия атрибута
														root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(key.c_str(), key.length(), allocator).Move(), Value(reinterpret_cast <const char *> (value), allocator).Move(), allocator);
													// Выполняем установку полученного значения
													else root[reinterpret_cast <const char *> (node->name)][size - 1].SetString(reinterpret_cast <const char *> (value), allocator);
												}
											}
										// Если полученное значение является числом с плавающей точкой
										} else if(this->_fmk->is(reinterpret_cast <const char *> (value), fmk_t::check_t::DECIMAL)) {
											/**
											 * Выполняем отлов ошибок
											 */
											try {
												// Если элемент не является массивом
												if(root[reinterpret_cast <const char *> (node->name)].IsObject()){
													// Если у ноды есть параметры
													if(node->properties != nullptr)
														// Выполняем формирования списка параметров
														root[reinterpret_cast <const char *> (node->name)].AddMember(Value(key.c_str(), key.length(), allocator).Move(), Value(::stod(reinterpret_cast <const char *> (value))).Move(), allocator);
													// Выполняем установку полученного значения
													else root[reinterpret_cast <const char *> (node->name)].SetDouble(::stod(reinterpret_cast <const char *> (value)));
												// Иначе добавляем в указанный индекс массива
												} else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
													// Получаем количество элементов в массиве
													const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
													// Если у ноды есть параметры
													if(node->properties != nullptr)
														// Выполняем добавление названия атрибута
														root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(key.c_str(), key.length(), allocator).Move(), Value(::stod(reinterpret_cast <const char *> (value))).Move(), allocator);
													// Выполняем установку полученного значения
													else root[reinterpret_cast <const char *> (node->name)][size - 1].SetDouble(::stod(reinterpret_cast <const char *> (value)));
												}
											/**
											 * Если возникает ошибка
											 */
											} catch(const std::exception &) {
												// Если элемент не является массивом
												if(root[reinterpret_cast <const char *> (node->name)].IsObject()){
													// Если у ноды есть параметры
													if(node->properties != nullptr)
														// Выполняем формирования списка параметров
														root[reinterpret_cast <const char *> (node->name)].AddMember(Value(key.c_str(), key.length(), allocator).Move(), Value(reinterpret_cast <const char *> (value), allocator).Move(), allocator);
													// Выполняем установку полученного значения
													else root[reinterpret_cast <const char *> (node->name)].SetString(reinterpret_cast <const char *> (value), allocator);
												// Иначе добавляем в указанный индекс массива
												} else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
													// Получаем количество элементов в массиве
													const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
													// Если у ноды есть параметры
													if(node->properties != nullptr)
														// Выполняем добавление названия атрибута
														root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(key.c_str(), key.length(), allocator).Move(), Value(reinterpret_cast <const char *> (value), allocator).Move(), allocator);
													// Выполняем установку полученного значения
													else root[reinterpret_cast <const char *> (node->name)][size - 1].SetString(reinterpret_cast <const char *> (value), allocator);
												}
											}
										// Если значение не является числом
										} else {
											// Флаг булевого значения
											bool flag = false;
											// Если строка является булевым значением
											if((flag = this->_fmk->compare("true", reinterpret_cast <const char *> (value))) || this->_fmk->compare("false", reinterpret_cast <const char *> (value))){
												// Если элемент не является массивом
												if(root[reinterpret_cast <const char *> (node->name)].IsObject()){
													// Если у ноды есть параметры
													if(node->properties != nullptr)
														// Выполняем формирования списка параметров
														root[reinterpret_cast <const char *> (node->name)].AddMember(Value(key.c_str(), key.length(), allocator).Move(), Value(flag).Move(), allocator);
													// Выполняем установку полученного значения
													else root[reinterpret_cast <const char *> (node->name)].SetBool(flag);
												// Иначе добавляем в указанный индекс массива
												} else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
													// Получаем количество элементов в массиве
													const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
													// Если у ноды есть параметры
													if(node->properties != nullptr)
														// Выполняем добавление названия атрибута
														root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(key.c_str(), key.length(), allocator).Move(), Value(flag).Move(), allocator);
													// Выполняем установку полученного значения
													else root[reinterpret_cast <const char *> (node->name)][size - 1].SetBool(flag);
												}
											// Если значение является обычной строкой
											} else {
												// Если элемент не является массивом
												if(root[reinterpret_cast <const char *> (node->name)].IsObject()){
													// Если у ноды есть параметры
													if(node->properties != nullptr)
														// Выполняем формирования списка параметров
														root[reinterpret_cast <const char *> (node->name)].AddMember(Value(key.c_str(), key.length(), allocator).Move(), Value(reinterpret_cast <const char *> (value), allocator).Move(), allocator);
													// Выполняем установку полученного значения
													else root[reinterpret_cast <const char *> (node->name)].SetString(reinterpret_cast <const char *> (value), allocator);
												// Иначе добавляем в указанный индекс массива
												} else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
													// Получаем количество элементов в массиве
													const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
													// Если у ноды есть параметры
													if(node->properties != nullptr)
														// Выполняем добавление названия атрибута
														root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(key.c_str(), key.length(), allocator).Move(), Value(reinterpret_cast <const char *> (value), allocator).Move(), allocator);
													// Выполняем установку полученного значения
													else root[reinterpret_cast <const char *> (node->name)][size - 1].SetString(reinterpret_cast <const char *> (value), allocator);
												}
											}
										}
										// Выполняем освобождение памяти выделенной под значение
										xmlFree(value);
									}
								// Если дочерних элементов не обнаружено
								} else {
									// Если корневой элемент не является объектом
									if(!root.IsObject())
										// Устанавливаем тип JSON как объект
										root.SetObject();
									// Если у ноды есть параметры
									if(node->properties != nullptr){
										// Если такой ключ уже существует
										if(root.HasMember(reinterpret_cast <const char *> (node->name))){
											// Если ключ не является массивом
											if(!root[reinterpret_cast <const char *> (node->name)].IsArray()){
												// Документ для извлечения данных
												Document result(kArrayType);
												// Добавляем в массив полученное значение
												result.PushBack(Value(root[reinterpret_cast <const char *> (node->name)], result.GetAllocator()), result.GetAllocator());
												// Добавляем пустой объект в список
												result.PushBack(Value(kObjectType).Move(), result.GetAllocator());
												// Создаём из текущего объекта массив
												root[reinterpret_cast <const char *> (node->name)].SetArray();
												// Копируем полученный результат
												root[reinterpret_cast <const char *> (node->name)].CopyFrom(result, result.GetAllocator());
											// Выполняем создание объекта внутри массива
											} else root[reinterpret_cast <const char *> (node->name)].PushBack(Value(kObjectType).Move(), allocator);
										// Если такой ключ ещё не существует
										} else root.AddMember(Value(reinterpret_cast <const char *> (node->name), allocator).Move(), Value(kObjectType).Move(), allocator);
										// Получаем список атрибутов
										xmlAttr * attribute = node->properties;
										// Выполняем перебор всего списка атрибутов
										while((attribute != nullptr) && (attribute->name != nullptr) && (attribute->children != nullptr)){
											// Выполняем получение значения
											xmlChar * value = xmlNodeListGetString(node->doc, attribute->children, 1);
											// Если полученное значение является числом
											if(this->_fmk->is(reinterpret_cast <const char *> (value), fmk_t::check_t::NUMBER)){
												/**
												 * Выполняем отлов ошибок
												 */
												try {
													// Если число является отрицательным
													if(reinterpret_cast <const char *> (value)[0] == '-'){
														// Если элемент не является массивом
														if(root[reinterpret_cast <const char *> (node->name)].IsObject())
															// Выполняем формирования списка параметров
															root[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(static_cast <int64_t> (::stoll(reinterpret_cast <const char *> (value)))).Move(), allocator);
														// Иначе добавляем в указанный индекс массива
														else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
															// Получаем количество элементов в массиве
															const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
															// Выполняем добавление названия атрибута
															root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(static_cast <int64_t> (::stoll(reinterpret_cast <const char *> (value)))).Move(), allocator);
														}
													// Если число является положительным
													} else {
														// Если элемент не является массивом
														if(root[reinterpret_cast <const char *> (node->name)].IsObject())
															// Выполняем формирования списка параметров
															root[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(static_cast <uint64_t> (::stoull(reinterpret_cast <const char *> (value)))).Move(), allocator);
														// Иначе добавляем в указанный индекс массива
														else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
															// Получаем количество элементов в массиве
															const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
															// Выполняем добавление названия атрибута
															root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(static_cast <uint64_t> (::stoull(reinterpret_cast <const char *> (value)))).Move(), allocator);
														}
													}
												/**
												 * Если возникает ошибка
												 */
									
												} catch(const std::exception &) {
													// Если элемент не является массивом
													if(root[reinterpret_cast <const char *> (node->name)].IsObject())
														// Выполняем формирования списка параметров
														root[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(reinterpret_cast <const char *> (value), allocator).Move(), allocator);
													// Иначе добавляем в указанный индекс массива
													else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
														// Получаем количество элементов в массиве
														const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
														// Выполняем добавление названия атрибута
														root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(reinterpret_cast <const char *> (value), allocator).Move(), allocator);
													}
												}
											// Если полученное значение является числом с плавающей точкой
											} else if(this->_fmk->is(reinterpret_cast <const char *> (value), fmk_t::check_t::DECIMAL)) {
												/**
												 * Выполняем отлов ошибок
												 */
												try {
													// Если элемент не является массивом
													if(root[reinterpret_cast <const char *> (node->name)].IsObject())
														// Выполняем формирования списка параметров
														root[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(::stod(reinterpret_cast <const char *> (value))).Move(), allocator);
													// Иначе добавляем в указанный индекс массива
													else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
														// Получаем количество элементов в массиве
														const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
														// Выполняем добавление названия атрибута
														root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(::stod(reinterpret_cast <const char *> (value))).Move(), allocator);
													}
												/**
												 * Если возникает ошибка
												 */
												} catch(const std::exception &) {
													// Если элемент не является массивом
													if(root[reinterpret_cast <const char *> (node->name)].IsObject())
														// Выполняем формирования списка параметров
														root[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(reinterpret_cast <const char *> (value), allocator).Move(), allocator);
													// Иначе добавляем в указанный индекс массива
													else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
														// Получаем количество элементов в массиве
														const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
														// Выполняем добавление названия атрибута
														root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(reinterpret_cast <const char *> (value), allocator).Move(), allocator);
													}
												}
											// Если значение является строковым
											} else {
												// Флаг булевого значения
												bool flag = false;
												// Если строка является булевым значением
												if((flag = this->_fmk->compare("true", reinterpret_cast <const char *> (value))) || this->_fmk->compare("false", reinterpret_cast <const char *> (value))){
													// Если элемент не является массивом
													if(root[reinterpret_cast <const char *> (node->name)].IsObject())
														// Выполняем формирования списка параметров
														root[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(flag).Move(), allocator);
													// Иначе добавляем в указанный индекс массива
													else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
														// Получаем количество элементов в массиве
														const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
														// Выполняем добавление названия атрибута
														root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(flag).Move(), allocator);
													}
												// Если значение является обычной строкой
												} else {
													// Если элемент не является массивом
													if(root[reinterpret_cast <const char *> (node->name)].IsObject())
														// Выполняем формирования списка параметров
														root[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(reinterpret_cast <const char *> (value), allocator).Move(), allocator);
													// Иначе добавляем в указанный индекс массива
													else if(root[reinterpret_cast <const char *> (node->name)].IsArray()) {
														// Получаем количество элементов в массиве
														const SizeType size = root[reinterpret_cast <const char *> (node->name)].Size();
														// Выполняем добавление названия атрибута
														root[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(reinterpret_cast <const char *> (attribute->name), allocator).Move(), Value(reinterpret_cast <const char *> (value), allocator).Move(), allocator);
													}
												}
											}
											// Выполняем итерацию по аттрибутам
											attribute = attribute->next;
											// Выполняем освобождение памяти выделенной под значение
											xmlFree(value);
										}
									// Если тег просто присутствует в списке
									} else {
										// Если указанный ключ существует в корневом разделе
										if(root.HasMember(reinterpret_cast <const char *> (node->name))){
											// Если корневой объект не является массивом
											if(!root[reinterpret_cast <const char *> (node->name)].IsArray())
												// Устанавливаем значение как булевое
												root[reinterpret_cast <const char *> (node->name)].SetBool(true);
										// Создаём булевое значение с положительным вложением
										} else root.AddMember(Value(reinterpret_cast <const char *> (node->name), allocator).Move(), Value(kTrueType).Move(), allocator);
									}
								}
							}
							// Выполняем итерацию ноды
							node = node->next;
						}
					};
					// Устанавливаем в корневой объект наш первый параметр
					result.AddMember(Value(reinterpret_cast <const char *> (node->name), result.GetAllocator()).Move(), Value(kObjectType).Move(), result.GetAllocator());
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
								/**
								 * Выполняем отлов ошибок
								 */
								try {
									// Если число является отрицательным
									if(reinterpret_cast <const char *> (value)[0] == '-')
										// Выполняем формирования списка параметров
										result[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), result.GetAllocator()).Move(), Value(static_cast <int64_t> (::stoll(reinterpret_cast <const char *> (value)))).Move(), result.GetAllocator());
									// Выполняем формирования списка параметров
									else result[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), result.GetAllocator()).Move(), Value(static_cast <uint64_t> (::stoull(reinterpret_cast <const char *> (value)))).Move(), result.GetAllocator());
								/**
								 * Если возникает ошибка
								 */
								} catch(const std::exception &) {
									// Выполняем формирования списка параметров
									result[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), result.GetAllocator()).Move(), Value(reinterpret_cast <const char *> (value), result.GetAllocator()).Move(), result.GetAllocator());
								}
							// Если полученное значение является числом с плавающей точкой
							} else if(this->_fmk->is(reinterpret_cast <const char *> (value), fmk_t::check_t::DECIMAL)) {
								/**
								 * Выполняем отлов ошибок
								 */
								try {
									// Выполняем формирования списка параметров
									result[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), result.GetAllocator()).Move(), Value(::stod(reinterpret_cast <const char *> (value))).Move(), result.GetAllocator());
								/**
								 * Если возникает ошибка
								 */
								} catch(const std::exception &) {
									// Выполняем формирования списка параметров
									result[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), result.GetAllocator()).Move(), Value(reinterpret_cast <const char *> (value), result.GetAllocator()).Move(), result.GetAllocator());
								}
							// Если значение является строковым
							} else {
								// Флаг булевого значения
								bool flag = false;
								// Если строка является булевым значением
								if((flag = this->_fmk->compare("true", reinterpret_cast <const char *> (value))) || this->_fmk->compare("false", reinterpret_cast <const char *> (value)))
									// Выполняем формирования списка параметров
									result[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), result.GetAllocator()).Move(), Value(flag).Move(), result.GetAllocator());
								// Выполняем формирования списка параметров
								else result[reinterpret_cast <const char *> (node->name)].AddMember(Value(reinterpret_cast <const char *> (attribute->name), result.GetAllocator()).Move(), Value(reinterpret_cast <const char *> (value), result.GetAllocator()).Move(), result.GetAllocator());
							}
							// Выполняем итерацию по аттрибутам
							attribute = attribute->next;
							// Выполняем освобождение памяти выделенной под значение
							xmlFree(value);
						}
					}
					// Выполняем получение значения
					xmlChar * value = xmlNodeListGetString(node->doc, node->xmlChildrenNode, 1);
					// Если значение ноды существует
					if(value != nullptr){
						// Получаем значение текущего блока данных
						const string item = reinterpret_cast <const char *> (value);
						// Выполняем удаление всех лишних символов
						this->_fmk->transform(item, fmk_t::transform_t::TRIM);
						// Если значение получено
						if(!item.empty()){
							// Получаем ключ записи
							string key("value");
							// Если элемент не является массивом
							if(result[reinterpret_cast <const char *> (node->name)].IsObject()){
								// Если такой ключ уже существует в объекте
								while(result[reinterpret_cast <const char *> (node->name)].HasMember(key.c_str()))
									// Выполняем изменение ключа
									key.insert(key.begin(), '_');
							// Иначе добавляем в указанный индекс массива
							} else if(result[reinterpret_cast <const char *> (node->name)].IsArray()) {
								// Получаем количество элементов в массиве
								const SizeType size = result[reinterpret_cast <const char *> (node->name)].Size();
								// Если такой ключ уже существует в объекте
								while(result[reinterpret_cast <const char *> (node->name)][size - 1].HasMember(key.c_str()))
									// Выполняем изменение ключа
									key.insert(key.begin(), '_');
							}
							// Если полученное значение является числом
							if(this->_fmk->is(item, fmk_t::check_t::NUMBER)){
								/**
								 * Выполняем отлов ошибок
								 */
								try {
									// Если число является отрицательным
									if(item.front() == '-'){
										// Если элемент не является массивом
										if(result[reinterpret_cast <const char *> (node->name)].IsObject()){
											// Если у ноды есть параметры
											if(node->properties != nullptr)
												// Выполняем формирования списка параметров
												result[reinterpret_cast <const char *> (node->name)].AddMember(Value(key.c_str(), key.length(), result.GetAllocator()).Move(), Value(static_cast <int64_t> (::stoll(item))).Move(), result.GetAllocator());
											// Выполняем установку полученного значения
											else result[reinterpret_cast <const char *> (node->name)].SetInt64(static_cast <int64_t> (::stoll(item)));
										// Иначе добавляем в указанный индекс массива
										} else if(result[reinterpret_cast <const char *> (node->name)].IsArray()) {
											// Получаем количество элементов в массиве
											const SizeType size = result[reinterpret_cast <const char *> (node->name)].Size();
											// Если у ноды есть параметры
											if(node->properties != nullptr)
												// Выполняем добавление названия атрибута
												result[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(key.c_str(), key.length(), result.GetAllocator()).Move(), Value(static_cast <int64_t> (::stoll(item))).Move(), result.GetAllocator());
											// Выполняем установку полученного значения
											else result[reinterpret_cast <const char *> (node->name)][size - 1].SetInt64(static_cast <int64_t> (::stoll(item)));
										}
									// Если число является положительным
									} else {
										// Если элемент не является массивом
										if(result[reinterpret_cast <const char *> (node->name)].IsObject()){
											// Если у ноды есть параметры
											if(node->properties != nullptr)
												// Выполняем формирования списка параметров
												result[reinterpret_cast <const char *> (node->name)].AddMember(Value(key.c_str(), key.length(), result.GetAllocator()).Move(), Value(static_cast <uint64_t> (::stoull(item))).Move(), result.GetAllocator());
											// Выполняем установку полученного значения
											else result[reinterpret_cast <const char *> (node->name)].SetUint64(static_cast <uint64_t> (::stoull(item)));
										// Иначе добавляем в указанный индекс массива
										} else if(result[reinterpret_cast <const char *> (node->name)].IsArray()) {
											// Получаем количество элементов в массиве
											const SizeType size = result[reinterpret_cast <const char *> (node->name)].Size();
											// Если у ноды есть параметры
											if(node->properties != nullptr)
												// Выполняем добавление названия атрибута
												result[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(key.c_str(), key.length(), result.GetAllocator()).Move(), Value(static_cast <uint64_t> (::stoull(item))).Move(), result.GetAllocator());
											// Выполняем установку полученного значения
											else result[reinterpret_cast <const char *> (node->name)][size - 1].SetUint64(static_cast <uint64_t> (::stoull(item)));
										}
									}
								/**
								 * Если возникает ошибка
								 */
								} catch(const std::exception &) {
									// Если элемент не является массивом
									if(result[reinterpret_cast <const char *> (node->name)].IsObject()){
										// Если у ноды есть параметры
										if(node->properties != nullptr)
											// Выполняем формирования списка параметров
											result[reinterpret_cast <const char *> (node->name)].AddMember(Value(key.c_str(), key.length(), result.GetAllocator()).Move(), Value(item.c_str(), item.length(), result.GetAllocator()).Move(), result.GetAllocator());
										// Выполняем установку полученного значения
										else result[reinterpret_cast <const char *> (node->name)].SetString(item.c_str(), item.length(), result.GetAllocator());
									// Иначе добавляем в указанный индекс массива
									} else if(result[reinterpret_cast <const char *> (node->name)].IsArray()) {
										// Получаем количество элементов в массиве
										const SizeType size = result[reinterpret_cast <const char *> (node->name)].Size();
										// Если у ноды есть параметры
										if(node->properties != nullptr)
											// Выполняем добавление названия атрибута
											result[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(key.c_str(), key.length(), result.GetAllocator()).Move(), Value(item.c_str(), item.length(), result.GetAllocator()).Move(), result.GetAllocator());
										// Выполняем установку полученного значения
										else result[reinterpret_cast <const char *> (node->name)][size - 1].SetString(item.c_str(), item.length(), result.GetAllocator());
									}
								}
							// Если полученное значение является числом с плавающей точкой
							} else if(this->_fmk->is(item, fmk_t::check_t::DECIMAL)) {
								/**
								 * Выполняем отлов ошибок
								 */
								try {
									// Если элемент не является массивом
									if(result[reinterpret_cast <const char *> (node->name)].IsObject()){
										// Если у ноды есть параметры
										if(node->properties != nullptr)
											// Выполняем формирования списка параметров
											result[reinterpret_cast <const char *> (node->name)].AddMember(Value(key.c_str(), key.length(), result.GetAllocator()).Move(), Value(::stod(item)).Move(), result.GetAllocator());
										// Выполняем установку полученного значения
										else result[reinterpret_cast <const char *> (node->name)].SetDouble(::stod(item));
									// Иначе добавляем в указанный индекс массива
									} else if(result[reinterpret_cast <const char *> (node->name)].IsArray()) {
										// Получаем количество элементов в массиве
										const SizeType size = result[reinterpret_cast <const char *> (node->name)].Size();
										// Если у ноды есть параметры
										if(node->properties != nullptr)
											// Выполняем добавление названия атрибута
											result[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(key.c_str(), key.length(), result.GetAllocator()).Move(), Value(::stod(item)).Move(), result.GetAllocator());
										// Выполняем установку полученного значения
										else result[reinterpret_cast <const char *> (node->name)][size - 1].SetDouble(::stod(item));
									}
								/**
								 * Если возникает ошибка
								 */
								} catch(const std::exception &) {
									// Если элемент не является массивом
									if(result[reinterpret_cast <const char *> (node->name)].IsObject()){
										// Если у ноды есть параметры
										if(node->properties != nullptr)
											// Выполняем формирования списка параметров
											result[reinterpret_cast <const char *> (node->name)].AddMember(Value(key.c_str(), key.length(), result.GetAllocator()).Move(), Value(item.c_str(), item.length(), result.GetAllocator()).Move(), result.GetAllocator());
										// Выполняем установку полученного значения
										else result[reinterpret_cast <const char *> (node->name)].SetString(item.c_str(), item.length(), result.GetAllocator());
									// Иначе добавляем в указанный индекс массива
									} else if(result[reinterpret_cast <const char *> (node->name)].IsArray()) {
										// Получаем количество элементов в массиве
										const SizeType size = result[reinterpret_cast <const char *> (node->name)].Size();
										// Если у ноды есть параметры
										if(node->properties != nullptr)
											// Выполняем добавление названия атрибута
											result[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(key.c_str(), key.length(), result.GetAllocator()).Move(), Value(item.c_str(), item.length(), result.GetAllocator()).Move(), result.GetAllocator());
										// Выполняем установку полученного значения
										else result[reinterpret_cast <const char *> (node->name)][size - 1].SetString(item.c_str(), item.length(), result.GetAllocator());
									}
								}
							// Если значение не является числом
							} else {
								// Флаг булевого значения
								bool flag = false;
								// Если строка является булевым значением
								if((flag = this->_fmk->compare("true", item)) || this->_fmk->compare("false", item)){
									// Если элемент не является массивом
									if(result[reinterpret_cast <const char *> (node->name)].IsObject()){
										// Если у ноды есть параметры
										if(node->properties != nullptr)
											// Выполняем формирования списка параметров
											result[reinterpret_cast <const char *> (node->name)].AddMember(Value(key.c_str(), key.length(), result.GetAllocator()).Move(), Value(flag).Move(), result.GetAllocator());
										// Выполняем установку полученного значения
										else result[reinterpret_cast <const char *> (node->name)].SetBool(flag);
									// Иначе добавляем в указанный индекс массива
									} else if(result[reinterpret_cast <const char *> (node->name)].IsArray()) {
										// Получаем количество элементов в массиве
										const SizeType size = result[reinterpret_cast <const char *> (node->name)].Size();
										// Если у ноды есть параметры
										if(node->properties != nullptr)
											// Выполняем добавление названия атрибута
											result[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(key.c_str(), key.length(), result.GetAllocator()).Move(), Value(flag).Move(), result.GetAllocator());
										// Выполняем установку полученного значения
										else result[reinterpret_cast <const char *> (node->name)][size - 1].SetBool(flag);
									}
								// Если значение является обычной строкой
								} else {
									// Если элемент не является массивом
									if(result[reinterpret_cast <const char *> (node->name)].IsObject()){
										// Если у ноды есть параметры
										if(node->properties != nullptr)
											// Выполняем формирования списка параметров
											result[reinterpret_cast <const char *> (node->name)].AddMember(Value(key.c_str(), key.length(), result.GetAllocator()).Move(), Value(item.c_str(), item.length(), result.GetAllocator()).Move(), result.GetAllocator());
										// Выполняем установку полученного значения
										else result[reinterpret_cast <const char *> (node->name)].SetString(item.c_str(), item.length(), result.GetAllocator());
									// Иначе добавляем в указанный индекс массива
									} else if(result[reinterpret_cast <const char *> (node->name)].IsArray()) {
										// Получаем количество элементов в массиве
										const SizeType size = result[reinterpret_cast <const char *> (node->name)].Size();
										// Если у ноды есть параметры
										if(node->properties != nullptr)
											// Выполняем добавление названия атрибута
											result[reinterpret_cast <const char *> (node->name)][size - 1].AddMember(Value(key.c_str(), key.length(), result.GetAllocator()).Move(), Value(item.c_str(), item.length(), result.GetAllocator()).Move(), result.GetAllocator());
										// Выполняем установку полученного значения
										else result[reinterpret_cast <const char *> (node->name)][size - 1].SetString(item.c_str(), item.length(), result.GetAllocator());
									}
								}
							}
						}
						// Выполняем освобождение памяти выделенной под значение
						xmlFree(value);
						// Выполняем парсинг всего XML объекта
						parseFn(result[reinterpret_cast <const char *> (node->name)], node->xmlChildrenNode, result.GetAllocator());
					}
				// Сообщаем, что переданные данные не соответствуют ожидаемым
				} else {
					/**
					 * Если включён режим отладки
					 */
					#if defined(DEBUG_MODE)
						// Выводим сообщение об ошибке
						this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(text), log_t::flag_t::WARNING, "Data received is not as expected");
					/**
					* Если режим отладки не включён
					*/
					#else
						// Выводим сообщение об ошибке
						this->_log->print("%s", log_t::flag_t::WARNING, "Data received is not as expected");
					#endif
				}
				// Выполняем очистку выделенной памяти под ноду
				// xmlFreeNode(node);
				// Выполняем очистку выделенных данных парсера
				xmlFreeDoc(doc);
				// Выполняем очистку глобальных параметров парсера
				xmlCleanupParser();
			// Сообщаем, что переданные данные не соответствуют ожидаемым
			} else {
				/**
				 * Если включён режим отладки
				 */
				#if defined(DEBUG_MODE)
					// Выводим сообщение об ошибке
					this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(text), log_t::flag_t::WARNING, "Data received is not as expected");
				/**
				* Если режим отладки не включён
				*/
				#else
					// Выводим сообщение об ошибке
					this->_log->print("%s", log_t::flag_t::WARNING, "Data received is not as expected");
				#endif
			}
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			/**
			 * Если включён режим отладки
			 */
			#if defined(DEBUG_MODE)
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(text), log_t::flag_t::CRITICAL, error.what());
			/**
			* Если режим отладки не включён
			*/
			#else
				// Выводим сообщение об ошибке
				this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
			#endif
		}
	}
	// Выводим результат
	return result;
}
/**
 * xml Метод конвертации объекта JSON в текст в формате XML
 * @param data     данные в объекте JSON
 * @param prettify флаг генерации читаемого формата
 * @return         текст после конвертации
 */
string anyks::Parser::xml(const Document & data, const bool prettify) noexcept {
	// Результат работы функции
	string result = "";
	// Если данные переданы
	if((data.IsObject() && !data.ObjectEmpty()) || (data.IsArray() && !data.Empty())){
		// Выполняем блокировку потока
		const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			/**
			 * Symbol Структура поиска запрещённого символа
			 */
			typedef struct Symbol {
				// Позиция символа для замены
				size_t pos;
				// Текст замены
				string val;
				/**
				 * Symbol Конструктор
				 */
				Symbol() noexcept : pos(string::npos), val{""} {}
			} symbol_t;
			/**
			 * findSymbolFn Функция поиска запрещённого символа
			 * @param text текст для поиска
			 * @param pos  начальная позиция поиска
			 * @return     позиция найденного символа
			 */
			auto findSymbolFn = [](const string & text, const size_t pos = 0) noexcept -> symbol_t {
				// Результат работы функции
				symbol_t result;
				// Если текст для парсинга передан
				if(!text.empty()){
					// Перебираем полученный текст
					for(size_t i = pos; i < text.length(); i++){
						// Выполняем проверку символа
						switch(text.at(i)){
							// Если найден символ кавычка
							case '"': {
								// Устанавливаем позицию символа
								result.pos = i;
								// Устанавливаем текст для замены
								result.val = "&quot;";
								// Выводим результат
								return result;
							}
							// Если найден символ апострофа
							case '\'': {
								// Устанавливаем позицию символа
								result.pos = i;
								// Устанавливаем текст для замены
								result.val = "&apos;";
								// Выводим результат
								return result;
							}
							// Если найден символ знака меньше
							case '<': {
								// Устанавливаем позицию символа
								result.pos = i;
								// Устанавливаем текст для замены
								result.val = "&lt;";
								// Выводим результат
								return result;
							}
							// Если найден символ знака больше
							case '>': {
								// Устанавливаем позицию символа
								result.pos = i;
								// Устанавливаем текст для замены
								result.val = "&gt;";
								// Выводим результат
								return result;
							}
							// Если найден символ знака амперсанда
							case '&': {
								// Устанавливаем позицию символа
								result.pos = i;
								// Устанавливаем текст для замены
								result.val = "&amp;";
								// Выводим результат
								return result;
							}
						}
					}
				}
				// Выводим результат
				return result;
			};
			/**
			 * removeBracketsFn Метод удаления скобок
			 * @param text текст в котором следует удалить скобки
			 */
			auto removeBracketsFn = [&findSymbolFn](const string & text) noexcept -> string {
				// Результат работы функции
				string result = text;
				// Если текст для обработки передан
				if(!result.empty()){
					// Позиция скобки в тексте
					symbol_t symbol;
					// Устанавливаем начальную позицию поиска
					symbol.pos = 0;
					// Флаг начала извлечения всего списка переменных
					Process:
					// Выполняем поиск скобки в тексте
					symbol = findSymbolFn(result, symbol.pos);
					// Если позиция скобки в тексте найдена
					if(symbol.pos != string::npos){
						// Выполняем замену в тексте скобки
						result.replace(symbol.pos, 1, symbol.val);
						// Увеличиваем текущую позицию
						symbol.pos += symbol.val.length();
						// Выполняем поиск скобок дальше
						goto Process;
					}
				}
				// Выводим полученный результат
				return result;
			};
			// Результат работы функции
			result = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>";
			// Если разрешено выполнять разложение XML-объекта
			if(prettify)
				// Выполняем добавление переноса строк
				result.append(1, '\n');
			/**
			 * parseFn Прототип функции парсинга XML документа
			 */
			function <void (string &, const Document &, const uint16_t)> parseFn;
			/**
			 * simpleFn, arrayFn, objectFn Прототип функций формирования разметки JSON
			 */
			function <void (const string &, const Value &, string &, const uint16_t)> simpleFn, arrayFn, objectFn;
			/**
			 * arrayFn Функция формирования разметки массива
			 * @param key    ключ записи для формирования
			 * @param value  значение записи для формирования
			 * @param result итоговый собранный результат в формате XML
			 * @param tabs   смещение в итоговом результате позиции разметки
			 */
			arrayFn = [&](const string & key, const Value & value, string & result, const uint16_t tabs) noexcept -> void {
				// Если значение является массивом
				if(value.IsArray()){
					// Получаем флаг генерации ключа
					const bool flag = this->_fmk->compare("item", key);
					// Если ключ является сгенерированным
					if(flag){
						// Если количество отступов больше нуля
						if(prettify && (tabs > 0)){
							// Выполняем установку количества отступов
							for(uint16_t i = 0; i < tabs; i++)
								// Выполняем добавление отступов
								result.append(1, '\t');
						}
						// Выполняем открытие тега
						result.append(1, '<');
						// Выполняем формирование результата
						result.append(key);
						// Выполняем закрытие тега
						result.append(1, '>');
						// Если разрешено выполнять разложение XML-объекта
						if(prettify)
							// Выполняем добавление переноса строк
							result.append(1, '\n');
					}
					// Выполняем переход по всему массиву
					for(auto & v : value.GetArray()){
						// Если значение является отрицательным 32-х битным числом
						if(v.IsInt()){
							// Если количество отступов больше нуля
							if(prettify && (tabs > 0)){
								// Выполняем установку количества отступов
								for(uint16_t i = 0; i < (tabs + (flag ? 1 : 0)); i++)
									// Выполняем добавление отступов
									result.append(1, '\t');
							}
							// Выполняем открытие тега
							result.append(1, '<');
							// Выполняем формирование результата
							result.append(key);
							// Выполняем закрытие тега
							result.append(1, '>');
							// Выполняем установку полученного числа
							result.append(std::to_string(v.GetInt()));
							// Выполняем закрытие тега
							result.append("</");
							// Выполняем установку тега
							result.append(key);
							// Выполняем закрытие тега
							result.append(1, '>');
							// Если разрешено выполнять разложение XML-объекта
							if(prettify)
								// Выполняем добавление переноса строк
								result.append(1, '\n');
						// Если значение является положительным 32-х битным числом
						} else if(v.IsUint()) {
							// Если количество отступов больше нуля
							if(prettify && (tabs > 0)){
								// Выполняем установку количества отступов
								for(uint16_t i = 0; i < (tabs + (flag ? 1 : 0)); i++)
									// Выполняем добавление отступов
									result.append(1, '\t');
							}
							// Выполняем открытие тега
							result.append(1, '<');
							// Выполняем формирование результата
							result.append(key);
							// Выполняем закрытие тега
							result.append(1, '>');
							// Выполняем установку полученного числа
							result.append(std::to_string(v.GetUint()));
							// Выполняем закрытие тега
							result.append("</");
							// Выполняем установку тега
							result.append(key);
							// Выполняем закрытие тега
							result.append(1, '>');
							// Если разрешено выполнять разложение XML-объекта
							if(prettify)
								// Выполняем добавление переноса строк
								result.append(1, '\n');
						// Если значение является отрицательным 64-х битным числом
						} else if(v.IsInt64()) {
							// Если количество отступов больше нуля
							if(prettify && (tabs > 0)){
								// Выполняем установку количества отступов
								for(uint16_t i = 0; i < (tabs + (flag ? 1 : 0)); i++)
									// Выполняем добавление отступов
									result.append(1, '\t');
							}
							// Выполняем открытие тега
							result.append(1, '<');
							// Выполняем формирование результата
							result.append(key);
							// Выполняем закрытие тега
							result.append(1, '>');
							// Выполняем установку полученного числа
							result.append(std::to_string(v.GetInt64()));
							// Выполняем закрытие тега
							result.append("</");
							// Выполняем установку тега
							result.append(key);
							// Выполняем закрытие тега
							result.append(1, '>');
							// Если разрешено выполнять разложение XML-объекта
							if(prettify)
								// Выполняем добавление переноса строк
								result.append(1, '\n');
						// Если значение является положительным 64-х битным числом
						} else if(v.IsUint64()) {
							// Если количество отступов больше нуля
							if(prettify && (tabs > 0)){
								// Выполняем установку количества отступов
								for(uint16_t i = 0; i < (tabs + (flag ? 1 : 0)); i++)
									// Выполняем добавление отступов
									result.append(1, '\t');
							}
							// Выполняем открытие тега
							result.append(1, '<');
							// Выполняем формирование результата
							result.append(key);
							// Выполняем закрытие тега
							result.append(1, '>');
							// Выполняем установку полученного числа
							result.append(std::to_string(v.GetUint64()));
							// Выполняем закрытие тега
							result.append("</");
							// Выполняем установку тега
							result.append(key);
							// Выполняем закрытие тега
							result.append(1, '>');
							// Если разрешено выполнять разложение XML-объекта
							if(prettify)
								// Выполняем добавление переноса строк
								result.append(1, '\n');
						// Если значение является числом с правающей точкой
						} else if(v.IsFloat()) {
							// Если количество отступов больше нуля
							if(prettify && (tabs > 0)){
								// Выполняем установку количества отступов
								for(uint16_t i = 0; i < (tabs + (flag ? 1 : 0)); i++)
									// Выполняем добавление отступов
									result.append(1, '\t');
							}
							// Выполняем открытие тега
							result.append(1, '<');
							// Выполняем формирование результата
							result.append(key);
							// Выполняем закрытие тега
							result.append(1, '>');
							// Выполняем установку полученного числа
							result.append(this->_fmk->noexp(v.GetFloat(), true));
							// Выполняем закрытие тега
							result.append("</");
							// Выполняем установку тега
							result.append(key);
							// Выполняем закрытие тега
							result.append(1, '>');
							// Если разрешено выполнять разложение XML-объекта
							if(prettify)
								// Выполняем добавление переноса строк
								result.append(1, '\n');
						// Если значение является числом с правающей точкой двойной точности
						} else if(v.IsDouble()) {
							// Если количество отступов больше нуля
							if(prettify && (tabs > 0)){
								// Выполняем установку количества отступов
								for(uint16_t i = 0; i < (tabs + (flag ? 1 : 0)); i++)
									// Выполняем добавление отступов
									result.append(1, '\t');
							}
							// Выполняем открытие тега
							result.append(1, '<');
							// Выполняем формирование результата
							result.append(key);
							// Выполняем закрытие тега
							result.append(1, '>');
							// Выполняем установку полученного числа
							result.append(this->_fmk->noexp(v.GetDouble(), true));
							// Выполняем закрытие тега
							result.append("</");
							// Выполняем установку тега
							result.append(key);
							// Выполняем закрытие тега
							result.append(1, '>');
							// Если разрешено выполнять разложение XML-объекта
							if(prettify)
								// Выполняем добавление переноса строк
								result.append(1, '\n');
						// Если значение является строкой
						} else if(v.IsString()) {
							// Если количество отступов больше нуля
							if(prettify && (tabs > 0)){
								// Выполняем установку количества отступов
								for(uint16_t i = 0; i < (tabs + (flag ? 1 : 0)); i++)
									// Выполняем добавление отступов
									result.append(1, '\t');
							}
							// Выполняем открытие тега
							result.append(1, '<');
							// Выполняем формирование результата
							result.append(key);
							// Выполняем закрытие тега
							result.append(1, '>');
							// Выполняем установку строки
							result.append(removeBracketsFn(v.GetString()));
							// Выполняем закрытие тега
							result.append("</");
							// Выполняем установку тега
							result.append(key);
							// Выполняем закрытие тега
							result.append(1, '>');
							// Если разрешено выполнять разложение XML-объекта
							if(prettify)
								// Выполняем добавление переноса строк
								result.append(1, '\n');
						// Если значение является булевым значением
						} else if(v.IsBool()) {
							// Если количество отступов больше нуля
							if(prettify && (tabs > 0)){
								// Выполняем установку количества отступов
								for(uint16_t i = 0; i < (tabs + (flag ? 1 : 0)); i++)
									// Выполняем добавление отступов
									result.append(1, '\t');
							}
							// Выполняем открытие тега
							result.append(1, '<');
							// Выполняем формирование результата
							result.append(key);
							// Выполняем закрытие тега
							result.append(1, '>');
							// Выполняем установку строки
							result.append(v.GetBool() ? "True" : "False");
							// Выполняем закрытие тега
							result.append("</");
							// Выполняем установку тега
							result.append(key);
							// Выполняем закрытие тега
							result.append(1, '>');
							// Если разрешено выполнять разложение XML-объекта
							if(prettify)
								// Выполняем добавление переноса строк
								result.append(1, '\n');
						// Если значение является пустым значением
						} else if(v.IsNull()) {
							// Если количество отступов больше нуля
							if(prettify && (tabs > 0)){
								// Выполняем установку количества отступов
								for(uint16_t i = 0; i < (tabs + (flag ? 1 : 0)); i++)
									// Выполняем добавление отступов
									result.append(1, '\t');
							}
							// Выполняем открытие тега
							result.append(1, '<');
							// Выполняем формирование результата
							result.append(key);
							// Выполняем закрытие тега
							result.append(1, '>');
							// Выполняем установку строки
							result.append("Null");
							// Выполняем закрытие тега
							result.append("</");
							// Выполняем установку тега
							result.append(key);
							// Выполняем закрытие тега
							result.append(1, '>');
							// Если разрешено выполнять разложение XML-объекта
							if(prettify)
								// Выполняем добавление переноса строк
								result.append(1, '\n');
						// Если значение является объектом
						} else if(v.IsObject())
							// Выполняем извлечение данных таблицы
							objectFn(key, v, result, (tabs + (flag ? 1 : 0)));
						// Если значение является массивом
						else if(v.IsArray()) {
							// Если ключ не является сгенерированным
							if(!flag && prettify && (tabs > 0)){
								// Выполняем установку количества отступов
								for(uint16_t i = 0; i < tabs; i++)
									// Выполняем добавление отступов
									result.append(1, '\t');
							}
							// Формируем объект объекта
							Document data(kObjectType);
							// Добавляем полученный объекта
							data.AddMember(Value("item", data.GetAllocator()).Move(), Value(v, data.GetAllocator()).Move(), data.GetAllocator());
							// Выполняем извлечение данных объекта
							parseFn(result, data, tabs + 1);
							// Если ключ не является сгенерированным
							if(!flag && prettify)
								// Выполняем добавление переноса строк
								result.append(1, '\n');
						}
					}
					// Если ключ является сгенерированным
					if(flag){
						// Если количество отступов больше нуля
						if(prettify && (tabs > 0)){
							// Выполняем установку количества отступов
							for(uint16_t i = 0; i < tabs; i++)
								// Выполняем добавление отступов
								result.append(1, '\t');
						}
						// Выполняем закрытие тега
						result.append("</");
						// Выполняем установку тега
						result.append(key);
						// Выполняем закрытие тега
						result.append(1, '>');
						// Если разрешено выполнять разложение XML-объекта
						if(prettify)
							// Выполняем добавление переноса строк
							result.append(1, '\n');
					}
				}
			};
			/**
			 * objectFn Функция формирования разметки объекта
			 * @param key    ключ записи для формирования
			 * @param value  значение записи для формирования
			 * @param result итоговый собранный результат в формате XML
			 * @param tabs   смещение в итоговом результате позиции разметки
			 */
			objectFn = [&](const string & key, const Value & value, string & result, const uint16_t tabs) noexcept -> void {
				// Если значение является объектом
				if(value.IsObject()){
					// Если количество отступов больше нуля
					if(prettify && (tabs > 0)){
						// Выполняем установку количества отступов
						for(uint16_t i = 0; i < tabs; i++)
							// Выполняем добавление отступов
							result.append(1, '\t');
					}
					// Выполняем открытие тега
					result.append(1, '<');
					// Выполняем формирование результата
					result.append(key);
					// Флаг формирования сложного тега
					bool difficult = false;
					// Выполняем поиск вложенных объектов и массивов
					for(auto & m : value.GetObj()){
						// Если найден массив или объект
						if((difficult = (m.value.IsObject() || m.value.IsArray())))
							// Выходим из цикла
							break;
					}
					// Если нужно сформировать простой тег
					if(!difficult){
						// Значение записи тега
						string item = "";
						// Позиция ключа поиска
						size_t pos = string::npos, count = 0;
						// Выполняем перебор всех значений объекта
						for(auto & m : value.GetObj()){
							// Получаем ключ записи
							const string & key = (this->_fmk->is(m.name.GetString(), fmk_t::check_t::NUMBER) ? this->_fmk->format("Item%s", m.name.GetString()) : m.name.GetString());
							// Если ключом является устанавливаемое значение							
							if((pos = key.rfind("value")) != string::npos){
								// Если значение ещё не установлено
								if(item.empty() || (pos > count)){
									// Если значение уже установлено и количество подчеркиваний больше чем было
									if(!item.empty() && (pos > count)){
										// Выполняем добавление разделителя параметра
										result.append(1, ' ');
										// Выполняем добавление ключа записи
										result.append(key.substr(pos - count));
										// Выполняем добавление знака присвоения
										result.append("=\"");
										// Выполняем добавление установленного значения
										result.append(item);
										// Выполняем добавление экранирование параметра
										result.append(1, '"');
									}
									// Запоминаем количество найденных подчеркиваний
									count = pos;
									// Если значение является отрицательным 32-х битным числом
									if(m.value.IsInt()){
										// Выполняем получение установленного числа
										item = std::to_string(m.value.GetInt());
										// Продолжаем дальше
										continue;
									// Если значение является положительным 32-х битным числом
									} else if(m.value.IsUint()) {
										// Выполняем получение установленного числа
										item = std::to_string(m.value.GetUint());
										// Продолжаем дальше
										continue;
									// Если значение является отрицательным 64-х битным числом
									} else if(m.value.IsInt64()) {
										// Выполняем получение установленного числа
										item = std::to_string(m.value.GetInt64());
										// Продолжаем дальше
										continue;
									// Если значение является положительным 64-х битным числом
									} else if(m.value.IsUint64()) {
										// Выполняем получение установленного числа
										item = std::to_string(m.value.GetUint64());
										// Продолжаем дальше
										continue;
									// Если значение является числом с плавающей точкой
									} else if(m.value.IsFloat()) {
										// Выполняем получение установленного числа
										item = this->_fmk->noexp(m.value.GetFloat(), true);
										// Продолжаем дальше
										continue;
									// Если значение является числом с плавающей точкой двойной точности
									} else if(m.value.IsDouble()) {
										// Выполняем получение установленного числа
										item = this->_fmk->noexp(m.value.GetDouble(), true);
										// Продолжаем дальше
										continue;
									// Если значение является строкой
									} else if(m.value.IsString()) {
										// Выполняем установку полученного значения
										item = removeBracketsFn(m.value.GetString());
										// Продолжаем дальше
										continue;
									// Если значение является булевым значением
									} else if(m.value.IsBool()) {
										// Выполняем установку полученного значения
										item = (m.value.GetBool() ? "True" : "False");
										// Продолжаем дальше
										continue;
									// Если значение является пустым значением
									} else if(m.value.IsNull()) {
										// Выполняем установку полученного значения
										item = "Null";
										// Продолжаем дальше
										continue;
									}
								}
							}
							// Если значение является отрицательным 32-х битным числом
							if(m.value.IsInt()){
								// Выполняем добавление разделителя параметра
								result.append(1, ' ');
								// Выполняем добавление ключа записи
								result.append(key);
								// Выполняем добавление знака присвоения
								result.append("=\"");
								// Выполняем установку полученного числа
								result.append(std::to_string(m.value.GetInt()));
								// Выполняем добавление экранирование параметра
								result.append(1, '"');
							// Если значение является положительным 32-х битным числом
							} else if(m.value.IsUint()) {
								// Выполняем добавление разделителя параметра
								result.append(1, ' ');
								// Выполняем добавление ключа записи
								result.append(key);
								// Выполняем добавление знака присвоения
								result.append("=\"");
								// Выполняем установку полученного числа
								result.append(std::to_string(m.value.GetUint()));
								// Выполняем добавление экранирование параметра
								result.append(1, '"');
							// Если значение является отрицательным 64-х битным числом
							} else if(m.value.IsInt64()) {
								// Выполняем добавление разделителя параметра
								result.append(1, ' ');
								// Выполняем добавление ключа записи
								result.append(key);
								// Выполняем добавление знака присвоения
								result.append("=\"");
								// Выполняем установку полученного числа
								result.append(std::to_string(m.value.GetInt64()));
								// Выполняем добавление экранирование параметра
								result.append(1, '"');
							// Если значение является положительным 64-х битным числом
							} else if(m.value.IsUint64()) {
								// Выполняем добавление разделителя параметра
								result.append(1, ' ');
								// Выполняем добавление ключа записи
								result.append(key);
								// Выполняем добавление знака присвоения
								result.append("=\"");
								// Выполняем установку полученного числа
								result.append(std::to_string(m.value.GetUint64()));
								// Выполняем добавление экранирование параметра
								result.append(1, '"');
							// Если значение является числом с правающей точкой
							} else if(m.value.IsFloat()) {
								// Выполняем добавление разделителя параметра
								result.append(1, ' ');
								// Выполняем добавление ключа записи
								result.append(key);
								// Выполняем добавление знака присвоения
								result.append("=\"");
								// Выполняем установку полученного числа
								result.append(this->_fmk->noexp(m.value.GetFloat(), true));
								// Выполняем добавление экранирование параметра
								result.append(1, '"');
							// Если значение является числом с правающей точкой двойной точности
							} else if(m.value.IsDouble()) {
								// Выполняем добавление разделителя параметра
								result.append(1, ' ');
								// Выполняем добавление ключа записи
								result.append(key);
								// Выполняем добавление знака присвоения
								result.append("=\"");
								// Выполняем установку полученного числа
								result.append(this->_fmk->noexp(m.value.GetDouble(), true));
								// Выполняем добавление экранирование параметра
								result.append(1, '"');
							// Если значение является строкой
							} else if(m.value.IsString()) {
								// Выполняем добавление разделителя параметра
								result.append(1, ' ');
								// Выполняем добавление ключа записи
								result.append(key);
								// Выполняем добавление знака присвоения
								result.append("=\"");
								// Выполняем добавление значения
								result.append(removeBracketsFn(m.value.GetString()));
								// Выполняем добавление экранирование параметра
								result.append(1, '"');
							// Если значение является булевым значением
							} else if(m.value.IsBool()) {
								// Выполняем добавление разделителя параметра
								result.append(1, ' ');
								// Выполняем добавление ключа записи
								result.append(key);
								// Выполняем добавление знака присвоения
								result.append("=\"");
								// Выполняем добавление значения
								result.append(m.value.GetBool() ? "True" : "False");
								// Выполняем добавление экранирование параметра
								result.append(1, '"');
							// Если значение является пустым значением
							} else if(m.value.IsNull()) {
								// Выполняем добавление разделителя параметра
								result.append(1, ' ');
								// Выполняем добавление ключа записи
								result.append(key);
								// Выполняем добавление знака присвоения
								result.append("=\"");
								// Выполняем добавление значения
								result.append("Null");
								// Выполняем добавление экранирование параметра
								result.append(1, '"');
							}
						}
						// Если значение не получено
						if(item.empty())
							// Выполняем установку закрывающего тега
							result.append("/>");
						// Если значение получено
						else {
							// Выполняем закрытие тега
							result.append(1, '>');
							// Выполняем добавление установленного значения
							result.append(item);
							// Выполняем закрытие тега
							result.append("</");
							// Выполняем установку тега
							result.append(key);
							// Выполняем закрытие тега
							result.append(1, '>');
						}
					// Если необходимо сформировать расширенный XML
					} else {
						// Выполняем закрытие тега
						result.append(1, '>');
						// Если разрешено выполнять разложение XML-объекта
						if(prettify)
							// Выполняем добавление переноса строк
							result.append(1, '\n');
						// Формируем объект объекта
						Document data;
						// Извлекаем данные значения
						data.CopyFrom(value, data.GetAllocator());
						// Выполняем извлечение данных объекта
						parseFn(result, data, tabs + 1);
						// Если количество отступов больше нуля
						if(prettify && (tabs > 0)){
							// Выполняем установку количества отступов
							for(uint16_t i = 0; i < tabs; i++)
								// Выполняем добавление отступов
								result.append(1, '\t');
						}
						// Выполняем закрытие тега
						result.append("</");
						// Выполняем установку тега
						result.append(key);
						// Выполняем закрытие тега
						result.append(1, '>');
					}
					// Если разрешено выполнять разложение XML-объекта
					if(prettify)
						// Выполняем добавление переноса строк
						result.append(1, '\n');
				}
			};
			/**
			 * simpleFn Функция формирования разметки простых данных
			 * @param key    ключ записи для формирования
			 * @param value  значение записи для формирования
			 * @param result итоговый собранный результат в формате XML
			 * @param tabs   смещение в итоговом результате позиции разметки
			 */
			simpleFn = [&](const string & key, const Value & value, string & result, const uint16_t tabs) noexcept -> void {
				// Если значение является отрицательным 32-х битным числом
				if(value.IsInt()){
					// Если количество отступов больше нуля
					if(prettify && (tabs > 0)){
						// Выполняем установку количества отступов
						for(uint16_t i = 0; i < tabs; i++)
							// Выполняем добавление отступов
							result.append(1, '\t');
					}
					// Выполняем открытие тега
					result.append(1, '<');
					// Выполняем формирование результата
					result.append(key);
					// Выполняем закрытие тега
					result.append(1, '>');
					// Добавляем полученное число
					result.append(std::to_string(value.GetInt()));
					// Выполняем закрытие тега
					result.append("</");
					// Выполняем установку тега
					result.append(key);
					// Выполняем закрытие тега
					result.append(1, '>');
					// Если разрешено выполнять разложение XML-объекта
					if(prettify)
						// Выполняем добавление переноса строк
						result.append(1, '\n');
				// Если значение является положительным 32-х битным числом
				} else if(value.IsUint()) {
					// Если количество отступов больше нуля
					if(prettify && (tabs > 0)){
						// Выполняем установку количества отступов
						for(uint16_t i = 0; i < tabs; i++)
							// Выполняем добавление отступов
							result.append(1, '\t');
					}
					// Выполняем открытие тега
					result.append(1, '<');
					// Выполняем формирование результата
					result.append(key);
					// Выполняем закрытие тега
					result.append(1, '>');
					// Добавляем полученное число
					result.append(std::to_string(value.GetUint()));
					// Выполняем закрытие тега
					result.append("</");
					// Выполняем установку тега
					result.append(key);
					// Выполняем закрытие тега
					result.append(1, '>');
					// Если разрешено выполнять разложение XML-объекта
					if(prettify)
						// Выполняем добавление переноса строк
						result.append(1, '\n');
				// Если значение является отрицательным 64-х битным числом
				} else if(value.IsInt()) {
					// Если количество отступов больше нуля
					if(prettify && (tabs > 0)){
						// Выполняем установку количества отступов
						for(uint16_t i = 0; i < tabs; i++)
							// Выполняем добавление отступов
							result.append(1, '\t');
					}
					// Выполняем открытие тега
					result.append(1, '<');
					// Выполняем формирование результата
					result.append(key);
					// Выполняем закрытие тега
					result.append(1, '>');
					// Добавляем полученное число
					result.append(std::to_string(value.GetInt64()));
					// Выполняем закрытие тега
					result.append("</");
					// Выполняем установку тега
					result.append(key);
					// Выполняем закрытие тега
					result.append(1, '>');
					// Если разрешено выполнять разложение XML-объекта
					if(prettify)
						// Выполняем добавление переноса строк
						result.append(1, '\n');
				// Если значение является положительным 64-х битным числом
				} else if(value.IsUint64()) {
					// Если количество отступов больше нуля
					if(prettify && (tabs > 0)){
						// Выполняем установку количества отступов
						for(uint16_t i = 0; i < tabs; i++)
							// Выполняем добавление отступов
							result.append(1, '\t');
					}
					// Выполняем открытие тега
					result.append(1, '<');
					// Выполняем формирование результата
					result.append(key);
					// Выполняем закрытие тега
					result.append(1, '>');
					// Добавляем полученное число
					result.append(std::to_string(value.GetUint64()));
					// Выполняем закрытие тега
					result.append("</");
					// Выполняем установку тега
					result.append(key);
					// Выполняем закрытие тега
					result.append(1, '>');
					// Если разрешено выполнять разложение XML-объекта
					if(prettify)
						// Выполняем добавление переноса строк
						result.append(1, '\n');
				// Если значение является числом с плавающей точкой
				} else if(value.IsFloat()) {
					// Если количество отступов больше нуля
					if(prettify && (tabs > 0)){
						// Выполняем установку количества отступов
						for(uint16_t i = 0; i < tabs; i++)
							// Выполняем добавление отступов
							result.append(1, '\t');
					}
					// Выполняем открытие тега
					result.append(1, '<');
					// Выполняем формирование результата
					result.append(key);
					// Выполняем закрытие тега
					result.append(1, '>');
					// Добавляем полученное число
					result.append(this->_fmk->noexp(value.GetFloat(), true));
					// Выполняем закрытие тега
					result.append("</");
					// Выполняем установку тега
					result.append(key);
					// Выполняем закрытие тега
					result.append(1, '>');
					// Если разрешено выполнять разложение XML-объекта
					if(prettify)
						// Выполняем добавление переноса строк
						result.append(1, '\n');
				// Если значение является числом с плавающей точкой двойной точности
				} else if(value.IsDouble()) {
					// Если количество отступов больше нуля
					if(prettify && (tabs > 0)){
						// Выполняем установку количества отступов
						for(uint16_t i = 0; i < tabs; i++)
							// Выполняем добавление отступов
							result.append(1, '\t');
					}
					// Выполняем открытие тега
					result.append(1, '<');
					// Выполняем формирование результата
					result.append(key);
					// Выполняем закрытие тега
					result.append(1, '>');
					// Добавляем полученное число
					result.append(this->_fmk->noexp(value.GetDouble(), true));
					// Выполняем закрытие тега
					result.append("</");
					// Выполняем установку тега
					result.append(key);
					// Выполняем закрытие тега
					result.append(1, '>');
					// Если разрешено выполнять разложение XML-объекта
					if(prettify)
						// Выполняем добавление переноса строк
						result.append(1, '\n');
				// Если значение является строкой
				} else if(value.IsString()) {
					// Если количество отступов больше нуля
					if(prettify && (tabs > 0)){
						// Выполняем установку количества отступов
						for(uint16_t i = 0; i < tabs; i++)
							// Выполняем добавление отступов
							result.append(1, '\t');
					}
					// Выполняем открытие тега
					result.append(1, '<');
					// Выполняем формирование результата
					result.append(key);
					// Выполняем закрытие тега
					result.append(1, '>');
					// Выполняем установку строки
					result.append(removeBracketsFn(value.GetString()));
					// Выполняем закрытие тега
					result.append("</");
					// Выполняем установку тега
					result.append(key);
					// Выполняем закрытие тега
					result.append(1, '>');
					// Если разрешено выполнять разложение XML-объекта
					if(prettify)
						// Выполняем добавление переноса строк
						result.append(1, '\n');
				// Если значение является булевым значением
				} else if(value.IsBool()) {
					// Если количество отступов больше нуля
					if(prettify && (tabs > 0)){
						// Выполняем установку количества отступов
						for(uint16_t i = 0; i < tabs; i++)
							// Выполняем добавление отступов
							result.append(1, '\t');
					}
					// Если значение истинное
					if(value.GetBool()){
						// Выполняем открытие тега
						result.append(1, '<');
						// Выполняем формирование результата
						result.append(key);
						// Выполняем закрытие тега
						result.append("/>");
					// Если значение ложное
					} else if(!value.GetBool()) {
						// Выполняем открытие тега
						result.append(1, '<');
						// Выполняем формирование результата
						result.append(key);
						// Выполняем закрытие тега
						result.append(1, '>');
						// Выполняем установку строки
						result.append("False");
						// Выполняем закрытие тега
						result.append("</");
						// Выполняем установку тега
						result.append(key);
						// Выполняем закрытие тега
						result.append(1, '>');
					}
					// Если разрешено выполнять разложение XML-объекта
					if(prettify)
						// Выполняем добавление переноса строк
						result.append(1, '\n');
				// Если значение является пустым значением
				} else if(value.IsNull()) {
					// Если количество отступов больше нуля
					if(prettify && (tabs > 0)){
						// Выполняем установку количества отступов
						for(uint16_t i = 0; i < tabs; i++)
							// Выполняем добавление отступов
							result.append(1, '\t');
					}
					// Выполняем открытие тега
					result.append(1, '<');
					// Выполняем формирование результата
					result.append(key);
					// Выполняем закрытие тега
					result.append(1, '>');
					// Выполняем установку строки
					result.append("Null");
					// Выполняем закрытие тега
					result.append("</");
					// Выполняем установку тега
					result.append(key);
					// Выполняем закрытие тега
					result.append(1, '>');
					// Если разрешено выполнять разложение XML-объекта
					if(prettify)
						// Выполняем добавление переноса строк
						result.append(1, '\n');
				// Если значение является объектом
				} else if(value.IsObject())
					// Выполняем разбор объекта
					objectFn(key, value, result, tabs);
				// Если значение является массивом
				else if(value.IsArray())
					// Выполняем разбор массива
					arrayFn(key, value, result, tabs);
			};
			/**
			 * parseFn Функция парсинга XML документа
			 * @param root корень объекта для записи результата
			 * @param node объект текущей ноды
			 * @param tabs количество отступов
			 */
			parseFn = [&](string & root, const Document & node, const uint16_t tabs) noexcept -> void {
				// Создаём объект результата
				string result = "";
				// Если нода является объектом
				if(node.IsObject()){
					// Переходим по всему объекту
					for(auto & m : node.GetObj()){
						// Получаем ключ записи
						const string & key = m.name.GetString();
						// Выполняем обработку полученного результата
						simpleFn(key, m.value, result, tabs);
					}
				// Если нода является массивом
				} else if(node.IsArray()) {
					// Получаем ключ записи
					const string & key = "item";
					// Переходим по всему массиву
					for(auto & v : node.GetArray())
						// Выполняем обработку полученного результата
						simpleFn(key, v, result, tabs);
				}
				// Если результат сформирован
				if(!result.empty())
					// Формируем объект ключа
					root.append(result);
			};
			// Если нет корневого элемента
			if(data.IsArray() || (data.MemberCount() > 1) || data.MemberBegin()->value.IsArray()){
				// Добавляем тип документа
				result.append("<!DOCTYPE root>");
				// Если разрешено выполнять разложение XML-объекта
				if(prettify)
					// Выполняем добавление переноса строк
					result.append(1, '\n');
				// Выполняем открытие тега
				result.append(1, '<');
				// Добавляем название блока
				result.append("root");
				// Выполняем закрытие тега
				result.append(1, '>');
				// Если разрешено выполнять разложение XML-объекта
				if(prettify)
					// Выполняем добавление переноса строк
					result.append(1, '\n');
				// Выполняем парсинг объекта XML
				parseFn(result, data, 1);
				// Выполняем закрытие тега
				result.append("</");
				// Выполняем установку тега
				result.append("root");
				// Выполняем закрытие тега
				result.append(1, '>');
				// Если разрешено выполнять разложение XML-объекта
				if(prettify)
					// Выполняем добавление переноса строк
					result.append(1, '\n');
			// Если корневой элемент присутствует
			} else {
				// Добавляем тип документа
				result.append(this->_fmk->format("<!DOCTYPE %s>", data.MemberBegin()->name.GetString()));
				// Если разрешено выполнять разложение XML-объекта
				if(prettify)
					// Выполняем добавление переноса строк
					result.append(1, '\n');
				// Выполняем парсинг объекта XML
				parseFn(result, data, 0);
			}
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			/**
			 * Если включён режим отладки
			 */
			#if defined(DEBUG_MODE)
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, {}, log_t::flag_t::CRITICAL, error.what());
			/**
			* Если режим отладки не включён
			*/
			#else
				// Выводим сообщение об ошибке
				this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
			#endif
		}
	}
	// Выводим результат
	return result;
}
/**
 * json Метод конвертации текста в формате JSON в объект JSON
 * @param text текст для конвертации
 * @return     объект в формате JSON
 */
Document anyks::Parser::json(const string & text) noexcept {
	// Результат работы функции
	Document result(kObjectType);
	// Если текст передан
	if(!text.empty()){
		// Выполняем блокировку потока
		const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем парсинг полученных текстовых данных
			if(result.Parse(text.c_str(), text.size()).HasParseError()){
				/**
				 * Если включён режим отладки
				 */
				#if defined(DEBUG_MODE)
					// Выводим сообщение об ошибке
					this->_log->debug("Parsing JSON: (offset %d): %s", __PRETTY_FUNCTION__, std::make_tuple(text), log_t::flag_t::CRITICAL, result.GetErrorOffset(), GetParseError_En(result.GetParseError()));
				/**
				* Если режим отладки не включён
				*/
				#else
					// Выводим сообщение об ошибке
					this->_log->print("Parsing JSON: (offset %d): %s", log_t::flag_t::CRITICAL, result.GetErrorOffset(), GetParseError_En(result.GetParseError()));
				#endif
			}
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			/**
			 * Если включён режим отладки
			 */
			#if defined(DEBUG_MODE)
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(text), log_t::flag_t::CRITICAL, error.what());
			/**
			* Если режим отладки не включён
			*/
			#else
				// Выводим сообщение об ошибке
				this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
			#endif
		}
	}
	// Выводим результат
	return result;
}
/**
 * json Метод конвертации объекта JSON в текст в формате JSON
 * @param data     данные в объекте JSON
 * @param prettify флаг генерации читаемого формата
 * @return         текст после конвертации
 */
string anyks::Parser::json(const Document & data, const bool prettify) noexcept {
	// Если данные переданы
	if((data.IsObject() && !data.ObjectEmpty()) || (data.IsArray() && !data.Empty())){
		// Выполняем блокировку потока
		const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Создаём результьрующий буфер
			rapidjson::StringBuffer result;
			// Выполняем очистку результирующего буфера
			result.Clear();
			// Если нам нужно вывести результат в красивом формате
			if(prettify){
				// Выполняем создание объекта писателя
				PrettyWriter <StringBuffer> writer(result);
				// Передаем данные объекта JSON писателю
				data.Accept(writer);
			// Если нужно вывести результат в обычном формате
			} else {
				// Выполняем создание объекта писателя
				Writer <StringBuffer> writer(result);
				// Передаем данные объекта JSON писателю
				data.Accept(writer);
			}
			// Извлекаем созданную запись сктроки в формате JSON
			return result.GetString();
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			/**
			 * Если включён режим отладки
			 */
			#if defined(DEBUG_MODE)
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, {}, log_t::flag_t::CRITICAL, error.what());
			/**
			* Если режим отладки не включён
			*/
			#else
				// Выводим сообщение об ошибке
				this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
			#endif
		}
	}
	// Выводим результат
	return "";
}
/**
 * cef Метод конвертации текста в формате CEF в объект JSON
 * @param text текст для конвертации
 * @param mode режим парсинга
 * @return     объект в формате JSON
 */
Document anyks::Parser::cef(const string & text, const cef_t::mode_t mode) noexcept {
	// Результат работы функции
	Document result(kObjectType);
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
			/**
			 * Если включён режим отладки
			 */
			#if defined(DEBUG_MODE)
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(text, static_cast <uint16_t> (mode)), log_t::flag_t::CRITICAL, error.what());
			/**
			* Если режим отладки не включён
			*/
			#else
				// Выводим сообщение об ошибке
				this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
			#endif
		}
	}
	// Выводим результат
	return result;
}
/**
 * cef Метод конвертации объекта JSON в текст в формате CEF
 * @param data данные в объекте JSON
 * @param mode режим парсинга
 * @return     текст после конвертации
 */
string anyks::Parser::cef(const Document & data, const cef_t::mode_t mode) noexcept {
	// Если данные переданы
	if(data.IsObject() && !data.ObjectEmpty()){
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
			/**
			 * Если включён режим отладки
			 */
			#if defined(DEBUG_MODE)
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, {}, log_t::flag_t::CRITICAL, error.what());
			/**
			* Если режим отладки не включён
			*/
			#else
				// Выводим сообщение об ошибке
				this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
			#endif
		}
	}
	// Выводим результат
	return "";
}
