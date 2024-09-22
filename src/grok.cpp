/**
 * @file: grok.cpp
 * @date: 2024-09-22
 * @license: GPL-3.0
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
#include <grok.hpp>

/**
 * reset Метод сброса параметров объекта
 */
void anyks::Grok::Var::reset() noexcept {
	// Выполняем блокировку потока
	const lock_guard <mutex> lock(this->_mtx);
	// Выполняем удаление названий переменных
	this->_names.clear();
	// Выполняем очистку шаблонов переменных
	this->_patterns.clear();
}
/**
 * count Метод получения количество добавленных переменных
 * @return количество добавленных переменных
 */
uint8_t anyks::Grok::Var::count() const noexcept {
	// Выводим количество переменных
	return static_cast <uint8_t> (this->_names.size());
}
/**
 * get Метод извлечения названия переменной которой соответствует текст
 * @param text  для получения переменной передан
 * @param index индекс запрашиваемой переменной
 * @return      название переменной, которой соответствует текст
 */
string anyks::Grok::Var::get(const string & text, const uint8_t index) noexcept {
	// Если текст для получения переменной передан
	if(!text.empty() && (index < static_cast <uint8_t> (this->_names.size()))){
		/**
		 * Выполняем обработку ошибки
		 */
		try {
			// Получаем название переменной
			const string & name = this->_names.at(static_cast <size_t> (index));
			// Получаем список подходящих нам переменных
			auto ret = this->_patterns.equal_range(name);
			// Переходим по всему списку регулярных выражений
			for(auto i = ret.first; i != ret.second; ++i){
				// Получаем строку текста для поиска
				const char * str = text.c_str();
				// Создаём объект матчинга
				unique_ptr <regmatch_t []> match(new regmatch_t [i->second.re_nsub + 1]);
				// Выполняем разбор регулярного выражения
				if(pcre2_regexec(&i->second, str, i->second.re_nsub + 1, match.get(), REG_NOTEMPTY) == 0){
					// Название полученной переменной
					string value = "";
					// Выполняем перебор всех полученных вариантов
					for(uint8_t j = 0; j < static_cast <uint8_t> (i->second.re_nsub + 1); j++){
						// Если результат получен
						if(match[j].rm_eo > match[j].rm_so){
							// Добавляем полученный результат в список результатов
							value.assign(str + match[j].rm_so, match[j].rm_eo - match[j].rm_so);
							// Если значение переменной получено
							if(!value.empty())
								// Выводим название переменной
								return name;
						}
					}	
				}
			}
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & e) {
			// Формируем полученную ошибку
			string error = e.what();
			// Добавляем описание
			error.append(". Input text [");
			// Добавляем переданный текст
			error.append(text);
			// Добавляем завершение строки
			error.append(1, ']');
			// Добавляем в список полученные ошибки
			this->_log->print("GROK: %s", log_t::flag_t::CRITICAL, error.c_str());
		}
	// Выполняем сброс параметров объекта
	} else this->reset();
	// Выводим результат
	return "";
}
/**
 * push Метод добавления переменной
 * @param name    название переменной
 * @param pattern шаблон регулярного выражения переменной
 */
void anyks::Grok::Var::push(const string & name, const string & pattern) noexcept {
	// Если название переменной и шаблон регулярного выражения переданы
	if(!name.empty() && !pattern.empty()){
		// Выполняем блокировку потока
		const lock_guard <mutex> lock(this->_mtx);
		// Добавляем название переменной
		this->_names.push_back(name);
		// Добавляем шаблон регулярного выражения
		auto ret = this->_patterns.emplace(name, regex_t());
		// Выполняем компиляцию регулярного выражения
		const int error = pcre2_regcomp(&ret->second, pattern.c_str(), REG_UTF | REG_ICASE);
		// Если возникла ошибка компиляции
		if(error != 0){
			// Создаём буфер данных для извлечения данных ошибки
			char buffer[256];
			// Выполняем заполнение нулями буфер данных
			::memset(buffer, '\0', sizeof(buffer));
			// Выполняем извлечение текста ошибки
			const size_t size = pcre2_regerror(error, &ret->second, buffer, sizeof(buffer) - 1);
			// Если текст ошибки получен
			if(size > 0){
				// Формируем полученную ошибку
				string error(buffer, size);
				// Добавляем описание
				error.append(". Input pattern [");
				// Добавляем переданный шаблон
				error.append(pattern);
				// Добавляем завершение строки
				error.append(1, ']');
				// Добавляем в список полученные ошибки
				this->_log->print("GROK: %s", log_t::flag_t::CRITICAL, error.c_str());
			}
			// Выполняем очистку шаблона
			this->_patterns.erase(name);
		}
	}
}
/**
 * init Метод инициализации шаблонов парсинга
 */
void anyks::Grok::init() noexcept {
	// Выполняем блокировку потока
	const lock_guard <mutex> lock(this->_mtx.patterns);
	// Если список шаблонов не собран
	if(this->_patterns.empty()){
		// Выполняем добавление базовых шаблонов
		this->_patterns.emplace("USERNAME", "[a-zA-Z0-9_-]+");
		this->_patterns.emplace("USER", "%{USERNAME}");
		this->_patterns.emplace("INT", "(?:[+-]?(?:[0-9]+))");
		this->_patterns.emplace("BASE10NUM", "(?<![0-9.+-])(?>[+-]?(?:(?:[0-9]+(?:\\.[0-9]+)?)|(?:\\.[0-9]+)))");
		this->_patterns.emplace("NUMBER", "(?:%{BASE10NUM})");
		this->_patterns.emplace("BASE16NUM", "(?<![0-9A-Fa-f])(?:[+-]?(?:0x)?(?:[0-9A-Fa-f]+))");
		this->_patterns.emplace("BASE16FLOAT", "\\b(?<![0-9A-Fa-f.])(?:[+-]?(?:0x)?(?:(?:[0-9A-Fa-f]+(?:\\.[0-9A-Fa-f]*)?)|(?:\\.[0-9A-Fa-f]+)))\\b");
		this->_patterns.emplace("POSINT", "\\b(?:[0-9]+)\\b");
		this->_patterns.emplace("WORD", "\\b\\w+\\b");
		this->_patterns.emplace("NOTSPACE", "\\S+");
		this->_patterns.emplace("SPACE", "\\s*");
		this->_patterns.emplace("DATA", ".*?");
		this->_patterns.emplace("GREEDYDATA", ".*");
		this->_patterns.emplace("QUOTEDSTRING", "(?:(?<!\\\\)(?:\"(?:\\\\.|[^\\\\\"])*\"|(?:'(?:\\\\.|[^\\\\'])*')|(?:`(?:\\\\.|[^\\\\`])*`)))");
		this->_patterns.emplace("MAC", "(?:%{CISCOMAC}|%{WINDOWSMAC}|%{COMMONMAC})");
		this->_patterns.emplace("CISCOMAC", "(?:(?:[A-Fa-f0-9]{4}\\.){2}[A-Fa-f0-9]{4})");
		this->_patterns.emplace("WINDOWSMAC", "(?:(?:[A-Fa-f0-9]{2}-){5}[A-Fa-f0-9]{2})");
		this->_patterns.emplace("COMMONMAC", "(?:(?:[A-Fa-f0-9]{2}:){5}[A-Fa-f0-9]{2})");
		this->_patterns.emplace("IP", "(?<![0-9])(?:(?:25[0-5]|2[0-4][0-9]|[0-1]?[0-9]{1,2})[.](?:25[0-5]|2[0-4][0-9]|[0-1]?[0-9]{1,2})[.](?:25[0-5]|2[0-4][0-9]|[0-1]?[0-9]{1,2})[.](?:25[0-5]|2[0-4][0-9]|[0-1]?[0-9]{1,2}))(?![0-9])");
		this->_patterns.emplace("HOSTNAME", "\\b(?:[0-9A-Za-z][0-9A-Za-z-]{0,62})(?:\\.(?:[0-9A-Za-z][0-9A-Za-z-]{0,62}))*(?:\\.?|\\b)");
		this->_patterns.emplace("HOST", "%{HOSTNAME}");
		this->_patterns.emplace("IPORHOST", "(?:%{HOSTNAME}|%{IP})");
		this->_patterns.emplace("HOSTPORT", "(?:%{IPORHOST=~/\\./}:%{POSINT})");
		this->_patterns.emplace("PATH", "(?:%{UNIXPATH}|%{WINPATH})");
		this->_patterns.emplace("UNIXPATH", "(?<![\\w\\\\/])(?:/(?:[\\w_%!$@:.,-]+|\\\\.)*)+");
		this->_patterns.emplace("LINUXTTY", "(?:/dev/pts/%{POSINT})");
		this->_patterns.emplace("BSDTTY", "(?:/dev/tty[pq][a-z0-9])");
		this->_patterns.emplace("TTY", "(?:%{BSDTTY}|%{LINUXTTY})");
		this->_patterns.emplace("WINPATH", "(?:[A-Za-z]+:|\\\\)(?:\\\\[^\\\\?*]*)+");
		this->_patterns.emplace("URIPROTO", "[A-Za-z]+(\\+[A-Za-z+]+)?");
		this->_patterns.emplace("URIHOST", "%{IPORHOST}(?::%{POSINT:port})?");
		this->_patterns.emplace("URIPATH", "(?:/[A-Za-z0-9$.+!*'(),~:#%_-]*)+");
		this->_patterns.emplace("URIPARAM", "\\?[A-Za-z0-9$.+!*'(),~#%&/=:;_-]*");
		this->_patterns.emplace("URIPATHPARAM", "%{URIPATH}(?:%{URIPARAM})?");
		this->_patterns.emplace("URI", "%{URIPROTO}://(?:%{USER}(?::[^@]*)?@)?(?:%{URIHOST})?(?:%{URIPATHPARAM})?");
		this->_patterns.emplace("MONTH", "\\b(?:Jan(?:uary)?|Feb(?:ruary)?|Mar(?:ch)?|Apr(?:il)?|May|Jun(?:e)?|Jul(?:y)?|Aug(?:ust)?|Sep(?:tember)?|Oct(?:ober)?|Nov(?:ember)?|Dec(?:ember)?)\\b");
		this->_patterns.emplace("MONTHNUM", "(?:0?[1-9]|1[0-2])");
		this->_patterns.emplace("MONTHDAY", "(?:3[01]|[1-2]?[0-9]|0?[1-9])");
		this->_patterns.emplace("DAY", "(?:Mon(?:day)?|Tue(?:sday)?|Wed(?:nesday)?|Thu(?:rsday)?|Fri(?:day)?|Sat(?:urday)?|Sun(?:day)?)");
		this->_patterns.emplace("YEAR", "[0-9]+");
		this->_patterns.emplace("HOUR", "(?:2[0123]|[01][0-9])");
		this->_patterns.emplace("MINUTE", "(?:[0-5][0-9])");
		this->_patterns.emplace("SECOND", "(?:(?:[0-5][0-9]|60)(?:[.,][0-9]+)?)");
		this->_patterns.emplace("TIME", "(?!<[0-9])%{HOUR}:%{MINUTE}(?::%{SECOND})(?![0-9])");
		this->_patterns.emplace("DATE_US", "%{MONTHNUM}[/-]%{MONTHDAY}[/-]%{YEAR}");
		this->_patterns.emplace("DATE_EU", "%{YEAR}[/-]%{MONTHNUM}[/-]%{MONTHDAY}");
		this->_patterns.emplace("ISO8601_TIMEZONE", "(?:Z|[+-]%{HOUR}(?::?%{MINUTE}))");
		this->_patterns.emplace("ISO8601_SECOND", "(?:%{SECOND}|60)");
		this->_patterns.emplace("TIMESTAMP_ISO8601", "%{YEAR}-%{MONTHNUM}-%{MONTHDAY}[T ]%{HOUR}:?%{MINUTE}(?::?%{SECOND})?%{ISO8601_TIMEZONE}?");
		this->_patterns.emplace("DATE", "%{DATE_US}|%{DATE_EU}");
		this->_patterns.emplace("DATESTAMP", "%{DATE}[- ]%{TIME}");
		this->_patterns.emplace("TZ", "(?:[PMCE][SD]T)");
		this->_patterns.emplace("DATESTAMP_RFC822", "%{DAY} %{MONTH} %{MONTHDAY} %{YEAR} %{TIME} %{TZ}");
		this->_patterns.emplace("DATESTAMP_OTHER", "%{DAY} %{MONTH} %{MONTHDAY} %{TIME} %{TZ} %{YEAR}");
		this->_patterns.emplace("SYSLOGTIMESTAMP", "%{MONTH} +%{MONTHDAY} %{TIME}");
		this->_patterns.emplace("PROG", "(?:[\\w._/-]+)");
		this->_patterns.emplace("SYSLOGPROG", "%{PROG:program}(?:\\[%{POSINT:pid}\\])?");
		this->_patterns.emplace("SYSLOGHOST", "%{IPORHOST}");
		this->_patterns.emplace("SYSLOGFACILITY", "<%{POSINT:facility}.%{POSINT:priority}>");
		this->_patterns.emplace("HTTPDATE", "%{MONTHDAY}/%{MONTH}/%{YEAR}:%{TIME} %{INT:ZONE}");
		this->_patterns.emplace("QS", "%{QUOTEDSTRING}");
		this->_patterns.emplace("SYSLOGBASE", "%{SYSLOGTIMESTAMP:timestamp} (?:%{SYSLOGFACILITY} )?%{SYSLOGHOST:logsource} %{SYSLOGPROG}:");
		this->_patterns.emplace("COMBINEDAPACHELOG", "%{IPORHOST:clientip} %{USER:ident} %{USER:auth} \\[%{HTTPDATE:timestamp}\\] \"%{WORD:verb} %{URIPATHPARAM:request} HTTP/%{NUMBER:httpversion}\" %{NUMBER:response} (?:%{NUMBER:bytes}|-) (?:\"(?:%{URI:referrer}|-)\"|%{QS:referrer}) %{QS:agent}");
	}
}
/**
 * clear Метод очистки параметров модуля
 */
void anyks::Grok::clear() noexcept {
	// Выполняем сброс собранных данных
	this->reset();
	// Выполняем блокировку потока
	const lock_guard <mutex> lock(this->_mtx.patterns);
	// Выполняем удаление списка ключей
	this->_keys.clear();
	// Очищаем список шаблонов
	this->_patterns.clear();
}
/**
 * reset Метод сброса собранных данных
 */
void anyks::Grok::reset() noexcept {
	// Выполняем блокировку потока
	const lock_guard <mutex> lock(this->_mtx.mapping);
	// Очищаем схему соответствий ключей
	this->_mapping.clear();
	// Выполняем сброс параметров переменной
	this->_variables.reset();
}
/**
 * clearPatterns Метод очистки списка добавленных шаблонов
 */
void anyks::Grok::clearPatterns() noexcept {
	// Если список ключей существует
	if(!this->_keys.empty()){
		// Выполняем блокировку потока
		const lock_guard <mutex> lock(this->_mtx.patterns);
		// Выполняем перебор списка ключей
		for(auto i = this->_keys.begin(); i != this->_keys.end();){
			// Выполняем удаление шаблона
			this->_patterns.erase(* i);
			// Выполняем удаление ключа
			i = this->_keys.erase(i);
		}
	}
}
/**
 * pattern Метод добавления шаблона
 * @param key название переменной
 * @param val регуляреное выражение соответствующее переменной
 */
void anyks::Grok::pattern(const string & key, const string & val) noexcept {
	// Если параметры шаблона переданы
	if(!key.empty() && !val.empty()){
		// Выполняем блокировку потока
		const lock_guard <mutex> lock(this->_mtx.patterns);
		// Выполняем добавление шаблона
		this->_keys.emplace(key);
		// Выполняем добавление шаблона
		this->_patterns.emplace(key, val);
	}
}
/**
 * generatePattern Метод генерации шаблона
 * @param key название шаблона в виде <name>
 * @param val значение шиблок (Регулярное выражение или Grok-шаблон)
 * @return    сгенерированный шаблон
 */
string anyks::Grok::generatePattern(const string & key, const string & val) noexcept {
	// Результат работы функции
	string result = "";
	// Если данные переданы верные
	if(!key.empty() && !val.empty() && (key.front() == '<') && (key.back() == '>')){
		// Получаем штамп времени в наносекундах
		chrono::nanoseconds ns = chrono::duration_cast <chrono::nanoseconds> (chrono::system_clock::now().time_since_epoch());
		// Название переменной
		string variable = "";
		// Получаем группу шаблона
		string group = key;
		// Удаляем экранирование блоков
		group.erase(0, 1).pop_back();
		// Переходим по всем символам группы
		for(auto & item : group)
			// Выполняем формирование названия переменной
			variable.append(1, ::toupper(item));
		// Добавляем разделитель
		variable.append(1, '_');
		// Добавлем текущее значение времени
		variable.append(std::to_string(ns.count()));
		// Выполняем добавление шаблона
		this->pattern(variable, val);
		// Формируем генерацию grok-шаблона
		result.append("%{");
		// Добавляем название сформированной переменной
		result.append(variable);
		// Добавляем разделитель
		result.append(1, ':');
		// Добавляем значение переменной
		result.append(group);
		// Закрываем сгенерированный шаблон
		result.append(1, '}');
	}
	// Выводим результат
	return result;
}
/**
 * build Метод сборки регулярного выражения
 * @param text текст регулярного выражения для сборки
 * @param pure флаг выполнения сборки чистого регулярного выражения
 * @param init флаг инициализации сборки
 * @param pos  начальная позиция в тексте
 * @return     идентификатор записи в кэше
 */
uint64_t anyks::Grok::build(string & text, const bool pure, const bool init, const size_t pos) const noexcept {
	// Если текст передан
	if(!text.empty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Идентификатор записи кэша
			uint64_t cid = 0;
			// Статус разделителя
			ss_t ss = ss_t::NONE;
			// Позиция переменной
			size_t begin = 0, end = 0;
			// Если установлен флаг инициализации
			if(!pure && init && this->_mode){
				// Выполняем генерацию идентификатора кэша
				cid = CityHash64(text.c_str(), text.size());
				// Выполняем поиск запись в кэше
				auto i = this->_cache.find(cid);
				// Если в кэше найдена запись
				if(i != this->_cache.end()){
					// Выполняем установку списка имён переменных
					this->_variables._names = i->second->names;
					// Выполняем установку шаблонов переменных
					this->_variables._patterns = i->second->patterns;
					// Выполняем установку регулярное выражение
					text = i->second->expression;
					// Выводим идентификатор записи в кэше
					return cid;
				}
			}
			// Если установлен флаг инициализации, корректируем наглых быдлокодеров
			if(!pure && init && this->_mode){
				// Выполняем корректировку всего правила Grok
				for(;;){
					// Получаем строку текста для поиска
					const char * str = text.c_str();
					// Создаём объект матчинга
					unique_ptr <regmatch_t []> match(new regmatch_t [this->_reg1.re_nsub + 1]);
					// Если возникла ошибка
					if(pcre2_regexec(&this->_reg1, str, this->_reg1.re_nsub + 1, match.get(), REG_NOTEMPTY) > 0)
						// Выходим из цикла корректировки
						break;
					// Если ошибок не получено
					else {
						// Выполняем перебор всех полученных вариантов
						for(uint8_t i = 1; i < static_cast <uint8_t> (this->_reg1.re_nsub + 1); i++){
							// Если результат получен
							if(match[i].rm_eo > 0){
								// Если открытая скобка найдена
								if(text.at(match[i].rm_so) == '(')
									// Выполняем замену открытой скобки
									text.replace(match[i].rm_so, 1, "(?:");
							}
						}
					}
				}
			}
			// Если установлен флаг инициализации, корректируем работу групп
			if(init && this->_mode){
				// Выполняем корректировку всего правила Grok
				for(;;){
					// Получаем строку текста для поиска
					const char * str = text.c_str();
					// Создаём объект матчинга
					unique_ptr <regmatch_t []> match(new regmatch_t [this->_reg2.re_nsub + 1]);
					// Если возникла ошибка
					if(pcre2_regexec(&this->_reg2, str, this->_reg2.re_nsub + 1, match.get(), REG_NOTEMPTY) > 0)
						// Выходим из цикла корректировки
						break;
					// Если ошибок не получено
					else {
						// Количество найденных скобок
						uint8_t brackets = 0;
						// Выполняем перебор всех полученных вариантов
						for(uint8_t i = 1; i < static_cast <uint8_t> (this->_reg2.re_nsub + 1); i++){
							// Если результат получен
							if(match[i].rm_eo > 0){
								// Выполняем сброс количества скобок
								brackets = 1;
								// Выполняем перебор всей полученной группы сообщения
								for(size_t j = static_cast <size_t> (match[i].rm_so); j < text.size(); j++){
									// Если найдена открывающаяся скобка
									if((text[j] == '(') && (text[j - 1] != '\\'))
										// Увеличиваем количество найденных скобок
										brackets++;
									// Если найдена закрывающая скобка
									else if((text[j] == ')') && (text[j - 1] != '\\'))
										// Уменьшаем количество найденных скобок
										brackets--;
									// Если найден конец выходим
									if(brackets == 0){
										// Получаем полную извлечённую строку
										const string & str = text.substr(match[i].rm_so, j - match[i].rm_so);
										// Получаем название группы
										string group = text.substr(match[i].rm_so, match[i].rm_eo - match[i].rm_so);
										// Получаем шаблон регулярного выражения
										string express = str.substr(group.length());
										// Получаем значение переменной
										const string & variable = const_cast <grok_t *> (this)->generatePattern(group, express);
										// Если переменная получена
										if(!variable.empty())
											// Заменяем в тексте полученные данные на нашу переменную
											text.replace(match[i].rm_so - 2, (j + 1) - (match[i].rm_so - 2), variable);
									}
								}
							}
						}
					}
				}
			}
			// Перебираем полученный текст
			for(size_t i = pos; i < text.length(); i++){
				// Определяем текущий статус
				switch(static_cast <uint8_t> (ss)){
					// Статус разделителя не определён
					case static_cast <uint8_t> (ss_t::NONE): {
						// Если найден символ процента
						if(text.at(i) == '%'){
							// Запоминаем начальную позицию
							begin = i;
							// Меняем статус
							ss = ss_t::FIRST;
						}
					} break;
					// Статус разделителя определён как начальный
					case static_cast <uint8_t> (ss_t::FIRST): {
						// Если найдено экранирование
						if(text.at(i) == '{')
							// Меняем статус
							ss = ss_t::SECOND;
						// Если получен какой-то другой символ, снимаем статус
						else ss = ss_t::NONE;
					} break;
					// Статус разделителя определён как конечный
					case static_cast <uint8_t> (ss_t::SECOND): {
						// Если найдено экранирование
						if(text.at(i) == '}'){
							// Устанавливаем конечную позицию
							end = (i + 1);
							// Если конец строки установлен
							if(end > begin){
								// Выполняем препарирование
								this->prepare(text, pure, begin, end);
								// Выполняем поиск оставшихся параметров
								this->build(text, pure, false, begin + 1);
								// Если шаблон не был переделан в регулярное выражение
								if(text.substr(begin, 2).compare("%{") == 0){
									// Формируем полученную ошибку
									string error = "Template is not found for ";
									// Добавляем начало строки
									error.append(1, '[');
									// Добавляем переданный текст
									error.append(text.substr(begin, end - begin));
									// Добавляем завершение строки
									error.append(1, ']');
									// Добавляем в список полученные ошибки
									this->_log->print("GROK: %s", log_t::flag_t::CRITICAL, error.c_str());
									// Выполняем очистку регулярного выражения
									text.clear();
								// Если текст регулярного выражения сформирован верно
								} else if(!text.empty() && (!pure && init && this->_mode)) {
									// Выполняем блокировку потока
									this->_mtx.cache.lock();
									// Выполняем создании записи кэша
									auto ret = const_cast <grok_t *> (this)->_cache.emplace(cid, unique_ptr <cache_t> (new cache_t));
									// Выполняем разблокировку потока
									this->_mtx.cache.unlock();
									// Выполняем установку регулярного выражения
									ret.first->second->expression = text;
									// Выполняем установку списка имён переменных
									ret.first->second->names = this->_variables._names;
									// Выполняем установку шаблонов переменных
									ret.first->second->patterns = this->_variables._patterns;
									// Выполняем компиляцию регулярного выражения
									const int error = pcre2_regcomp(&ret.first->second->reg, ret.first->second->expression.c_str(), REG_UTF | REG_ICASE);
									// Если возникла ошибка компиляции
									if(error != 0){
										// Создаём буфер данных для извлечения данных ошибки
										char buffer[256];
										// Выполняем заполнение нулями буфер данных
										::memset(buffer, '\0', sizeof(buffer));
										// Выполняем извлечение текста ошибки
										const size_t size = pcre2_regerror(error, &ret.first->second->reg, buffer, sizeof(buffer) - 1);
										// Если текст ошибки получен
										if(size > 0){
											// Формируем полученную ошибку
											string error(buffer, size);
											// Добавляем описание
											error.append(". Input pattern [");
											// Добавляем переданный шаблон
											error.append(ret.first->second->expression);
											// Добавляем завершение строки
											error.append(1, ']');
											// Добавляем в список полученные ошибки
											this->_log->print("GROK: %s", log_t::flag_t::CRITICAL, error.c_str());
										}
										// Выполняем сброс собранных данных переменных
										this->_variables.reset();
										// Выполняем перебор всех шаблонов
										for(auto & item : ret.first->second->patterns)
											// Выполняем удаление скомпилированного регулярного выражения
											pcre2_regfree(&item.second);
										// Выполняем блокировку потока
										this->_mtx.cache.lock();
										// Выполняем удаление записи из кэша
										const_cast <grok_t *> (this)->_cache.erase(cid);
										// Выполняем разблокировку потока
										this->_mtx.cache.unlock();
										// Выполняем зануление идентификатора записи
										cid = 0;
									}
								}
								// Выводим результат
								return cid;
							}
						}
					} break;
				}
			}
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & e) {
			// Формируем полученную ошибку
			string error = e.what();
			// Добавляем описание
			error.append(". Input text [");
			// Добавляем переданный текст
			error.append(text);
			// Добавляем завершение строки
			error.append(1, ']');
			// Добавляем в список полученные ошибки
			this->_log->print("GROK: %s", log_t::flag_t::CRITICAL, error.c_str());
		}
	}
	// Выводим результат
	return 0;
}
/**
 * prepare Метод обработки полученной переменной Grok
 * @param text  текст в котором найдена переменная Grok
 * @param pure  флаг выполнения сборки чистого регулярного выражения
 * @param begin начальная позиция переменной в тексте
 * @param end   конечная позиция переменной в тексте
 * @return      результат обработанного текста
 */
string & anyks::Grok::prepare(string & text, const bool pure, const size_t begin, const size_t end) const noexcept {
	// Позиция переменной в правиле
	size_t pos = 0;
	// Флаг получения переменной
	bool mode = false;
	// Создаем ключ и название переменной
	string key = "", variable = "";
	// Получаем данные правила
	string rule = text.substr(begin + 2, (end - 1) - (begin + 2));
	// Выполняем поиск переменной
	if((mode = (pos = rule.find(":")) != string::npos)){
		// Если нужно собрать полное регулярное выражение
		if(!pure)
			// Получаем ключ переменной
			key = rule.substr(pos + 1);
		// Получаем название переменной
		variable = rule.substr(0, pos);
	// Если переменная не найдена
	} else variable = std::move(rule);
	// Выполняем поиск переменной
	auto i = this->_patterns.find(variable);
	// Если переменная найдена
	if(i != this->_patterns.end()){
		// Получаем значение шаблона
		string pattern = i->second;
		// Выполняем поиск существование переменной
		if((pos = i->second.find("%{")) != string::npos)
			// Выполняем замену шаблона на регулярное выражение
			this->build(pattern, pure, false, pos);
		// Если ключ получен
		if(!pure && !key.empty())
			// Выполняем добавление переменной
			this->_variables.push(key, pattern);
		// Выполняем замену
		text.replace(begin, end - begin, (mode && !pure ? "(" : "") + pattern + (mode && !pure ? ")" : ""));
	}
	// Выводим результат
	return text;
}
/**
 * parse Метод выполнения парсинга текста
 * @param text текст для парсинга
 * @param cid  идентификатор записи в кэше
 * @return     результат выполнения регулярного выражения
 */
bool anyks::Grok::parse(const string & text, const uint64_t cid) noexcept {
	// Результат работы функции
	bool result = false;
	// Если данные текста и правил переданы
	if(!text.empty() && (cid > 0)){
		/**
		 * Выполняем обработку ошибки
		 */
		try {
			// Выполняем поиск идентификатора регулярного выражения в кэше
			auto i = this->_cache.find(cid);
			// Если идентификатор регулярного выражения в кэше найден
			if(i != this->_cache.end()){
				// Получаем строку текста для поиска
				const char * str = text.c_str();
				// Создаём объект матчинга
				unique_ptr <regmatch_t []> match(new regmatch_t [i->second->reg.re_nsub + 1]);
				// Выполняем разбор регулярного выражения
				const int error = pcre2_regexec(&i->second->reg, str, i->second->reg.re_nsub + 1, match.get(), REG_NOTEMPTY);
				// Если ошибок не получено
				if((result = (error == 0))){
					// Название полученной переменной
					string value = "";
					// Выполняем перебор всех полученных вариантов
					for(uint8_t j = 1; j < static_cast <uint8_t> (i->second->reg.re_nsub + 1); j++){
						// Если результат получен
						if(match[j].rm_eo > match[j].rm_so){
							// Добавляем полученный результат в список результатов
							value.assign(str + match[j].rm_so, match[j].rm_eo - match[j].rm_so);
							// Если значение переменной получено
							if(!value.empty()){
								// Извлекаем переменную которой соответствует текст
								const string & key = this->_variables.get(value, j - 1);
								// Если название переменной получено
								if(!key.empty()){
									// Выполняем блокировку потока
									const lock_guard <mutex> lock(this->_mtx.mapping);
									// Выполняем добавления полученных данных в схему соответствий
									this->_mapping.emplace(key, value);
								}
							}
						}
					}
				}
			}
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & e) {
			// Формируем полученную ошибку
			string error = e.what();
			// Добавляем описание
			error.append(". Input text [");
			// Добавляем переданный текст
			error.append(text);
			// Добавляем завершение строки
			error.append(1, ']');
			// Добавляем в список полученные ошибки
			this->_log->print("GROK: %s", log_t::flag_t::CRITICAL, error.c_str());
		}
	}
	// Выводим результат
	return result;
}
/**
 * parse Метод выполнения парсинга текста
 * @param text текст для парсинга
 * @param rule правило парсинга текста
 * @return     результат выполнения регулярного выражения
 */
bool anyks::Grok::parse(const string & text, const string & rule) noexcept {
	// Результат работы функции
	bool result = false;
	// Если данные текста и правил переданы
	if(!text.empty() && !rule.empty()){
		/**
		 * Выполняем обработку ошибки
		 */
		try {
			// Объект контекста регулярного выражения
			regex_t reg;
			// Выполняем компиляцию регулярного выражения
			const int error = pcre2_regcomp(&reg, rule.c_str(), REG_UTF | REG_ICASE);
			// Если возникла ошибка компиляции
			if(error != 0){
				// Создаём буфер данных для извлечения данных ошибки
				char buffer[256];
				// Выполняем заполнение нулями буфер данных
				::memset(buffer, '\0', sizeof(buffer));
				// Выполняем извлечение текста ошибки
				const size_t size = pcre2_regerror(error, &reg, buffer, sizeof(buffer) - 1);
				// Если текст ошибки получен
				if(size > 0){
					// Формируем полученную ошибку
					string error(buffer, size);
					// Добавляем описание
					error.append(". Input pattern [");
					// Добавляем переданный шаблон
					error.append(rule);
					// Добавляем завершение строки
					error.append(1, ']');
					// Добавляем входящий текст
					error.append(", Input text [");
					// Добавляем переданный текст
					error.append(text);
					// Добавляем завершение строки
					error.append(1, ']');
					// Добавляем в список полученные ошибки
					this->_log->print("GROK: %s", log_t::flag_t::CRITICAL, error.c_str());
				}
			// Если регулярное выражение удачно скомпилированно
			} else {
				// Получаем строку текста для поиска
				const char * str = text.c_str();
				// Создаём объект матчинга
				unique_ptr <regmatch_t []> match(new regmatch_t [reg.re_nsub + 1]);
				// Выполняем разбор регулярного выражения
				const int error = pcre2_regexec(&reg, str, reg.re_nsub + 1, match.get(), REG_NOTEMPTY);
				// Если ошибок не получено
				if((result = (error == 0))){
					// Название полученной переменной
					string value = "";
					// Выполняем перебор всех полученных вариантов
					for(uint8_t i = 1; i < static_cast <uint8_t> (reg.re_nsub + 1); i++){
						// Если результат получен
						if(match[i].rm_eo > match[i].rm_so){
							// Добавляем полученный результат в список результатов
							value.assign(str + match[i].rm_so, match[i].rm_eo - match[i].rm_so);
							// Если значение переменной получено
							if(!value.empty()){
								// Извлекаем переменную которой соответствует текст
								const string & key = this->_variables.get(value, i - 1);
								// Если название переменной получено
								if(!key.empty()){
									// Выполняем блокировку потока
									const lock_guard <mutex> lock(this->_mtx.mapping);
									// Выполняем добавления полученных данных в схему соответствий
									this->_mapping.emplace(key, value);
								}
							}
						}
					}
				}
				// Выполняем удаление скомпилированного регулярного выражения
				pcre2_regfree(&reg);
			}
		/**
		 * Если возникает ошибка
		 */
		} catch(const std::exception & e) {
			// Формируем полученную ошибку
			string error = e.what();
			// Добавляем описание
			error.append(". Input text [");
			// Добавляем переданный текст
			error.append(text);
			// Добавляем завершение строки
			error.append(1, ']');
			// Добавляем в список полученные ошибки
			this->_log->print("GROK: %s", log_t::flag_t::CRITICAL, error.c_str());
		}
	}
	// Выводим результат
	return result;
}
/**
 * dump Метод извлечения данных в виде JSON
 * @return json объект дампа данных
 */
json anyks::Grok::dump() const noexcept {
	// Результат работы функции
	json result = json::object();
	// Если схема соответствия ключей сформированна
	if(!this->_mapping.empty()){
		// Выполняем сборку объекта результатов
		for(auto & item : this->_mapping){
			// Если результат получен в виде числа с плавающей точкой
			if(this->_fmk->is(item.second, fmk_t::check_t::DECIMAL))
				// Выполняем конвертацию в число с плавающей точкой
				result.emplace(item.first, std::stod(item.second));
			// Если результат получен в виде обычного числа
			else if(this->_fmk->is(item.second, fmk_t::check_t::NUMBER)) {
				// Получаем переданное число
				const long long number = std::stoll(item.second);
				// Если число положительное
				if(number > 0)
					// Выполняем конвертацию в число
					result.emplace(item.first, std::stoull(item.second));
				// Выполняем конвертацию в число
				else result.emplace(item.first, number);
			// Формируем результат в формате JSON
			} else result.emplace(item.first, item.second);
		}
	}
	// Выводим результат
	return result;
}
/**
 * mapping Метод извлечения карты полученных значений
 * @return карта полученных значений 
 */
const unordered_map <string, string> & anyks::Grok::mapping() const noexcept {
	// Выводим список полученных значений
	return this->_mapping;
}
/**
 * get Метод извлечения записи по ключу
 * @param key ключ записи для извлечения
 * @return    значение записи ключа
 */
string anyks::Grok::get(const string & key) const noexcept {
	// Результат работы функции
	string result = "";
	// Если ключ записи передан
	if(!key.empty()){
		// Выполняем поиск ключа в схеме соответствий
		auto i = this->_mapping.find(key);
		// Если ключ в схеме соответствий найден
		if(i != this->_mapping.end())
			// Выводим результат
			return i->second;
	}
	// Выводим результат
	return result;
}
/**
 * Grok Конструктор
 * @param fmk объект фреймворка
 * @param log объект для работы с логами
 */
anyks::Grok::Grok(const fmk_t * fmk, const log_t * log) noexcept : _mode(false), _variables(log), _fmk(fmk), _log(log) {
	/**
	 * Выполняем сборку регулярного выражения для исправления скобок
	 */
	{
		// Выполняем компиляцию регулярного выражения
		const int error = pcre2_regcomp(&this->_reg1, "(^|[^\\\\])(\\((?!\\?))", REG_UTF | REG_ICASE);
		// Если возникла ошибка компиляции
		if(!(this->_mode = (error == 0))){
			// Создаём буфер данных для извлечения данных ошибки
			char buffer[256];
			// Выполняем заполнение нулями буфер данных
			::memset(buffer, '\0', sizeof(buffer));
			// Выполняем извлечение текста ошибки
			const size_t size = pcre2_regerror(error, &this->_reg1, buffer, sizeof(buffer) - 1);
			// Если текст ошибки получен
			if(size > 0){
				// Формируем полученную ошибку
				string error(buffer, size);
				// Добавляем в список полученные ошибки
				this->_log->print("GROK: %s", log_t::flag_t::CRITICAL, error.c_str());
			}
			// Выходим из приложения
			::exit(EXIT_FAILURE);
		}
	}
	/**
	 * Выполняем сборку регулярного выражения для формирования групп
	 */
	{
		// Выполняем компиляцию регулярного выражения
		const int error = pcre2_regcomp(&this->_reg2, "(?:\\(\\?\\s*(\\<\\w+\\>))", REG_UTF | REG_ICASE);
		// Если возникла ошибка компиляции
		if(!(this->_mode = (error == 0))){
			// Создаём буфер данных для извлечения данных ошибки
			char buffer[256];
			// Выполняем заполнение нулями буфер данных
			::memset(buffer, '\0', sizeof(buffer));
			// Выполняем извлечение текста ошибки
			const size_t size = pcre2_regerror(error, &this->_reg2, buffer, sizeof(buffer) - 1);
			// Если текст ошибки получен
			if(size > 0){
				// Формируем полученную ошибку
				string error(buffer, size);
				// Добавляем в список полученные ошибки
				this->_log->print("GROK: %s", log_t::flag_t::CRITICAL, error.c_str());
			}
			// Выходим из приложения
			::exit(EXIT_FAILURE);
		}
	}
}
/**
 * ~Grok Деструктор
 */
anyks::Grok::~Grok() noexcept {
	// Если кэш заполнен
	if(!this->_cache.empty()){
		// Выполняем перебор всего списка кэша
		for(auto & i : this->_cache){
			// Выполняем удаление скомпилированного регулярного выражения
			pcre2_regfree(&i.second->reg);
			// Выполняем перебор всех шаблонов
			for(auto & j : i.second->patterns)
				// Выполняем удаление скомпилированного регулярного выражения
				pcre2_regfree(&j.second);
		}
	}
	// Выполняем очистку памяти выделенную под регулярное выражение для исправления скобок
	pcre2_regfree(&this->_reg1);
	// Выполняем очистку памяти выделенную под регулярное выражение для формирования групп
	pcre2_regfree(&this->_reg2);
}
/**
 * Оператор вывода данные контейнера в качестве строки
 * @return данные контейнера в качестве строки
 */
anyks::Grok::operator std::string() const noexcept {
	// Выводим дамп собранных данных
	return this->dump().dump(4);
}
/**
 * Оператор [<<] вывода в поток Grok контейнера
 * @param os   поток куда нужно вывести данные
 * @param grok контенер для присвоения
 */
ostream & anyks::operator << (ostream & os, const grok_t & grok) noexcept {
	// Записываем в поток распарсенные данные с помощью Grok
	os << grok.dump().dump(4);
	// Выводим результат
	return os;
}
