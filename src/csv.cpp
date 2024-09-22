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
 * Выполняем работу для Windows
 */
#if defined(_WIN32) || defined(_WIN64)
	/**
	 * seek Метод установки позиции в файле
	 * @param file     объект открытого файла
	 * @param distance дистанцию на которую нужно переместить позицию
	 * @param position текущая позиция в файле
	 * @return         перенос позиции в файле
	 */
	static int64_t seek(HANDLE file, const int64_t distance, const DWORD position) noexcept {
		// Создаём объект большого числа
		LARGE_INTEGER li;
		// Устанавливаем начальное значение позиции
		li.QuadPart = distance;
		// Выполняем установку позиции в файле
		li.LowPart = SetFilePointer(file, li.LowPart, &li.HighPart, position);
		// Если мы получили ошибку установки позиции
		if((li.LowPart == INVALID_SET_FILE_POINTER) && (GetLastError() != NO_ERROR))
			// Сбрасываем значение установленной позиции
			li.QuadPart = -1;
		// Выводим значение установленной позиции
		return li.QuadPart;
	}
#endif

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
					/**
					 * Выполняем работу для Windows
					 */
					#if defined(_WIN32) || defined(_WIN64)
						// Выполняем открытие файла на добавление
						HANDLE file = CreateFileW(this->_fmk->convert(filename).c_str(), FILE_APPEND_DATA, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
						// Если открыть файл открыт нормально
						if(file != INVALID_HANDLE_VALUE){
							// Выполняем добавление данных в файл
							WriteFile(file, static_cast <LPCVOID> (buffer.data()), static_cast <DWORD> (buffer.size()), 0, nullptr);
							// Выполняем закрытие файла
							CloseHandle(file);
						}
					/**
					 * Выполняем работу для Unix
					 */
					#else
						// Файловый поток для добавления
						ofstream file(filename, (ios::binary | ios::app));
						// Если файл открыт на добавление
						if(file.is_open()){
							// Выполняем добавление данных в файл
							file.write(buffer.data(), buffer.size());
							// Закрываем файл
							file.close();
						}
					#endif
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
		/**
		 * Выполняем перехват ошибок
		 */
		try {
			// Структура проверка статистики
			struct stat info;
			// Если тип определён
			if((::stat(filename.c_str(), &info) == 0) && S_ISREG(info.st_mode)){
				// Размер файла для парсинга
				uintmax_t size = 0;
				{
					/**
					 * Выполняем работу для Windows
					 */
					#if defined(_WIN32) || defined(_WIN64)
						// Создаём объект работы с файлом
						HANDLE file = CreateFileW(this->_fmk->convert(filename).c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
						// Если открыть файл открыт нормально
						if(file != INVALID_HANDLE_VALUE){
							// Получаем размер файла
							size = static_cast <uintmax_t> (GetFileSize(file, nullptr));
							// Выполняем закрытие файла
							CloseHandle(file);
						}
					/**
					 * Выполняем работу для Unix
					 */
					#else
						// Зануляем объект структуры информационных данных файла
						::memset(&info, 0, sizeof(info));
						// Выполняем извлечение данных статистики
						const int status = ::stat(filename.c_str(), &info);
						// Если тип определён
						if(status == 0)
							// Выводим размер файла
							size = static_cast <uintmax_t> (info.st_size);
						// Если прочитать файла не вышло
						else {
							// Открываем файл на чтение
							ifstream file(filename, ios::in);
							// Если файл открыт
							if(file.is_open()){
								// Перемещаем указатель в конец файла
								file.seekg(0, file.end);
								// Определяем размер файла
								size = file.tellg();
								// Возвращаем указатель обратно
								file.seekg(0, file.beg);
								// Закрываем файл
								file.close();
							}
						}
					#endif
				}
				// Если размер файла получен
				if(size > 0){
					// Устанавливаем размер буфера
					vector <char> buffer;
					// Смещение в бинарном буфере, актуальный размер буфера и позиция записи в буфер
					uintmax_t offset = 0, actual = 0, pos = 0;
					// Выполняем обработку всех буферов данных
					while((size - offset) > 0){
						// Выполняем сброс позиции записи в буфер
						pos = 0;
						// Получаем актуальный размер буфера данных
						actual = ((size - offset) > CSV_BUFFER_SIZE ? CSV_BUFFER_SIZE : (size - offset));
						// Если буфер данных ещё пустой
						if(buffer.empty())
							// Выделяем память в буфере для нужных нам размеров
							buffer.resize(actual, 0);
						// Если буфер уже создан ранее
						else {
							// Получаем позицию записи в буфере
							pos = buffer.size();
							// Выделяем память для буфера
							buffer.resize(buffer.size() + actual, 0);
						}
						/**
						 * Выполняем работу для Windows
						 */
						#if defined(_WIN32) || defined(_WIN64)
							// Создаём объект работы с файлом
							HANDLE file = CreateFileW(this->_fmk->convert(filename).c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
							// Если открыть файл открыт нормально
							if(file != INVALID_HANDLE_VALUE){
								// Устанавливаем позицию чтения
								if(seek(file, offset, FILE_BEGIN) > -1)
									// Выполняем чтение из файла в буфер данные
									ReadFile(file, static_cast <LPVOID> (buffer.data() + pos), static_cast <DWORD> (buffer.size() - pos), 0, nullptr);
								// Выполняем закрытие файла
								CloseHandle(file);
								// Выполняем чтение полученного буфера
								const size_t bytes = this->read(buffer, delim, (size - offset) <= CSV_BUFFER_SIZE);
								// Если количество обработанных байт меньше размера буфера
								if(bytes < buffer.size())
									// Выполняем удаление из буфера обработанных данных
									buffer.erase(buffer.begin(), buffer.begin() + bytes);
								// Очищаем весь буфер целиком
								else buffer.clear();
							}
						/**
						 * Выполняем работу для Unix
						 */
						#else
							// Открываем файл на чтение
							ifstream file(filename, ios::in);
							// Если файл открыт
							if(file.is_open()){
								// Устанавливаем позицию чтения
								file.seekg(offset, file.beg);
								// Выполняем чтение данных из файла
								file.read(buffer.data() + pos, buffer.size() - pos);
								// Закрываем файл
								file.close();
								// Выполняем чтение полученного буфера
								const size_t bytes = this->read(buffer, delim, (size - offset) <= CSV_BUFFER_SIZE);
								// Если количество обработанных байт меньше размера буфера
								if(bytes < buffer.size())
									// Выполняем удаление из буфера обработанных данных
									buffer.erase(buffer.begin(), buffer.begin() + bytes);
								// Очищаем весь буфер целиком
								else buffer.clear();
							}
						#endif
						// Увеличиваем смещение в бинарном буфере
						offset += actual;
					};
				}
			}
		/**
		 * Если возникает ошибка
		 */
		} catch (const ios_base::failure & error) {
			// Выводим сообщение инициализации метода класса скрипта торговой платформы
			this->_log->print("CSV: %s for filename %s", log_t::flag_t::CRITICAL, error.what(), filename.c_str());
		}
	// Выводим сообщение об ошибке
	} else this->_log->print("CSV: %s", log_t::flag_t::CRITICAL, "file address for reading was not set");
}
/**
 * read Метод чтения данных из буфера
 * @param buffer буфер бинарных данных
 * @param delim  используемый разделитель
 * @param end    обрабатываемый блок данных является последним
 * @return       количество обработанных байт
 */
size_t anyks::CSV::read(const vector <char> & buffer, const char delim, const bool end) noexcept {
	// Результат работы функции
	size_t result = 0;
	// Если буфер данных получен
	if(!buffer.empty()){
		// Смещение в буфере и длина полученной строки
		size_t length = 0;
		// Значение текущей и предыдущей буквы
		char letter = 0, old = 0;
		// Получаем данные буфера
		const char * data = buffer.data();
		// Получаем размер файла
		const uintmax_t size = buffer.size();
		// Переходим по всему буферу
		for(uintmax_t i = 0; i < size; i++){
			// Получаем значение текущей буквы
			letter = data[i];
			// Если текущая буква является переносом строк
			if((i > 0) && ((letter == '\n') || (i == (size - 1)))){
				// Если предыдущая буква была возвратом каретки, уменьшаем длину строки
				length = ((old == '\r' ? i - 1 : i) - result);
				// Если это конец файла, корректируем размер последнего байта
				if(length == 0)
					// Выполняем кооректировку размера
					length = 1;
				// Если мы получили последний символ и он не является переносом строки
				if((i == (size - 1)) && (letter != '\n')){
					// Если блок данных является последним
					if(end)
						// Выполняем компенсацию размера строки
						length++;
					// Выходим изцикла
					else break;
				}
				// Если длина слова получена, выводим полученную строку
				this->prepare(data + result, length, delim);
				// Выполняем смещение
				result = (i + 1);
			}
			// Запоминаем предыдущую букву
			old = letter;
		}
		// Если данные не все прочитаны, выводим как есть
		if(end && (result == 0) && (size > 0))
			// Выводим полученную строку
			this->prepare(data, size, delim);
	// Выводим сообщение об ошибке
	} else this->_log->print("CSV: %s", log_t::flag_t::CRITICAL, "buffer for reading is empty");
	// Выводим результат
	return result;
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
						string item = this->_mapping.at(i).at(index);
						// Выполняем приведение строки к нижнему регистру
						this->_fmk->transform(item, fmk_t::transform_t::LOWER);
						// Если запись является числом
						if(this->_fmk->is(item, fmk_t::check_t::NUMBER)){
							// Получаем переданное число
							const long long number = std::stoll(item);
							// Если число положительное
							if(number > 0)
								// Добавляем полученное значение в массив
								result[key].push_back(::stoull(item));
							// Добавляем полученное значение в массив
							else result[key].push_back(number);
						// Если запись является числом с плавающей точкой
						} else if(this->_fmk->is(item, fmk_t::check_t::DECIMAL))
							// Добавляем полученное значение в массив
							result[key].push_back(::stod(item));
						// Если число является булевым истинным значением
						else if(item.compare("true") == 0)
							// Добавляем полученное значение в массив
							result[key].push_back(true);
						// Если число является булевым ложным значением
						else if(item.compare("false") == 0)
							// Добавляем полученное значение в массив
							result[key].push_back(false);
						// Добавляем полученное значение в массив
						else result[key].push_back(this->_mapping.at(i).at(index));
					}
				}
				// Выполняем смещение индекса
				index++;
			}
		// Создаём массив как основной результат
		} else result = this->_mapping;
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
				// Выполняем добавляем новый список ключей
				this->_mapping.push_back(vector <string> ());
				// Выполняем перебор всего списка массива
				for(auto & item : el.value()){
					// Если запись является строкой
					if(item.is_string())
						// Выполняем добавление всех записей
						this->_mapping.back().push_back(item.get <string> ());
					// Если запись является числом
					else if(item.is_number())
						// Выполняем добавление всех записей
						this->_mapping.back().push_back(to_string(item.get <double> ()));
					// Если запись является булевым значением
					else if(item.is_boolean())
						// Выполняем добавление всех записей
						this->_mapping.back().push_back(item.get <bool> () ? "true" : "false");
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
				else if(item2.is_number())
					// Выполняем добавление всех записей
					this->_mapping.back().push_back(to_string(item2.get <double> ()));
				// Если запись является булевым значением
				else if(item2.is_boolean())
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
