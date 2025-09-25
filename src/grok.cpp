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
 * @copyright: Copyright © 2025
 */

/**
 * Подключаем заголовочный файл
 */
#include <grok.hpp>

/**
 * Подписываемся на пространство имён awh
 */
using namespace awh;
/**
 * Подписываемся на пространство имён rapidjson
 */
using namespace rapidjson;

/**
 * @brief Функция проверки на экранирование строки
 *
 * @param text текст для проверки экранирования
 * @param pos  позиция в тексте символа скобки
 * @return     результат проверки
 */
static bool isEscaped(const string & text, size_t pos) noexcept {
	// Количество найденных экранирований
	size_t count = 0;
	/**
	 *  Выполняем поиск количество экранирований
	 */
	while((pos >= 0) && (text.at(--pos) == '\\'))
		// Увеличиваем найденное количество экранирований
		count++;
	// Если количество экранирований чётное
	return ((count % 2) != 0);
}
/**
 * @brief Метод сброса параметров объекта
 *
 */
void anyks::Grok::Variables::reset() noexcept {
	/**
	 * Выполняем отлов ошибок
	 */
	try {
		// Выполняем блокировку потока
		const lock_guard <std::mutex> lock(this->_mtx);
		// Если список регулярных выражений шаблонов создан
		if(!this->_patterns.empty()){
			// Выполняем перебор всех шаблонов
			for(auto & item : this->_patterns)
				// Выполняем удаление скомпилированного регулярного выражения
				::pcre2_regfree(&item.second);
		}
		// Выполняем удаление названий переменных
		this->_names.clear();
		// Выполняем очистку шаблонов переменных
		this->_patterns.clear();
		// Выполняем очистку названий переменных
		vector <string> ().swap(this->_names);
		// Выполняем освобождение памяти списка шаблонов переменных
		std::unordered_multimap <string, regex_t> ().swap(this->_patterns);
	/**
	 * Если возникает ошибка
	 */
	} catch(const exception & error) {
		// Выводим сообщение об ошибке
		::fprintf(stderr, "\"Grok:Variables:reset\": %s\n", error.what());
	}
}
/**
 * @brief Метод получения количество добавленных переменных
 *
 * @return количество добавленных переменных
 */
uint8_t anyks::Grok::Variables::count() const noexcept {
	// Выводим количество переменных
	return static_cast <uint8_t> (this->_names.size());
}
/**
 * @brief Метод извлечения названия переменной которой соответствует текст
 *
 * @param text  для получения переменной передан
 * @param index индекс запрашиваемой переменной
 * @return      название переменной, которой соответствует текст
 */
string anyks::Grok::Variables::get(const string & text, const uint8_t index) noexcept {
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
				regmatch_t match[i->second.re_nsub + 1];
				// Выполняем разбор регулярного выражения
				if(::pcre2_regexec(&i->second, str, i->second.re_nsub + 1, match, REG_NOTEMPTY) == 0){
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
		} catch(const exception & error) {
			// Формируем полученную ошибку
			string message = error.what();
			// Добавляем описание
			message.append(". Input text [");
			// Добавляем переданный текст
			message.append(text);
			// Добавляем завершение строки
			message.append(1, ']');
			// Выводим сообщение об ошибке
			::fprintf(stderr, "\"Grok:Variables:get\": %s\n", message.c_str());
		}
	}
	// Выводим результат
	return "";
}
/**
 * @brief Метод добавления переменной
 *
 * @param name    название переменной
 * @param pattern шаблон регулярного выражения переменной
 */
void anyks::Grok::Variables::push(const string & name, const string & pattern) noexcept {
	// Если название переменной и шаблон регулярного выражения переданы
	if(!name.empty() && !pattern.empty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем блокировку потока
			const lock_guard <std::mutex> lock(this->_mtx);
			// Добавляем название переменной
			this->_names.push_back(name);
			// Добавляем шаблон регулярного выражения
			auto ret = this->_patterns.emplace(name, regex_t());
			// Выполняем компиляцию регулярного выражения
			const int error = ::pcre2_regcomp(&ret->second, pattern.c_str(), REG_UTF | REG_ICASE);
			// Если возникла ошибка компиляции
			if(error != 0){
				// Создаём буфер данных для извлечения данных ошибки
				char buffer[256];
				// Выполняем заполнение нулями буфер данных
				::memset(buffer, '\0', sizeof(buffer));
				// Выполняем извлечение текста ошибки
				const size_t size = ::pcre2_regerror(error, &ret->second, buffer, sizeof(buffer) - 1);
				// Если текст ошибки получен
				if(size > 0){
					// Формируем полученную ошибку
					string error(buffer, size);
					// Добавляем описание
					error.append(". Input name [");
					// Добавляем переданный название переменной
					error.append(name);
					// Добавляем завершение строки
					error.append(1, ']');
					// Добавляем разделитель
					error.append(1, ' ');
					// Добавляем название шаблона
					error.append("pattern [");
					// Добавляем переданный название шаблона
					error.append(pattern);
					// Добавляем завершение строки
					error.append(1, ']');
					// Выводим сообщение об ошибке
					::fprintf(stderr, "\"Grok:Variables:push\": %s\n", error.c_str());
				}
				// Выполняем очистку шаблона
				this->_patterns.erase(name);
			}
		/**
		 * Если возникает ошибка
		 */
		} catch(const exception & error) {
			// Формируем полученную ошибку
			string message = error.what();
			// Добавляем описание
			message.append(". Input name [");
			// Добавляем переданный название переменной
			message.append(name);
			// Добавляем завершение строки
			message.append(1, ']');
			// Добавляем разделитель
			message.append(1, ' ');
			// Добавляем название шаблона
			message.append("pattern [");
			// Добавляем переданный название шаблона
			message.append(pattern);
			// Добавляем завершение строки
			message.append(1, ']');
			// Выводим сообщение об ошибке
			::fprintf(stderr, "\"Grok:Variables:push\": %s\n", message.c_str());
		}
	}
}
/**
 * @brief деструктор
 *
 */
anyks::Grok::Variables::~Variables() noexcept {
	/**
	 * Выполняем отлов ошибок
	 */
	try {
		// Если список регулярных выражений шаблонов создан
		if(!this->_patterns.empty()){
			// Выполняем перебор всех шаблонов
			for(auto & item : this->_patterns)
				// Выполняем удаление скомпилированного регулярного выражения
				::pcre2_regfree(&item.second);
		}
	/**
	 * Если возникает ошибка
	 */
	} catch(const exception & error) {
		// Выводим сообщение об ошибке
		::fprintf(stderr, "\"Grok:Variables:~\": %s\n", error.what());
	}
}
/**
 * @brief Метод очистки параметров модуля
 *
 */
void anyks::Grok::clear() noexcept {
	/**
	 * Выполняем отлов ошибок
	 */
	try {
		// Выполняем удаление кэша
		this->_cache.clear();
		// Выполняем очистку внешних шаблонов
		this->clearPatterns();
		// Выполняем полную очистку памяти кэша
		std::map <uint64_t, std::unique_ptr <cache_t>> ().swap(this->_cache);
	/**
	 * Если возникает ошибка
	 */
	} catch(const exception & error) {
		/**
		 * Если включён режим отладки
		 */
		#if DEBUG_MODE
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
 * @brief Метод сброса собранных данных
 *
 * @param cid идентификатор записи в кэше
 */
void anyks::Grok::reset(const uint64_t cid) noexcept {
	/**
	 * Выполняем отлов ошибок
	 */
	try {
		// Выполняем блокировку потока
		const lock_guard <std::mutex> lock(this->_mtx.mapping);
		// Выполняем поиск указанного идентификатора в кэше
		auto i = this->_cache.find(cid);
		// Если в кэше найден идентификатор
		if(i != this->_cache.end()){
			// Очищаем схему соответствий ключей
			i->second->mapping.clear();
			// Выполняем очистку памяти ключей соответствий
			std::unordered_map <string, string> ().swap(i->second->mapping);
		}
	/**
	 * Если возникает ошибка
	 */
	} catch(const exception & error) {
		/**
		 * Если включён режим отладки
		 */
		#if DEBUG_MODE
			// Выводим сообщение об ошибке
			this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(cid), log_t::flag_t::CRITICAL, error.what());
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
 * @brief Метод очистки списка добавленных шаблонов
 *
 */
void anyks::Grok::clearPatterns() noexcept {
	// Если список ключей существует
	if(!this->_patternsExternal.empty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем блокировку потока
			const lock_guard <std::mutex> lock(this->_mtx.patterns);
			// Очищаем список именованных групп
			this->_nameGroups.clear();
			// Очищаем список внешних шаблонов
			this->_patternsExternal.clear();
			// Выполняем освобождение памяти именованных групп
			std::map <uint64_t, string> ().swap(this->_nameGroups);
			// Выполняем освобождение памяти списка внешних шаблонов
			std::unordered_map <string, string> ().swap(this->_patternsExternal);
		/**
		 * Если возникает ошибка
		 */
		} catch(const exception & error) {
			/**
			 * Если включён режим отладки
			 */
			#if DEBUG_MODE
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
 * @brief Метод удаления добавленного шаблона
 *
 * @param name название шаблона для удаления
 */
void anyks::Grok::removePattern(const string & name) noexcept {
	// Если список ключей существует
	if(!name.empty() && !this->_patternsExternal.empty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем блокировку потока
			const lock_guard <std::mutex> lock(this->_mtx.patterns);
			// Выполняем поиск указанного шаблона
			auto i = this->_patternsExternal.find(name);
			// Если шаблон найден
			if(i != this->_patternsExternal.end())
				// Выполняем удаление шаблона
				this->_patternsExternal.erase(i);
		/**
		 * Если возникает ошибка
		 */
		} catch(const exception & error) {
			/**
			 * Если включён режим отладки
			 */
			#if DEBUG_MODE
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(name), log_t::flag_t::CRITICAL, error.what());
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
 * @brief Метод извлечения именованных групп
 *
 * @param text текст для извлечения именованных групп
 */
void anyks::Grok::namedGroups(string & text) const noexcept {
	// Если текст для обработки передан
	if(!text.empty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем перебор всех именованных групп
			for(;;){
				// Получаем данные именованной группы
				const auto & result = this->namedGroup(text, 0);
				// Если результат получен
				if((result.first > -1) && (result.second > -1) && (result.second > result.first)){
					// Название группы и регулярное выражение
					string group = "", express = "";
					// Начало и конец названия группы
					size_t beginGroup = 0, endGrounp = 0;
					// Выполняем перебор всех полученных символов
					for(size_t i = (result.first + 2); i < (result.second - 1); i++){
						/**
						 * Определяем текущий символ
						 */
						switch(text.at(i)){
							// Если символом является символ начального экранирования названия группы
							case '<':
								// Выполняем сброс конца экранирования названия группы
								endGrounp = 0;
								// Устанавливаем значения начала экранирования группы
								beginGroup = i;
							break;
							// Если символом является символ конечного экранирования названия группы
							case '>': {
								// Устанавливаем значения конца экранирования группы
								endGrounp = (i + 1);
								// Завершаем работу цикла
								i = (result.second - 1);
							} break;
						}
					}
					// Получаем название группы
					group = text.substr(beginGroup, endGrounp - beginGroup);
					// Получаем шаблон регулярного выражения
					express = text.substr(endGrounp, (result.second - 1) - endGrounp);
					// Получаем значение переменной
					const string & var = const_cast <grok_t *> (this)->generatePattern(group, express);
					// Если переменная получена
					if(!var.empty())
						// Заменяем в тексте полученные данные на нашу переменную
						text.replace(result.first, result.second - result.first, var);
				// Выходим из цикла
				} else break;
			}
		/**
		 * Если возникает ошибка
		 */
		} catch(const exception & error) {
			/**
			 * Если включён режим отладки
			 */
			#if DEBUG_MODE
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
}
/**
 * @brief Метод извлечения первой блоковой переменной в тексте
 *
 * @param text текст из которого следует извлечь переменные
 * @return     первая блоковая переменная
 */
anyks::Grok::let_t anyks::Grok::variable(const string & text) const noexcept {
	// Результат работы функции
	let_t result;
	// Если текст для парсинга передан
	if(!text.empty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Статус разделителя
			ss_t ss = ss_t::NONE;
			// Позиция переменной в тексте
			size_t begin = 0, end = 0;
			// Перебираем полученный текст
			for(size_t i = 0; i < text.length(); i++){
				/**
				 * Определяем текущий статус
				 */
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
						/**
						 * Определяем символ
						 */
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
		/**
		 * Если возникает ошибка
		 */
		} catch(const exception & error) {
			/**
			 * Если включён режим отладки
			 */
			#if DEBUG_MODE
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
 * @brief Метод удаления скобок
 *
 * @param text текст в котором следует удалить скобки
 */
void anyks::Grok::removeBrackets(string & text) const noexcept {
	// Если текст для обработки передан
	if(!text.empty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Позиция скобки в тексте
			ssize_t pos = -1;
			// Флаг начала извлечения всего списка переменных
			Process:
			// Выполняем поиск скобки в тексте
			pos = this->bracket(text, static_cast <size_t> (pos + 1));
			// Если позиция скобки в тексте найдена
			if(pos > -1){
				// Выполняем замену в тексте скобки
				text.replace(static_cast <size_t> (pos), 1, "(?:");
				// Увеличиваем текущую позицию
				pos += 2;
				// Выполняем поиск скобок дальше
				goto Process;
			}
		/**
		 * Если возникает ошибка
		 */
		} catch(const exception & error) {
			/**
			 * Если включён режим отладки
			 */
			#if DEBUG_MODE
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
}
/**
 * @brief Метод поиска скобки для замены
 *
 * @param text текст для поиска
 * @param pos  начальная позиция для поиска
 * @return     позиция найденной скобки
 */
ssize_t anyks::Grok::bracket(const string & text, const size_t pos) const noexcept {
	// Если текст для парсинга передан
	if(!text.empty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Количество подряд идущих экранирований
			uint16_t shielding = 0;
			// Перебираем полученный текст
			for(size_t i = pos; i < text.length(); i++){
				/**
				 * Определяем текущий символ
				 */
				switch(text.at(i)){
					// Если символом является символ экранирования
					case '\\':
						// Увеличиваем количество найденных экранов
						shielding++;
					break;
					// Если найденный символ открытая скобка
					case '(': {
						// Если экранирования у скобки нет
						if((shielding % 2) == 0){
							// Если следующий символ не последний
							if(((i + 1) < text.length()) && (text.at(i + 1) != '?'))
								// Выходим из функции
								return static_cast <ssize_t> (i);
						}
						// Выполняем сброс количества экранирований
						shielding = 0;
					} break;
					// Если найденный символ любой другой
					default: shielding = 0;
				}
			}
		/**
		 * Если возникает ошибка
		 */
		} catch(const exception & error) {
			/**
			 * Если включён режим отладки
			 */
			#if DEBUG_MODE
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(text, pos), log_t::flag_t::CRITICAL, error.what());
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
	return -1;
}
/**
 * @brief Метод получения позиции именованной группы
 *
 * @param text текст для поиска
 * @param pos  начальная позиция для поиска
 * @return     позиция найденной именованной группы
 */
std::pair <ssize_t, ssize_t> anyks::Grok::namedGroup(const string & text, const size_t pos) const noexcept {
	// Результат работы функции
	std::pair <ssize_t, ssize_t> result = {-1, -1};
	// Если текст передан не пустой
	if(!text.empty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Количество подряд идущих экранирований
			uint16_t shielding = 0;
			// Количество найденных открытых скобок и индекс нужной скобки
			uint16_t openBracket = 0, indexBracket = 0;
			// Перебираем полученный текст
			for(size_t i = pos; i < text.length(); i++){
				/**
				 * Определяем текущий символ
				 */
				switch(text.at(i)){
					// Если символом является символ экранирования
					case '\\':
						// Увеличиваем количество найденных экранов
						shielding++;
					break;
					// Если найденный символ открытая скобка
					case '(': {
						// Увеличиваем количество открытых скобок
						openBracket++;
						// Если экранирования у скобки нет
						if((shielding % 2) == 0){
							// Если следующий символ не последний
							if(((i + 1) < text.length()) && (text.at(i + 1) == '?') &&
							   ((i + 2) < text.length()) && (text.at(i + 2) == '<')){
								// Устанавливаем индекс нужной нам скобки
								indexBracket = openBracket;
								// Выходим из функции
								result.first = static_cast <ssize_t> (i);
							}
						}
						// Выполняем сброс количества экранирований
						shielding = 0;
					} break;
					// Если найденный символ закрытая скобка
					case ')': {
						// Если экранирования у скобки нет
						if((shielding % 2) == 0){
							// Если открытая скобка найдена
							if((result.first > -1) && (indexBracket == openBracket)){
								// Устанавливаем конечную позицию
								result.second = static_cast <ssize_t> (i + 1);
								// Выходим из цикла
								return result;
							}
						}
						// Уменьшаем количество открытых скобок
						openBracket--;
						// Выполняем сброс количества экранирований
						shielding = 0;
					} break;
					// Если найденный символ любой другой
					default: shielding = 0;
				}
			}
		/**
		 * Если возникает ошибка
		 */
		} catch(const exception & error) {
			/**
			 * Если включён режим отладки
			 */
			#if DEBUG_MODE
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(text, pos), log_t::flag_t::CRITICAL, error.what());
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
 * @brief Метод обработки полученной переменной Grok
 *
 * @param text текст в котором найдена переменная Grok
 * @param lets разрешить обработку блочных переменных
 * @return     список извлечённых переменных
 */
vector <std::pair <string, string>> anyks::Grok::prepare(string & text, const bool lets) const noexcept {
	// Результат работы функции
	vector <std::pair <string, string>> result;
	// Если текст для обработки передан
	if(!text.empty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
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
					// Получаем начальную позицию в тексте
					const size_t pos = (let.pos - 2);
					// Получаем размер текста для модификации
					const size_t size = (let.size + 3);
					// Выполняем обработку нашего шаблона
					const auto & vars = this->prepare(pattern, lets);
					// Создаём суффикс формирования результирующего регулярного выражения
					const string suffix = (lets ? ")" : "");
					// Создаём префикс формирования результирующего регулярного выражения
					// const string prefix = (lets ? ((pos > 0) ? ((text.at(pos - 1) != '(') ? "(" : "(?:") : "(") : "");
					const string prefix = (lets ? ((pos > 0) ? (((text.at(pos - 1) != '(') || ::isEscaped(text, pos - 1)) ? "(" : "(?:") : "(") : "");
					// Выполняем замену
					text.replace(pos, size, prefix + pattern + suffix);
					// Выполняем добавления переменной в список результата
					result.emplace_back(::move(value), ::move(pattern));
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
						// Получаем начальную позицию в тексте
						const size_t pos = (let.pos - 2);
						// Получаем размер текста для модификации
						const size_t size = (let.size + 3);
						// Выполняем обработку нашего шаблона
						const auto & vars = this->prepare(pattern, lets);
						// Создаём суффикс формирования результирующего регулярного выражения
						const string suffix = (lets ? ")" : "");
						// Создаём префикс формирования результирующего регулярного выражения
						const string prefix = (lets ? ((pos > 0) ? ((text.at(pos - 1) != '(') ? "(" : "(?:") : "(") : "");
						// Выполняем замену
						text.replace(pos, size, prefix + pattern + suffix);
						// Выполняем добавления переменной в список результата
						result.emplace_back(::move(value), ::move(pattern));
						// Если мы получили список переменных из обраотанного шаблона
						if(!vars.empty())
							// Выполняем добавления полученных шаблонов в результат
							result.insert(result.end(), vars.begin(), vars.end());
						// Выполняем поиск переменных дальше
						goto Process;
					}
				}
			}
		/**
		 * Если возникает ошибка
		 */
		} catch(const exception & error) {
			/**
			 * Если включён режим отладки
			 */
			#if DEBUG_MODE
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(text, lets), log_t::flag_t::CRITICAL, error.what());
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
 * @brief Метод добавления списка поддерживаемых шаблонов
 *
 * @param patterns список поддерживаемых шаблонов
 */
void anyks::Grok::patterns(const json & patterns) noexcept {
	// Если шаблоны переданы
	if(patterns.IsObject() && !patterns.ObjectEmpty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Объект блоковой переменной
			let_t let;
			// Список регулярных выражений в которых содержатся блоковые переменные
			std::unordered_map <string, string> items;
			// Выполняем перебор всего списка значений
			for(auto & m : patterns.GetObj()){
				// Если значение является строкой
				if(m.value.IsString()){
					// Выполняем получение блоковой переменной из текста
					let = this->variable(m.value.GetString());
					// Если разделитель не найден
					if((let.delim == 0) && (let.pos > 0) && (let.size > 0))
						// Выполняем формирование списка регулярных выражений
						items.emplace(m.name.GetString(), m.value.GetString());
					// Выполняем добавление полученных шаблонов как они есть
					else this->pattern(m.name.GetString(), m.value.GetString(), event_t::EXTERNAL);
				}
			}
			// Если список регулярных выражений в которых содержатся блоковые переменные собран
			if(!items.empty()){
				// Выполняем перебор всего списка регулярных выражений
				for(auto & item : items)
					// Выполняем добавление нашего шаблона
					this->pattern(item.first, item.second, event_t::EXTERNAL);
			}
		/**
		 * Если возникает ошибка
		 */
		} catch(const exception & error) {
			/**
			 * Если включён режим отладки
			 */
			#if DEBUG_MODE
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
 * @brief Метод добавления шаблона
 *
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
 * @brief Метод добавления шаблона
 *
 * @param key   название переменной
 * @param val   регуляреное выражение соответствующее переменной
 * @param event тип выполняемого события
 */
void anyks::Grok::pattern(const string & key, const string & val, const event_t event) noexcept {
	// Если параметры шаблона переданы
	if(!key.empty() && !val.empty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем блокировку потока
			const lock_guard <std::mutex> lock(this->_mtx.patterns);
			// Выполняем копирование текста регулярного выражения
			string text = val;
			// Выполняем удаление лишних скобок
			this->removeBrackets(text);
			// Выполняем обработку полученных шаблонов
			this->prepare(text, false);
			// Если текст регулярного выражения получен
			if(!text.empty()){
				/**
				 * Определяем тип события
				 */
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
		/**
		 * Если возникает ошибка
		 */
		} catch(const exception & error) {
			/**
			 * Если включён режим отладки
			 */
			#if DEBUG_MODE
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(key, val, static_cast <uint16_t> (event)), log_t::flag_t::CRITICAL, error.what());
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
 * @brief Метод генерации шаблона
 *
 * @param key название шаблона в виде <name>
 * @param val значение шиблок (Регулярное выражение или Grok-шаблон)
 * @return    сгенерированный шаблон
 */
string anyks::Grok::generatePattern(const string & key, const string & val) noexcept {
	// Результат работы функции
	string result = "";
	// Если данные переданы верные
	if(!key.empty() && !val.empty() && (key.front() == '<') && (key.back() == '>')){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Создаём название переменной
			string name = key;
			// Добавляем разделитель
			name.append(1, '_');
			// Добавляем значение
			name.append(val);
			// Выполняем генерацию идентификатора кэша
			const uint64_t id = ::CityHash64(name.c_str(), name.size());
			// Выполняем поиск идентификатора в списке переменных
			auto i = this->_nameGroups.find(id);
			// Если менованная группа найдена
			if(i != this->_nameGroups.end())
				// Выводим полученный результат
				return i->second;
			// Если именованная группа не найдена
			else {
				// Название переменной
				string var = "";
				// Получаем группу шаблона
				string group = key;
				// Удаляем экранирование блоков
				group.erase(0, 1).pop_back();
				// Переходим по всем символам группы
				for(auto & item : group)
					// Выполняем формирование названия переменной
					var.append(1, ::toupper(item));
				// Добавляем разделитель
				var.append(1, '_');
				// Добавлем текущее значение времени
				var.append(std::to_string(id));
				// Выполняем добавление шаблона
				this->pattern(var, val);
				// Формируем генерацию grok-шаблона
				result.append("%{");
				// Добавляем название сформированной переменной
				result.append(var);
				// Добавляем разделитель
				result.append(1, ':');
				// Добавляем значение переменной
				result.append(group);
				// Закрываем сгенерированный шаблон
				result.append(1, '}');
				// Выполняем блокировку потока потока
				const lock_guard <std::mutex> lock(this->_mtx.cache);
				// Добавляем полученный результат в кэш
				this->_nameGroups.emplace(id, result);
			}
		/**
		 * Если возникает ошибка
		 */
		} catch(const exception & error) {
			/**
			 * Если включён режим отладки
			 */
			#if DEBUG_MODE
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(key, val), log_t::flag_t::CRITICAL, error.what());
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
 * @brief Метод сборки регулярного выражения
 *
 * @param text текст регулярного выражения для сборки
 * @return     идентификатор записи в кэше
 */
uint64_t anyks::Grok::build(string & text) const noexcept {
	// Результат работы функции
	uint64_t result = 0;
	// Если текст передан
	if(!text.empty()){
		// Выполняем генерацию идентификатора кэша
		result = ::CityHash64(text.c_str(), text.size());
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем поиск кэша для текущего регулярного выражения
			auto i = this->_cache.find(result);
			// Если кэш для регулярного выражения найден
			if(i != this->_cache.end()){
				// Выполняем установку регулярное выражение
				text = i->second->express.expression;
				// Выводим идентификатор записи в кэше
				return result;
			// Если в кэше регулярного выражения нет
			} else {
				// Выполняем удаление лишних скобок
				this->removeBrackets(text);
				// Выполняем блокировку потока
				const_cast <grok_t *> (this)->_mtx.cache.lock();
				// Выполняем создании записи кэша
				auto ret = const_cast <grok_t *> (this)->_cache.emplace(result, std::make_unique <cache_t> (this->_log));
				// Выполняем разблокировку потока
				const_cast <grok_t *> (this)->_mtx.cache.unlock();
				// Выполняем генерацию именованных групп
				this->namedGroups(text);
				// Если текст существует а не сломан
				if(!text.empty()){
					// Выполняем обработку полученных шаблонов
					const auto & vars = this->prepare(text);
					// Если список переменных получен
					if(!vars.empty()){
						// Выполняем перебор списка переменных
						for(auto & var : vars)
							// Выполняем добавление переменной
							ret.first->second->vars.push(var.first, var.second);
					}
					// Выполняем установку регулярного выражения
					ret.first->second->express.expression = text;
					// Выполняем компиляцию регулярного выражения
					const int error = ::pcre2_regcomp(&ret.first->second->express.reg, ret.first->second->express.expression.c_str(), REG_UTF | REG_ICASE);
					// Если возникла ошибка компиляции
					if(!(ret.first->second->express.mode = (error == 0))){
						// Создаём буфер данных для извлечения данных ошибки
						char buffer[256];
						// Выполняем заполнение нулями буфер данных
						::memset(buffer, '\0', sizeof(buffer));
						// Выполняем извлечение текста ошибки
						const size_t size = ::pcre2_regerror(error, &ret.first->second->express.reg, buffer, sizeof(buffer) - 1);
						// Если текст ошибки получен
						if(size > 0){
							// Формируем полученную ошибку
							string error(buffer, size);
							/**
							 * Если включён режим отладки
							 */
							#if DEBUG_MODE
								// Выводим сообщение об ошибке
								this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(text), log_t::flag_t::CRITICAL, error.c_str());
							/**
							* Если режим отладки не включён
							*/
							#else
								// Выводим сообщение об ошибке
								this->_log->print("%s", log_t::flag_t::CRITICAL, error.c_str());
							#endif
						}
						// Выполняем сброс собранных данных переменных
						ret.first->second->vars.reset();
						// Выполняем блокировку потока потока для удаления кэша
						const lock_guard <std::mutex> lock(const_cast <grok_t *> (this)->_mtx.cache);
						// Выполняем удаление записи из кэша
						const_cast <grok_t *> (this)->_cache.erase(result);
						// Выполняем зануление идентификатора записи
						result = 0;
					}
				// Формируем сообщение об ошибке
				} else {
					/**
					 * Если включён режим отладки
					 */
					#if DEBUG_MODE
						// Выводим сообщение об ошибке
						this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(ret.first->second->express.expression), log_t::flag_t::CRITICAL, "Regular expression is broken");
					/**
					* Если режим отладки не включён
					*/
					#else
						// Выводим сообщение об ошибке
						this->_log->print("%s", log_t::flag_t::CRITICAL, "Regular expression is broken");
					#endif
					// Выполняем блокировку потока потока для удаления кэша
					const lock_guard <std::mutex> lock(const_cast <grok_t *> (this)->_mtx.cache);
					// Выполняем удаление записи из кэша
					const_cast <grok_t *> (this)->_cache.erase(result);
					// Выполняем зануление идентификатора записи
					result = 0;
				}
			}
		/**
		 * Если возникает ошибка
		 */
		} catch(const exception & error) {
			/**
			 * Если включён режим отладки
			 */
			#if DEBUG_MODE
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
 * @brief Метод выполнения парсинга текста
 *
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
			if((i != this->_cache.end()) && i->second->express.mode){
				// Создаём объект матчинга
				regmatch_t match[i->second->express.reg.re_nsub + 1];
				// Выполняем разбор регулярного выражения
				const int error = ::pcre2_regexec(&i->second->express.reg, text.c_str(), i->second->express.reg.re_nsub + 1, match, REG_NOTEMPTY);
				// Если ошибок не получено
				if((result = (error == 0))){
					// Название полученной переменной
					string value = "";
					// Выполняем перебор всех полученных вариантов
					for(uint8_t j = 1; j < static_cast <uint8_t> (i->second->express.reg.re_nsub + 1); j++){
						// Если результат получен
						if(match[j].rm_eo > match[j].rm_so){
							// Добавляем полученный результат в список результатов
							value.assign(text.c_str() + match[j].rm_so, match[j].rm_eo - match[j].rm_so);
							// Если значение переменной получено
							if(!value.empty()){
								// Извлекаем переменную которой соответствует текст
								const string & key = i->second->vars.get(value, j - 1);
								// Если название переменной получено
								if(!key.empty()){
									// Выполняем блокировку потока
									const lock_guard <std::mutex> lock(this->_mtx.mapping);
									// Выполняем добавления полученных данных в схему соответствий
									i->second->mapping.emplace(key, value);
								}
							}
						}
					}
				}
			}
		/**
		 * Если возникает ошибка
		 */
		} catch(const exception & error) {
			/**
			 * Если включён режим отладки
			 */
			#if DEBUG_MODE
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(text, cid), log_t::flag_t::CRITICAL, error.what());
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
 * @brief Метод извлечения данных в виде JSON
 *
 * @param cid идентификатор записи в кэше
 * @return    json объект дампа данных
 */
anyks::json anyks::Grok::dump(const uint64_t cid) const noexcept {
	// Результат работы функции
	json result(kObjectType);
	/**
	 * Выполняем отлов ошибок
	 */
	try {
		// Выполняем поиск идентификатора регулярного выражения в кэше
		auto i = this->_cache.find(cid);
		// Если идентификатор регулярного выражения в кэше найден
		if(i != this->_cache.end()){
			// Если схема соответствия ключей сформированна
			if(!i->second->mapping.empty()){
				// Выполняем сборку объекта результатов
				for(auto & item : i->second->mapping){
					// Если результат получен в виде обычного числа
					if(this->_fmk->is(item.second, fmk_t::check_t::NUMBER)){
						/**
						 * Выполняем отлов ошибок
						 */
						try {
							// Если число положительное
							if(item.second.front() != '-')
								// Выполняем конвертацию в число
								result.AddMember(Value(item.first.c_str(), item.first.length(), result.GetAllocator()).Move(), Value(static_cast <uint64_t> (::stoull(item.second))).Move(), result.GetAllocator());
							// Выполняем конвертацию в число
							else result.AddMember(Value(item.first.c_str(), item.first.length(), result.GetAllocator()).Move(), Value(static_cast <int64_t> (::stoll(item.second))).Move(), result.GetAllocator());
						/**
						 * Если возникает ошибка
						 */
						} catch(const exception & error) {
							// Выполняем конвертацию в число
							result.AddMember(Value(item.first.c_str(), item.first.length(), result.GetAllocator()).Move(), Value(item.second.c_str(), item.second.length(), result.GetAllocator()).Move(), result.GetAllocator());
						}
					// Если результат получен в виде числа с плавающей точкой
					} else if(this->_fmk->is(item.second, fmk_t::check_t::DECIMAL)) {
						/**
						 * Выполняем отлов ошибок
						 */
						try {
							// Выполняем конвертацию в число с плавающей точкой
							result.AddMember(Value(item.first.c_str(), item.first.length(), result.GetAllocator()).Move(), Value(::stod(item.second)).Move(), result.GetAllocator());
						/**
						 * Если возникает ошибка
						 */
						} catch(const exception & error) {
							// Выполняем конвертацию в число
							result.AddMember(Value(item.first.c_str(), item.first.length(), result.GetAllocator()).Move(), Value(item.second.c_str(), item.second.length(), result.GetAllocator()).Move(), result.GetAllocator());
						}
					// Формируем результат в формате JSON
					} else result.AddMember(Value(item.first.c_str(), item.first.length(), result.GetAllocator()).Move(), Value(item.second.c_str(), item.second.length(), result.GetAllocator()).Move(), result.GetAllocator());
				}
			}
		}
	/**
	 * Если возникает ошибка
	 */
	} catch(const exception & error) {
		/**
		 * Если включён режим отладки
		 */
		#if DEBUG_MODE
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
 * @brief Метод извлечения записи по ключу
 *
 * @param key ключ записи для извлечения
 * @param cid идентификатор записи в кэше
 * @return    значение записи ключа
 */
string anyks::Grok::get(const string & key, const uint64_t cid) const noexcept {
	// Результат работы функции
	string result = "";
	/**
	 * Выполняем отлов ошибок
	 */
	try {
		// Выполняем поиск идентификатора регулярного выражения в кэше
		auto i = this->_cache.find(cid);
		// Если идентификатор регулярного выражения в кэше найден
		if(i != this->_cache.end()){
			// Если ключ записи передан
			if(!key.empty()){
				// Выполняем поиск ключа в схеме соответствий
				auto j = i->second->mapping.find(key);
				// Если ключ в схеме соответствий найден
				if(j != i->second->mapping.end())
					// Выводим результат
					return j->second;
			}
		}
	/**
	 * Если возникает ошибка
	 */
	} catch(const exception & error) {
		/**
		 * Если включён режим отладки
		 */
		#if DEBUG_MODE
			// Выводим сообщение об ошибке
			this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(key, cid), log_t::flag_t::CRITICAL, error.what());
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
 * @brief Конструктор
 *
 * @param fmk объект фреймворка
 * @param log объект для работы с логами
 */
anyks::Grok::Grok(const fmk_t * fmk, const log_t * log) noexcept : _fmk(fmk), _log(log) {
	/**
	 * Выполняем добавление базовых шаблонов
	 * https://docs.streamsets.com/platform-datacollector/latest/datacollector/UserGuide/Apx-GrokPatterns/GrokPatterns_title.html
	 */
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
	this->pattern("URN", "urn:[0-9A-Za-z][0-9A-Za-z-]{0,31}:(?:%[0-9a-fA-F]{2}|[0-9A-Za-z()+,.:=@;$_!*'/?#-])+", event_t::INTERNAL);
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
	this->pattern("URIQUERY", "[A-Za-z0-9$.+!*'|(){},~@#%&/=:;_?\\-\\[\\]<>]*", event_t::INTERNAL);
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
	this->pattern("HTTPDERROR_DATE", "%{DAY} %{MONTH} %{MONTHDAY} %{TIME} %{YEAR}", event_t::INTERNAL);
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
	this->pattern("JAVATHREAD", "(?:[A-Z]{2}-Processor[\\d]+)", event_t::INTERNAL);
	this->pattern("JAVALOGMESSAGE", "(?:.*)", event_t::INTERNAL);
	this->pattern("SYSLOGBASE", "%{SYSLOGTIMESTAMP:timestamp} (?:%{SYSLOGFACILITY} )?%{SYSLOGHOST:logsource} %{SYSLOGPROG}:", event_t::INTERNAL);
	this->pattern("COMMONAPACHELOG", "%{IPORHOST:clientip} %{USER:ident} %{USER:auth} \\[%{HTTPDATE:timestamp}\\] \"(?:%{WORD:verb} %{NOTSPACE:request}(?: HTTP/%{NUMBER:httpversion})?|%{DATA:rawrequest})\" %{NUMBER:response} (?:%{NUMBER:bytes}|-)", event_t::INTERNAL);
	this->pattern("COMBINEDAPACHELOG", "%{COMMONAPACHELOG} %{QS:referrer} %{QS:agent}", event_t::INTERNAL);
	this->pattern("LOGLEVEL", "([Aa]lert|ALERT|[Tt]race|TRACE|[Dd]ebug|DEBUG|[Nn]otice|NOTICE|[Ii]nfo|INFO|[Ww]arn?(?:ing)?|WARN?(?:ING)?|[Ee]rr?(?:or)?|ERR?(?:OR)?|[Cc]rit?(?:ical)?|CRIT?(?:ICAL)?|[Ff]atal|FATAL|[Ss]evere|SEVERE|EMERG(?:ENCY)?|[Ee]merg(?:ency)?)", event_t::INTERNAL);
	this->pattern("EMAILLOCALPART", "[a-zA-Z0-9!#$%&'*+\\-/=?^_`{|}~]{1,64}(?:\\.[a-zA-Z0-9!#$%&'*+\\-/=?^_`{|}~]{1,62}){0,63}", event_t::INTERNAL);
	this->pattern("EMAILADDRESS", "%{EMAILLOCALPART}@%{HOSTNAME}", event_t::INTERNAL);
	this->pattern("HTTPDUSER", "%{EMAILADDRESS}|%{USER}", event_t::INTERNAL);
	this->pattern("HTTPD20_ERRORLOG", "\\[%{HTTPDERROR_DATE:timestamp}\\] \\[%{LOGLEVEL:loglevel}\\] (?:\\[client %{IPORHOST:clientip}\\] ){0,1}%{GREEDYDATA:errormsg}", event_t::INTERNAL);
	this->pattern("HTTPD24_ERRORLOG", "\\[%{HTTPDERROR_DATE:timestamp}\\] \\[%{WORD:module}:%{LOGLEVEL:loglevel}\\] \\[pid %{POSINT:pid}:tid %{NUMBER:tid}\\]( \\(%{POSINT:proxy_errorcode}\\)%{DATA:proxy_errormessage}:)?( \\[client %{IPORHOST:client}:%{POSINT:clientport}\\])? %{DATA:errorcode}: %{GREEDYDATA:message}", event_t::INTERNAL);
	this->pattern("HTTPD_ERRORLOG", "%{HTTPD20_ERRORLOG}|%{HTTPD24_ERRORLOG}", event_t::INTERNAL);
	this->pattern("EXIM_MSGID", "[0-9A-Za-z]{6}-[0-9A-Za-z]{6}-[0-9A-Za-z]{2}", event_t::INTERNAL);
	this->pattern("EXIM_FLAGS", "(<=|[-=>*]>|[*]{2}|==)", event_t::INTERNAL);
	this->pattern("EXIM_DATE", "%{YEAR:exim_year}-%{MONTHNUM:exim_month}-%{MONTHDAY:exim_day} %{TIME:exim_time}", event_t::INTERNAL);
	this->pattern("EXIM_PID", "\\[%{POSINT}\\]", event_t::INTERNAL);
	this->pattern("EXIM_QT", "((\\d+y)?(\\d+w)?(\\d+d)?(\\d+h)?(\\d+m)?(\\d+s)?)", event_t::INTERNAL);
	this->pattern("EXIM_EXCLUDE_TERMS", "(Message is frozen|(Start|End) queue run| Warning: | retry time not reached | no (IP address|host name) found for (IP address|host) | unexpected disconnection while reading SMTP command | no immediate delivery: |another process is handling this message)", event_t::INTERNAL);
	this->pattern("EXIM_REMOTE_HOST", "(H=(%{NOTSPACE:remote_hostname} )?(\\(%{NOTSPACE:remote_heloname}\\) )?\\[%{IP:remote_host}\\])", event_t::INTERNAL);
	this->pattern("EXIM_INTERFACE", "(I=\\[%{IP:exim_interface}\\](:%{NUMBER:exim_interface_port}))", event_t::INTERNAL);
	this->pattern("EXIM_PROTOCOL", "(P=%{NOTSPACE:protocol})", event_t::INTERNAL);
	this->pattern("EXIM_MSG_SIZE", "(S=%{NUMBER:exim_msg_size})", event_t::INTERNAL);
	this->pattern("EXIM_HEADER_ID", "(id=%{NOTSPACE:exim_header_id})", event_t::INTERNAL);
	this->pattern("EXIM_SUBJECT", "(T=%{QS:exim_subject})", event_t::INTERNAL);
	this->pattern("BRO_HTTP", "%{NUMBER:ts}\\t%{NOTSPACE:uid}\\t%{IP:orig_h}\\t%{INT:orig_p}\\t%{IP:resp_h}\\t%{INT:resp_p}\\t%{INT:trans_depth}\\t%{GREEDYDATA:method}\\t%{GREEDYDATA:domain}\\t%{GREEDYDATA:uri}\\t%{GREEDYDATA:referrer}\\t%{GREEDYDATA:user_agent}\\t%{NUMBER:request_body_len}\\t%{NUMBER:response_body_len}\\t%{GREEDYDATA:status_code}\\t%{GREEDYDATA:status_msg}\\t%{GREEDYDATA:info_code}\\t%{GREEDYDATA:info_msg}\\t%{GREEDYDATA:filename}\\t%{GREEDYDATA:bro_tags}\\t%{GREEDYDATA:username}\\t%{GREEDYDATA:password}\\t%{GREEDYDATA:proxied}\\t%{GREEDYDATA:orig_fuids}\\t%{GREEDYDATA:orig_mime_types}\\t%{GREEDYDATA:resp_fuids}\\t%{GREEDYDATA:resp_mime_types}", event_t::INTERNAL);
	this->pattern("BRO_DNS", "%{NUMBER:ts}\\t%{NOTSPACE:uid}\\t%{IP:orig_h}\\t%{INT:orig_p}\\t%{IP:resp_h}\\t%{INT:resp_p}\\t%{WORD:proto}\\t%{INT:trans_id}\\t%{GREEDYDATA:query}\\t%{GREEDYDATA:qclass}\\t%{GREEDYDATA:qclass_name}\\t%{GREEDYDATA:qtype}\\t%{GREEDYDATA:qtype_name}\\t%{GREEDYDATA:rcode}\\t%{GREEDYDATA:rcode_name}\\t%{GREEDYDATA:AA}\\t%{GREEDYDATA:TC}\\t%{GREEDYDATA:RD}\\t%{GREEDYDATA:RA}\\t%{GREEDYDATA:Z}\\t%{GREEDYDATA:answers}\\t%{GREEDYDATA:TTLs}\\t%{GREEDYDATA:rejected}", event_t::INTERNAL);
	this->pattern("BRO_CONN", "%{NUMBER:ts}\\t%{NOTSPACE:uid}\\t%{IP:orig_h}\\t%{INT:orig_p}\\t%{IP:resp_h}\\t%{INT:resp_p}\\t%{WORD:proto}\\t%{GREEDYDATA:service}\\t%{NUMBER:duration}\\t%{NUMBER:orig_bytes}\\t%{NUMBER:resp_bytes}\\t%{GREEDYDATA:conn_state}\\t%{GREEDYDATA:local_orig}\\t%{GREEDYDATA:missed_bytes}\\t%{GREEDYDATA:history}\\t%{GREEDYDATA:orig_pkts}\\t%{GREEDYDATA:orig_ip_bytes}\\t%{GREEDYDATA:resp_pkts}\\t%{GREEDYDATA:resp_ip_bytes}\\t%{GREEDYDATA:tunnel_parents}", event_t::INTERNAL);
	this->pattern("BRO_FILES", "%{NUMBER:ts}\\t%{NOTSPACE:fuid}\\t%{IP:tx_hosts}\\t%{IP:rx_hosts}\\t%{NOTSPACE:conn_uids}\\t%{GREEDYDATA:source}\\t%{GREEDYDATA:depth}\\t%{GREEDYDATA:analyzers}\\t%{GREEDYDATA:mime_type}\\t%{GREEDYDATA:filename}\\t%{GREEDYDATA:duration}\\t%{GREEDYDATA:local_orig}\\t%{GREEDYDATA:is_orig}\\t%{GREEDYDATA:seen_bytes}\\t%{GREEDYDATA:total_bytes}\\t%{GREEDYDATA:missing_bytes}\\t%{GREEDYDATA:overflow_bytes}\\t%{GREEDYDATA:timedout}\\t%{GREEDYDATA:parent_fuid}\\t%{GREEDYDATA:md5}\\t%{GREEDYDATA:sha1}\\t%{GREEDYDATA:sha256}\\t%{GREEDYDATA:extracted}", event_t::INTERNAL);
	this->pattern("BACULA_TIMESTAMP", "%{MONTHDAY}-%{MONTH} %{HOUR}:%{MINUTE}", event_t::INTERNAL);
	this->pattern("BACULA_HOST", "[a-zA-Z0-9-]+", event_t::INTERNAL);
	this->pattern("BACULA_VOLUME", "%{USER}", event_t::INTERNAL);
	this->pattern("BACULA_DEVICE", "%{USER}", event_t::INTERNAL);
	this->pattern("BACULA_DEVICEPATH", "%{UNIXPATH}", event_t::INTERNAL);
	this->pattern("BACULA_CAPACITY", "%{INT}{1,3}(,%{INT}{3})*", event_t::INTERNAL);
	this->pattern("BACULA_VERSION", "%{USER}", event_t::INTERNAL);
	this->pattern("BACULA_JOB", "%{USER}", event_t::INTERNAL);
	this->pattern("BACULA_LOG_MAX_CAPACITY", "User defined maximum volume capacity %{BACULA_CAPACITY} exceeded on device \\\"%{BACULA_DEVICE:device}\\\" \\(?:%{BACULA_DEVICEPATH}\\)", event_t::INTERNAL);
	this->pattern("BACULA_LOG_END_VOLUME", "End of medium on Volume \\\"%{BACULA_VOLUME:volume}\\\" Bytes=%{BACULA_CAPACITY} Blocks=%{BACULA_CAPACITY} at %{MONTHDAY}-%{MONTH}-%{YEAR} %{HOUR}:%{MINUTE}.", event_t::INTERNAL);
	this->pattern("BACULA_LOG_NEW_VOLUME", "Created new Volume \\\"%{BACULA_VOLUME:volume}\\\" in catalog.", event_t::INTERNAL);
	this->pattern("BACULA_LOG_NEW_LABEL", "Labeled new Volume \\\"%{BACULA_VOLUME:volume}\\\" on device \\\"%{BACULA_DEVICE:device}\\\" \\(?:%{BACULA_DEVICEPATH}\\).", event_t::INTERNAL);
	this->pattern("BACULA_LOG_WROTE_LABEL", "Wrote label to prelabeled Volume \\\"%{BACULA_VOLUME:volume}\\\" on device \\\"%{BACULA_DEVICE}\\\" \\(?:%{BACULA_DEVICEPATH}\\)", event_t::INTERNAL);
	this->pattern("BACULA_LOG_NEW_MOUNT", "New volume \\\"%{BACULA_VOLUME:volume}\\\" mounted on device \\\"%{BACULA_DEVICE:device}\\\" \\(?:%{BACULA_DEVICEPATH}\\) at %{MONTHDAY}-%{MONTH}-%{YEAR} %{HOUR}:%{MINUTE}.", event_t::INTERNAL);
	this->pattern("BACULA_LOG_NOOPEN", "\\s+Cannot open %{DATA}: ERR=%{GREEDYDATA:berror}", event_t::INTERNAL);
	this->pattern("BACULA_LOG_NOOPENDIR", "\\s+Could not open directory %{DATA}: ERR=%{GREEDYDATA:berror}", event_t::INTERNAL);
	this->pattern("BACULA_LOG_NOSTAT", "\\s+Could not stat %{DATA}: ERR=%{GREEDYDATA:berror}", event_t::INTERNAL);
	this->pattern("BACULA_LOG_NOJOBS", "There are no more Jobs associated with Volume \\\"%{BACULA_VOLUME:volume}\\\". Marking it purged.", event_t::INTERNAL);
	this->pattern("BACULA_LOG_ALL_RECORDS_PRUNED", "All records pruned from Volume \\\"%{BACULA_VOLUME:volume}\\\"; marking it \\\"Purged\\\"", event_t::INTERNAL);
	this->pattern("BACULA_LOG_BEGIN_PRUNE_JOBS", "Begin pruning Jobs older than %{INT} month %{INT} days .", event_t::INTERNAL);
	this->pattern("BACULA_LOG_BEGIN_PRUNE_FILES", "Begin pruning Files.", event_t::INTERNAL);
	this->pattern("BACULA_LOG_PRUNED_JOBS", "Pruned %{INT} Jobs* for client %{BACULA_HOST:client} from catalog.", event_t::INTERNAL);
	this->pattern("BACULA_LOG_PRUNED_FILES", "Pruned Files from %{INT} Jobs* for client %{BACULA_HOST:client} from catalog.", event_t::INTERNAL);
	this->pattern("BACULA_LOG_ENDPRUNE", "End auto prune.", event_t::INTERNAL);
	this->pattern("BACULA_LOG_STARTJOB", "Start Backup JobId %{INT}, Job=%{BACULA_JOB:job}", event_t::INTERNAL);
	this->pattern("BACULA_LOG_STARTRESTORE", "Start Restore Job %{BACULA_JOB:job}", event_t::INTERNAL);
	this->pattern("BACULA_LOG_USEDEVICE", "Using Device \\\"%{BACULA_DEVICE:device}\\\"", event_t::INTERNAL);
	this->pattern("BACULA_LOG_DIFF_FS", "\\s+%{UNIXPATH} is a different filesystem. Will not descend from %{UNIXPATH} into it.", event_t::INTERNAL);
	this->pattern("BACULA_LOG_JOBEND", "Job write elapsed time = %{DATA:elapsed}, Transfer rate = %{NUMBER} (?:K|M|G)? Bytes/second", event_t::INTERNAL);
	this->pattern("BACULA_LOG_NOPRUNE_JOBS", "No Jobs found to prune.", event_t::INTERNAL);
	this->pattern("BACULA_LOG_NOPRUNE_FILES", "No Files found to prune.", event_t::INTERNAL);
	this->pattern("BACULA_LOG_VOLUME_PREVWRITTEN", "Volume \\\"%{BACULA_VOLUME:volume}\\\" previously written, moving to end of data.", event_t::INTERNAL);
	this->pattern("BACULA_LOG_READYAPPEND", "Ready to append to end of Volume \\\"%{BACULA_VOLUME:volume}\\\" size=%{INT}", event_t::INTERNAL);
	this->pattern("BACULA_LOG_CANCELLING", "Cancelling duplicate JobId=%{INT}.", event_t::INTERNAL);
	this->pattern("BACULA_LOG_MARKCANCEL", "JobId %{INT}, Job %{BACULA_JOB:job} marked to be canceled.", event_t::INTERNAL);
	this->pattern("BACULA_LOG_CLIENT_RBJ", "shell command: run ClientRunBeforeJob \\\"%{GREEDYDATA:runjob}\\\"", event_t::INTERNAL);
	this->pattern("BACULA_LOG_VSS", "(?:Generate\\s+)?VSS (?:Writer)?", event_t::INTERNAL);
	this->pattern("BACULA_LOG_MAXSTART", "Fatal error: Job canceled because max start delay time exceeded.", event_t::INTERNAL);
	this->pattern("BACULA_LOG_DUPLICATE", "Fatal error: JobId %{INT:duplicate} already running. Duplicate job not allowed.", event_t::INTERNAL);
	this->pattern("BACULA_LOG_NOJOBSTAT", "Fatal error: No Job status returned from FD.", event_t::INTERNAL);
	this->pattern("BACULA_LOG_FATAL_CONN", "Fatal error: bsock.c:\\d+ Unable to connect to (?:Client:\\s+%{BACULA_HOST:client}|Storage daemon) on %{HOSTNAME}:%{POSINT}. ERR=(?<berror>%{GREEDYDATA})", event_t::INTERNAL);
	this->pattern("BACULA_LOG_NO_CONNECT", "Warning: bsock.c:\\d+ Could not connect to (?:Client:\\s+%{BACULA_HOST:client}|Storage daemon) on %{HOSTNAME}:%{POSINT}. ERR=(?<berror>%{GREEDYDATA})", event_t::INTERNAL);
	this->pattern("BACULA_LOG_NO_AUTH", "Fatal error: Unable to authenticate with File daemon at %{HOSTNAME}. Possible causes:", event_t::INTERNAL);
	this->pattern("BACULA_LOG_NOSUIT", "No prior or suitable Full backup found in catalog. Doing FULL backup.", event_t::INTERNAL);
	this->pattern("BACULA_LOG_NOPRIOR", "No prior Full backup Job record found.", event_t::INTERNAL);
	this->pattern("BACULA_LOG_JOB", "(?:Error:\\s+)?Bacula %{BACULA_HOST} %{BACULA_VERSION} \\(%{BACULA_VERSION}\\):", event_t::INTERNAL);
	this->pattern("BACULA_LOGLINE", "%{BACULA_TIMESTAMP:bts} %{BACULA_HOST:hostname} JobId %{INT:jobid}: (?:%{BACULA_LOG_MAX_CAPACITY}|%{BACULA_LOG_END_VOLUME}|%{BACULA_LOG_NEW_VOLUME}|%{BACULA_LOG_NEW_LABEL}|%{BACULA_LOG_WROTE_LABEL}|%{BACULA_LOG_NEW_MOUNT}|%{BACULA_LOG_NOOPEN}|%{BACULA_LOG_NOOPENDIR}|%{BACULA_LOG_NOSTAT}|%{BACULA_LOG_NOJOBS}|%{BACULA_LOG_ALL_RECORDS_PRUNED}|%{BACULA_LOG_BEGIN_PRUNE_JOBS}|%{BACULA_LOG_BEGIN_PRUNE_FILES}|%{BACULA_LOG_PRUNED_JOBS}|%{BACULA_LOG_PRUNED_FILES}|%{BACULA_LOG_ENDPRUNE}|%{BACULA_LOG_STARTJOB}|%{BACULA_LOG_STARTRESTORE}|%{BACULA_LOG_USEDEVICE}|%{BACULA_LOG_DIFF_FS}|%{BACULA_LOG_JOBEND}|%{BACULA_LOG_NOPRUNE_JOBS}|%{BACULA_LOG_NOPRUNE_FILES}|%{BACULA_LOG_VOLUME_PREVWRITTEN}|%{BACULA_LOG_READYAPPEND}|%{BACULA_LOG_CANCELLING}|%{BACULA_LOG_MARKCANCEL}|%{BACULA_LOG_CLIENT_RBJ}|%{BACULA_LOG_VSS}|%{BACULA_LOG_MAXSTART}|%{BACULA_LOG_DUPLICATE}|%{BACULA_LOG_NOJOBSTAT}|%{BACULA_LOG_FATAL_CONN}|%{BACULA_LOG_NO_CONNECT}|%{BACULA_LOG_NO_AUTH}|%{BACULA_LOG_NOSUIT}|%{BACULA_LOG_JOB}|%{BACULA_LOG_NOPRIOR})", event_t::INTERNAL);
	this->pattern("S3_REQUEST_LINE", "(?:%{WORD:verb} %{NOTSPACE:request}(?: HTTP/%{NUMBER:httpversion})?|%{DATA:rawrequest})", event_t::INTERNAL);
	this->pattern("S3_ACCESS_LOG", "%{WORD:owner} %{NOTSPACE:bucket} \\[%{HTTPDATE:timestamp}\\] %{IP:clientip} %{NOTSPACE:requester} %{NOTSPACE:request_id} %{NOTSPACE:operation} %{NOTSPACE:key} (?:\"%{S3_REQUEST_LINE}\"|-) (?:%{INT:response:int}|-) (?:-|%{NOTSPACE:error_code}) (?:%{INT:bytes:int}|-) (?:%{INT:object_size:int}|-) (?:%{INT:request_time_ms:int}|-) (?:%{INT:turnaround_time_ms:int}|-) (?:%{QS:referrer}|-) (?:\"?%{QS:agent}\"?|-) (?:-|%{NOTSPACE:version_id})", event_t::INTERNAL);
	this->pattern("ELB_URIPATHPARAM", "%{URIPATH:path}(?:%{URIPARAM:params})?", event_t::INTERNAL);
	this->pattern("ELB_URI", "%{URIPROTO:proto}://(?:%{USER}(?::[^@]*)?@)?(?:%{URIHOST:urihost})?(?:%{ELB_URIPATHPARAM})?", event_t::INTERNAL);
	this->pattern("ELB_REQUEST_LINE", "(?:%{WORD:verb} %{ELB_URI:request}(?: HTTP/%{NUMBER:httpversion})?|%{DATA:rawrequest})", event_t::INTERNAL);
	this->pattern("ELB_ACCESS_LOG", "%{TIMESTAMP_ISO8601:timestamp} %{NOTSPACE:elb} %{IP:clientip}:%{INT:clientport:int} (?:(%{IP:backendip}:?:%{INT:backendport:int})|-) %{NUMBER:request_processing_time:float} %{NUMBER:backend_processing_time:float} %{NUMBER:response_processing_time:float} %{INT:response:int} %{INT:backend_response:int} %{INT:received_bytes:int} %{INT:bytes:int} \"%{ELB_REQUEST_LINE}\"", event_t::INTERNAL);
	this->pattern("HAPROXYTIME", "(?!<[0-9])%{HOUR:haproxy_hour}:%{MINUTE:haproxy_minute}(?::%{SECOND:haproxy_second})(?![0-9])", event_t::INTERNAL);
	this->pattern("HAPROXYDATE", "%{MONTHDAY:haproxy_monthday}/%{MONTH:haproxy_month}/%{YEAR:haproxy_year}:%{HAPROXYTIME:haproxy_time}.%{INT:haproxy_milliseconds}", event_t::INTERNAL);
	this->pattern("HAPROXYCAPTUREDREQUESTHEADERS", "%{DATA:captured_request_headers}", event_t::INTERNAL);
	this->pattern("HAPROXYCAPTUREDRESPONSEHEADERS", "%{DATA:captured_response_headers}", event_t::INTERNAL);
	this->pattern("HAPROXYHTTPBASE", "%{IP:client_ip}:%{INT:client_port} \\[%{HAPROXYDATE:accept_date}\\] %{NOTSPACE:frontend_name} %{NOTSPACE:backend_name}/%{NOTSPACE:server_name} %{INT:time_request}/%{INT:time_queue}/%{INT:time_backend_connect}/%{INT:time_backend_response}/%{NOTSPACE:time_duration} %{INT:http_status_code} %{NOTSPACE:bytes_read} %{DATA:captured_request_cookie} %{DATA:captured_response_cookie} %{NOTSPACE:termination_state} %{INT:actconn}/%{INT:feconn}/%{INT:beconn}/%{INT:srvconn}/%{NOTSPACE:retries} %{INT:srv_queue}/%{INT:backend_queue} (\\{%{HAPROXYCAPTUREDREQUESTHEADERS}\\})?( )?(\\{%{HAPROXYCAPTUREDRESPONSEHEADERS}\\})?( )?\"(<BADREQ>|(%{WORD:http_verb} (%{URIPROTO:http_proto}://)?(?:%{USER:http_user}(?::[^@]*)?@)?(?:%{URIHOST:http_host})?(?:%{URIPATHPARAM:http_request})?( HTTP/%{NUMBER:http_version})?))?\"", event_t::INTERNAL);
	this->pattern("HAPROXYHTTP", "(?:%{SYSLOGTIMESTAMP:syslog_timestamp}|%{TIMESTAMP_ISO8601:timestamp8601}) %{IPORHOST:syslog_server} %{SYSLOGPROG}: %{HAPROXYHTTPBASE}", event_t::INTERNAL);
	this->pattern("HAPROXYTCP", "(?:%{SYSLOGTIMESTAMP:syslog_timestamp}|%{TIMESTAMP_ISO8601:timestamp8601}) %{IPORHOST:syslog_server} %{SYSLOGPROG}: %{IP:client_ip}:%{INT:client_port} \\[%{HAPROXYDATE:accept_date}\\] %{NOTSPACE:frontend_name} %{NOTSPACE:backend_name}/%{NOTSPACE:server_name} %{INT:time_queue}/%{INT:time_backend_connect}/%{NOTSPACE:time_duration} %{NOTSPACE:bytes_read} %{NOTSPACE:termination_state} %{INT:actconn}/%{INT:feconn}/%{INT:beconn}/%{INT:srvconn}/%{NOTSPACE:retries} %{INT:srv_queue}/%{INT:backend_queue}", event_t::INTERNAL);
	this->pattern("RT_FLOW_EVENT", "(?:RT_FLOW_SESSION_CREATE|RT_FLOW_SESSION_CLOSE|RT_FLOW_SESSION_DENY)", event_t::INTERNAL);
	this->pattern("RT_FLOW1", "%{RT_FLOW_EVENT:event}: %{GREEDYDATA:close-reason}: %{IP:src-ip}/%{INT:src-port}->%{IP:dst-ip}/%{INT:dst-port} %{DATA:service} %{IP:nat-src-ip}/%{INT:nat-src-port}->%{IP:nat-dst-ip}/%{INT:nat-dst-port} %{DATA:src-nat-rule-name} %{DATA:dst-nat-rule-name} %{INT:protocol-id} %{DATA:policy-name} %{DATA:from-zone} %{DATA:to-zone} %{INT:session-id} \\d+\\(%{DATA:sent}\\) \\d+\\(%{DATA:received}\\) %{INT:elapsed-time} .*", event_t::INTERNAL);
	this->pattern("RT_FLOW2", "%{RT_FLOW_EVENT:event}: session created %{IP:src-ip}/%{INT:src-port}->%{IP:dst-ip}/%{INT:dst-port} %{DATA:service} %{IP:nat-src-ip}/%{INT:nat-src-port}->%{IP:nat-dst-ip}/%{INT:nat-dst-port} %{DATA:src-nat-rule-name} %{DATA:dst-nat-rule-name} %{INT:protocol-id} %{DATA:policy-name} %{DATA:from-zone} %{DATA:to-zone} %{INT:session-id} .*", event_t::INTERNAL);
	this->pattern("RT_FLOW3", "%{RT_FLOW_EVENT:event}: session denied %{IP:src-ip}/%{INT:src-port}->%{IP:dst-ip}/%{INT:dst-port} %{DATA:service} %{INT:protocol-id}\\(\\d\\) %{DATA:policy-name} %{DATA:from-zone} %{DATA:to-zone} .*", event_t::INTERNAL);
	this->pattern("SYSLOG5424PRINTASCII", "[!-~]+", event_t::INTERNAL);
	this->pattern("SYSLOGBASE2", "(?:%{SYSLOGTIMESTAMP:timestamp}|%{TIMESTAMP_ISO8601:timestamp8601}) (?:%{SYSLOGFACILITY} )?%{SYSLOGHOST:logsource}+(?: %{SYSLOGPROG}:|)", event_t::INTERNAL);
	this->pattern("SYSLOGPAMSESSION", "%{SYSLOGBASE} (?=%{GREEDYDATA:message})%{WORD:pam_module}\\(%{DATA:pam_caller}\\): session %{WORD:pam_session_state} for user %{USERNAME:username}(?: by %{GREEDYDATA:pam_by})?", event_t::INTERNAL);
	this->pattern("CRON_ACTION", "[A-Z ]+", event_t::INTERNAL);
	this->pattern("CRONLOG", "%{SYSLOGBASE} \\(%{USER:user}\\) %{CRON_ACTION:action} \\(%{DATA:message}\\)", event_t::INTERNAL);
	this->pattern("SYSLOGLINE", "%{SYSLOGBASE2} %{GREEDYDATA:message}", event_t::INTERNAL);
	this->pattern("SYSLOG5424PRI", "<%{NONNEGINT:syslog5424_pri}>", event_t::INTERNAL);
	this->pattern("SYSLOG5424SD", "\\[%{DATA}\\]+", event_t::INTERNAL);
	this->pattern("SYSLOG5424BASE", "%{SYSLOG5424PRI}%{NONNEGINT:syslog5424_ver} +(?:%{TIMESTAMP_ISO8601:syslog5424_ts}|-) +(?:%{HOSTNAME:syslog5424_host}|-) +(-|%{SYSLOG5424PRINTASCII:syslog5424_app}) +(-|%{SYSLOG5424PRINTASCII:syslog5424_proc}) +(-|%{SYSLOG5424PRINTASCII:syslog5424_msgid}) +(?:%{SYSLOG5424SD:syslog5424_sd}|-|)", event_t::INTERNAL);
	this->pattern("SYSLOG5424LINE", "%{SYSLOG5424BASE} +%{GREEDYDATA:syslog5424_msg}", event_t::INTERNAL);
	this->pattern("MCOLLECTIVEAUDIT", "%{TIMESTAMP_ISO8601:timestamp}:", event_t::INTERNAL);
	this->pattern("MCOLLECTIVE", "., \\[%{TIMESTAMP_ISO8601:timestamp} #%{POSINT:pid}\\]%{SPACE}%{LOGLEVEL:event_level}", event_t::INTERNAL);
	this->pattern("MONGO_LOG", "%{SYSLOGTIMESTAMP:timestamp} \\[%{WORD:component}\\] %{GREEDYDATA:message}", event_t::INTERNAL);
	this->pattern("MONGO_QUERY", "\\{ (?<={ ).*(?= } ntoreturn:) \\}", event_t::INTERNAL);
	this->pattern("MONGO_SLOWQUERY", "%{WORD} %{MONGO_WORDDASH:database}\\.%{MONGO_WORDDASH:collection} %{WORD}: %{MONGO_QUERY:query} %{WORD}:%{NONNEGINT:ntoreturn} %{WORD}:%{NONNEGINT:ntoskip} %{WORD}:%{NONNEGINT:nscanned}.*nreturned:%{NONNEGINT:nreturned}..+ (?<duration>[0-9]+)ms", event_t::INTERNAL);
	this->pattern("MONGO_WORDDASH", "\\b[\\w-]+\\b", event_t::INTERNAL);
	this->pattern("MONGO3_SEVERITY", "\\w", event_t::INTERNAL);
	this->pattern("MONGO3_COMPONENT", "%{WORD}|-", event_t::INTERNAL);
	this->pattern("MONGO3_LOG", "%{TIMESTAMP_ISO8601:timestamp} %{MONGO3_SEVERITY:severity} %{MONGO3_COMPONENT:component}%{SPACE}(?:\\[%{DATA:context}\\])? %{GREEDYDATA:message}", event_t::INTERNAL);
	this->pattern("POSTGRESQL", "%{DATESTAMP:timestamp} %{TZ} %{DATA:user_id} %{GREEDYDATA:connection_id} %{POSINT:pid}", event_t::INTERNAL);
	this->pattern("RUUID", "\\h{32}", event_t::INTERNAL);
	this->pattern("RCONTROLLER", "(?<controller>[^#]+)#(?<action>\\w+)", event_t::INTERNAL);
	this->pattern("RAILS3HEA", " (?m)Started %{WORD:verb} \"%{URIPATHPARAM:request}\" for %{IPORHOST:clientip} at (?<timestamp>%{YEAR}-%{MONTHNUM}-%{MONTHDAY} %{HOUR}:%{MINUTE}:%{SECOND} %{ISO8601_TIMEZONE})", event_t::INTERNAL);
	this->pattern("RPROCESSING", "\\W*Processing by %{RCONTROLLER} as (?<format>\\S+)(?:\\W*Parameters: {%{DATA:params}}\\W*)?", event_t::INTERNAL);
	this->pattern("RAILS3FOOT", "Completed %{NUMBER:response}%{DATA} in %{NUMBER:totalms}ms %{RAILS3PROFILE}%{GREEDYDATA}", event_t::INTERNAL);
	this->pattern("RAILS3PROFILE", "(?:\\(Views: %{NUMBER:viewms}ms \\| ActiveRecord: %{NUMBER:activerecordms}ms|\\(ActiveRecord: %{NUMBER:activerecordms}ms)?", event_t::INTERNAL);
	this->pattern("RAILS3", "%{RAILS3HEAD}(?:%{RPROCESSING})?(?<context>(?:%{DATA}\\n)*)(?:%{RAILS3FOOT})?", event_t::INTERNAL);
	this->pattern("REDISTIMESTAMP", "%{MONTHDAY} %{MONTH} %{TIME}", event_t::INTERNAL);
	this->pattern("REDISLOG", "\\[%{POSINT:pid}\\] %{REDISTIMESTAMP:timestamp} \\*", event_t::INTERNAL);
	this->pattern("RUBY_LOGLEVEL", "(?:DEBUG|FATAL|ERROR|WARN|INFO)", event_t::INTERNAL);
	this->pattern("RUBY_LOGGER", "[DFEWI], \\[%{TIMESTAMP_ISO8601:timestamp} #%{POSINT:pid}\\] *%{RUBY_LOGLEVEL:loglevel} -- +%{DATA:progname}: %{GREEDYDATA:message}", event_t::INTERNAL);
	this->pattern("CATALINA_DATESTAMP", "%{MONTH} %{MONTHDAY}, 20%{YEAR} %{HOUR}:?%{MINUTE}(?::?%{SECOND}) (?:AM|PM)", event_t::INTERNAL);
	this->pattern("TOMCAT_DATESTAMP", "20%{YEAR}-%{MONTHNUM}-%{MONTHDAY} %{HOUR}:?%{MINUTE}(?::?%{SECOND}) %{ISO8601_TIMEZONE}", event_t::INTERNAL);
	this->pattern("CATALINALOG", "%{CATALINA_DATESTAMP:timestamp} %{JAVACLASS:class} %{JAVALOGMESSAGE:logmessage}", event_t::INTERNAL);
	this->pattern("TOMCATLOG", "%{TOMCAT_DATESTAMP:timestamp} \\| %{LOGLEVEL:level} \\| %{JAVACLASS:class} - %{JAVALOGMESSAGE:logmessage}", event_t::INTERNAL);
	this->pattern("NETSCREENSESSIONLOG", "%{SYSLOGTIMESTAMP:date} %{IPORHOST:device} %{IPORHOST}: NetScreen device_id=%{WORD:device_id}%{DATA}: start_time=%{QUOTEDSTRING:start_time} duration=%{INT:duration} policy_id=%{INT:policy_id} service=%{DATA:service} proto=%{INT:proto} src zone=%{WORD:src_zone} dst zone=%{WORD:dst_zone} action=%{WORD:action} sent=%{INT:sent} rcvd=%{INT:rcvd} src=%{IPORHOST:src_ip} dst=%{IPORHOST:dst_ip} src_port=%{INT:src_port} dst_port=%{INT:dst_port} src-xlated ip=%{IPORHOST:src_xlated_ip} port=%{INT:src_xlated_port} dst-xlated ip=%{IPORHOST:dst_xlated_ip} port=%{INT:dst_xlated_port} session_id=%{INT:session_id} reason=%{GREEDYDATA:reason}", event_t::INTERNAL);
	this->pattern("SHOREWALL", "(?:%{SYSLOGTIMESTAMP:timestamp}) (?:%{WORD:nf_host}) kernel:.*Shorewall:(?:%{WORD:nf_action1})?:(?:%{WORD:nf_action2})?.*IN=(?:%{USERNAME:nf_in_interface})?.*(?:OUT= *MAC=(?:%{COMMONMAC:nf_dst_mac}):(?:%{COMMONMAC:nf_src_mac})?|OUT=%{USERNAME:nf_out_interface}).*SRC=(?:%{IPV4:nf_src_ip}).*DST=(?:%{IPV4:nf_dst_ip}).*LEN=(?:%{WORD:nf_len}).?*TOS=(?:%{WORD:nf_tos}).?*PREC=(?:%{WORD:nf_prec}).?*TTL=(?:%{INT:nf_ttl}).?*ID=(?:%{INT:nf_id}).?*PROTO=(?:%{WORD:nf_protocol}).?*SPT=(?:%{INT:nf_src_port}?.*DPT=%{INT:nf_dst_port}?.*)", event_t::INTERNAL);
	this->pattern("NAGIOSTIME", "\\[%{NUMBER:nagios_epoch}\\]", event_t::INTERNAL);
	this->pattern("NAGIOS_TYPE_CURRENT_SERVICE_STATE", "CURRENT SERVICE STATE", event_t::INTERNAL);
	this->pattern("NAGIOS_TYPE_CURRENT_HOST_STATE", "CURRENT HOST STATE", event_t::INTERNAL);
	this->pattern("NAGIOS_TYPE_SERVICE_NOTIFICATION", "SERVICE NOTIFICATION", event_t::INTERNAL);
	this->pattern("NAGIOS_TYPE_HOST_NOTIFICATION", "HOST NOTIFICATION", event_t::INTERNAL);
	this->pattern("NAGIOS_TYPE_SERVICE_ALERT", "SERVICE ALERT", event_t::INTERNAL);
	this->pattern("NAGIOS_TYPE_HOST_ALERT", "HOST ALERT", event_t::INTERNAL);
	this->pattern("NAGIOS_TYPE_SERVICE_FLAPPING_ALERT", "SERVICE FLAPPING ALERT", event_t::INTERNAL);
	this->pattern("NAGIOS_TYPE_HOST_FLAPPING_ALERT", "HOST FLAPPING ALERT", event_t::INTERNAL);
	this->pattern("NAGIOS_TYPE_SERVICE_DOWNTIME_ALERT", "SERVICE DOWNTIME ALERT", event_t::INTERNAL);
	this->pattern("NAGIOS_TYPE_HOST_DOWNTIME_ALERT", "HOST DOWNTIME ALERT", event_t::INTERNAL);
	this->pattern("NAGIOS_TYPE_PASSIVE_SERVICE_CHECK", "PASSIVE SERVICE CHECK", event_t::INTERNAL);
	this->pattern("NAGIOS_TYPE_PASSIVE_HOST_CHECK", "PASSIVE HOST CHECK", event_t::INTERNAL);
	this->pattern("NAGIOS_TYPE_SERVICE_EVENT_HANDLER", "SERVICE EVENT HANDLER", event_t::INTERNAL);
	this->pattern("NAGIOS_TYPE_HOST_EVENT_HANDLER", "HOST EVENT HANDLER", event_t::INTERNAL);
	this->pattern("NAGIOS_TYPE_EXTERNAL_COMMAND", "EXTERNAL COMMAND", event_t::INTERNAL);
	this->pattern("NAGIOS_TYPE_TIMEPERIOD_TRANSITION", "TIMEPERIOD TRANSITION", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_DISABLE_SVC_CHECK", "DISABLE_SVC_CHECK", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_ENABLE_SVC_CHECK", "ENABLE_SVC_CHECK", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_DISABLE_HOST_CHECK", "DISABLE_HOST_CHECK", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_ENABLE_HOST_CHECK", "ENABLE_HOST_CHECK", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_PROCESS_SERVICE_CHECK_RESULT", "PROCESS_SERVICE_CHECK_RESULT", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_PROCESS_HOST_CHECK_RESULT", "PROCESS_HOST_CHECK_RESULT", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_SCHEDULE_SERVICE_DOWNTIME", "SCHEDULE_SERVICE_DOWNTIME", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_SCHEDULE_HOST_DOWNTIME", "SCHEDULE_HOST_DOWNTIME", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_DISABLE_HOST_SVC_NOTIFICATIONS", "DISABLE_HOST_SVC_NOTIFICATIONS", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_ENABLE_HOST_SVC_NOTIFICATIONS", "ENABLE_HOST_SVC_NOTIFICATIONS", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_DISABLE_HOST_NOTIFICATIONS", "DISABLE_HOST_NOTIFICATIONS", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_ENABLE_HOST_NOTIFICATIONS", "ENABLE_HOST_NOTIFICATIONS", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_DISABLE_SVC_NOTIFICATIONS", "DISABLE_SVC_NOTIFICATIONS", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_ENABLE_SVC_NOTIFICATIONS", "ENABLE_SVC_NOTIFICATIONS", event_t::INTERNAL);
	this->pattern("NAGIOS_WARNING", "Warning:%{SPACE}%{GREEDYDATA:nagios_message}", event_t::INTERNAL);
	this->pattern("NAGIOS_CURRENT_SERVICE_STATE", "%{NAGIOS_TYPE_CURRENT_SERVICE_STATE:nagios_type}: %{DATA:nagios_hostname};%{DATA:nagios_service};%{DATA:nagios_state};%{DATA:nagios_statetype};%{DATA:nagios_statecode};%{GREEDYDATA:nagios_message}", event_t::INTERNAL);
	this->pattern("NAGIOS_CURRENT_HOST_STATE", "%{NAGIOS_TYPE_CURRENT_HOST_STATE:nagios_type}: %{DATA:nagios_hostname};%{DATA:nagios_state};%{DATA:nagios_statetype};%{DATA:nagios_statecode};%{GREEDYDATA:nagios_message}", event_t::INTERNAL);
	this->pattern("NAGIOS_SERVICE_NOTIFICATION", "%{NAGIOS_TYPE_SERVICE_NOTIFICATION:nagios_type}: %{DATA:nagios_notifyname};%{DATA:nagios_hostname};%{DATA:nagios_service};%{DATA:nagios_state};%{DATA:nagios_contact};%{GREEDYDATA:nagios_message}", event_t::INTERNAL);
	this->pattern("NAGIOS_HOST_NOTIFICATION", "%{NAGIOS_TYPE_HOST_NOTIFICATION:nagios_type}: %{DATA:nagios_notifyname};%{DATA:nagios_hostname};%{DATA:nagios_state};%{DATA:nagios_contact};%{GREEDYDATA:nagios_message}", event_t::INTERNAL);
	this->pattern("NAGIOS_SERVICE_ALERT", "%{NAGIOS_TYPE_SERVICE_ALERT:nagios_type}: %{DATA:nagios_hostname};%{DATA:nagios_service};%{DATA:nagios_state};%{DATA:nagios_statelevel};%{NUMBER:nagios_attempt};%{GREEDYDATA:nagios_message}", event_t::INTERNAL);
	this->pattern("NAGIOS_HOST_ALERT", "%{NAGIOS_TYPE_HOST_ALERT:nagios_type}: %{DATA:nagios_hostname};%{DATA:nagios_state};%{DATA:nagios_statelevel};%{NUMBER:nagios_attempt};%{GREEDYDATA:nagios_message}", event_t::INTERNAL);
	this->pattern("NAGIOS_SERVICE_FLAPPING_ALERT", "%{NAGIOS_TYPE_SERVICE_FLAPPING_ALERT:nagios_type}: %{DATA:nagios_hostname};%{DATA:nagios_service};%{DATA:nagios_state};%{GREEDYDATA:nagios_message}", event_t::INTERNAL);
	this->pattern("NAGIOS_HOST_FLAPPING_ALERT", "%{NAGIOS_TYPE_HOST_FLAPPING_ALERT:nagios_type}: %{DATA:nagios_hostname};%{DATA:nagios_state};%{GREEDYDATA:nagios_message}", event_t::INTERNAL);
	this->pattern("NAGIOS_SERVICE_DOWNTIME_ALERT", "%{NAGIOS_TYPE_SERVICE_DOWNTIME_ALERT:nagios_type}: %{DATA:nagios_hostname};%{DATA:nagios_service};%{DATA:nagios_state};%{GREEDYDATA:nagios_comment}", event_t::INTERNAL);
	this->pattern("NAGIOS_HOST_DOWNTIME_ALERT", "%{NAGIOS_TYPE_HOST_DOWNTIME_ALERT:nagios_type}: %{DATA:nagios_hostname};%{DATA:nagios_state};%{GREEDYDATA:nagios_comment}", event_t::INTERNAL);
	this->pattern("NAGIOS_PASSIVE_SERVICE_CHECK", "%{NAGIOS_TYPE_PASSIVE_SERVICE_CHECK:nagios_type}: %{DATA:nagios_hostname};%{DATA:nagios_service};%{DATA:nagios_state};%{GREEDYDATA:nagios_comment}", event_t::INTERNAL);
	this->pattern("NAGIOS_PASSIVE_HOST_CHECK", "%{NAGIOS_TYPE_PASSIVE_HOST_CHECK:nagios_type}: %{DATA:nagios_hostname};%{DATA:nagios_state};%{GREEDYDATA:nagios_comment}", event_t::INTERNAL);
	this->pattern("NAGIOS_SERVICE_EVENT_HANDLER", "%{NAGIOS_TYPE_SERVICE_EVENT_HANDLER:nagios_type}: %{DATA:nagios_hostname};%{DATA:nagios_service};%{DATA:nagios_state};%{DATA:nagios_statelevel};%{DATA:nagios_event_handler_name}", event_t::INTERNAL);
	this->pattern("NAGIOS_HOST_EVENT_HANDLER", "%{NAGIOS_TYPE_HOST_EVENT_HANDLER:nagios_type}: %{DATA:nagios_hostname};%{DATA:nagios_state};%{DATA:nagios_statelevel};%{DATA:nagios_event_handler_name}", event_t::INTERNAL);
	this->pattern("NAGIOS_TIMEPERIOD_TRANSITION", "%{NAGIOS_TYPE_TIMEPERIOD_TRANSITION:nagios_type}: %{DATA:nagios_service};%{DATA:nagios_unknown1};%{DATA:nagios_unknown2}", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_LINE_DISABLE_SVC_CHECK", "%{NAGIOS_TYPE_EXTERNAL_COMMAND:nagios_type}: %{NAGIOS_EC_DISABLE_SVC_CHECK:nagios_command};%{DATA:nagios_hostname};%{DATA:nagios_service}", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_LINE_DISABLE_HOST_CHECK", "%{NAGIOS_TYPE_EXTERNAL_COMMAND:nagios_type}: %{NAGIOS_EC_DISABLE_HOST_CHECK:nagios_command};%{DATA:nagios_hostname}", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_LINE_ENABLE_SVC_CHECK", "%{NAGIOS_TYPE_EXTERNAL_COMMAND:nagios_type}: %{NAGIOS_EC_ENABLE_SVC_CHECK:nagios_command};%{DATA:nagios_hostname};%{DATA:nagios_service}", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_LINE_ENABLE_HOST_CHECK", "%{NAGIOS_TYPE_EXTERNAL_COMMAND:nagios_type}: %{NAGIOS_EC_ENABLE_HOST_CHECK:nagios_command};%{DATA:nagios_hostname}", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_LINE_PROCESS_SERVICE_CHECK_RESULT", "%{NAGIOS_TYPE_EXTERNAL_COMMAND:nagios_type}: %{NAGIOS_EC_PROCESS_SERVICE_CHECK_RESULT:nagios_command};%{DATA:nagios_hostname};%{DATA:nagios_service};%{DATA:nagios_state};%{GREEDYDATA:nagios_check_result}", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_LINE_PROCESS_HOST_CHECK_RESULT", "%{NAGIOS_TYPE_EXTERNAL_COMMAND:nagios_type}: %{NAGIOS_EC_PROCESS_HOST_CHECK_RESULT:nagios_command};%{DATA:nagios_hostname};%{DATA:nagios_state};%{GREEDYDATA:nagios_check_result}", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_LINE_DISABLE_HOST_SVC_NOTIFICATIONS", "%{NAGIOS_TYPE_EXTERNAL_COMMAND:nagios_type}: %{NAGIOS_EC_DISABLE_HOST_SVC_NOTIFICATIONS:nagios_command};%{GREEDYDATA:nagios_hostname}", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_LINE_DISABLE_HOST_NOTIFICATIONS", "%{NAGIOS_TYPE_EXTERNAL_COMMAND:nagios_type}: %{NAGIOS_EC_DISABLE_HOST_NOTIFICATIONS:nagios_command};%{GREEDYDATA:nagios_hostname}", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_LINE_DISABLE_SVC_NOTIFICATIONS", "%{NAGIOS_TYPE_EXTERNAL_COMMAND:nagios_type}: %{NAGIOS_EC_DISABLE_SVC_NOTIFICATIONS:nagios_command};%{DATA:nagios_hostname};%{GREEDYDATA:nagios_service}", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_LINE_ENABLE_HOST_SVC_NOTIFICATIONS", "%{NAGIOS_TYPE_EXTERNAL_COMMAND:nagios_type}: %{NAGIOS_EC_ENABLE_HOST_SVC_NOTIFICATIONS:nagios_command};%{GREEDYDATA:nagios_hostname}", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_LINE_ENABLE_HOST_NOTIFICATIONS", "%{NAGIOS_TYPE_EXTERNAL_COMMAND:nagios_type}: %{NAGIOS_EC_ENABLE_HOST_NOTIFICATIONS:nagios_command};%{GREEDYDATA:nagios_hostname}", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_LINE_ENABLE_SVC_NOTIFICATIONS", "%{NAGIOS_TYPE_EXTERNAL_COMMAND:nagios_type}: %{NAGIOS_EC_ENABLE_SVC_NOTIFICATIONS:nagios_command};%{DATA:nagios_hostname};%{GREEDYDATA:nagios_service}", event_t::INTERNAL);
	this->pattern("NAGIOS_EC_LINE_SCHEDULE_HOST_DOWNTIME", "%{NAGIOS_TYPE_EXTERNAL_COMMAND:nagios_type}: %{NAGIOS_EC_SCHEDULE_HOST_DOWNTIME:nagios_command};%{DATA:nagios_hostname};%{NUMBER:nagios_start_time};%{NUMBER:nagios_end_time};%{NUMBER:nagios_fixed};%{NUMBER:nagios_trigger_id};%{NUMBER:nagios_duration};%{DATA:author};%{DATA:comment}", event_t::INTERNAL);
	this->pattern("NAGIOSLOGLINE", "%{NAGIOSTIME} (?:%{NAGIOS_WARNING}|%{NAGIOS_CURRENT_SERVICE_STATE}|%{NAGIOS_CURRENT_HOST_STATE}|%{NAGIOS_SERVICE_NOTIFICATION}|%{NAGIOS_HOST_NOTIFICATION}|%{NAGIOS_SERVICE_ALERT}|%{NAGIOS_HOST_ALERT}|%{NAGIOS_SERVICE_FLAPPING_ALERT}|%{NAGIOS_HOST_FLAPPING_ALERT}|%{NAGIOS_SERVICE_DOWNTIME_ALERT}|%{NAGIOS_HOST_DOWNTIME_ALERT}|%{NAGIOS_PASSIVE_SERVICE_CHECK}|%{NAGIOS_PASSIVE_HOST_CHECK}|%{NAGIOS_SERVICE_EVENT_HANDLER}|%{NAGIOS_HOST_EVENT_HANDLER}|%{NAGIOS_TIMEPERIOD_TRANSITION}|%{NAGIOS_EC_LINE_DISABLE_SVC_CHECK}|%{NAGIOS_EC_LINE_ENABLE_SVC_CHECK}|%{NAGIOS_EC_LINE_DISABLE_HOST_CHECK}|%{NAGIOS_EC_LINE_ENABLE_HOST_CHECK}|%{NAGIOS_EC_LINE_PROCESS_HOST_CHECK_RESULT}|%{NAGIOS_EC_LINE_PROCESS_SERVICE_CHECK_RESULT}|%{NAGIOS_EC_LINE_SCHEDULE_HOST_DOWNTIME}|%{NAGIOS_EC_LINE_DISABLE_HOST_SVC_NOTIFICATIONS}|%{NAGIOS_EC_LINE_ENABLE_HOST_SVC_NOTIFICATIONS}|%{NAGIOS_EC_LINE_DISABLE_HOST_NOTIFICATIONS}|%{NAGIOS_EC_LINE_ENABLE_HOST_NOTIFICATIONS}|%{NAGIOS_EC_LINE_DISABLE_SVC_NOTIFICATIONS}|%{NAGIOS_EC_LINE_ENABLE_SVC_NOTIFICATIONS})", event_t::INTERNAL);
	this->pattern("CISCO_TAGGED_SYSLOG", "^<%{POSINT:syslog_pri}>%{CISCOTIMESTAMP:timestamp}( %{SYSLOGHOST:sysloghost})? ?: %%{CISCOTAG:ciscotag}:", event_t::INTERNAL);
	this->pattern("CISCOTIMESTAMP", "%{MONTH} +%{MONTHDAY}(?: %{YEAR})? %{TIME}", event_t::INTERNAL);
	this->pattern("CISCOTAG", "[A-Z0-9]+-%{INT}-(?:[A-Z0-9_]+)", event_t::INTERNAL);
	this->pattern("CISCO_ACTION", "Built|Teardown|Deny|Denied|denied|requested|permitted|denied by ACL|discarded|est-allowed|Dropping|created|deleted", event_t::INTERNAL);
	this->pattern("CISCO_REASON", "Duplicate TCP SYN|Failed to locate egress interface|Invalid transport field|No matching connection|DNS Response|DNS Query|(?:%{WORD}\\s*)*", event_t::INTERNAL);
	this->pattern("CISCO_DIRECTION", "Inbound|inbound|Outbound|outbound", event_t::INTERNAL);
	this->pattern("CISCO_INTERVAL", "first hit|%{INT}-second interval", event_t::INTERNAL);
	this->pattern("CISCO_XLATE_TYPE", "static|dynamic", event_t::INTERNAL);
	this->pattern("CISCOFW104001", "\\((?:Primary|Secondary)\\) Switching to ACTIVE - %{GREEDYDATA:switch_reason}", event_t::INTERNAL);
	this->pattern("CISCOFW104002", "\\((?:Primary|Secondary)\\) Switching to STANDBY - %{GREEDYDATA:switch_reason}", event_t::INTERNAL);
	this->pattern("CISCOFW104003", "\\((?:Primary|Secondary)\\) Switching to FAILED\\.", event_t::INTERNAL);
	this->pattern("CISCOFW104004", "\\((?:Primary|Secondary)\\) Switching to OK\\.", event_t::INTERNAL);
	this->pattern("CISCOFW105003", "\\((?:Primary|Secondary)\\) Monitoring on [Ii]nterface %{GREEDYDATA:interface_name} waiting", event_t::INTERNAL);
	this->pattern("CISCOFW105004", "\\((?:Primary|Secondary)\\) Monitoring on [Ii]nterface %{GREEDYDATA:interface_name} normal", event_t::INTERNAL);
	this->pattern("CISCOFW105005", "\\((?:Primary|Secondary)\\) Lost Failover communications with mate on [Ii]nterface %{GREEDYDATA:interface_name}", event_t::INTERNAL);
	this->pattern("CISCOFW105008", "\\((?:Primary|Secondary)\\) Testing [Ii]nterface %{GREEDYDATA:interface_name}", event_t::INTERNAL);
	this->pattern("CISCOFW105009", "\\((?:Primary|Secondary)\\) Testing on [Ii]nterface %{GREEDYDATA:interface_name} (?:Passed|Failed)", event_t::INTERNAL);
	this->pattern("CISCOFW106001", "%{CISCO_DIRECTION:direction} %{WORD:protocol} connection %{CISCO_ACTION:action} from %{IP:src_ip}/%{INT:src_port} to %{IP:dst_ip}/%{INT:dst_port} flags %{GREEDYDATA:tcp_flags} on interface %{GREEDYDATA:interface}", event_t::INTERNAL);
	this->pattern("CISCOFW106006_106007_106010", "%{CISCO_ACTION:action} %{CISCO_DIRECTION:direction} %{WORD:protocol} (?:from|src) %{IP:src_ip}/%{INT:src_port}(\\(%{DATA:src_fwuser}\\))? (?:to|dst) %{IP:dst_ip}/%{INT:dst_port}(\\(%{DATA:dst_fwuser}\\))? (?:on interface %{DATA:interface}|due to %{CISCO_REASON:reason})", event_t::INTERNAL);
	this->pattern("CISCOFW106014", "%{CISCO_ACTION:action} %{CISCO_DIRECTION:direction} %{WORD:protocol} src %{DATA:src_interface}:%{IP:src_ip}(\\(%{DATA:src_fwuser}\\))? dst %{DATA:dst_interface}:%{IP:dst_ip}(\\(%{DATA:dst_fwuser}\\))? \\(type %{INT:icmp_type}, code %{INT:icmp_code}\\)", event_t::INTERNAL);
	this->pattern("CISCOFW106015", "%{CISCO_ACTION:action} %{WORD:protocol} \\(%{DATA:policy_id}\\) from %{IP:src_ip}/%{INT:src_port} to %{IP:dst_ip}/%{INT:dst_port} flags %{DATA:tcp_flags}  on interface %{GREEDYDATA:interface}", event_t::INTERNAL);
	this->pattern("CISCOFW106021", "%{CISCO_ACTION:action} %{WORD:protocol} reverse path check from %{IP:src_ip} to %{IP:dst_ip} on interface %{GREEDYDATA:interface}", event_t::INTERNAL);
	this->pattern("CISCOFW106023", "%{CISCO_ACTION:action}( protocol)? %{WORD:protocol} src %{DATA:src_interface}:%{DATA:src_ip}(/%{INT:src_port})?(\\(%{DATA:src_fwuser}\\))? dst %{DATA:dst_interface}:%{DATA:dst_ip}(/%{INT:dst_port})?(\\(%{DATA:dst_fwuser}\\))?( \\(type %{INT:icmp_type}, code %{INT:icmp_code}\\))? by access-group \"?%{DATA:policy_id}\"? \\[%{DATA:hashcode1}, %{DATA:hashcode2}\\]", event_t::INTERNAL);
	this->pattern("CISCOFW106100_2_3", "access-list %{NOTSPACE:policy_id} %{CISCO_ACTION:action} %{WORD:protocol} for user '%{DATA:src_fwuser}' %{DATA:src_interface}/%{IP:src_ip}\\(%{INT:src_port}\\) -> %{DATA:dst_interface}/%{IP:dst_ip}\\(%{INT:dst_port}\\) hit-cnt %{INT:hit_count} %{CISCO_INTERVAL:interval} \\[%{DATA:hashcode1}, %{DATA:hashcode2}\\]", event_t::INTERNAL);
	this->pattern("CISCOFW106100", "access-list %{NOTSPACE:policy_id} %{CISCO_ACTION:action} %{WORD:protocol} %{DATA:src_interface}/%{IP:src_ip}\\(%{INT:src_port}\\)(\\(%{DATA:src_fwuser}\\))? -> %{DATA:dst_interface}/%{IP:dst_ip}\\(%{INT:dst_port}\\)(\\(%{DATA:src_fwuser}\\))? hit-cnt %{INT:hit_count} %{CISCO_INTERVAL:interval} \\[%{DATA:hashcode1}, %{DATA:hashcode2}\\]", event_t::INTERNAL);
	this->pattern("CISCOFW110002", "%{CISCO_REASON:reason} for %{WORD:protocol} from %{DATA:src_interface}:%{IP:src_ip}/%{INT:src_port} to %{IP:dst_ip}/%{INT:dst_port}", event_t::INTERNAL);
	this->pattern("CISCOFW302010", "%{INT:connection_count} in use, %{INT:connection_count_max} most used", event_t::INTERNAL);
	this->pattern("CISCOFW302013_302014_302015_302016", "%{CISCO_ACTION:action}(?: %{CISCO_DIRECTION:direction})? %{WORD:protocol} connection %{INT:connection_id} for %{DATA:src_interface}:%{IP:src_ip}/%{INT:src_port}( \\(%{IP:src_mapped_ip}/%{INT:src_mapped_port}\\))?(\\(%{DATA:src_fwuser}\\))? to %{DATA:dst_interface}:%{IP:dst_ip}/%{INT:dst_port}( \\(%{IP:dst_mapped_ip}/%{INT:dst_mapped_port}\\))?(\\(%{DATA:dst_fwuser}\\))?( duration %{TIME:duration} bytes %{INT:bytes})?(?: %{CISCO_REASON:reason})?( \\(%{DATA:user}\\))?", event_t::INTERNAL);
	this->pattern("CISCOFW302020_302021", "%{CISCO_ACTION:action}(?: %{CISCO_DIRECTION:direction})? %{WORD:protocol} connection for faddr %{IP:dst_ip}/%{INT:icmp_seq_num}(?:\\(%{DATA:fwuser}\\))? gaddr %{IP:src_xlated_ip}/%{INT:icmp_code_xlated} laddr %{IP:src_ip}/%{INT:icmp_code}( \\(%{DATA:user}\\))?", event_t::INTERNAL);
	this->pattern("CISCOFW305011", "%{CISCO_ACTION:action} %{CISCO_XLATE_TYPE:xlate_type} %{WORD:protocol} translation from %{DATA:src_interface}:%{IP:src_ip}(/%{INT:src_port})?(\\(%{DATA:src_fwuser}\\))? to %{DATA:src_xlated_interface}:%{IP:src_xlated_ip}/%{DATA:src_xlated_port}", event_t::INTERNAL);
	this->pattern("CISCOFW313001_313004_313008", "%{CISCO_ACTION:action} %{WORD:protocol} type=%{INT:icmp_type}, code=%{INT:icmp_code} from %{IP:src_ip} on interface %{DATA:interface}( to %{IP:dst_ip})?", event_t::INTERNAL);
	this->pattern("CISCOFW313005", "%{CISCO_REASON:reason} for %{WORD:protocol} error message: %{WORD:err_protocol} src %{DATA:err_src_interface}:%{IP:err_src_ip}(\\(%{DATA:err_src_fwuser}\\))? dst %{DATA:err_dst_interface}:%{IP:err_dst_ip}(\\(%{DATA:err_dst_fwuser}\\))? \\(type %{INT:err_icmp_type}, code %{INT:err_icmp_code}\\) on %{DATA:interface} interface\\.  Original IP payload: %{WORD:protocol} src %{IP:orig_src_ip}/%{INT:orig_src_port}(\\(%{DATA:orig_src_fwuser}\\))? dst %{IP:orig_dst_ip}/%{INT:orig_dst_port}(\\(%{DATA:orig_dst_fwuser}\\))?", event_t::INTERNAL);
	this->pattern("CISCOFW321001", "Resource '%{WORD:resource_name}' limit of %{POSINT:resource_limit} reached for system", event_t::INTERNAL);
	this->pattern("CISCOFW402117", "%{WORD:protocol}: Received a non-IPSec packet \\(protocol= %{WORD:orig_protocol}\\) from %{IP:src_ip} to %{IP:dst_ip}", event_t::INTERNAL);
	this->pattern("CISCOFW402119", "%{WORD:protocol}: Received an %{WORD:orig_protocol} packet \\(SPI= %{DATA:spi}, sequence number= %{DATA:seq_num}\\) from %{IP:src_ip} \\(user= %{DATA:user}\\) to %{IP:dst_ip} that failed anti-replay checking", event_t::INTERNAL);
	this->pattern("CISCOFW419001", "%{CISCO_ACTION:action} %{WORD:protocol} packet from %{DATA:src_interface}:%{IP:src_ip}/%{INT:src_port} to %{DATA:dst_interface}:%{IP:dst_ip}/%{INT:dst_port}, reason: %{GREEDYDATA:reason}", event_t::INTERNAL);
	this->pattern("CISCOFW419002", "%{CISCO_REASON:reason} from %{DATA:src_interface}:%{IP:src_ip}/%{INT:src_port} to %{DATA:dst_interface}:%{IP:dst_ip}/%{INT:dst_port} with different initial sequence number", event_t::INTERNAL);
	this->pattern("CISCOFW500004", "%{CISCO_REASON:reason} for protocol=%{WORD:protocol}, from %{IP:src_ip}/%{INT:src_port} to %{IP:dst_ip}/%{INT:dst_port}", event_t::INTERNAL);
	this->pattern("CISCOFW602303_602304", "%{WORD:protocol}: An %{CISCO_DIRECTION:direction} %{GREEDYDATA:tunnel_type} SA \\(SPI= %{DATA:spi}\\) between %{IP:src_ip} and %{IP:dst_ip} \\(user= %{DATA:user}\\) has been %{CISCO_ACTION:action}", event_t::INTERNAL);
	this->pattern("CISCOFW710001_710002_710003_710005_710006", "%{WORD:protocol} (?:request|access) %{CISCO_ACTION:action} from %{IP:src_ip}/%{INT:src_port} to %{DATA:dst_interface}:%{IP:dst_ip}/%{INT:dst_port}", event_t::INTERNAL);
	this->pattern("CISCOFW713172", "Group = %{GREEDYDATA:group}, IP = %{IP:src_ip}, Automatic NAT Detection Status:\\s+Remote end\\s*%{DATA:is_remote_natted}\\s*behind a NAT device\\s+This\\s+end\\s*%{DATA:is_local_natted}\\s*behind a NAT device", event_t::INTERNAL);
	this->pattern("CISCOFW733100", "\\[\\s*%{DATA:drop_type}\\s*\\] drop %{DATA:drop_rate_id} exceeded. Current burst rate is %{INT:drop_rate_current_burst} per second, max configured rate is %{INT:drop_rate_max_burst}; Current average rate is %{INT:drop_rate_current_avg} per second, max configured rate is %{INT:drop_rate_max_avg}; Cumulative total count is %{INT:drop_total_count}", event_t::INTERNAL);
}
