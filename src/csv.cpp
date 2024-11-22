/**
 * @file: csv.cpp
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
#include <csv.hpp>

/**
 * prepare Метод выполнения препарирования полученных данных строки
 * @param buffer буфер данных для препарирования
 * @param size   размер буфера данных для препарирования
 * @param delim  используемый разделитель
 */
void anyks::CSV::prepare(const char * buffer, const size_t size, const char delim) noexcept {
	// Если данные переданы
	if((buffer != nullptr) && (size > 0)){
		// Результирующая строка
		string text = "";
		// Статус получения кавычек
		bool quote = false;
		// Список собранных результатов
		vector <string> result;
		// Выполняем перебор всех полученных символов
		for(size_t i = 0; i < size; i++){
			// Выполняем проверку, является ли символ кавычками
			if(buffer[i] == '"'){
				// Если кавычки ещё не установлены
				if(!quote){
					// Устанавливаем флаг кавычек
					quote = !quote;
					// Если следующий символ тоже кавычки
					if(buffer[i + 1] == '"'){
						// Если и следующий символ тоже кавычки
						if(buffer[i + 2] == '"'){
							// Добавляем символ в строку
							text.append(1, buffer[i]);
							// Выполняем смещение на два символа
							i += 2;
						// Если следующий символ является разделителем
						} else if((buffer[i + 2] == delim) || ((delim == '0') && ((buffer[i + 2] == ',') || (buffer[i + 2] == ';') || (buffer[i + 2] == '|')))) {
							// Снимаем флаг кавычек
							quote = !quote;
							// Добавляем пустой разделитель
							result.push_back("");
							// Выполняем смещение на два символа
							i += 2;
						// Если мы получили какой-то другой символ
						} else {
							// Добавляем символ в строку
							text.append(1, buffer[i]);
							// Выполняем смещение на один символ
							i++;
						}
					}
				// Если кавычки уже установлены
				} else {
					// Если следующий символ тоже кавычки
					if(buffer[i + 1] == '"'){
						// Добавляем символ в строку
						text.append(1, buffer[i]);
						// Если и следующий символ тоже кавычки
						if(buffer[i + 2] == '"'){
							// Выполняем смещение на два символа
							i += 2;
							// Снимаем флаг кавычек
							quote = !quote;
						// Выполняем смещение на один символ
						} else i++;
					// Снимаем флаг кавычек
					} else quote = !quote;
				}
				// Если мы достигли конца строки
				if((i + 1) == size)
					// Выполняем сборку результатов
					result.push_back(std::move(text));
			// Если символ не является кавычками, но является разделителем или концом строки
			} else if(!quote && (((delim != '0') && (buffer[i] == delim)) || ((i + 1) == size))) {
				// Если мы достигли конца строки
				if((i + 1) == size)
					// Добавляем символ в строку
					text.append(1, buffer[i]);
				// Выполняем сборку результатов
				result.push_back(std::move(text));
				// Очищаем результирующую строку
				text.clear();
			// Если символ не является кавычками, но является произвольным разделителем или концом строки
			} else if(!quote && (((delim == '0') && ((buffer[i] == ',') || (buffer[i] == ';') || (buffer[i] == '|'))) || ((i + 1) == size))) {
				// Выполняем установку разделителя
				(* const_cast <char *> (&delim)) = buffer[i];
				// Если мы достигли конца строки
				if((i + 1) == size)
					// Добавляем символ в строку
					text.append(1, buffer[i]);
				// Выполняем сборку результатов
				result.push_back(std::move(text));
				// Очищаем результирующую строку
				text.clear();
			// Добавляем символ как он есть
			} else text.append(1, buffer[i]);
		}
		// Если результат получен
		if(!result.empty())
			// Формируем итоговый результат
			this->_mapping.push_back(std::move(result));
	// Выводим сообщение об ошибке
	} else this->_log->print("CSV: %s", log_t::flag_t::CRITICAL, "data for preparing received corrupted");
}
/**
 * prepare Метод выполнения препарирования полученных данных строки
 * @param buffer   буфер данных для препарирования
 * @param size     размер буфера данных для препарирования
 * @param callback функция обратного вызова
 * @param delim    используемый разделитель
 */
void anyks::CSV::prepare(const char * buffer, const size_t size, function <void (const vector <string> &)> callback, const char delim) noexcept {
	// Если данные переданы
	if((buffer != nullptr) && (size > 0) && (callback != nullptr)){
		// Результирующая строка
		string text = "";
		// Статус получения кавычек
		bool quote = false;
		// Список собранных результатов
		vector <string> result;
		// Выполняем перебор всех полученных символов
		for(size_t i = 0; i < size; i++){
			// Выполняем проверку, является ли символ кавычками
			if(buffer[i] == '"'){
				// Если кавычки ещё не установлены
				if(!quote){
					// Устанавливаем флаг кавычек
					quote = !quote;
					// Если следующий символ тоже кавычки
					if(buffer[i + 1] == '"'){
						// Если и следующий символ тоже кавычки
						if(buffer[i + 2] == '"'){
							// Добавляем символ в строку
							text.append(1, buffer[i]);
							// Выполняем смещение на два символа
							i += 2;
						// Если следующий символ является разделителем
						} else if((buffer[i + 2] == delim) || ((delim == '0') && ((buffer[i + 2] == ',') || (buffer[i + 2] == ';') || (buffer[i + 2] == '|')))) {
							// Снимаем флаг кавычек
							quote = !quote;
							// Добавляем пустой разделитель
							result.push_back("");
							// Выполняем смещение на два символа
							i += 2;
						// Если мы получили какой-то другой символ
						} else {
							// Добавляем символ в строку
							text.append(1, buffer[i]);
							// Выполняем смещение на один символ
							i++;
						}
					}
				// Если кавычки уже установлены
				} else {
					// Если следующий символ тоже кавычки
					if(buffer[i + 1] == '"'){
						// Добавляем символ в строку
						text.append(1, buffer[i]);
						// Если и следующий символ тоже кавычки
						if(buffer[i + 2] == '"'){
							// Выполняем смещение на два символа
							i += 2;
							// Снимаем флаг кавычек
							quote = !quote;
						// Выполняем смещение на один символ
						} else i++;
					// Снимаем флаг кавычек
					} else quote = !quote;
				}
				// Если мы достигли конца строки
				if((i + 1) == size)
					// Выполняем сборку результатов
					result.push_back(std::move(text));
			// Если символ не является кавычками, но является разделителем или концом строки
			} else if(!quote && (((delim != '0') && (buffer[i] == delim)) || ((i + 1) == size))) {
				// Если мы достигли конца строки
				if((i + 1) == size)
					// Добавляем символ в строку
					text.append(1, buffer[i]);
				// Выполняем сборку результатов
				result.push_back(std::move(text));
				// Очищаем результирующую строку
				text.clear();
			// Если символ не является кавычками, но является произвольным разделителем или концом строки
			} else if(!quote && (((delim == '0') && ((buffer[i] == ',') || (buffer[i] == ';') || (buffer[i] == '|'))) || ((i + 1) == size))) {
				// Выполняем установку разделителя
				(* const_cast <char *> (&delim)) = buffer[i];
				// Если мы достигли конца строки
				if((i + 1) == size)
					// Добавляем символ в строку
					text.append(1, buffer[i]);
				// Выполняем сборку результатов
				result.push_back(std::move(text));
				// Очищаем результирующую строку
				text.clear();
			// Добавляем символ как он есть
			} else text.append(1, buffer[i]);
		}
		// Если результат получен
		if(!result.empty())
			// Формируем итоговый результат
			callback(std::move(result));
	// Выводим сообщение об ошибке
	} else this->_log->print("CSV: %s", log_t::flag_t::CRITICAL, "data for preparing received corrupted");
}
/**
 * clear Метод очистки данных
 */
void anyks::CSV::clear() noexcept {
	// Выполняем очистку собранных данных
	this->_mapping.clear();
}
/**
 * header Метод установки флага использования заголовков
 * @param mode флаг использования заголовков
 */
void anyks::CSV::header(const bool mode) noexcept {
	// Устанавливаем флаг использования заголовков
	this->_header = mode;
}
/**
 * parse Метод выполнения парсинга текста
 * @param text текст для парсинга
 */
void anyks::CSV::parse(const string & text) noexcept {
	// Если текст передан
	if(!text.empty())
		// Выполняем парсинг переданной строки
		this->parse(text, '0');
	// Выводим сообщение об ошибке
	else this->_log->print("CSV: %s", log_t::flag_t::CRITICAL, "data for parsing received corrupted");
}
/**
 * parse Метод выполнения парсинга текста
 * @param text  текст для парсинга
 * @param delim используемый разделитель
 */
void anyks::CSV::parse(const string & text, const char delim) noexcept {
	// Если текст передан
	if(!text.empty()){
		// Значение текущей и предыдущей буквы
		char letter = 0, old = 0;
		// Смещение в буфере и длина полученной строки
		size_t offset = 0, length = 0;
		// Получаем размер файла
		const uintmax_t size = text.size();
		// Переходим по всему буферу
		for(uintmax_t i = 0; i < size; i++){
			// Получаем значение текущей буквы
			letter = text.at(i);
			// Если текущая буква является переносом строк
			if((i > 0) && ((letter == '\n') || (i == (size - 1)))){
				// Если предыдущая буква была возвратом каретки, уменьшаем длину строки
				length = ((old == '\r' ? i - 1 : i) - offset);
				// Если это конец файла, корректируем размер последнего байта
				if(length == 0)
					// Выполняем кооректировку размера
					length = 1;
				// Если мы получили последний символ и он не является переносом строки
				if((i == (size - 1)) && (letter != '\n'))
					// Выполняем компенсацию размера строки
					length++;
				// Если длина слова получена, выводим полученную строку
				this->prepare(text.data() + offset, length, delim);
				// Выполняем смещение
				offset = (i + 1);
			}
			// Запоминаем предыдущую букву
			old = letter;
		}
		// Если данные не все прочитаны, выводим как есть
		if((offset == 0) && (size > 0))
			// Выводим полученную строку
			this->prepare(text.data(), size, delim);
	// Выводим сообщение об ошибке
	} else this->_log->print("CSV: %s", log_t::flag_t::CRITICAL, "data for parsing received corrupted");
}
/**
 * cols Метод получения количества столбцов
 * @return количество столбцов
 */
size_t anyks::CSV::cols() const noexcept {
	// Если карта данных заполнена
	if(!this->_mapping.empty())
		// Выводим количество столбцов
		return this->_mapping.front().size();
	// Сообщаем, что количество столбцов не определено
	return 0;
}
/**
 * rows Метод получения количества строк
 * @return количество строк
 */
size_t anyks::CSV::rows() const noexcept {
	// Выводим количество строк
	return this->_mapping.size();
}
/**
 * row Метод получения строки
 * @param index индекс строки
 * @param delim используемый разделитель
 * @return      сформированная строка
 */
string anyks::CSV::row(const size_t index, const char delim) noexcept {
	// Результат работы функции
	string result = "";
	// Если переданный индекс меньше количества записей
	if(index < this->_mapping.size()){
		// Символ для сравнения
		char letter = 0;
		// Флаг проверки на требование экранирования
		bool shielding = false;
		// Выполняем перебор всех столбцов
		for(auto & item : this->_mapping.at(index)){
			// Получаем значение столбца
			string col = item;
			// Выполняем сброс флага экранирования
			shielding = false;
			// Выполняем перебор всех символов столбца
			for(size_t i = 0; i < col.size(); i++){
				// Получаем символ для проверки
				letter = col.at(i);
				// Если флаг ещё не установлен
				if(!shielding)
					// Если найдены символы требующие экранирования
					shielding = (
						(letter == ';') || (letter == ',') || (letter == '|') || ::isspace(letter) ||
						(letter == 32) || (letter == ' ') || (letter == '\t') || (letter == '\n') ||
						(letter == '\r') || (letter == '\f') || (letter == '\v')
					);
				// Если найдена кавычка
				if(letter == '"'){
					// Добавляем ещё одну кавычку
					col.insert(col.begin() + i, 1, '"');
					// увеличиваем значение индекса
					i++;
				}
			}
			// Если результат уже собран
			if(!result.empty())
				// Выполняем добавление разделителя
				result.append(1, delim);
			// Если требуется экранирование
			if(shielding)
				// Добавляем экранирование
				result.append(1, '"');
			// Добавляем запись
			result.append(col);
			// Если требуется экранирование
			if(shielding)
				// Добавляем экранирование
				result.append(1, '"');
		}
	// Выводим сообщение об ошибке
	} else this->_log->print("CSV: %s", log_t::flag_t::CRITICAL, "object contains no data");
	// Выводим результат
	return result;
}
/**
 * write Метод записи данных в файл
 * @param filename адрес файла контейнера CSV для записи
 * @param delim    используемый разделитель
 */
void anyks::CSV::write(const string & filename, const char delim) noexcept {
	// Если файл для чтения передан
	if(!filename.empty()){
		/**
		 * Выполняем перехват ошибок
		 */
		try {
			// Бинарный буфер данных для записи
			string buffer = "";
			// Получаем максимальное количество строк
			const size_t rows = this->rows();
			// Выполняем перебор полученного количества строк
			for(size_t i = 0; i < rows; i++){
				// Получаем данные для добавления в файл
				buffer.append(this->row(i, delim));
				// Добавляем разделитель строки
				buffer.append("\r\n");
				// Если размер буфера превышает максимальный размер отправки
				if((buffer.size() >= CSV_BUFFER_SIZE) || (i == (rows - 1))){
					// Выполняем добавление в файл полученного буфера данных
					this->_fs.append(filename, buffer.data(), buffer.size());
					// Выполняем очистку буфера данных
					buffer.clear();
				}
			}
		/**
		 * Если возникает ошибка
		 */
		} catch (const std::ios_base::failure & error) {
			// Выводим сообщение инициализации метода класса скрипта торговой платформы
			this->_log->print("CSV: %s for filename %s", log_t::flag_t::CRITICAL, error.what(), filename.c_str());
		}
	// Выводим сообщение об ошибке
	} else this->_log->print("CSV: %s", log_t::flag_t::CRITICAL, "file address for writing was not set");
}
/**
 * read Метод чтения данных из файла
 * @param filename адрес файла контейнера CSV для чтения
 */
void anyks::CSV::read(const string & filename) noexcept {
	// Если файл для чтения передан
	if(!filename.empty())
		// Выполняем чтение файла с неустановленным разделителем
		this->read(filename, '0');
	// Выводим сообщение об ошибке
	else this->_log->print("CSV: %s", log_t::flag_t::CRITICAL, "file address for reading was not set");
}
/**
 * read Метод чтения данных из файла
 * @param filename адрес файла контейнера CSV для чтения
 * @param delim    используемый разделитель
 */
void anyks::CSV::read(const string & filename, const char delim) noexcept {
	// Если файл для чтения передан
	if(!filename.empty()){
		// Выполняем чтение файла
		this->_fs.readFile3(filename, [delim, this](const string & text) noexcept -> void {
			// Выводим полученную строку
			this->prepare(text.data(), text.size(), delim);
		});
	// Выводим сообщение об ошибке
	} else this->_log->print("CSV: %s", log_t::flag_t::CRITICAL, "file address for reading was not set");
}
/**
 * read Метод чтения данных из файла
 * @param filename адрес файла контейнера CSV для чтения
 * @param callback функция обратного вызова
 * @param delim    используемый разделитель
 */
void anyks::CSV::read(const string & filename, function <void (const vector <string> &)> callback, const char delim) noexcept {
	// Если файл для чтения передан
	if(!filename.empty()){
		// Выполняем чтение файла
		this->_fs.readFile3(filename, [delim, callback, this](const string & text) noexcept -> void {
			// Если функция обратного вызова передана
			if(callback != nullptr)
				// Выводим полученную строку
				this->prepare(text.data(), text.size(), callback, delim);
		});
	// Выводим сообщение об ошибке
	} else this->_log->print("CSV: %s", log_t::flag_t::CRITICAL, "file address for reading was not set");
}
/**
 * dump Метод создания дампа данных
 * @return дамп данных в формате JSON
 */
json anyks::CSV::dump() const noexcept {
	// Результат работы функции
	json result = json::object();
	// Если данные реально собранны
	if(!this->_mapping.empty()){
		// Если нужно сформировать заголовки
		if(this->_header){
			// Текущее значение индекса
			size_t index = 0;
			// Переходим по всему списку ключей
			for(auto & key : this->_mapping.front()){
				// Создаём объект массива
				result[key] = json::array();
				// Переходим по всем остальным ключам
				for(size_t i = 1; i < this->_mapping.size(); i++){
					// Если индекс соответствует номеру записи
					if(index < this->_mapping.at(i).size()){
						// Получаем запись для проверки
						const string item = this->_mapping.at(i).at(index);
						// Выполняем приведение строки к нижнему регистру
						this->_fmk->transform(item, fmk_t::transform_t::LOWER);
						// Если запись является числом
						if(this->_fmk->is(item, fmk_t::check_t::NUMBER)){
							/**
							 * Выполняем отлов ошибок
							 */
							try {
								// Если число положительное
								if(item.front() != '-')
									// Добавляем полученное значение в массив
									result[key].push_back(::stoull(item));
								// Добавляем полученное значение в массив
								else result[key].push_back(::stoll(item));
							/**
							 * Если возникает ошибка
							 */
							} catch(const std::exception &) {
								// Добавляем полученное значение в массив
								result[key].push_back(this->_mapping.at(i).at(index));
							}
						// Если запись является числом с плавающей точкой
						} else if(this->_fmk->is(item, fmk_t::check_t::DECIMAL)) {
							/**
							 * Выполняем отлов ошибок
							 */
							try {
								// Добавляем полученное значение в массив
								result[key].push_back(::stold(item));
							/**
							 * Если возникает ошибка
							 */
							} catch(const std::exception &) {
								// Добавляем полученное значение в массив
								result[key].push_back(this->_mapping.at(i).at(index));
							}
						// Если число является булевым истинным значением
						} else if(this->_fmk->compare("true", item))
							// Добавляем полученное значение в массив
							result[key].push_back(true);
						// Если число является булевым ложным значением
						else if(this->_fmk->compare("false", item))
							// Добавляем полученное значение в массив
							result[key].push_back(false);
						// Добавляем полученное значение в массив
						else result[key].push_back(this->_mapping.at(i).at(index));
					}
				}
				// Выполняем смещение индекса
				index++;
			}
		// Если формировать заголовок не требуется
		} else {
			// Выполняем создание массива
			result = json::array();
			// Выполняем перебор всего списка собранной карты
			for(size_t i = 0; i < this->_mapping.size(); i++){
				// Выполняем добавление нового массива
				result.push_back(json::array());
				// Выполняем перебор всего списка строк
				for(auto j = 0; j < this->_mapping.at(i).size(); j++){
					// Получаем строку значения
					const string & item = this->_mapping.at(i).at(j);
					// Если запись является числом
					if(this->_fmk->is(item, fmk_t::check_t::NUMBER)){
						/**
						 * Выполняем отлов ошибок
						 */
						try {
							// Если число положительное
							if(item.front() != '-')
								// Добавляем полученное значение в массив
								result.back().push_back(::stoull(item));
							// Добавляем полученное значение в массив
							else result.back().push_back(::stoll(item));
						/**
						 * Если возникает ошибка
						 */
						} catch(const std::exception &) {
							// Добавляем полученное значение в массив
							result.back().push_back(item);
						}
					// Если запись является числом с плавающей точкой
					} else if(this->_fmk->is(item, fmk_t::check_t::DECIMAL)) {
						/**
						 * Выполняем отлов ошибок
						 */
						try {
							// Добавляем полученное значение в массив
							result.back().push_back(::stold(item));
						/**
						 * Если возникает ошибка
						 */
						} catch(const std::exception &) {
							// Добавляем полученное значение в массив
							result.back().push_back(item);
						}
					// Если число является булевым истинным значением
					} else if(this->_fmk->compare("true", item))
						// Добавляем полученное значение в массив
						result.back().push_back(true);
					// Если число является булевым ложным значением
					else if(this->_fmk->compare("false", item))
						// Добавляем полученное значение в массив
						result.back().push_back(false);
					// Добавляем полученное значение в массив
					else result.back().push_back(item);
				}
			}
		}
	}
	// Выводим результат
	return result;
}
/**
 * dump Метод установки дампа данных
 * @param dump дамп данных в формате JSON
 */
void anyks::CSV::dump(const json & dump) noexcept {
	// Если дамп данных передан и является объектом
	if((this->_header = (dump.is_object() && !dump.empty()))){
		// Выполняем очистку схему соответствий ключей расширения
		this->_mapping.clear();
		// Выполняем добавляем новый список ключей
		this->_mapping.push_back(vector <string> ());
		// Выполняем перебор переданного дампа данных
		for(auto & el : dump.items()){
			// Если данные получены верные
			if(el.value().is_array()){
				// Добавляем заголовочные записи
				this->_mapping.front().push_back(el.key());
				// Переходим по всем элементам массива
				for(size_t i = 0; i < el.value().size(); i++){
					// Если индекс нулевой
					if((i + 1) == this->_mapping.size())
						// Выполняем добавляем новый список ключей
						this->_mapping.push_back(vector <string> ());
					// Получаем значение записи
					const json & item = el.value().at(i);
					// Если запись является строкой
					if(item.is_string())
						// Выполняем добавление всех записей
						this->_mapping.at(i + 1).push_back(item.get <string> ());
					// Если запись является числом
					else if(item.is_number()) {
						// Временное значение переменной
						double intpart = 0;
						// Выполняем извлечение числа
						const double number = item.get <double> ();
						// Выполняем проверку есть ли дробная часть у числа
						if(::modf(number, &intpart) == 0){
							// Если число является положительным
							if(number > 0.)
								// Преобразуем значение в тип INT64
								this->_mapping.at(i + 1).push_back(std::to_string(item.get <uint64_t> ()));
							// Если число является отрицательным
							else this->_mapping.at(i + 1).push_back(std::to_string(item.get <int64_t> ()));
						// Если у числа имеется дробная часть
						} else this->_mapping.at(i + 1).push_back(this->_fmk->noexp(number, true));
					// Если запись является булевым значением
					} else if(item.is_boolean())
						// Выполняем добавление всех записей
						this->_mapping.at(i + 1).push_back(item.get <bool> () ? "true" : "false");
				
				}
			}
		}
	// Если дамп данных передан и является массивом
	} else if(dump.is_array() && !dump.empty()) {
		// Выполняем перебор всего списка массива
		for(auto & item1 : dump){
			// Выполняем добавляем новый список ключей
			this->_mapping.push_back(vector <string> ());
			// Выполняем перебор всего списка дочерних элементов
			for(auto & item2 : item1){
				// Если запись является строкой
				if(item2.is_string())
					// Выполняем добавление всех записей
					this->_mapping.back().push_back(item2.get <string> ());
				// Если запись является числом
				else if(item2.is_number()) {
					// Временное значение переменной
					double intpart = 0;
					// Выполняем извлечение числа
					const double number = item2.get <double> ();
					// Выполняем проверку есть ли дробная часть у числа
					if(::modf(number, &intpart) == 0){
						// Если число является положительным
						if(number > 0.)
							// Преобразуем значение в тип INT64
							this->_mapping.back().push_back(std::to_string(item2.get <uint64_t> ()));
						// Если число является отрицательным
						else this->_mapping.back().push_back(std::to_string(item2.get <int64_t> ()));
					// Если у числа имеется дробная часть
					} else this->_mapping.back().push_back(this->_fmk->noexp(number, true));
				// Если запись является булевым значением
				} else if(item2.is_boolean())
					// Выполняем добавление всех записей
					this->_mapping.back().push_back(item2.get <bool> () ? "true" : "false");
			}
		}
	}
}
/**
 * get Метод извлечения данных контейнера
 * @return собранные данные контейнера
 */
const vector <vector <string>> & anyks::CSV::get() const noexcept {
	// Выводим собранные данные контейнера
	return this->_mapping;
}
/**
 * Оператор вывода данные контейнера в качестве строки
 * @return данные контейнера в качестве строки
 */
anyks::CSV::operator std::string() const noexcept {
	// Выводим результат
	return this->dump().dump(4);
}
/**
 * Оператор [=] присвоения контейнеров
 * @param csv контенер для присвоения
 * @return    текущий объект
 */
anyks::CSV & anyks::CSV::operator = (const CSV & csv) noexcept {
	// Выполняем копирование флага работы с заголовками
	this->_header = csv._header;
	// Выполняем копирвоание полученных параметров
	this->_mapping = csv._mapping;
	// Выводим текущий объект
	return (* this);
}
/**
 * Оператор [=] присвоения контейнеров
 * @param csv контенер для присвоения
 * @return    текущий объект
 */
anyks::CSV & anyks::CSV::operator = (const string & csv) noexcept {
	// Выполняем парсинг строки
	this->parse(csv);
	// Выводим текущий объект
	return (* this);
}
/**
 * Оператор [>>] чтения из потока CSV контейнера
 * @param is  поток для чтения
 * @param csv контенер для присвоения
 */
istream & anyks::operator >> (istream & is, csv_t & csv) noexcept {
	// Данные CSV контейнера
	string data = "";
	// Считываем данные CSV контейнера
	is >> data;
	// Если данные CSV контейнера получены
	if(!data.empty())
		// Устанавливаем данные CSV контейнера
		csv.parse(data);
	// Выводим результат
	return is;
}
/**
 * Оператор [<<] вывода в поток CSV контейнера
 * @param os  поток куда нужно вывести данные
 * @param csv контенер для присвоения
 */
ostream & anyks::operator << (ostream & os, const csv_t & csv) noexcept {
	// Записываем в поток CSV сообщение
	os << csv.dump().dump(4);
	// Выводим результат
	return os;
}
