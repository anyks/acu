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
#include "grok.hpp"

/**
 * Используем стандартное пространство имён
 */
using namespace std;

/**
 * Используем пространство имён проекта
 */
using namespace anyks;

/**
 * @brief Метод очистки параметров модуля
 *
 */
void anyks::Grok::clear() noexcept {
	/**
	 * Выполняем отлов ошибок
	 */
	try {
		// Выполняем блокировку потока
		const lock_guard <std::mutex> lock(this->_mtx);
		// Выполняем удаление кэша
		this->_cache.clear();
		// Выполняем освобождение памяти кэша
		std::map <uint64_t, std::unique_ptr <cache_t>> ().swap(this->_cache);
		// Выполняем восстановление встроенного набора шаблонов
		this->_grok.reset();
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
		const lock_guard <std::mutex> lock(this->_mtx);
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
	/**
	 * Выполняем отлов ошибок
	 */
	try {
		// Выполняем блокировку потока
		const lock_guard <std::mutex> lock(this->_mtx);
		// Выполняем удаление кэша: собранные выражения опирались на снятые шаблоны
		this->_cache.clear();
		// Выполняем восстановление встроенного набора шаблонов
		this->_grok.reset();
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
 * @brief Метод удаления добавленного шаблона
 *
 * @param name название шаблона для удаления
 */
void anyks::Grok::removePattern(const string & name) noexcept {
	// Если название шаблона передано
	if(!name.empty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем блокировку потока
			const lock_guard <std::mutex> lock(this->_mtx);
			// Выполняем удаление шаблона из реестра
			this->_grok.erase(name);
			// Выполняем удаление кэша: собранные выражения могли на него опираться
			this->_cache.clear();
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
			// Выполняем блокировку потока
			const lock_guard <std::mutex> lock(this->_mtx);
			// Выполняем перебор всего списка значений
			for(auto & m : patterns.GetObj()){
				// Если значение является строкой
				if(m.value.IsString())
					// Выполняем добавление шаблона в реестр
					this->addPattern(m.name.GetString(), m.value.GetString());
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
	if(!key.empty() && !val.empty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем блокировку потока
			const lock_guard <std::mutex> lock(this->_mtx);
			// Выполняем добавление шаблона в реестр
			this->addPattern(key, val);
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
}
/**
 * @brief Метод добавления шаблона в реестр под удержанной блокировкой
 *
 * @param key название переменной
 * @param val регуляреное выражение соответствующее переменной
 */
void anyks::Grok::addPattern(const string & key, const string & val) noexcept {
	// Если шаблон принят реестром
	if(this->_grok.pattern(key, val)) {
		// Выполняем удаление кэша: новый шаблон меняет смысл прежних ссылок
		this->_cache.clear();
	// Если шаблон отклонён
	} else {
		/**
		 * Если включён режим отладки
		 */
		#if DEBUG_MODE
			// Выводим сообщение об ошибке
			this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(key, val, static_cast <uint16_t> (this->_grok.error())), log_t::flag_t::CRITICAL, "Grok template is rejected by the registry");
		/**
		* Если режим отладки не включён
		*/
		#else
			// Выводим сообщение об ошибке
			this->_log->print("%s %u", log_t::flag_t::CRITICAL, "Grok template is rejected by the registry, code:", static_cast <unsigned int> (this->_grok.error()));
		#endif
	}
}
/**
 * @brief Метод перевода именованных групп в ссылки шаблонов Grok
 *
 * @param text текст регулярного выражения для обработки
 */
void anyks::Grok::namedGroups(string & text) const noexcept {
	// Выполняем перебор всех именованных группы текста
	for(;;){
		// Получаем границы очередной группы вида «(?<имя>выражение)»
		const auto & group = this->namedGroup(text);
		// Если группа не найдена
		if(group.first < 0)
			// Завершаем обработку
			break;
		// Начало и конец названия группы внутри скобок
		const size_t begin = static_cast <size_t> (group.first) + 3;
		// Получаем текст группы до закрывающей скобки названия
		const size_t end = text.find('>', begin);
		// Если название группы не закрыто
		if((end == string::npos) || (end >= static_cast <size_t> (group.second)))
			// Завершаем обработку
			break;
		// Получаем название группы
		const string name = text.substr(begin, end - begin);
		// Получаем выражение группы
		string body = text.substr(end + 1, static_cast <size_t> (group.second) - (end + 2));
		// Если название и выражение группы получены
		if(!name.empty() && !body.empty()){
			// Выполняем перевод вложенных именованных группы: они становятся ссылками
			// того же вида, иначе поле внутренней группы осталось бы без имени
			this->namedGroups(body);
			// Формируем название вспомогательного шаблона: оно определяет выражение
			// однозначно, поэтому повторная сборка того же текста добавляет тот же шаблон
			string key = "ACU_NAMED_GROUP_";
			for(auto & item : name)
				key.append(1, static_cast <char> (::toupper(item)));
			key.append(1, '_');
			key.append(std::to_string(static_cast <unsigned long long> (std::hash <string> ()(body))));
			// Выполняем добавление выражения группы в реестр шаблонов
			this->_grok.pattern(key, body);
			// Выполняем замену группы на ссылку шаблона с объявленным полем
			text.replace(static_cast <size_t> (group.first), static_cast <size_t> (group.second) - static_cast <size_t> (group.first),
				"%{" + key + ":" + name + "}");
		// Если группа пуста
		} else break;
	}
}
/**
 * @brief Метод поиска именованной группы в тексте выражения
 *
 * @param text текст регулярного выражения для поиска
 * @return     границы найденной группы, либо «-1, -1» при её отсутствии
 */
std::pair <ssize_t, ssize_t> anyks::Grok::namedGroup(const string & text) const noexcept {
	// Результат работы функции
	std::pair <ssize_t, ssize_t> result = {-1, -1};
	// Количество открытых скобок и глубина найденной группы
	size_t depth = 0;
	// Выполняем перебор символов текста
	for(size_t i = 0; i < text.length(); i++){
		// Если найден символ экранирования
		if(text.at(i) == '\\')
			// Пропускаем следующий за ним символ
			i++;
		// Если найдено начало группы
		else if(text.at(i) == '('){
			// Если группа именованная
			if((result.first < 0) && ((i + 3) < text.length()) && (text.at(i + 1) == '?') && (text.at(i + 2) == '<') && (text.at(i + 3) != '?') && (text.at(i + 3) != '!')){
				// Устанавливаем начало группы
				result.first = static_cast <ssize_t> (i);
				// Устанавливаем глубину группы
				depth = 1;
			// Иначе увеличиваем глубину вложенности
			} else if(result.first > -1) depth++;
		// Если найдено начало группы без имени
		} else if(result.first < 0) continue;
		// Если найдено завершение группы
		if(text.at(i) == ')'){
			// Уменьшаем глубину вложенности
			if(depth > 0) depth--;
			// Если группа закрыта
			if(depth == 0){
				// Устанавливаем конец группы
				result.second = static_cast <ssize_t> (i + 1);
				// Выводим границы группы
				return result;
			}
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
		// Выполняем генерацию идентификатора кэша: ключ живёт только внутри процесса
		// и наружу не отдаётся, поэтому годится любой устойчивый хэш текста выражения
		result = static_cast <uint64_t> (std::hash <string> ()(text));
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем блокировку потока
			const lock_guard <std::mutex> lock(this->_mtx);
			// Выполняем поиск кэша для текущего регулярного выражения
			auto i = this->_cache.find(result);
			// Если кэш для регулярного выражения найден
			if(i != this->_cache.end()){
				// Выполняем установку регулярного выражения
				text = i->second->exp->expression;
			// Если в кэше регулярного выражения нет
			} else {
				// Выполняем перевод именованных групп текста в ссылки шаблонов: модуль Grok
				// объявляет поля ссылками «%{ШАБЛОН:поле}», а не именами групп выражения
				this->namedGroups(text);
				// Формируем набор режимов сборки: текст разбирается как UTF-8,
				// регистр символов не учитывается, пустое совпадение не считается
				const vector <awh::regex::flag_t> flags {
					awh::regex::flag_t::CASELESS,
					awh::regex::flag_t::UTF,
					awh::regex::flag_t::NOTEMPTY
				};
				// Выполняем сборку шаблона
				const auto & exp = this->_grok.build(text, flags);
				// Если выражение собрано
				if(exp){
					// Выполняем создании записи кэша
					auto ret = this->_cache.emplace(result, std::make_unique <cache_t> ());
					// Выполняем установку собранного шаблона
					ret.first->second->exp = exp;
					// Выполняем установку регулярного выражения
					text = exp->expression;
				// Если выражение не собрано
				} else {
					/**
					 * Если включён режим отладки
					 */
					#if DEBUG_MODE
						// Выводим сообщение об ошибке
						this->_log->debug("%s", __PRETTY_FUNCTION__, std::make_tuple(text, static_cast <uint16_t> (this->_grok.error())), log_t::flag_t::CRITICAL, "Regular expression is broken");
					/**
					* Если режим отладки не включён
					*/
					#else
						// Выводим сообщение об ошибке
						this->_log->print("%s %u", log_t::flag_t::CRITICAL, "Regular expression is broken, code:", static_cast <unsigned int> (this->_grok.error()));
					#endif
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
			// Выполняем зануление идентификатора записи
			result = 0;
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
			// Выполняем блокировку потока
			const lock_guard <std::mutex> lock(this->_mtx);
			// Выполняем поиск идентификатора регулярного выражения в кэше
			auto i = this->_cache.find(cid);
			// Если идентификатор регулярного выражения в кэше найден
			if(i != this->_cache.end()){
				// Выполняем сбор списка извлечённых полей
				vector <awh::Grok::value_t> values;
				// Выполняем разбор текста собранным выражением
				if((result = this->_grok.exec(text, i->second->exp, values))){
					// Выполняем перебор всех полученных полей
					for(auto & item : values){
						// Если название и значение поля получены
						if(!item.name.empty() && !item.value.empty())
							// Выполняем добавления полученных данных в схему соответствий
							i->second->mapping.emplace(item.name, item.value);
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
		// Выполняем блокировку потока
		const lock_guard <std::mutex> lock(this->_mtx);
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
		// Выполняем блокировку потока
		const lock_guard <std::mutex> lock(this->_mtx);
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
anyks::Grok::Grok(const fmk_t * fmk, const log_t * log) noexcept : _fmk(fmk), _log(log) {}
