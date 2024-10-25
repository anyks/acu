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
void anyks::Grok::Variable::reset() noexcept {
	// Выполняем блокировку потока
	const lock_guard <std::mutex> lock(this->_mtx);
	// Если список регулярных выражений шаблонов создан
	if(!this->_patterns.empty()){
		// Выполняем перебор всех шаблонов
		for(auto & item : this->_patterns)
			// Выполняем удаление скомпилированного регулярного выражения
			pcre2_regfree(&item.second);
	}
	// Выполняем удаление названий переменных
	this->_names.clear();
	// Выполняем очистку шаблонов переменных
	this->_patterns.clear();
}
/**
 * count Метод получения количество добавленных переменных
 * @return количество добавленных переменных
 */
uint8_t anyks::Grok::Variable::count() const noexcept {
	// Выводим количество переменных
	return static_cast <uint8_t> (this->_names.size());
}
/**
 * get Метод извлечения названия переменной которой соответствует текст
 * @param text  для получения переменной передан
 * @param index индекс запрашиваемой переменной
 * @return      название переменной, которой соответствует текст
 */
string anyks::Grok::Variable::get(const string & text, const uint8_t index) noexcept {
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
				std::unique_ptr <regmatch_t []> match(new regmatch_t [i->second.re_nsub + 1]);
				// Выполняем разбор регулярного выражения
				if(pcre2_regexec(&i->second, str, i->second.re_nsub + 1, match.get(), REG_NOTEMPTY) == 0){
					// Название полученной переменной
					string value = "";
					// Выполняем перебор всех полученных вариантов
					for(uint8_t j = 0; j < static_cast <uint8_t> (i->second.re_nsub + 1); j++){
						// Если результат получен
						if(match[j].rm_eo > match[j].rm_so){
							// Формируем полученный результат
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
void anyks::Grok::Variable::push(const string & name, const string & pattern) noexcept {
	// Если название переменной и шаблон регулярного выражения переданы
	if(!name.empty() && !pattern.empty()){
		// Выполняем блокировку потока
		const lock_guard <std::mutex> lock(this->_mtx);
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
 * clear Метод очистки параметров модуля
 */
void anyks::Grok::clear() noexcept {
	// Выполняем сброс собранных данных
	this->reset();
	// Выполняем блокировку потока
	const lock_guard <std::mutex> lock(this->_mtx.patterns);
	// Очищаем список шаблонов
	this->_patternsExternal.clear();
}
/**
 * reset Метод сброса собранных данных
 */
void anyks::Grok::reset() noexcept {
	// Выполняем блокировку потока
	const lock_guard <std::mutex> lock(this->_mtx.mapping);
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
	if(!this->_patternsExternal.empty()){
		// Выполняем блокировку потока
		const lock_guard <std::mutex> lock(this->_mtx.patterns);
		// Очищаем список шаблонов
		this->_patternsExternal.clear();
	}
}
/**
 * variable Метод извлечения первой блоковой переменной в тексте
 * @param text текст из которого следует извлечь переменные
 * @return     первая блоковая переменная
 */
anyks::Grok::let_t anyks::Grok::variable(const string & text) const noexcept {
	// Результат работы функции
	let_t result;
	// Если текст для парсинга передан
	if(!text.empty()){
		// Статус разделителя
		ss_t ss = ss_t::NONE;
		// Позиция переменной в тексте
		size_t begin = 0, end = 0;
		// Перебираем полученный текст
		for(size_t i = 0; i < text.length(); i++){
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
					// Если найдено начальное экранирование
					if(text.at(i) == '{')
						// Меняем статус
						ss = ss_t::SECOND;
					// Если получен какой-то другой символ, снимаем статус
					else ss = ss_t::NONE;
				} break;
				// Статус разделителя определён как конечный
				case static_cast <uint8_t> (ss_t::SECOND): {
					// Определяем символ
					switch(text.at(i)){
						// Если найдено конечное экранирование
						case '}': {
							// Устанавливаем конечную позицию
							end = (i + 1);
							// Если конец строки установлен
							if(end > begin){
								// Устанавливаем позицию переменной
								result.pos = (begin + 2);
								// Устанавливаем размер переменной
								result.size = ((end - 1) - (begin + 2));
								// Выводим результат
								return result;
							}
						} break;
						// Если найден разделитель переменной
						case ':':
							// Устанавливаем позицию разделителя
							result.delim = i;
						break;
					}
				} break;
			}
		}
	}
	// Выводим результат
	return result;
}
/**
 * prepare Метод обработки полученной переменной Grok
 * @param text текст в котором найдена переменная Grok
 * @param lets разрешить обработку блочных переменных
 * @return     список извлечённых переменных
 */
vector <pair <string, string>> anyks::Grok::prepare(string & text, const bool lets) const noexcept {
	// Результат работы функции
	vector <pair <string, string>> result;
	// Если текст для обработки передан
	if(!text.empty()){
		// Флаг начала извлечения всего списка переменных
		Process:
		// Выполняем получение блоковой переменной из текста
		let_t let = this->variable(text);
		// Если разделитель не найден
		if((lets || (let.delim == 0)) && (let.pos > 0) && (let.size > 0)){
			// Ключ и значение переменной
			string key = "", value = "";
			// Если разделитель найден
			if(let.delim > 0){
				// Выполняем установку ключа
				key = text.substr(let.pos, let.delim - let.pos);
				// Выполняем установку значения переменной
				value = text.substr(let.delim + 1, let.size - ((let.delim + 1) - let.pos));
			// Иначе устанавливаем ключ переменной как он есть
			} else key = text.substr(let.pos, let.size);
			// Выполняем поиск переменной среди внутренних шаблонов
			auto i = this->_patternsInternal.find(key);
			// Если переменная среди внутренних шаблонов найдена
			if(i != this->_patternsInternal.end()){
				// Выполняем копирование полученного шаблона
				string pattern = i->second;
				// Выполняем обработку нашего шаблона
				const auto & vars = this->prepare(pattern, lets);
				// Выполняем замену
				text.replace(let.pos - 2, let.size + 3, (lets ? "(" : "") + pattern + (lets ? ")" : ""));
				// Выполняем добавления переменной в список результата
				result.emplace_back(std::move(value), std::move(pattern));
				// Если мы получили список переменных из обраотанного шаблона
				if(!vars.empty())
					// Выполняем добавления полученных шаблонов в результат
					result.insert(result.end(), vars.begin(), vars.end());
				// Выполняем поиск переменных дальше
				goto Process;
			// Если переменная среди внутренних шаблонов не найдена
			} else {
				// Выполняем поиск переменной среди внешних шаблонов
				auto i = this->_patternsExternal.find(key);
				// Если переменная среди внешних шаблонов найдена
				if(i != this->_patternsExternal.end()){
					// Выполняем копирование полученного шаблона
					string pattern = i->second;
					// Выполняем обработку нашего шаблона
					const auto & vars = this->prepare(pattern, lets);
					// Выполняем замену
					text.replace(let.pos - 2, let.size + 3, (lets ? "(" : "") + pattern + (lets ? ")" : ""));
					// Выполняем добавления переменной в список результата
					result.emplace_back(std::move(value), std::move(pattern));
					// Если мы получили список переменных из обраотанного шаблона
					if(!vars.empty())
						// Выполняем добавления полученных шаблонов в результат
						result.insert(result.end(), vars.begin(), vars.end());
					// Выполняем поиск переменных дальше
					goto Process;
				}
			}
		}
	}
	// Выводим результат
	return result;
}
/**
 * patterns Метод добавления списка поддерживаемых шаблонов
 * @param patterns список поддерживаемых шаблонов
 */
void anyks::Grok::patterns(const json & patterns) noexcept {
	// Если шаблоны переданы
	if(!patterns.empty() && patterns.is_object()){
		// Объект блоковой переменной
		let_t let;
		// Список регулярных выражений в которых содержатся блоковые переменные
		std::unordered_map <string, string> items;
		// Выполняем перебор всего списка значений
		for(auto & el : patterns.items()){
			// Если значение является строкой
			if(el.value().is_string()){
				// Выполняем получение блоковой переменной из текста
				let = this->variable(el.value().get <string> ());
				// Если разделитель не найден
				if((let.delim == 0) && (let.pos > 0) && (let.size > 0))
					// Выполняем формирование списка регулярных выражений
					items.emplace(el.key(), el.value().get <string> ());
				// Выполняем добавление полученных шаблонов как они есть
				else this->pattern(el.key(), el.value().get <string> (), event_t::EXTERNAL);
			}
		}
		// Если список регулярных выражений в которых содержатся блоковые переменные собран
		if(!items.empty()){
			// Выполняем перебор всего списка регулярных выражений
			for(auto & item : items)
				// Выполняем добавление нашего шаблона
				this->pattern(item.first, item.second, event_t::EXTERNAL);
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
	if(!key.empty() && !val.empty())
		// Выполняем добавление нашего шаблона
		this->pattern(key, val, event_t::EXTERNAL);
}
/**
 * pattern Метод добавления шаблона
 * @param key   название переменной
 * @param val   регуляреное выражение соответствующее переменной
 * @param event тип выполняемого события
 */
void anyks::Grok::pattern(const string & key, const string & val, const event_t event) noexcept {
	// Если параметры шаблона переданы
	if(!key.empty() && !val.empty()){
		// Выполняем блокировку потока
		const lock_guard <std::mutex> lock(this->_mtx.patterns);
		// Выполняем копирование текста регулярного выражения
		string text = val;
		// Выполняем корректировку всего правила Grok
		for(;;){
			// Получаем строку текста для поиска
			const char * str = text.c_str();
			// Создаём объект матчинга
			std::unique_ptr <regmatch_t []> match(new regmatch_t [this->_reg1.re_nsub + 1]);
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
		// Выполняем обработку полученных шаблонов
		this->prepare(text, false);
		// Если текст регулярного выражения получен
		if(!text.empty()){
			// Определяем тип события
			switch(static_cast <uint8_t> (event)){
				// Если событие является внутренним
				case static_cast <uint8_t> (event_t::INTERNAL):
					// Выполняем добавление шаблона
					this->_patternsInternal.emplace(key, text);
				break;
				// Если событие является внешним
				case static_cast <uint8_t> (event_t::EXTERNAL):
					// Выполняем добавление шаблона
					this->_patternsExternal.emplace(key, text);
				break;
			}
		}
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
		variable.append(std::to_string(this->_fmk->timestamp(fmk_t::stamp_t::NANOSECONDS)));
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
				std::unique_ptr <regmatch_t []> match(new regmatch_t [i->second->reg.re_nsub + 1]);
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
									const lock_guard <std::mutex> lock(this->_mtx.mapping);
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
				std::unique_ptr <regmatch_t []> match(new regmatch_t [reg.re_nsub + 1]);
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
									const lock_guard <std::mutex> lock(this->_mtx.mapping);
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
 * build Метод сборки регулярного выражения
 * @param text текст регулярного выражения для сборки
 * @param pure флаг выполнения сборки чистого регулярного выражения
 * @return     идентификатор записи в кэше
 */
uint64_t anyks::Grok::build(string & text, const bool pure) const noexcept {
	// Результат работы функции
	uint64_t result = 0;
	// Если текст передан
	if(!text.empty() && this->_mode){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем генерацию идентификатора кэша
			result = CityHash64(text.c_str(), text.size());
			// Выполняем поиск запись в кэше
			auto i = this->_cache.find(result);
			// Если в кэше найдена запись
			if(i != this->_cache.end()){
				// Выполняем установку регулярное выражение
				text = i->second->expression;
				// Выводим идентификатор записи в кэше
				return result;
			}
			// Если нам не нужно собирать чистое регулярное выражение
			if(!pure){
				// Выполняем корректировку всего правила Grok
				for(;;){
					// Получаем строку текста для поиска
					const char * str = text.c_str();
					// Создаём объект матчинга
					std::unique_ptr <regmatch_t []> match(new regmatch_t [this->_reg1.re_nsub + 1]);
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
			// Выполняем корректировку всего правила Grok
			for(;;){
				// Получаем строку текста для поиска
				const char * str = text.c_str();
				// Создаём объект матчинга
				std::unique_ptr <regmatch_t []> match(new regmatch_t [this->_reg2.re_nsub + 1]);
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
			// Выполняем обработку полученных шаблонов
			const auto & vars = this->prepare(text);
			// Если список переменных получен
			if(!vars.empty()){
				// Выполняем перебор списка переменных
				for(auto & var : vars)
					// Выполняем добавление переменной
					this->_variables.push(var.first, var.second);
			}
			// Если текст регулярного выражения сформирован верно
			if(!pure && !text.empty()){
				// Выполняем блокировку потока
				this->_mtx.cache.lock();
				// Выполняем создании записи кэша
				auto ret = const_cast <grok_t *> (this)->_cache.emplace(result, std::unique_ptr <cache_t> (new cache_t));
				// Выполняем разблокировку потока
				this->_mtx.cache.unlock();
				// Выполняем установку регулярного выражения
				ret.first->second->expression = text;
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
					// Выполняем блокировку потока
					this->_mtx.cache.lock();
					// Выполняем удаление записи из кэша
					const_cast <grok_t *> (this)->_cache.erase(result);
					// Выполняем разблокировку потока
					this->_mtx.cache.unlock();
					// Выполняем зануление идентификатора записи
					result = 0;
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
			// Если результат получен в виде обычного числа
			if(this->_fmk->is(item.second, fmk_t::check_t::NUMBER)){
				// Получаем переданное число
				const long long number = std::stoll(item.second);
				// Если число положительное
				if(number > 0)
					// Выполняем конвертацию в число
					result.emplace(item.first, std::stoull(item.second));
				// Выполняем конвертацию в число
				else result.emplace(item.first, number);
			// Если результат получен в виде числа с плавающей точкой
			} else if(this->_fmk->is(item.second, fmk_t::check_t::DECIMAL))
				// Выполняем конвертацию в число с плавающей точкой
				result.emplace(item.first, std::stod(item.second));
			// Формируем результат в формате JSON
			else result.emplace(item.first, item.second);
		}
	}
	// Выводим результат
	return result;
}
/**
 * mapping Метод извлечения карты полученных значений
 * @return карта полученных значений 
 */
const std::unordered_map <string, string> & anyks::Grok::mapping() const noexcept {
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
	// Выполняем добавление базовых шаблонов
	this->pattern("USERNAME", "[a-zA-Z0-9._-]+", event_t::INTERNAL);
	this->pattern("USER", "%{USERNAME}", event_t::INTERNAL);
	this->pattern("INT", "(?:[+-]?(?:[0-9]+))", event_t::INTERNAL);
	this->pattern("NONNEGINT", "\\b(?:[0-9]+)\\b", event_t::INTERNAL);
	this->pattern("BASE10NUM", "(?<![0-9.+-])(?>[+-]?(?:(?:[0-9]+(?:\\.[0-9]+)?)|(?:\\.[0-9]+)))", event_t::INTERNAL);
	this->pattern("NUMBER", "(?:%{BASE10NUM})", event_t::INTERNAL);
	this->pattern("BASE16NUM", "(?<![0-9A-Fa-f])(?:[+-]?(?:0x)?(?:[0-9A-Fa-f]+))", event_t::INTERNAL);
	this->pattern("BASE16FLOAT", "\\b(?<![0-9A-Fa-f.])(?:[+-]?(?:0x)?(?:(?:[0-9A-Fa-f]+(?:\\.[0-9A-Fa-f]*)?)|(?:\\.[0-9A-Fa-f]+)))\\b", event_t::INTERNAL);
	this->pattern("POSINT", "\\b(?:[1-9][0-9]*)\\b", event_t::INTERNAL);
	this->pattern("WORD", "\\b\\w+\\b", event_t::INTERNAL);
	this->pattern("NOTSPACE", "\\S+", event_t::INTERNAL);
	this->pattern("SPACE", "\\s*", event_t::INTERNAL);
	this->pattern("DATA", ".*?", event_t::INTERNAL);
	this->pattern("GREEDYDATA", ".*", event_t::INTERNAL);
	this->pattern("UUID", "[A-Fa-f0-9]{8}-(?:[A-Fa-f0-9]{4}-){3}[A-Fa-f0-9]{12}", event_t::INTERNAL);
	this->pattern("QUOTEDSTRING", "(?>(?<!\\\\)(?>\"(?>\\\\.|[^\\\\\"]+)+\"|\"\"|(?>'(?>\\\\.|[^\\\\']+)+')|''|(?>`(?>\\\\.|[^\\\\`]+)+`)|``))", event_t::INTERNAL);
	this->pattern("CISCOMAC", "(?:(?:[A-Fa-f0-9]{4}\\.){2}[A-Fa-f0-9]{4})", event_t::INTERNAL);
	this->pattern("WINDOWSMAC", "(?:(?:[A-Fa-f0-9]{2}-){5}[A-Fa-f0-9]{2})", event_t::INTERNAL);
	this->pattern("COMMONMAC", "(?:(?:[A-Fa-f0-9]{2}:){5}[A-Fa-f0-9]{2})", event_t::INTERNAL);
	this->pattern("MAC", "(?:%{CISCOMAC}|%{WINDOWSMAC}|%{COMMONMAC})", event_t::INTERNAL);
	this->pattern("IPV4", "(?<![0-9])(?:(?:25[0-5]|2[0-4][0-9]|[0-1]?[0-9]{1,2})[.](?:25[0-5]|2[0-4][0-9]|[0-1]?[0-9]{1,2})[.](?:25[0-5]|2[0-4][0-9]|[0-1]?[0-9]{1,2})[.](?:25[0-5]|2[0-4][0-9]|[0-1]?[0-9]{1,2}))(?![0-9])", event_t::INTERNAL);
	this->pattern("IPV6", "((([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}|((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){5}(((:[0-9A-Fa-f]{1,4}){1,2})|:((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){4}(((:[0-9A-Fa-f]{1,4}){1,3})|((:[0-9A-Fa-f]{1,4})?:((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){3}(((:[0-9A-Fa-f]{1,4}){1,4})|((:[0-9A-Fa-f]{1,4}){0,2}:((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){2}(((:[0-9A-Fa-f]{1,4}){1,5})|((:[0-9A-Fa-f]{1,4}){0,3}:((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){1}(((:[0-9A-Fa-f]{1,4}){1,6})|((:[0-9A-Fa-f]{1,4}){0,4}:((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3}))|:))|(:(((:[0-9A-Fa-f]{1,4}){1,7})|((:[0-9A-Fa-f]{1,4}){0,5}:((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3}))|:)))(%.+)?", event_t::INTERNAL);
	this->pattern("IP", "(?:%{IPV6}|%{IPV4})", event_t::INTERNAL);
	this->pattern("HOSTNAME", "\\b(?:[0-9A-Za-z][0-9A-Za-z-]{0,62})(?:\\.(?:[0-9A-Za-z][0-9A-Za-z-]{0,62}))*(\\.?|\\b)", event_t::INTERNAL);
	this->pattern("HOST", "%{HOSTNAME}", event_t::INTERNAL);
	this->pattern("IPORHOST", "(?:%{HOSTNAME}|%{IP})", event_t::INTERNAL);
	this->pattern("HOSTPORT", "%{IPORHOST}:%{POSINT}", event_t::INTERNAL);
	this->pattern("UNIXPATH", "(?>/(?>[\\w_%!$@:.,~-]+|\\\\.)*)+", event_t::INTERNAL);
	this->pattern("WINPATH", "((?>[A-Za-z]+:|\\\\)(?:\\\\[^\\\\?*]*)+", event_t::INTERNAL);
	this->pattern("PATH", "(?:%{UNIXPATH}|%{WINPATH})", event_t::INTERNAL);
	this->pattern("LINUXTTY", "(?:/dev/pts/%{POSINT})", event_t::INTERNAL);
	this->pattern("BSDTTY", "(?:/dev/tty[pq][a-z0-9])", event_t::INTERNAL);
	this->pattern("TTY", "(?:/dev/(pts|tty([pq])?)(\\w+)?/?(?:[0-9]+))", event_t::INTERNAL);
	this->pattern("URIPROTO", "[A-Za-z]+(\\+[A-Za-z+]+)?", event_t::INTERNAL);
	this->pattern("URIHOST", "%{IPORHOST}(?::%{POSINT:port})?", event_t::INTERNAL);
	this->pattern("URIPATH", "(?:/[A-Za-z0-9$.+!*'(){},~:;=@#%_\\-]*)+", event_t::INTERNAL);
	this->pattern("URIPARAM", "\\?[A-Za-z0-9$.+!*'|(){},~@#%&/=:;_?\\-\\[\\]]*", event_t::INTERNAL);
	this->pattern("#URIPARAM", "\\?(?:[A-Za-z0-9]+(?:=(?:[^&]*))?(?:&(?:[A-Za-z0-9]+", event_t::INTERNAL);
	this->pattern("URIPATHPARAM", "%{URIPATH}(?:%{URIPARAM})?", event_t::INTERNAL);
	this->pattern("URI", "%{URIPROTO}://(?:%{USER}(?::[^@]*)?@)?(?:%{URIHOST})?(?:%{URIPATHPARAM})?", event_t::INTERNAL);
	this->pattern("MONTH", "\\b(?:Jan(?:uary)?|Feb(?:ruary)?|Mar(?:ch)?|Apr(?:il)?|May|Jun(?:e)?|Jul(?:y)?|Aug(?:ust)?|Sep(?:tember)?|Oct(?:ober)?|Nov(?:ember)?|Dec(?:ember)?)\\b", event_t::INTERNAL);
	this->pattern("MONTHNUM", "(?:0?[1-9]|1[0-2])", event_t::INTERNAL);
	this->pattern("MONTHNUM2", "(?:0[1-9]|1[0-2])", event_t::INTERNAL);
	this->pattern("MONTHDAY", "(?:(?:0[1-9])|(?:[12][0-9])|(?:3[01])|[1-9])", event_t::INTERNAL);
	this->pattern("DAY", "(?:Mon(?:day)?|Tue(?:sday)?|Wed(?:nesday)?|Thu(?:rsday)?|Fri(?:day)?|Sat(?:urday)?|Sun(?:day)?)", event_t::INTERNAL);
	this->pattern("YEAR", "(?>\\d\\d){1,2}", event_t::INTERNAL);
	this->pattern("HOUR", "(?:2[0123]|[01]?[0-9])", event_t::INTERNAL);
	this->pattern("MINUTE", "(?:[0-5][0-9])", event_t::INTERNAL);
	this->pattern("SECOND", "(?:(?:[0-5]?[0-9]|60)(?:[:.,][0-9]+)?)", event_t::INTERNAL);
	this->pattern("TIME", "(?!<[0-9])%{HOUR}:%{MINUTE}(?::%{SECOND})(?![0-9])", event_t::INTERNAL);
	this->pattern("DATE_US", "%{MONTHNUM}[/-]%{MONTHDAY}[/-]%{YEAR}", event_t::INTERNAL);
	this->pattern("DATE_EU", "%{MONTHDAY}[./-]%{MONTHNUM}[./-]%{YEAR}", event_t::INTERNAL);
	this->pattern("ISO8601_TIMEZONE", "(?:Z|[+-]%{HOUR}(?::?%{MINUTE}))", event_t::INTERNAL);
	this->pattern("ISO8601_SECOND", "(?:%{SECOND}|60)", event_t::INTERNAL);
	this->pattern("TIMESTAMP_ISO8601", "%{YEAR}-%{MONTHNUM}-%{MONTHDAY}[T ]%{HOUR}:?%{MINUTE}(?::?%{SECOND})?%{ISO8601_TIMEZONE}?", event_t::INTERNAL);
	this->pattern("DATE", "%{DATE_US}|%{DATE_EU}", event_t::INTERNAL);
	this->pattern("DATESTAMP", "%{DATE}[- ]%{TIME}", event_t::INTERNAL);
	this->pattern("TZ", "(?:[PMCE][SD]T|UTC)", event_t::INTERNAL);
	this->pattern("DATESTAMP_RFC822", "%{DAY} %{MONTH} %{MONTHDAY} %{YEAR} %{TIME} %{TZ}", event_t::INTERNAL);
	this->pattern("DATESTAMP_RFC2822", "%{DAY}, %{MONTHDAY} %{MONTH} %{YEAR} %{TIME} %{ISO8601_TIMEZONE}", event_t::INTERNAL);
	this->pattern("DATESTAMP_OTHER", "%{DAY} %{MONTH} %{MONTHDAY} %{TIME} %{TZ} %{YEAR}", event_t::INTERNAL);
	this->pattern("DATESTAMP_EVENTLOG", "%{YEAR}%{MONTHNUM2}%{MONTHDAY}%{HOUR}%{MINUTE}%{SECOND}", event_t::INTERNAL);
	this->pattern("SYSLOGTIMESTAMP", "%{MONTH} +%{MONTHDAY} %{TIME}", event_t::INTERNAL);
	this->pattern("PROG", "(?:[\\w._/%-]+)", event_t::INTERNAL);
	this->pattern("SYSLOGPROG", "%{PROG:program}(?:\\[%{POSINT:pid}\\])?", event_t::INTERNAL);
	this->pattern("SYSLOGHOST", "%{IPORHOST}", event_t::INTERNAL);
	this->pattern("SYSLOGFACILITY", "<%{NONNEGINT:facility}.%{NONNEGINT:priority}>", event_t::INTERNAL);
	this->pattern("HTTPDATE", "%{MONTHDAY}/%{MONTH}/%{YEAR}:%{TIME} %{INT}", event_t::INTERNAL);
	this->pattern("QS", "%{QUOTEDSTRING}", event_t::INTERNAL);
	this->pattern("JAVACLASS", "(?:[a-zA-Z$_][a-zA-Z$_0-9]*\\.)*[a-zA-Z$_][a-zA-Z$_0-9]*", event_t::INTERNAL);
	this->pattern("JAVAFILE", "(?:[A-Za-z0-9_. -]+)", event_t::INTERNAL);
	this->pattern("JAVAMETHOD", "(?:(<init>)|[a-zA-Z$_][a-zA-Z$_0-9]*)", event_t::INTERNAL);
	this->pattern("JAVASTACKTRACEPART", "%{SPACE}at %{JAVACLASS:class}\\.%{JAVAMETHOD:method}\\(%{JAVAFILE:file}(?::%{NUMBER:line})?\\)", event_t::INTERNAL);
	this->pattern("SYSLOGBASE", "%{SYSLOGTIMESTAMP:timestamp} (?:%{SYSLOGFACILITY} )?%{SYSLOGHOST:logsource} %{SYSLOGPROG}:", event_t::INTERNAL);
	this->pattern("COMMONAPACHELOG", "%{IPORHOST:clientip} %{USER:ident} %{USER:auth} \\[%{HTTPDATE:timestamp}\\] \"(?:%{WORD:verb} %{NOTSPACE:request}(?: HTTP/%{NUMBER:httpversion})?|%{DATA:rawrequest})\" %{NUMBER:response} (?:%{NUMBER:bytes}|-)", event_t::INTERNAL);
	this->pattern("COMBINEDAPACHELOG", "%{COMMONAPACHELOG} %{QS:referrer} %{QS:agent}", event_t::INTERNAL);
	this->pattern("LOGLEVEL", "([Aa]lert|ALERT|[Tt]race|TRACE|[Dd]ebug|DEBUG|[Nn]otice|NOTICE|[Ii]nfo|INFO|[Ww]arn?(?:ing)?|WARN?(?:ING)?|[Ee]rr?(?:or)?|ERR?(?:OR)?|[Cc]rit?(?:ical)?|CRIT?(?:ICAL)?|[Ff]atal|FATAL|[Ss]evere|SEVERE|EMERG(?:ENCY)?|[Ee]merg(?:ency)?)", event_t::INTERNAL);
}
/**
 * ~Grok Деструктор
 */
anyks::Grok::~Grok() noexcept {
	// Выполняем сброс собранных данных переменных
	this->_variables.reset();
	// Если кэш заполнен
	if(!this->_cache.empty()){
		// Выполняем перебор всего списка кэша
		for(auto & i : this->_cache)
			// Выполняем удаление скомпилированного регулярного выражения
			pcre2_regfree(&i.second->reg);
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
