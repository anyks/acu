/**
 * @file: syslog.cpp
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
#include <syslog.hpp>

/**
 * clear Метод очистки данных
 */
void anyks::SysLog::clear() noexcept {
	/**
	 * Выполняем отлов ошибок
	 */
	try {
		// Выполняем блокировку потока
		const lock_guard <std::recursive_mutex> lock(this->_mtx);
		// Устанавливаем версию сообщения
		this->_ver = 0;
		// Устанавливаем приоритет сообщения
		this->_pri = 0;
		// Устанавливаем идентификатор процесса
		this->_pid = 0;
		// Устанавливаем штамп времени
		this->_timestamp = 0;
		// Устанавливаем название приложения
		this->_app = "-";
		// Устанавливаем идентификатор сообщения
		this->_mid = "-";
		// Устанавливаем хост сообщения
		this->_host = "-";
		// Устанавливаем формат даты сообщения
		this->_format = FORMAT;
		// Выполняем очистку сообщения
		this->_message.clear();
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
/**
 * parse Метод парсинга строки в формате SysLog
 * @param syslog строка в формате SysLog
 * @param std    стандарт SysLog
 */
void anyks::SysLog::parse(const string & syslog, const std_t std) noexcept {
	// Если даныне переданы
	if(!syslog.empty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем блокировку потока
			const lock_guard <std::recursive_mutex> lock(this->_mtx);
			// Определяем активированный режим парсинга
			switch(static_cast <uint8_t> (this->_mode)){
				// Если активирован нативный режим парсинга
				case static_cast <uint8_t> (mode_t::NATIVE): {
					// Идентификатор структурированных данных
					string sid = "";
					// Позиция начала строки
					size_t pos = 0;
					// Статус поиска
					uint8_t status = 0;
					// Количество пробелов в штампе времени
					uint8_t spaces = 0;
					// Устанавливаем стандарт
					this->_std = std;
					// Выполняем перебор всех символов
					for(size_t i = 0; i < syslog.length(); i++){
						// Если мы получили первый символ
						if(i == 0){
							// Если первый символ не является экранирование приоритета
							if(syslog.front() != '<'){
								// Увеличиваем значение статуса
								status += 2;
								// Если установлен стандарт как автоматический
								if(std == std_t::AUTO){
									// Если символ является числом
									if(this->_fmk->is(string(1, syslog.front()), fmk_t::check_t::NUMBER))
										// Устанавливаем стандарт RFC5424
										(* const_cast <std_t *> (&std)) = std_t::RFC5424;
									// Устанавливаем стандарт RFC3164
									else (* const_cast <std_t *> (&std)) = std_t::RFC3164;
									// Устанавливаем стандарт
									this->_std = std;
								}
							// Выполняем смещение позиции поиска
							} else pos = (i + 1);
						// Если это не первый символ
						} else {
							// Если статус поиска установлен как поиск приоритета или версии сообщения
							if((status == 0) || (status == 1)){
								// Определяем статус парсинга
								switch(status){
									// Если производится сбор приоритета
									case 0: {
										// Если обнаружен экранирующий символ приоритета
										if(syslog.at(i) == '>'){
											/**
											 * Выполняем отлов ошибок
											 */
											try {
												// Извлекаем значение приоритета
												this->_pri = static_cast <uint16_t> (::stoi(syslog.substr(pos, i - pos)));
											/**
											 * Если возникает ошибка
											 */
											} catch(const std::exception &) {
												// Добавляем полученное значение в массив
												this->_pri = 0;
											}
											// Запоминаем начало строки с версией
											pos = (i + 1);
											// Увеличиваем значение статуса
											status++;
											// Если следующие символы существуют
											if((std == std_t::AUTO) && (pos < syslog.length())){
												// Если следующий символ является числом
												if(this->_fmk->is(string(1, syslog.at(pos)), fmk_t::check_t::NUMBER))
													// Устанавливаем стандарт RFC5424
													(* const_cast <std_t *> (&std)) = std_t::RFC5424;
												// Если версия не указана
												else {
													// Увеличиваем значение статуса
													status++;
													// Устанавливаем стандарт RFC3164
													(* const_cast <std_t *> (&std)) = std_t::RFC3164;
												}
												// Устанавливаем стандарт
												this->_std = std;
											// Если версия протокола соответствует RFC3164
											} else if(std == std_t::RFC3164)
												// Увеличиваем значение статуса
												status++;
										}
									} break;
									// Если производится сбор версии
									case 1: {
										// Если версия протокола соответствует RFC5424
										if(std == std_t::RFC5424){
											// Если установлен пробел
											if(syslog.at(i) == ' '){
												/**
												 * Выполняем отлов ошибок
												 */
												try {
													// Получаем версию сообщения
													this->_ver = static_cast <uint8_t> (::stoi(syslog.substr(pos, i - pos)));
												/**
												 * Если возникает ошибка
												 */
												} catch(const std::exception &) {
													// Получаем версию сообщения
													this->_ver = 0;
												}
												// Запоминаем начало строки с версией
												pos = (i + 1);
												// Увеличиваем значение статуса
												status++;
											}
										// Выводим сообщение, что стандарты перепутаны
										} else {
											/**
											 * Если включён режим отладки
											 */
											#if defined(DEBUG_MODE)
												// Выводим сообщение об ошибке
												this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(syslog, static_cast <uint16_t> (std)), log_t::flag_t::CRITICAL, "SysLog standards are mixed up");
											/**
											* Если режим отладки не включён
											*/
											#else
												// Выводим сообщение об ошибке
												this->_log->print("%s", log_t::flag_t::CRITICAL, "SysLog standards are mixed up");
											#endif
										}
									} break;
								}
							// Если статус поиска производится другой
							} else {
								// Определяем тип стандарта
								switch(static_cast <uint8_t> (std)){
									// Если установлен стандарт RFC3164
									case static_cast <uint8_t> (std_t::RFC3164): {
										// Определяем статус парсинга
										switch(status){
											// Если производится сбор даты
											case 2: {
												// Если установлен пробел
												if(syslog.at(i) == ' '){
													// Если пробелов получено меньше двух
													if(spaces < 2)
														// Увеличиваем количество пробелов
														spaces++;
													// Иначе мы получили дату
													else {
														// Получаем текущее значение даты
														const time_t timestamp = time(nullptr);
														// Создаем структуру времени
														std::tm * tm = ::localtime(&timestamp);
														// Получаем значение даты
														string date = syslog.substr(pos, i - pos);
														// Выполняем поиск пробела в дате
														const size_t space = date.rfind(' ');
														// Если позиция пробела в дате найдена
														if(space != string::npos){
															// Выполняем вставку в строку года
															date.replace(space, 1, " " + std::to_string(1900 + tm->tm_year) + " ");
															// Устанавливаем дату сообщения
															this->date(date, "%b %d %Y %H:%M:%S");
														}
														// Запоминаем начало строки с версией
														pos = (i + 1);
														// Увеличиваем значение статуса
														status++;
													}								
												// Если мы перебрали все оставшиеся символы
												} else if(i == (syslog.length() - 1))
													// Устанавливаем полученное сообщение
													this->_message = syslog.substr(pos);
											} break;
											// Если производится сбор хоста сообщения
											case 3: {
												// Если установлен пробел
												if(syslog.at(i) == ' '){
													// Устанавливаем хост сообщения
													this->_host = syslog.substr(pos, i - pos);
													// Запоминаем начало строки с версией
													pos = (i + 1);
													// Увеличиваем значение статуса
													status++;
												// Если мы перебрали все оставшиеся символы
												} else if(i == (syslog.length() - 1))
													// Устанавливаем полученное сообщение
													this->_message = syslog.substr(pos);
											} break;
											// Если производится сбор названия приложения сообщения
											case 4: {
												// Если установлен разделитель тега
												if(syslog.at(i) == ':'){
													// Устанавливаем название сообщения
													this->_app = syslog.substr(pos, i - pos);
													{
														// Выполняем поиск идентификатора процесса
														const size_t pos = this->_app.find('[');
														// Если экранирование процесса найдено
														if(pos != string::npos){
															// Устанавливаем название сообщения
															const string & pid = this->_app.substr(pos + 1, this->_app.length() - (pos + 2));
															// Выполняем установку идентификатора процесса
															if(this->_fmk->is(pid, fmk_t::check_t::NUMBER)){
																/**
																 * Выполняем отлов ошибок
																 */
																try {
																	// Выполняем получение идентификатора процесса
																	this->_pid = static_cast <pid_t> (::stoi(pid));
																/**
																 * Если возникает ошибка
																 */
																} catch(const std::exception &) {
																	// Выполняем получение идентификатора процесса
																	this->_pid = 0;
																}
															// Устанавливаем идентификатор процесса по умолчанию
															} else this->_pid = 0;
															// Удаляем лишние символы
															this->_app.replace(pos, this->_app.length() - pos, "");
														}
													}
													// Увеличиваем значение статуса
													status++;
													// Запоминаем начало строки с версией
													pos = (i + 2);
												// Если установлен пробел
												} else if(syslog.at(i) == ' ')
													// Увеличиваем значение статуса
													status++;
												// Если мы перебрали все оставшиеся символы
												else if(i == (syslog.length() - 1))
													// Устанавливаем полученное сообщение
													this->_message = syslog.substr(pos);
											} break;
											// Если производится сбор сообщения
											case 5: {
												// Устанавливаем полученное сообщение
												this->_message = syslog.substr(pos);
												// Выходим из цикла
												return;
											}
										}
									} break;
									// Если установлен стандарт RFC5424
									case static_cast <uint8_t> (std_t::RFC5424): {
										// Определяем статус парсинга
										switch(status){
											// Если производится сбор даты сообщения
											case 2: {
												// Если установлен пробел
												if(syslog.at(i) == ' '){
													// Устанавливаем дату сообщения
													this->date(syslog.substr(pos, i - pos), "%Y-%m-%dT%H:%M:%S");
													// Запоминаем начало строки с версией
													pos = (i + 1);
													// Увеличиваем значение статуса
													status++;
												}
											} break;
											// Если производится сбор хоста сообщения
											case 3: {
												// Если установлен пробел
												if(syslog.at(i) == ' '){
													// Устанавливаем хост сообщения
													this->_host = syslog.substr(pos, i - pos);
													// Запоминаем начало строки с версией
													pos = (i + 1);
													// Увеличиваем значение статуса
													status++;
												}
											} break;
											// Если производится сбор названия приложения сообщения
											case 4: {
												// Если установлен пробел
												if(syslog.at(i) == ' '){
													// Устанавливаем название сообщения
													this->_app = syslog.substr(pos, i - pos);
													// Запоминаем начало строки с версией
													pos = (i + 1);
													// Увеличиваем значение статуса
													status++;
												}
											} break;
											// Если производится сбор идентификатора процесса приславшего сообщения
											case 5: {
												// Если установлен пробел
												if(syslog.at(i) == ' '){
													// Устанавливаем название сообщения
													const string & pid = syslog.substr(pos, i - pos);
													// Если идентификатор процесса не установлен
													if(pid.compare("-") == 0)
														// Устанавливаем идентификатор процесса по умолчанию
														this->_pid = 0;
													// Выполняем установку идентификатора процесса
													else if(this->_fmk->is(pid, fmk_t::check_t::NUMBER)) {
														/**
														 * Выполняем отлов ошибок
														 */
														try {
															// Устанавливаем идентификатор процесса
															this->_pid = static_cast <pid_t> (::stoi(pid));
														/**
														 * Если возникает ошибка
														 */
														} catch(const std::exception &) {
															// Выполняем получение идентификатора процесса
															this->_pid = 0;
														}
													// Устанавливаем идентификатор процесса по умолчанию
													} else this->_pid = 0;
													// Запоминаем начало строки с версией
													pos = (i + 1);
													// Увеличиваем значение статуса
													status++;
												}
											} break;
											// Если производится сбор идентификатора сообщения
											case 6: {
												// Если установлен пробел
												if(syslog.at(i) == ' '){
													// Устанавливаем идентификатор сообщения
													this->_mid = syslog.substr(pos, i - pos);
													// Запоминаем начало строки с версией
													pos = (i + 1);
													// Увеличиваем значение статуса
													status++;
												}
											} break;
											// Если производится сбор списка структурированных данных
											case 7: {
												// Если установлен пробел
												if(syslog.at(i) == '['){
													// Запоминаем начало строки с версией
													pos = (i + 1);
													// Увеличиваем значение статуса
													status++;
												// Если структурированные данные не установлены
												} else if((syslog.at(i) == '-') || (syslog.at(i) == ' ')) {
													// Запоминаем начало строки с версией
													pos = (i + (syslog.at(i) == '-' ? 2 : 1));
													// Увеличиваем значение статуса
													status = 10;
												}
											} break;
											// Если производится сбор идентификатора структурированных данных
											case 8: {
												// Если установлен пробел
												if(syslog.at(i) == ' '){
													// Получаем идентификатор структурированных данных
													sid = syslog.substr(pos, i - pos);
													// Запоминаем начало строки с версией
													pos = (i + 1);
													// Увеличиваем значение статуса
													status++;
												}
											} break;
											// Если производится сбор параметров структурированных данных
											case 9: {
												// Если установлен пробел
												if(syslog.at(i) == ' '){
													// Получаем строку параметров
													const string & param = syslog.substr(pos, i - pos);
													// Выполняем поиск разделителя
													const size_t sep = param.find("=");
													// Если разделитель найден
													if(sep != string::npos){
														// Получаем ключ
														string key = param.substr(0, sep);
														// Получаем значение
														string val = param.substr(sep + 1);
														// Выполняем удаление кавычек
														val.assign(val.begin() + 1, val.end() - 1);
														// Получаем идентификатор структурированных данных
														auto i = this->_sd.find(sid);
														// Если идентификатор существует
														if(i != this->_sd.end())
															// Устанавливаем структурированные данные
															i->second.emplace(std::move(key), std::move(val));
														// Иначе добавляем новые структурированные данные
														else this->_sd.emplace(sid, std::unordered_map <string, string> {{std::move(key), std::move(val)}});
													}
													// Запоминаем начало строки с версией
													pos = (i + 1);
												// Если установлен символ завершения сбора структурированных данных
												} else if(syslog.at(i) == ']') {
													// Получаем строку параметров
													const string & param = syslog.substr(pos, i - pos);
													// Выполняем поиск разделителя
													const size_t sep = param.find("=");
													// Если разделитель найден
													if(sep != string::npos){
														// Получаем ключ
														string key = param.substr(0, sep);
														// Получаем значение
														string val = param.substr(sep + 1);
														// Выполняем удаление кавычек
														val.assign(val.begin() + 1, val.end() - 1);
														// Получаем идентификатор структурированных данных
														auto i = this->_sd.find(sid);
														// Если идентификатор существует
														if(i != this->_sd.end())
															// Устанавливаем структурированные данные
															i->second.emplace(std::move(key), std::move(val));
														// Иначе добавляем новые структурированные данные
														else this->_sd.emplace(sid, std::unordered_map <string, string> {{std::move(key), std::move(val)}});
													}
													// Запоминаем начало строки с версией
													pos = (i + 1);
													// Увеличиваем значение статуса
													status = 7;
												}
											} break;
											// Если производится сбор сообщения
											case 10: {
												// Устанавливаем полученное сообщение
												this->_message = syslog.substr(pos);
												// Выходим из цикла
												return;
											}
										}
									} break;
								}
							}
						}
					}
				} break;
				// Если активирован режим парсинга регулярными выражениями
				case static_cast <uint8_t> (mode_t::REGEXP): {
					// Выполняем парсинг полученного сообщения
					auto result = this->_reg.exec(syslog, this->_exp.rfc3164);
					// Если результат получен, значит сообщение соответствует стандарту RFC3164
					if(!result.empty() && (result.size() > 1)){
						// Если стандарт установлен как 3164
						if(std == std_t::RFC5424){
							/**
							 * Если включён режим отладки
							 */
							#if defined(DEBUG_MODE)
								// Выводим сообщение об ошибке
								this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(syslog, static_cast <uint16_t> (std)), log_t::flag_t::CRITICAL, "Text does not comply with RFC5424 SysLog standard");
							/**
							* Если режим отладки не включён
							*/
							#else
								// Выводим сообщение об ошибке
								this->_log->print("%s", log_t::flag_t::CRITICAL, "Text does not comply with RFC5424 SysLog standard");
							#endif
						}
						// Устанавливаем стандарт
						this->_std = std_t::RFC3164;
						// Выполняем перебор всего всех полученных параметров
						for(size_t i = 1; i < result.size(); i++){
							// Выполняем получение значения
							const string & item = result.at(i);
							// Если значение получено
							if(!item.empty()){
								// Определяем номер элемента
								switch(i){
									// Если мы получили приоритетное значение
									case 1: {
										/**
										 * Выполняем отлов ошибок
										 */
										try {
											// Извлекаем значение приоритета
											this->_pri = static_cast <uint16_t> (::stoi(item));
										/**
										 * Если возникает ошибка
										 */
										} catch(const std::exception &) {
											// Извлекаем значение приоритета
											this->_pri = 0;
										}
									} break;
									// Если мы получили значение даты
									case 2: {
										// Выполняем парсинг даты
										const auto & date = this->_reg.exec(item, this->_exp.date1);
										// Если формат даты детектирован
										if(!date.empty() && (date.size() > 1)){
											// Сформированный формат даты
											string format = "";
											// Выполняем перебор всех параметров даты
											for(size_t j = 1; j < date.size(); j++){
												// Если дата получена
												if(!date.at(j).empty()){
													// Определяем тип полученных данных
													switch(j){
														// Если мы получили день недели
														case 1:
															// Выполняем формирование формата даты
															format = "%a";
														break;
														// Если мы получили месяц
														case 2: {
															// Если формат уже сформирован
															if(!format.empty())
																// Выполняем добавление разделителя
																format.append(1, ' ');
															// Выполняем добавление месяца
															format.append("%b");
														} break;
														// Если мы получили день недели
														case 3: {
															// Если формат уже сформирован
															if(!format.empty())
																// Выполняем добавление разделителя
																format.append(1, ' ');
															// Выполняем добавление месяца
															format.append("%d");
														} break;
														// Если мы получили время
														case 4: {
															// Если формат уже сформирован
															if(!format.empty())
																// Выполняем добавление разделителя
																format.append(1, ' ');
															// Выполняем добавление месяца
															format.append("%H:%M:%S");
														} break;
														// Если мы получили год
														case 5: {
															// Если формат уже сформирован
															if(!format.empty())
																// Выполняем добавление разделителя
																format.append(1, ' ');
															// Выполняем добавление месяца
															format.append("%Y");
														} break;
													}
												// Если мы получили год но он пустой
												} else if(j == 5) {
													// Если формат уже сформирован
													if(!format.empty())
														// Выполняем добавление разделителя
														format.append(1, ' ');
													// Выполняем добавление месяца
													format.append("%Y");
													// Получаем текущее значение даты
													const time_t timestamp = ::time(nullptr);
													// Создаем структуру времени
													std::tm * tm = ::localtime(&timestamp);
													// Выполняем добавление разделителя
													const_cast <string &> (item).append(1, ' ');
													// Выполняем добавление текущего года
													const_cast <string &> (item).append(std::to_string(1900 + tm->tm_year));
												}
											}
											// Устанавливаем дату сообщения
											this->date(item, format);
										// Если парсинг даты не выполнен
										} else {
											// Если формат даты детектирован
											if(this->_reg.test(item, this->_exp.date2))
												// Устанавливаем дату сообщения
												this->date(item, "%Y-%m-%d %H:%M:%S");
											// Если парсинг даты не выполнен
											else {
												// Если формат даты детектирован
												if(this->_reg.test(item, this->_exp.date3))
													// Устанавливаем дату сообщения
													this->date(item, "%Y-%m-%dT%H:%M:%S");
												// Если парсинг даты не выполнен
												else this->_log->print("SysLog parse: %s", log_t::flag_t::WARNING, "date format is not defined");
											}
										}
									} break;
									// Если мы получили хост машины
									case 3:
										// Устанавливаем хост сообщения
										this->_host = item;
									break;
									// Если мы получили название приложения
									case 4:
										// Устанавливаем название сообщения
										this->_app = item;
									break;
									// Если мы получили идентификатор процесса
									case 5: {
										// Выполняем установку идентификатора процесса
										if(this->_fmk->is(item, fmk_t::check_t::NUMBER)){
											/**
											 * Выполняем отлов ошибок
											 */
											try {
												// Выполняем получение идентификатора процесса
												this->_pid = static_cast <pid_t> (::stoi(item));
											/**
											 * Если возникает ошибка
											 */
											} catch(const std::exception &) {
												// Выполняем получение идентификатора процесса
												this->_pid = 0;
											}
										// Устанавливаем идентификатор процесса по умолчанию
										} else this->_pid = 0;
									} break;
									// Если мы получили текст сообщения
									case 6:
										// Устанавливаем полученное сообщение
										this->_message = item;
									break;
								}
							}
						}
					// Если результат не получен
					} else {
						// Если стандарт установлен как 3164
						if(std == std_t::RFC3164){
							/**
							 * Если включён режим отладки
							 */
							#if defined(DEBUG_MODE)
								// Выводим сообщение об ошибке
								this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(syslog, static_cast <uint16_t> (std)), log_t::flag_t::CRITICAL, "Text does not comply with RFC3164 SysLog standard");
							/**
							* Если режим отладки не включён
							*/
							#else
								// Выводим сообщение об ошибке
								this->_log->print("%s", log_t::flag_t::CRITICAL, "Text does not comply with RFC3164 SysLog standard");
							#endif
						}
						// Выполняем парсинг полученного сообщения
						result = this->_reg.exec(syslog, this->_exp.rfc5424);
						// Если результат получен, значит сообщение соответствует стандарту RFC5424
						if(!result.empty() && (result.size() > 1)){
							// Устанавливаем стандарт
							this->_std = std_t::RFC5424;
							// Выполняем перебор всего всех полученных параметров
							for(size_t i = 1; i < result.size(); i++){
								// Выполняем получение значения
								const string & item = result.at(i);
								// Если значение получено
								if(!item.empty()){
									// Определяем номер элемента
									switch(i){
										// Если мы получили приоритетное значение
										case 1: {
											/**
											 * Выполняем отлов ошибок
											 */
											try {
												// Извлекаем значение приоритета
												this->_pri = static_cast <uint16_t> (::stoi(item));
											/**
											 * Если возникает ошибка
											 */
											} catch(const std::exception &) {
												// Извлекаем значение приоритета
												this->_pri = 0;
											}
										} break;
										// Если мы получили версию сообщения
										case 2: {
											/**
											 * Выполняем отлов ошибок
											 */
											try {
												// Получаем версию сообщения
												this->_ver = static_cast <uint8_t> (::stoi(item));
											/**
											 * Если возникает ошибка
											 */
											} catch(const std::exception &) {
												// Получаем версию сообщения
												this->_ver = 0;
											}
										} break;
										// Если мы получили значение даты
										case 3: {
											// Выполняем парсинг даты
											const auto & date = this->_reg.exec(item, this->_exp.date1);
											// Если формат даты детектирован
											if(!date.empty() && (date.size() > 1)){
												// Сформированный формат даты
												string format = "";
												// Выполняем перебор всех параметров даты
												for(size_t j = 1; j < date.size(); j++){
													// Если дата получена
													if(!date.at(j).empty()){
														// Определяем тип полученных данных
														switch(j){
															// Если мы получили день недели
															case 1:
																// Выполняем формирование формата даты
																format = "%a";
															break;
															// Если мы получили месяц
															case 2: {
																// Если формат уже сформирован
																if(!format.empty())
																	// Выполняем добавление разделителя
																	format.append(1, ' ');
																// Выполняем добавление месяца
																format.append("%b");
															} break;
															// Если мы получили день недели
															case 3: {
																// Если формат уже сформирован
																if(!format.empty())
																	// Выполняем добавление разделителя
																	format.append(1, ' ');
																// Выполняем добавление месяца
																format.append("%d");
															} break;
															// Если мы получили время
															case 4: {
																// Если формат уже сформирован
																if(!format.empty())
																	// Выполняем добавление разделителя
																	format.append(1, ' ');
																// Выполняем добавление месяца
																format.append("%H:%M:%S");
															} break;
															// Если мы получили год
															case 5: {
																// Если формат уже сформирован
																if(!format.empty())
																	// Выполняем добавление разделителя
																	format.append(1, ' ');
																// Выполняем добавление месяца
																format.append("%Y");
															} break;
														}
													// Если мы получили год но он пустой
													} else if(j == 5) {
														// Если формат уже сформирован
														if(!format.empty())
															// Выполняем добавление разделителя
															format.append(1, ' ');
														// Выполняем добавление месяца
														format.append("%Y");
														// Получаем текущее значение даты
														const time_t timestamp = ::time(nullptr);
														// Создаем структуру времени
														std::tm * tm = ::localtime(&timestamp);
														// Выполняем добавление разделителя
														const_cast <string &> (item).append(1, ' ');
														// Выполняем добавление текущего года
														const_cast <string &> (item).append(std::to_string(1900 + tm->tm_year));
													}
												}
												// Устанавливаем дату сообщения
												this->date(item, format);
											// Если парсинг даты не выполнен
											} else {
												// Если формат даты детектирован
												if(this->_reg.test(item, this->_exp.date2))
													// Устанавливаем дату сообщения
													this->date(item, "%Y-%m-%d %H:%M:%S");
												// Если парсинг даты не выполнен
												else {
													// Если формат даты детектирован
													if(this->_reg.test(item, this->_exp.date3))
														// Устанавливаем дату сообщения
														this->date(item, "%Y-%m-%dT%H:%M:%S");
													// Если парсинг даты не выполнен
													else this->_log->print("SysLog parse: %s", log_t::flag_t::WARNING, "date format is not defined");
												}
											}
										} break;
										// Если мы получили хост машины
										case 4:
											// Устанавливаем хост сообщения
											this->_host = item;
										break;
										// Если мы получили название приложения
										case 5:
											// Устанавливаем название сообщения
											this->_app = item;
										break;
										// Если мы получили идентификатор процесса
										case 6: {
											// Выполняем установку идентификатора процесса
											if(this->_fmk->is(item, fmk_t::check_t::NUMBER)){
												/**
												 * Выполняем отлов ошибок
												 */
												try {
													// Выполняем получение идентификатора процесса
													this->_pid = static_cast <pid_t> (::stoi(item));
												/**
												 * Если возникает ошибка
												 */
												} catch(const std::exception &) {
													// Выполняем получение идентификатора процесса
													this->_pid = 0;
												}
											// Устанавливаем идентификатор процесса по умолчанию
											} else this->_pid = 0;
										} break;
										// Если мы получили идентификатор сообщения
										case 7:
											// Устанавливаем идентификатор сообщения
											this->_mid = item;
										break;
										// Если мы получили параметры сообщения
										case 8: {
											// Смещение в сообщении
											size_t offset = 0;
											// Выполняем извлечение всех сообщений
											for(;;){
												// Выполняем получение списка сообщений
												const auto & messages = this->_reg.match(item.c_str() + offset, this->_exp.mess);
												// Если список сообщений получен
												if(!messages.empty()){
													// Получаем текст сообщения
													const string & message = item.substr(messages.front().first + offset, messages.front().second);
													// Если параметры установлены
													if(message.front() != '-'){
														// Выполняем извлечение параметров
														const auto & params = this->_reg.exec(message, this->_exp.params);
														// Если список параметров получен
														if(!params.empty() && (params.size() > 1)){
															// Идентификатор структурированных данных
															string sid = "";
															// Выполняем перебор всего списка параметров
															for(size_t i = 1; i < params.size(); i++){
																// Если идентификатор структурированных данных получен
																if(i == 1)
																	// Устанавливаем идентификатор структурированных данных
																	sid = params.at(i);
																// Выполняем перебор всех параметров
																else {
																	// Смещение в сообщении
																	size_t offset = 0;
																	// Получаем строку текста для обработки
																	const string & text = params.at(i);
																	// Выполняем извлечение всех сообщений
																	for(;;){
																		// Выполняем извлечение всего списка установленных параметров
																		const auto & items = this->_reg.match(text.c_str() + offset, this->_exp.items);
																		// Если список параметров получен
																		if(!items.empty()){
																			// Если элементов параметров получены 4 штуки
																			if(items.size() >= 4){
																				// Получаем ключ записи
																				const string & key = text.substr(items.at(1).first + offset, items.at(1).second);
																				// Получаем значение записи
																				const string & value = (items.back().first == 0 ? text.substr(items.at(2).first + offset, items.at(2).second) : text.substr(items.back().first + offset, items.back().second));
																				// Получаем идентификатор структурированных данных
																				auto i = this->_sd.find(sid);
																				// Если идентификатор существует
																				if(i != this->_sd.end())
																					// Устанавливаем структурированные данные
																					i->second.emplace(key, value);
																				// Иначе добавляем новые структурированные данные
																				else this->_sd.emplace(sid, std::unordered_map <string, string> {{key,  value}});
																			}
																			// Увеличиваем длину сообщения
																			offset += (items.front().first + items.front().second);
																		// Выходим из цикла
																		} else break;
																	}
																}
															}
														}
													}
													// Увеличиваем длину сообщения
													offset += (messages.front().first + messages.front().second);
												// Выходим из цикла
												} else break;
											}
										} break;
										// Если мы получили текст сообщения
										case 9:
											// Устанавливаем полученное сообщение
											this->_message = this->_fmk->transform(item, fmk_t::transform_t::TRIM);
										break;
									}
								}
							}
						// Выводим сообщение, что текст не соответствует стандартам SysLog
						} else {
							/**
							 * Если включён режим отладки
							 */
							#if defined(DEBUG_MODE)
								// Выводим сообщение об ошибке
								this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(syslog, static_cast <uint16_t> (std)), log_t::flag_t::CRITICAL, "Text does not comply with SysLog standards RFC3164 and RFC5424");
							/**
							* Если режим отладки не включён
							*/
							#else
								// Выводим сообщение об ошибке
								this->_log->print("%s", log_t::flag_t::CRITICAL, "Text does not comply with SysLog standards RFC3164 and RFC5424");
							#endif
						}
					}
				} break;
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
				this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(syslog, static_cast <uint16_t> (std)), log_t::flag_t::CRITICAL, error.what());
			/**
			* Если режим отладки не включён
			*/
			#else
				// Выводим сообщение об ошибке
				this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
			#endif
		}
	}
}
/**
 * has Метод проверки существования идентификатора структурированных данных
 * @param id идентификатор структурированных данных для проверки
 * @return   результат проверки существования идентификатора структурированных данных
 */
bool anyks::SysLog::has(const string & id) const noexcept {
	// Результат работы функции
	bool result = false;
	// Если идентификатор структурированных данных передан
	if(!id.empty())
		// Выполняем проверку на существование идентификатора структурированных данных
		result = (this->_sd.find(id) != this->_sd.end());
	// Выводим результат
	return result;
}
/**
 * has Метод проверки существования ключа структурированных данных
 * @param id  идентификатор структурированных данных
 * @param key ключ структурированных данных для проверки
 * @return    результат проверки существования структурированных данных
 */
bool anyks::SysLog::has(const string & id, const string & key) const noexcept {
	// Результат работы функции
	bool result = false;
	// Если идентификатор и ключ структурированных данных переданы
	if(!id.empty() && !key.empty()){
		// Выполняем извлечение данных запрашиваемого идентификатора структурированных данных
		auto i = this->_sd.find(id);
		// Если структурированные данные существуют
		if(i != this->_sd.end())
			// Выполняем проверку сущестования ключа структурированных данных
			result = (i->second.find(key) != i->second.end());
	}
	// Выводим результат
	return result;
}
/**
 * sd Метод получения структурированных данных
 * @param id  идентификатор структурированных данных
 * @param key ключ структурированных данных для извлечения
 * @return    структурированные данные
 */
const string & anyks::SysLog::sd(const string & id, const string & key) const noexcept {
	// Результат работы функции
	static const string result = "";
	// Если идентификатор и ключ структурированных данных переданы
	if(!id.empty() && !key.empty()){
		// Выполняем извлечение данных запрашиваемого идентификатора структурированных данных
		auto i = this->_sd.find(id);
		// Если структурированные данные существуют
		if(i != this->_sd.end()){
			// Выполняем извлечение данных ключа структурированных данных
			auto j = i->second.find(key);
			// Если ключ структурированных данных существует
			if(j != i->second.end())
				// Выводим результат
				return j->second;
		}
	}
	// Выводим результат
	return result;
}
/**
 * sd Метод получения списка структурированных данных
 * @param id идентификатор структурированных данных
 * @return   список структурированных данных
 */
const std::unordered_map <string, string> & anyks::SysLog::sd(const string & id) const noexcept {
	// Результат работы функции
	static const std::unordered_map <string, string> result;
	// Если идентификатор структурированных данных передан
	if(!id.empty()){
		// Выполняем извлечение данных запрашиваемого идентификатора структурированных данных
		auto i = this->_sd.find(id);
		// Если структурированные данные существуют
		if(i != this->_sd.end())
			// Выводим список полученных структурированных данных
			return i->second;
	}
	// Выводим результат
	return result;
}
/**
 * sd Метод установки структурированных данных
 * @param id идентификатор структурированных данных
 * @param sd список структурированных данных
 */
void anyks::SysLog::sd(const string & id, const std::unordered_map <string, string> & sd) noexcept {
	// Если идентификатор и список структурированных данных переданы
	if(!id.empty() && !sd.empty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем блокировку потока
			const lock_guard <std::recursive_mutex> lock(this->_mtx);
			// Выполняем извлечение данных запрашиваемого идентификатора структурированных данных
			auto i = this->_sd.find(id);
			// Если структурированные данные существуют
			if(i != this->_sd.end()){
				// Выполняем переход по всему списку переданных ключей
				for(auto & item : sd){
					// Если данный ключ уже существует
					if(i->second.find(item.first) != i->second.end())
						// Заменяем данные переданного ключа
						i->second[item.first] = item.second;
					// Иначе добавляем новые данные ключа и значения
					else i->second.emplace(item.first, item.second);
				}
			// Добавляем структурированные данные как они есть
			} else this->_sd.emplace(std::forward <const string> (id), std::forward <const std::unordered_map <string, string>> (sd));
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & error) {
			/**
			 * Если включён режим отладки
			 */
			#if defined(DEBUG_MODE)
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(id, sd.size()), log_t::flag_t::CRITICAL, error.what());
			/**
			* Если режим отладки не включён
			*/
			#else
				// Выводим сообщение об ошибке
				this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
			#endif
		}
	}
}
/**
 * std Метод получения стандарта сообщения
 * @return стандарт сообщения
 */
anyks::SysLog::std_t anyks::SysLog::std() const noexcept {
	// Выводим данные установленного стандарта
	return this->_std;
}
/**
 * version Метод извлечения версии сообщения
 * @return версия сообщения
 */
uint8_t anyks::SysLog::version() const noexcept {
	// Выводим версию сообщения
	return this->_ver;
}
/**
 * version Метод установки версии сообщения
 * @param version версия сообщения для установки
 */
void anyks::SysLog::version(const uint8_t version) noexcept {
	// Выполняем блокировку потока
	const lock_guard <std::recursive_mutex> lock(this->_mtx);
	// Устанавливаем версию сообщения
	this->_ver = version;
}
/**
 * category Метод извлечения категории сообщения
 * @return категория сообщения
 */
uint8_t anyks::SysLog::category() const noexcept {
	// Выводим категорию сообщения
	return static_cast <uint8_t> (::floor(this->_pri / 8));
}
/**
 * importance Метод получения важности сообщения
 * @return важность сообщения
 */
uint8_t anyks::SysLog::importance() const noexcept {
	// Выводим важность сообщения
	return static_cast <uint8_t> (this->_pri - (::floor(this->_pri / 8) * 8));
}
/**
 * pri Метод установки приоритета
 * @param category   категория сообщения для установки
 * @param importance важность сообщения для установки
 */
void anyks::SysLog::pri(const uint8_t category, const uint8_t importance) noexcept {
	// Выполняем блокировку потока
	const lock_guard <std::recursive_mutex> lock(this->_mtx);
	// Выполняем установку приоритера сообщения
	this->_pri = (static_cast <uint16_t> (category) * static_cast <uint16_t> (8) + static_cast <uint16_t> (importance));
}
/**
 * host Метод получения хоста сообщения
 * @return хост сообщения
 */
string anyks::SysLog::host() const noexcept {
	// Выводим хост сообщения
	return this->_host;
}
/**
 * host Метод установки хоста сообщения
 * @param host хост сообщения для установки
 */
void anyks::SysLog::host(const string & host) noexcept {
	// Выполняем блокировку потока
	const lock_guard <std::recursive_mutex> lock(this->_mtx);
	// Если хост передан
	if(host.empty())
		// Выполняем установку пустого значения хоста
		this->_host = "-";
	// Устанавливаем хост сообщения
	else this->_host = std::forward <const string> (host);
}
/**
 * application Метод получения названия приложения сообщения
 * @return название приложения сообщения
 */
string anyks::SysLog::application() const noexcept {
	// Выводим название приложения приславшего сообщения
	return this->_app;
}
/**
 * application Метод установки названия приложения сообщения
 * @param app назование приложения для установки
 */
void anyks::SysLog::application(const string & app) noexcept {
	// Выполняем блокировку потока
	const lock_guard <std::recursive_mutex> lock(this->_mtx);
	// Если название приложения передано
	if(app.empty())
		// Выполняем установку пустого значения названия приложения
		this->_app = "-";
	// Устанавливаем название приложения приславшего сообщение
	else this->_app = std::forward <const string> (app);
}
/**
 * pid Метод получения идентификатора процесса сообщения
 * @return идентификатор процесса сообщения
 */
pid_t anyks::SysLog::pid() const noexcept {
	// Выводим идентификатор процесса
	return this->_pid;
}
/**
 * pid Метод установки идентификатора процесса
 * @param pid идентификатор процесса для установки
 */
void anyks::SysLog::pid(const pid_t pid) noexcept {
	// Выполняем блокировку потока
	const lock_guard <std::recursive_mutex> lock(this->_mtx);
	// Устанавливаем идентификатор процесса
	this->_pid = pid;
}
/**
 * mid Метод получения идентификатора сообщения
 * @return идентификатор полученного сообщения 
 */
string anyks::SysLog::mid() const noexcept {
	// Выводим идентификатор сообщения
	return this->_mid;
}
/**
 * mid Метод установки идентификатора сообщения
 * @param mid идентификатор сообщения для установки
 */
void anyks::SysLog::mid(const string & mid) noexcept {
	// Выполняем блокировку потока
	const lock_guard <std::recursive_mutex> lock(this->_mtx);
	// Если идентификатор сообщения передан
	if(mid.empty())
		// Выполняем установку пустого значения идентификатора сообщения
		this->_mid = "-";
	// Устанавливаем идентификатор сообщения
	else this->_mid = std::forward <const string> (mid);
}
/**
 * message Метод получения сообщения
 * @return полученное сообщение
 */
string anyks::SysLog::message() const noexcept {
	// Выводим сообщение
	return this->_message;
}
/**
 * message Метод установки сообщения
 * @param message сообщение для установки
 */
void anyks::SysLog::message(const string & message) noexcept {
	// Выполняем блокировку потока
	const lock_guard <std::recursive_mutex> lock(this->_mtx);
	// Устанавливаем сообщение
	this->_message = std::forward <const string> (message);
}
/**
 * format Метод получения установленного формата даты
 * @return установленный формат даты
 */
string anyks::SysLog::format() const noexcept {
	// Выводим установленный формат даты
	return this->_format;
}
/**
 * format Метод установки формата даты
 * @param format формат даты для установки
 */
void anyks::SysLog::format(const string & format) noexcept {
	// Выполняем блокировку потока
	const lock_guard <std::recursive_mutex> lock(this->_mtx);
	// Если формат даты передан
	if(!format.empty())
		// Выполняем установку формата даты
		this->_format = format;
}
/**
 * date Метод получения даты сообщения
 * @param format формат даты сообщения
 * @return       дата сообщения в указанном формате
 */
string anyks::SysLog::date(const string & format) const noexcept {
	/**
	 * Выполняем отлов ошибок
	 */
	try {
		// Создаём объект потока
		std::stringstream transTime;
		// Создаем структуру времени
		std::tm * tm = ::localtime(&this->_timestamp);
		// Если формат даты сообщения установлен
		if(!format.empty()){
			// Выполняем извлечение даты
			transTime << std::put_time(tm, format.c_str());
			// Устанавливаем формат даты сообщения
			const_cast <SysLog *> (this)->_format = std::forward <const string> (format);
		// Выполняем парсинг даты
		} else transTime << std::put_time(tm, this->_format.c_str());
		// Выводим результат сформированной даты
		return transTime.str();
	/**
	 * Если возникает ошибка
	 */
	} catch(const std::exception & error) {
		/**
		 * Если включён режим отладки
		 */
		#if defined(DEBUG_MODE)
			// Выводим сообщение об ошибке
			this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(format), log_t::flag_t::CRITICAL, error.what());
		/**
		* Если режим отладки не включён
		*/
		#else
			// Выводим сообщение об ошибке
			this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
		#endif
	}
	// Выводим пустое значение
	return "";
}
/**
 * date Метод установки даты сообщения
 * @param date   дата сообщения для установки
 * @param format формат даты сообщения для установки
 */
void anyks::SysLog::date(const string & date, const string & format) noexcept {
	// Если данные даты переданы
	if(!date.empty() && !format.empty()){
		// Выполняем блокировку потока
		const lock_guard <std::recursive_mutex> lock(this->_mtx);
		// Если формат даты сообщения установлен
		if(!format.empty())
			// Выполняем парсинг даты
			this->_timestamp = this->_fmk->str2time(date.c_str(), format.c_str());
		// Выполняем парсинг даты
		else this->_timestamp = this->_fmk->str2time(date.c_str(), this->_format.c_str());
	}
}
/**
 * syslog Метод получения данных в формате SysLog
 * @return данные в формате SysLog
 */
string anyks::SysLog::syslog() const noexcept {
	/**
	 * Выполняем отлов ошибок
	 */
	try {
		// Результат работы функции
		string result = "";
		// Определяем стандарт SysLog
		switch(static_cast <uint8_t> (this->_std)){
			// Если установлен стандарт RFC3164
			case static_cast <uint8_t> (std_t::RFC3164): {
				// Если приоритет установлен
				if(this->_pri > 0){
					// Устанавливаем начало экранирования
					result.append(1, '<');
					// Устанавливаем значение приоритета
					result.append(std::to_string(this->_pri));
					// Устанавливаем конец экранирования
					result.append(1, '>');
				}
				// Если штамп времени установлен
				if(this->_timestamp > 0){
					// Получаем текущее значение даты
					const time_t timestamp = ::time(nullptr);
					// Создаем структуру времени
					std::tm * tm = ::localtime(&timestamp);
					// Если установленный год совпадает с текущим годом
					if(std::to_string(1900 + tm->tm_year).compare(this->date("%Y")) == 0)
						// Устанавливаем дату сообщения
						result.append(this->date("%b %d %H:%M:%S"));
					// Выполняем установку года формирования лога
					else result.append(this->date("%b %d %H:%M:%S %Y"));
					// Если хост, название приложения или сообщение переданы
					if(!this->_host.empty() || !this->_app.empty() || !this->_message.empty())
						// Устанавливаем разделитель
						result.append(1, ' ');
				}
				// Если хост сообщения установлен
				if(!this->_host.empty() && (this->_host.front() != '-')){
					// Устанавливаем хост сообщения
					result.append(this->_host);
					// Если хост, название приложения или сообщение переданы
					if(!this->_app.empty() || !this->_message.empty())
						// Устанавливаем разделитель
						result.append(1, ' ');
				}
				// Если название приложения установлено
				if(!this->_app.empty() && (this->_app.front() != '-')){
					// Устанавливаем название сообщения
					result.append(this->_app);
					// Если идентификатор процесса передан
					if(this->_pid > 0){
						// Устанавливаем открытие экранирования
						result.append(1, '[');
						// Устанавливаем идентификатор процесса
						result.append(std::to_string(this->_pid));
						// Устанавливаем закрытие экранирования
						result.append(1, ']');
					}
					// Устанавливаем разделитель
					result.append(1, ':');
					// Если хост, название приложения или сообщение переданы
					if(!this->_message.empty())
						// Устанавливаем разделитель
						result.append(1, ' ');
				}
				// Если сообщение установлено
				if(!this->_message.empty())
					// Устанавливаем сообщение
					result.append(this->_message);
			} break;
			// Если установлен стандарт RFC5424
			case static_cast <uint8_t> (std_t::RFC5424): {
				// Если приоритет установлен
				if(this->_pri > 0){
					// Устанавливаем начало экранирования
					result.append(1, '<');
					// Устанавливаем значение приоритета
					result.append(std::to_string(this->_pri));
					// Устанавливаем конец экранирования
					result.append(1, '>');
					// Устанавливаем версию сообщения
					result.append(std::to_string(this->_ver));
					// Устанавливаем разделитель
					result.append(1, ' ');
				}
				// Если штамп времени установлен
				if(this->_timestamp > 0)
					// Устанавливаем дату сообщения
					result.append(this->date(FORMAT));
				// Устанавливаем пропуск
				else result.append(1, '-');
				// Устанавливаем разделитель
				result.append(1, ' ');
				// Если хост сообщения установлен
				if(!this->_host.empty())
					// Устанавливаем хост сообщения
					result.append(this->_host);
				// Устанавливаем пропуск
				else result.append(1, '-');
				// Устанавливаем разделитель
				result.append(1, ' ');
				// Если название приложения установлено
				if(!this->_app.empty())
					// Устанавливаем название сообщения
					result.append(this->_app);
				// Устанавливаем пропуск
				else result.append(1, '-');
				// Устанавливаем разделитель
				result.append(1, ' ');
				// Если идентификатор процесса передан
				if(this->_pid > 0)
					// Устанавливаем идентификатор процесса
					result.append(std::to_string(this->_pid));
				// Устанавливаем пропуск
				else result.append(1, '-');
				// Устанавливаем разделитель
				result.append(1, ' ');
				// Если идентификатор сообщения установлен
				if(!this->_mid.empty())
					// Устанавливаем идентификатор сообщения
					result.append(this->_mid);
				// Устанавливаем пропуск
				else result.append(1, '-');
				// Устанавливаем разделитель
				result.append(1, ' ');
				// Если список структурированных данных установлен
				if(!this->_sd.empty()){
					// Выполняем перебор структурированных данных
					for(auto & item1 : this->_sd){
						// Флаг установки разделителя
						bool separator = false;
						// Устанавливаем начало разделителя
						result.append(1, '[');
						// Устанавливаем идентификатор структурированных данных
						result.append(item1.first);
						// Устанавливаем разделитель
						result.append(1, ' ');
						// Выполняем перебор оставшихся структур данных
						for(auto & item2 : item1.second){
							// Если флаг установки разделителя не установлен
							if(!separator)
								// Устанавливаем флаг разделителя
								separator = !separator;
							// Устанавливаем разделитель
							else result.append(1, ' ');
							// Добавляем ключ структурированных данных
							result.append(item2.first);
							// Добавляем разделитель
							result.append(1, '=');
							// Добавляем начало экранирования
							result.append(1, '"');
							// Добавляем значение
							result.append(item2.second);
							// Добавляем конец экранирования
							result.append(1, '"');
						}
						// Устанавливаем конец разделителя
						result.append(1, ']');
					}
				// Устанавливаем пропуск
				} else result.append(1, '-');
				// Если сообщение установлено
				if(!this->_message.empty()){
					// Устанавливаем разделитель
					result.append(1, ' ');
					// Устанавливаем сообщение
					result.append(this->_message);
				}
			} break;
		}
		// Выводим результат
		return result;
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
	// Выводим результат
	return "";
}
/**
 * dump Метод извлечения данных в виде JSON
 * @return json объект дампа данных
 */
json anyks::SysLog::dump() const noexcept {
	// Результат работы функции
	json result(kObjectType);
	/**
	 * Выполняем отлов ошибок
	 */
	try {
		// Определяем стандарт SysLog
		switch(static_cast <uint8_t> (this->_std)){
			// Если установлен стандарт RFC3164
			case static_cast <uint8_t> (std_t::RFC3164):
				// Устанавливаем стандарт сообщения
				result.AddMember(Value("RFC", result.GetAllocator()).Move(), Value(3164).Move(), result.GetAllocator());
			break;
			// Если установлен стандарт RFC5424
			case static_cast <uint8_t> (std_t::RFC5424):
				// Устанавливаем стандарт сообщения
				result.AddMember(Value("RFC", result.GetAllocator()).Move(), Value(5424).Move(), result.GetAllocator());
			break;
		}
		// Если штамп времени установлен
		if(this->_timestamp > 0){
			// Получаем значение даты
			const string & date = this->date();
			// Устанавливаем дату сообщения
			result.AddMember(Value("date", result.GetAllocator()).Move(), Value(date.c_str(), date.length(), result.GetAllocator()).Move(), result.GetAllocator());
		}
		// Если приоритет установлен
		if(this->_pri > 0){
			// Устанавливаем категорию сообщения
			result.AddMember(Value("category", result.GetAllocator()).Move(), Value(static_cast <uint32_t> (this->category())).Move(), result.GetAllocator());
			// Устанавливаем важность сообщения
			result.AddMember(Value("importance", result.GetAllocator()).Move(), Value(static_cast <uint32_t> (this->importance())).Move(), result.GetAllocator());
		}
		// Если версия сообщения получена
		if(this->_ver > 0)
			// Выполняем установку версии сообщения
			result.AddMember(Value("version", result.GetAllocator()).Move(), Value(static_cast <uint32_t> (this->_ver)).Move(), result.GetAllocator());
		// Если хост установлен
		if(!this->_host.empty() && (this->_host.compare("-") != 0))
			// Устанавливаем хост сообщения
			result.AddMember(Value("host", result.GetAllocator()).Move(), Value(this->_host.c_str(), this->_host.length(), result.GetAllocator()).Move(), result.GetAllocator());
		// Если название приложения установлено
		if(!this->_app.empty() && (this->_app.compare("-") != 0))
			// Устанавливаем название сообщения
			result.AddMember(Value("application", result.GetAllocator()).Move(), Value(this->_app.c_str(), this->_app.length(), result.GetAllocator()).Move(), result.GetAllocator());
		// Если идентификатор процесса передан
		if(this->_pid > 0)
			// Устанавливаем идентификатор процесса
			result.AddMember(Value("pid", result.GetAllocator()).Move(), Value(static_cast <uint64_t> (this->_pid)).Move(), result.GetAllocator());
		// Если идентификатор сообщения установлен
		if(!this->_mid.empty() && (this->_mid.compare("-") != 0))
			// Устанавливаем идентификатор сообщения
			result.AddMember(Value("mid", result.GetAllocator()).Move(), Value(this->_mid.c_str(), this->_mid.length(), result.GetAllocator()).Move(), result.GetAllocator());
		// Если сообщение установлено
		if(!this->_message.empty() && (this->_message.compare("-") != 0))
			// Устанавливаем полученное сообщение
			result.AddMember(Value("message", result.GetAllocator()).Move(), Value(this->_message.c_str(), this->_message.length(), result.GetAllocator()).Move(), result.GetAllocator());
		// Если список структурированных данных установлен
		if(!this->_sd.empty()){
			// Добавляем объект структурированных данных
			result.AddMember(Value("sd", result.GetAllocator()).Move(), Value(kObjectType).Move(), result.GetAllocator());
			// Выполняем перебор структурированных данных
			for(auto & sd : this->_sd){
				// Устанавливаем идентификатор структурированных данных
				result["sd"].AddMember(Value(sd.first.c_str(), sd.first.length(), result.GetAllocator()).Move(), Value(kObjectType).Move(), result.GetAllocator());
				// Выполняем перебор оставшихся структур данных
				for(auto & param : sd.second){
					// Если запись является числом
					if(this->_fmk->is(param.second, fmk_t::check_t::NUMBER)){
						/**
						 * Выполняем отлов ошибок
						 */
						try {
							// Если число положительное
							if(param.second.front() != '-')
								// Добавляем полученные парасетры структурированных данных
								result["sd"][sd.first.c_str()].AddMember(Value(param.first.c_str(), param.first.length(), result.GetAllocator()).Move(), Value(static_cast <uint64_t> (::stoull(param.second))).Move(), result.GetAllocator());
							// Добавляем полученные парасетры структурированных данных
							else result["sd"][sd.first.c_str()].AddMember(Value(param.first.c_str(), param.first.length(), result.GetAllocator()).Move(), Value(static_cast <int64_t> (::stoll(param.second))).Move(), result.GetAllocator());
						/**
						 * Если возникает ошибка
						 */
						} catch(const std::exception &) {
							// Добавляем полученные парасетры структурированных данных
							result["sd"][sd.first.c_str()].AddMember(Value(param.first.c_str(), param.first.length(), result.GetAllocator()).Move(), Value(param.second.c_str(), param.second.length(), result.GetAllocator()).Move(), result.GetAllocator());
						}
					// Если запись является числом с плавающей точкой
					} else if(this->_fmk->is(param.second, fmk_t::check_t::DECIMAL)) {
						/**
						 * Выполняем отлов ошибок
						 */
						try {
							// Добавляем полученные парасетры структурированных данных
							result["sd"][sd.first.c_str()].AddMember(Value(param.first.c_str(), param.first.length(), result.GetAllocator()).Move(), Value(::stod(param.second)).Move(), result.GetAllocator());
						/**
						 * Если возникает ошибка
						 */
						} catch(const std::exception &) {
							// Добавляем полученные парасетры структурированных данных
							result["sd"][sd.first.c_str()].AddMember(Value(param.first.c_str(), param.first.length(), result.GetAllocator()).Move(), Value(param.second.c_str(), param.second.length(), result.GetAllocator()).Move(), result.GetAllocator());
						}
					// Если число является булевым истинным значением
					} else if(this->_fmk->compare("true", param.second))
						// Добавляем полученные парасетры структурированных данных
						result["sd"][sd.first.c_str()].AddMember(Value(param.first.c_str(), param.first.length(), result.GetAllocator()).Move(), Value(true).Move(), result.GetAllocator());
					// Если число является булевым ложным значением
					else if(this->_fmk->compare("false", param.second))
						// Добавляем полученные парасетры структурированных данных
						result["sd"][sd.first.c_str()].AddMember(Value(param.first.c_str(), param.first.length(), result.GetAllocator()).Move(), Value(false).Move(), result.GetAllocator());
					// Добавляем полученные парасетры структурированных данных
					else result["sd"][sd.first.c_str()].AddMember(Value(param.first.c_str(), param.first.length(), result.GetAllocator()).Move(), Value(param.second.c_str(), param.second.length(), result.GetAllocator()).Move(), result.GetAllocator());
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
			this->_log->debug("%s", __PRETTY_FUNCTION__, {}, log_t::flag_t::CRITICAL, error.what());
		/**
		* Если режим отладки не включён
		*/
		#else
			// Выводим сообщение об ошибке
			this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
		#endif
	}
	// Выводим результат
	return result;
}
/**
 * dump Метод установки данных в формате JSON
 * @param dump данные в формате JSON
 */
void anyks::SysLog::dump(const json & dump) noexcept {
	// Если данные получены
	if(dump.IsObject() && !dump.ObjectEmpty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем блокировку потока
			const lock_guard <std::recursive_mutex> lock(this->_mtx);
			// Категория и важность сообщения
			uint8_t category = 0, importance = 0;
			// Если стандарт сообщения передан
			if(dump.HasMember("RFC") && dump["RFC"].IsNumber()){
				// Определяем тип переданного стандарта
				switch(dump["RFC"].GetUint()){
					// Если передан стандарт RFC3164
					case 3164:
						// Устанавливаем стандарт RFC3164
						this->_std = std_t::RFC3164;
					break;
					// Если передан стандарт RFC5424
					case 5424:
						// Устанавливаем стандарт RFC5424
						this->_std = std_t::RFC5424;
					break;
				}
			}
			// Если версия сообщения передана
			if(dump.HasMember("version") && dump["version"].IsUint())
				// Устанавливаем версию сообщения
				this->_ver = static_cast <uint8_t> (dump["version"].GetUint());
			// Если дата передана
			if(dump.HasMember("date") && dump["date"].IsString())
				// Устанавливаем дату сообщения
				this->date(dump["date"].GetString(), this->_format);
			// Если идентификатор процесса передан
			if(dump.HasMember("pid") && dump["pid"].IsUint64())
				// Устанавливаем идентификатор процесса
				this->_pid = dump["pid"].GetUint64();
			// Если идентификатор сообщения передан
			if(dump.HasMember("mid") && dump["mid"].IsString())
				// Устанавливаем идентификатор сообщения
				this->_mid = dump["mid"].GetString();
			// Если сообщение передано
			if(dump.HasMember("message") && dump["message"].IsString())
				// Устанавливаем сообщение
				this->_message = dump["message"].GetString();
			// Если хост сообщения передан
			if(dump.HasMember("host") && dump["host"].IsString())
				// Устанавливаем хост сообщения
				this->_host = dump["host"].GetString();
			// Если название приложения передано
			if(dump.HasMember("application") && dump["application"].IsString())
				// Устанавливаем название приложения
				this->_app = dump["application"].GetString();
			// Если категория сообщения передана
			if(dump.HasMember("category") && dump["category"].IsUint())
				// Устанавливаем категорию сообщения
				category = static_cast <uint8_t> (dump["category"].GetUint());
			// Если важность сообщения передана
			if(dump.HasMember("importance") && dump["importance"].IsUint())
				// Устанавливаем важность сообщения
				importance = static_cast <uint8_t> (dump["importance"].GetUint());
			// Выполняем установку приоритета
			this->pri(category, importance);
			// Если список структурированных данных передан
			if(dump.HasMember("sd") && dump["sd"].IsObject() && !dump["sd"].ObjectEmpty()){
				// Выполняем очистку списка структурированных данных
				this->_sd.clear();
				// Выполняем перебор списка параметров
				for(auto & m : dump["sd"].GetObj()){
					// Если объект структурированных данных передан
					if(m.value.IsObject() && !m.value.ObjectEmpty()){
						// Выполняем перебор всех параметров
						for(auto & item : m.value.GetObj()){
							// Выполняем поиск объекта структурированных данных
							auto i = this->_sd.find(m.name.GetString());
							// Если объект структурированных данных уже создан
							if(i != this->_sd.end()){
								// Если параметр является числом с плавающей точкой
								if(item.value.IsDouble())
									// Выполняем установку полученного числа
									i->second.emplace(item.name.GetString(), this->_fmk->noexp(item.value.GetDouble(), true));
								// Если параметр является числом с отрицательным значением
								else if(item.value.IsInt64())
									// Выполняем установку полученного числа
									i->second.emplace(item.name.GetString(), std::to_string(item.value.GetInt64()));
								// Если параметр является числом с положительным значением
								else if(item.value.IsUint64())
									// Выполняем установку полученного числа
									i->second.emplace(item.name.GetString(), std::to_string(item.value.GetUint64()));
								// Если параметр является булевым значением
								else if(item.value.IsBool())
									// Выполняем установку булевого значения
									i->second.emplace(item.name.GetString(), item.value.GetBool() ? "true" : "false");
								// Если параметр является строковым значением
								else if(item.value.IsString())
									// Выполняем установку булевого значения
									i->second.emplace(item.name.GetString(), item.value.GetString());
							// Если объект структурированных данных ещё не создан
							} else {
								// Если параметр является числом с плавающей точкой
								if(item.value.IsDouble())
									// Выполняем установку полученного числа
									this->_sd.emplace(m.name.GetString(), std::unordered_map <string, string> {{item.name.GetString(), this->_fmk->noexp(item.value.GetDouble(), true)}});
								// Если параметр является числом с отрицательным значением
								else if(item.value.IsInt64())
									// Выполняем установку полученного числа
									this->_sd.emplace(m.name.GetString(), std::unordered_map <string, string> {{item.name.GetString(), std::to_string(item.value.GetInt64())}});
								// Если параметр является числом с положительным значением
								else if(item.value.IsUint64())
									// Выполняем установку полученного числа
									this->_sd.emplace(m.name.GetString(), std::unordered_map <string, string> {{item.name.GetString(), std::to_string(item.value.GetUint64())}});
								// Если параметр является булевым значением
								else if(item.value.IsBool())
									// Выполняем установку булевого значения
									this->_sd.emplace(m.name.GetString(), std::unordered_map <string, string> {{item.name.GetString(), item.value.GetBool() ? "true" : "false"}});
								// Если параметр является строковым значением
								else if(item.value.IsString())
									// Выполняем установку булевого значения
									this->_sd.emplace(m.name.GetString(), std::unordered_map <string, string> {{item.name.GetString(), item.value.GetString()}});
							}
						}
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
}
/**
 * mode Метод получения установленного режима парсинга
 * @return установленный режим парсинга
 */
anyks::SysLog::mode_t anyks::SysLog::mode() const noexcept {
	// Выполняем извлечение установленного режима парсинга
	return this->_mode;
}
/**
 * mode Метод установки режима парсинга
 * @param mode режим парсинга для установки
 */
void anyks::SysLog::mode(const mode_t mode) noexcept {
	// Выполняем блокировку потока
	const lock_guard <std::recursive_mutex> lock(this->_mtx);
	// Выполняем установку режим парсинга
	this->_mode = mode;
}
/**
 * Оператор вывода данные контейнера в качестве строки
 * @return данные контейнера в качестве строки
 */
anyks::SysLog::operator std::string() const noexcept {
	// Выводим данные контейнера
	return this->syslog();
}
/**
 * Оператор [!=] сравнения контейнеров
 * @param syslog контенер для сравнения
 * @return       результат сравнения
 */
bool anyks::SysLog::operator != (const syslog_t & syslog) const noexcept {
	// Выводим результат
	return (this->syslog().compare(syslog.syslog()) != 0);
}
/**
 * Оператор [==] сравнения контейнеров
 * @param syslog контенер для сравнения
 * @return       результат сравнения
 */
bool anyks::SysLog::operator == (const syslog_t & syslog) const noexcept {
	// Выводим результат
	return (this->syslog().compare(syslog.syslog()) == 0);
}
/**
 * Оператор [=] присвоения контейнеров
 * @param syslog контенер для присвоения
 * @return       текущий объект
 */
anyks::SysLog & anyks::SysLog::operator = (const syslog_t & syslog) noexcept {
	/**
	 * Выполняем отлов ошибок
	 */
	try {
		// Выполняем блокировку потока
		const lock_guard <std::recursive_mutex> lock(this->_mtx);
		// Устанавливаем список структурированных данных
		this->_sd = syslog._sd;
		// Устанавливаем стандарт сообщения SysLog
		this->_std = syslog._std;
		// Устанавливаем версию сообщения
		this->_ver = syslog._ver;
		// Устанавливаем приоритет сообщения
		this->_pri = syslog._pri;
		// Устанавливаем идентификатор процесса
		this->_pid = syslog._pid;
		// Устанавливаем название приложения
		this->_app = syslog._app;
		// Устанавливаем идентификатор сообщения
		this->_mid = syslog._mid;
		// Устанавливаем хост сообщения
		this->_host = syslog._host;
		// Устанавливаем формат даты сообщения
		this->_format = syslog._format;
		// Выполняем очистку сообщения
		this->_message = syslog._message;
		// Устанавливаем штамп времени
		this->_timestamp = syslog._timestamp;
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
	// Выводим текущий объект
	return (* this);
}
/**
 * Оператор [=] присвоения контейнеров
 * @param syslog контенер для присвоения
 * @return       текущий объект
 */
anyks::SysLog & anyks::SysLog::operator = (const string & syslog) noexcept {
	// Если данные SysLog контейнера получены
	if(!syslog.empty())
		// Выполняем парсинг SysLog сообщения
		this->parse(syslog);
	// Выводим текущий объект
	return (* this);
}
/**
 * SysLog Конструктор
 * @param fmk объект фреймворка
 * @param log объект для работы с логами
 */
anyks::SysLog::SysLog(const fmk_t * fmk, const log_t * log) noexcept :
 _std(std_t::AUTO), _ver(0), _pri(0), _mode(mode_t::REGEXP), _app{"-"}, _host{"-"},
 _pid(0), _mid{"-"}, _message{""}, _format{FORMAT}, _timestamp(0), _fmk(fmk), _log(log) {
	// Выполняем сборку регулярных выражений для распознавания формат даты (Sat Jan  8 20:07:41 2011)
	this->_exp.date1 = this->_reg.build("([a-z]+\\s+)?([a-z]+)\\s+(\\d+)\\s+(\\d{1,2}\\:\\d{1,2}\\:\\d{1,2})(?:\\s+(\\d{2,4}))?", {
		regexp_t::option_t::UTF8,
		regexp_t::option_t::UCP,
		regexp_t::option_t::CASELESS
	});
	// Выполняем сборку регулярных выражений для распознавания формат даты (2024-10-04 13:29:47)
	this->_exp.date2 = this->_reg.build("(\\d{2,4}\\-\\d{1,2}\\-\\d{1,2})\\s+(\\d{1,2}\\:\\d{1,2}\\:\\d{1,2})", {
		regexp_t::option_t::UTF8,
		regexp_t::option_t::UCP
	});
	// Выполняем сборку регулярных выражений для распознавания формат даты (2003-10-11T22:14:15.003Z)
	this->_exp.date3 = this->_reg.build("(\\d{2,4}\\-\\d{1,2}\\-\\d{1,2})T(\\d{1,2}\\:\\d{1,2}\\:\\d{1,2})(?:\\.(\\d+)Z)?", {
		regexp_t::option_t::UTF8,
		regexp_t::option_t::UCP
	});
	// Выполняем сборку регулярных выражений для извлечения сообщений для RFC5424
	this->_exp.mess = this->_reg.build("(\\-|\\[[^\\]]+\\])", {
		regexp_t::option_t::UTF8,
		regexp_t::option_t::UCP
	});
	// Выполняем сборку регулярных выражений для извлечения параметров сообщений RFC5424
	this->_exp.params = this->_reg.build("\\[([\\w\\@\\-]+)\\s+(.*)\\]", {
		regexp_t::option_t::UTF8,
		regexp_t::option_t::UCP
	});
	// Выполняем сборку регулярных выражений для извлечения параметров сообщения RFC5424
	this->_exp.items = this->_reg.build("([\\w\\-]+)\\=(?:\\\"([^\\\"]+)\\\"|(\\d+))", {
		regexp_t::option_t::UTF8,
		regexp_t::option_t::UCP
	});
	// Выполняем сборку регулярных выражений для парсинга всего сообщения RFC3164
	this->_exp.rfc3164 = this->_reg.build("(?:<(\\d+)>)?((?:(?:[a-z]+\\s+)?[a-z]+\\s+\\d+\\s+\\d{1,2}\\:\\d{1,2}\\:\\d{1,2}(?:\\s+\\d{2,4})?)|\\d{2,4}\\-\\d{1,2}\\-\\d{1,2}\\s+\\d{1,2}\\:\\d{1,2}\\:\\d{1,2}|\\d{2,4}\\-\\d{1,2}\\-\\d{1,2}T\\d{1,2}\\:\\d{1,2}\\:\\d{1,2}(?:\\.\\d+Z)?)\\s([^\\s\\:]+)\\s+([^\\[\\:]+)(?:\\[(\\d+)\\])?\\:\\s*(.+)", {
		regexp_t::option_t::UTF8,
		regexp_t::option_t::UCP,
		regexp_t::option_t::CASELESS
	});
	// Выполняем сборку регулярных выражений для парсинга всего сообщения RFC5424
	this->_exp.rfc5424 = this->_reg.build("(?:<(\\d+)>)?(?:(\\d+)\\s+)?((?:(?:[a-z]+\\s+)?[a-z]+\\s+\\d+\\s+\\d{1,2}\\:\\d{1,2}\\:\\d{1,2}(?:\\s+\\d{2,4})?)|\\d{2,4}\\-\\d{1,2}\\-\\d{1,2}\\s+\\d{1,2}\\:\\d{1,2}\\:\\d{1,2}|\\d{2,4}\\-\\d{1,2}\\-\\d{1,2}T\\d{1,2}\\:\\d{1,2}\\:\\d{1,2}(?:\\.\\d+Z)?)\\s([^\\s]+)\\s([^\\s]+)\\s+([\\d\\-]+)\\s+([\\w\\-]+)\\s+((?:(?:\\-|\\[[^\\]]+\\])\\s*)+)\\s*(.+)", {
		regexp_t::option_t::UTF8,
		regexp_t::option_t::UCP,
		regexp_t::option_t::CASELESS
	});
}
/**
 * Оператор [>>] чтения из потока SysLog контейнера
 * @param is     поток для чтения
 * @param syslog контенер для присвоения
 */
istream & anyks::operator >> (istream & is, syslog_t & syslog) noexcept {
	// Данные SysLog контейнера
	string data = "";
	// Считываем данные SysLog контейнера
	is >> data;
	// Если данные SysLog контейнера получены
	if(!data.empty())
		// Выполняем парсинг SysLog сообщения
		syslog.parse(data);
	// Выводим результат
	return is;
}
/**
 * Оператор [<<] вывода в поток SysLog контейнера
 * @param os     поток куда нужно вывести данные
 * @param syslog контенер для присвоения
 */
ostream & anyks::operator << (ostream & os, const syslog_t & syslog) noexcept {
	// Записываем в поток SysLog сообщение
	os << syslog.syslog();
	// Выводим результат
	return os;
}
