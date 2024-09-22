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
	// Устанавливаем версию сообщения
	this->_ver = 1;
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
}
/**
 * parse Метод парсинга строки в формате SysLog
 * @param syslog строка в формате SysLog
 * @param std    стандарт SysLog
 */
void anyks::SysLog::parse(const string & syslog, const std_t std) noexcept {
	// Если даныне переданы
	if(!syslog.empty()){
		// Идентификатор структурированных данных
		string sid = "";
		// Позиция начала строки
		size_t pos = 0;
		// Статус поиска
		uint8_t status = 0;
		// Количество пробелов в штампе времени
		uint8_t spaces = 0;
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
								// Извлекаем значение приоритета
								this->_pri = static_cast <uint16_t> (::stoi(syslog.substr(pos, i - pos)));
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
								}
							}
						} break;
						// Если производится сбор версии
						case 1: {
							// Если установлен пробел
							if(syslog.at(i) == ' '){
								// Получаем версию сообщения
								this->_ver = static_cast <uint8_t> (::stoi(syslog.substr(pos, i - pos)));
								// Запоминаем начало строки с версией
								pos = (i + 1);
								// Увеличиваем значение статуса
								status++;
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
												if(this->_fmk->is(pid, fmk_t::check_t::NUMBER))
													// Выполняем получение идентификатора процесса
													this->_pid = static_cast <pid_t> (::stoi(pid));
												// Устанавливаем идентификатор процесса по умолчанию
												else this->_pid = 0;
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
										else if(this->_fmk->is(pid, fmk_t::check_t::NUMBER))
											// Устанавливаем идентификатор процесса
											this->_pid = static_cast <pid_t> (::stoi(pid));
										// Устанавливаем идентификатор процесса по умолчанию
										else this->_pid = 0;
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
											else this->_sd.emplace(sid, unordered_map <string, string> {{std::move(key), std::move(val)}});
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
											else this->_sd.emplace(sid, unordered_map <string, string> {{std::move(key), std::move(val)}});
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
const unordered_map <string, string> & anyks::SysLog::sd(const string & id) const noexcept {
	// Результат работы функции
	static const unordered_map <string, string> result;
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
void anyks::SysLog::sd(const string & id, const unordered_map <string, string> & sd) noexcept {
	// Если идентификатор и список структурированных данных переданы
	if(!id.empty() && !sd.empty()){
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
		} else this->_sd.emplace(std::forward <const string> (id), std::forward <const unordered_map <string, string>> (sd));
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
}
/**
 * date Метод установки даты сообщения
 * @param date   дата сообщения для установки
 * @param format формат даты сообщения для установки
 */
void anyks::SysLog::date(const string & date, const string & format) noexcept {
	// Если данные даты переданы
	if(!date.empty() && !format.empty()){
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
				result.append(to_string(this->_pri));
				// Устанавливаем конец экранирования
				result.append(1, '>');
			}
			// Если штамп времени установлен
			if(this->_timestamp > 0){
				// Устанавливаем дату сообщения
				result.append(this->date("%b %d %H:%M:%S"));
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
					result.append(to_string(this->_pid));
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
				result.append(to_string(this->_pri));
				// Устанавливаем конец экранирования
				result.append(1, '>');
				// Устанавливаем версию сообщения
				result.append(to_string(this->_ver));
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
				result.append(to_string(this->_pid));
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
}
/**
 * dump Метод извлечения данных в виде JSON
 * @return json объект дампа данных
 */
json anyks::SysLog::dump() const noexcept {
	// Результат работы функции
	json result = json::object();
	// Определяем стандарт SysLog
	switch(static_cast <uint8_t> (this->_std)){
		// Если установлен стандарт RFC3164
		case static_cast <uint8_t> (std_t::RFC3164):
			// Устанавливаем стандарт сообщения
			result.emplace("RFC", 3164);
		break;
		// Если установлен стандарт RFC5424
		case static_cast <uint8_t> (std_t::RFC5424):
			// Устанавливаем стандарт сообщения
			result.emplace("RFC", 5424);
		break;
	}
	// Если штамп времени установлен
	if(this->_timestamp > 0)
		// Устанавливаем дату сообщения
		result.emplace("date", this->date());
	// Если приоритет установлен
	if(this->_pri > 0){
		// Устанавливаем категорию сообщения
		result.emplace("category", this->category());
		// Устанавливаем важность сообщения
		result.emplace("importance", this->importance());
	}
	// Если хост установлен
	if(!this->_host.empty() && (this->_host.compare("-") != 0))
		// Устанавливаем хост сообщения
		result.emplace("host", this->_host);
	// Если название приложения установлено
	if(!this->_app.empty() && (this->_app.compare("-") != 0))
		// Устанавливаем название сообщения
		result.emplace("application", this->_app);
	// Если идентификатор процесса передан
	if(this->_pid > 0)
		// Устанавливаем идентификатор процесса
		result.emplace("pid", this->_pid);
	// Если идентификатор сообщения установлен
	if(!this->_mid.empty() && (this->_mid.compare("-") != 0))
		// Устанавливаем идентификатор сообщения
		result.emplace("mid", this->_mid);
	// Если сообщение установлено
	if(!this->_message.empty() && (this->_message.compare("-") != 0))
		// Устанавливаем полученное сообщение
		result.emplace("message", this->_message);
	// Если список структурированных данных установлен
	if(!this->_sd.empty()){
		// Добавляем объект структурированных данных
		result.emplace("sd", json::object());
		// Выполняем перебор структурированных данных
		for(auto & item1 : this->_sd){
			// Устанавливаем идентификатор структурированных данных
			result.at("sd").emplace(item1.first, json::object());
			// Выполняем перебор оставшихся структур данных
			for(auto & item2 : item1.second)
				// Добавляем полученные парасетры структурированных данных
				result.at("sd").at(item1.first).emplace(item2.first, item2.second);
		}
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
	if(!dump.empty() && dump.is_object()){
		// Категория и важность сообщения
		uint8_t category = 0, importance = 0;
		// Если стандарт сообщения передан
		if(dump.contains("RFC") && dump.at("RFC").is_number()){
			// Определяем тип переданного стандарта
			switch(dump.at("RFC").get <uint16_t> ()){
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
		// Если дата передана
		if(dump.contains("date") && dump.at("date").is_string())
			// Устанавливаем дату сообщения
			this->date(dump.at("date").get <string> (), this->_format);
		// Если идентификатор процесса передан
		if(dump.contains("pid") && dump.at("pid").is_number())
			// Устанавливаем идентификатор процесса
			this->_pid = dump.at("pid").get <pid_t> ();
		// Если идентификатор сообщения передан
		if(dump.contains("mid") && dump.at("mid").is_string())
			// Устанавливаем идентификатор сообщения
			this->_mid = dump.at("mid").get <string> ();
		// Если сообщение передано
		if(dump.contains("message") && dump.at("message").is_string())
			// Устанавливаем сообщение
			this->_message = dump.at("message").get <string> ();
		// Если хост сообщения передан
		if(dump.contains("host") && dump.at("host").is_string())
			// Устанавливаем хост сообщения
			this->_host = dump.at("host").get <string> ();
		// Если название приложения передано
		if(dump.contains("application") && dump.at("application").is_string())
			// Устанавливаем название приложения
			this->_app = dump.at("application").get <string> ();
		// Если категория сообщения передана
		if(dump.contains("category") && dump.at("category").is_number())
			// Устанавливаем категорию сообщения
			category = dump.at("category").get <uint8_t> ();
		// Если важность сообщения передана
		if(dump.contains("importance") && dump.at("importance").is_number())
			// Устанавливаем важность сообщения
			importance = dump.at("importance").get <uint8_t> ();
		// Выполняем установку приоритета
		this->pri(category, importance);
		// Если список структурированных данных передан
		if(dump.contains("sd") && dump.at("sd").is_object()){
			// Выполняем очистку списка структурированных данных
			this->_sd.clear();
			// Выполняем перебор списка параметров
			for(auto & el : dump.at("sd").items()){
				// Если объект структурированных данных передан
				if(!el.value().empty() && el.value().is_object()){
					// Выполняем перебор всех параметров
					for(auto & item : el.value().items()){
						// Если параметр является строкой
						if(!item.value().empty() && item.value().is_string()){
							// Выполняем поиск объекта структурированных данных
							auto i = this->_sd.find(el.key());
							// Если объект структурированных данных уже создан
							if(i != this->_sd.end())
								// Устанавливаем новые структурированные данные
								i->second.emplace(item.key(), item.value().get <string> ());
							// Добавляем новое значение записи
							else this->_sd.emplace(el.key(), unordered_map <string, string> {{item.key(), item.value().get <string> ()}});
						}
					}
				}
			}
		}
	}
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
