/**
 * @file: grok.hpp
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

#ifndef __ANYKS_ACU_GROK__
#define __ANYKS_ACU_GROK__

/**
 * Разрешаем сборку под Windows
 */
#include <global.hpp>

/**
 * Подключаем зависимые заголовки
 */
#include <set>
#include <mutex>
#include <ctime>
#include <chrono>
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <pcre2posix.h>
#include <cityhash/city.h>
#include <nlohmann/json.hpp>

/**
 * Модули AWH
 */
#include <sys/fmk.hpp>
#include <sys/log.hpp>

// Объявляем пространство имен
using namespace std;
using namespace awh;

// Активируем пространство имён json
using json = nlohmann::json;

/**
 * anyks пространство имён
 */
namespace anyks {
	/**
	 * Grok Класс модуля GROK
	 */
	typedef class ACUSHARED_EXPORT Grok {
		private:
			/**
			 * Статус определения разделителя
			 */
			enum class ss_t : uint8_t {
				NONE   = 0x00, // Статус не установлен
				FIRST  = 0x01, // Статус начальный
				SECOND = 0x02  // Статус конечный
			};
		private:
			/**
			 * Cache Структура кэша собранных данных
			 */
			typedef struct Cache {
				// Объект контекста регулярного выражения
				regex_t reg;
				// Регулярное выражение в текстовом виде
				string expression;
				// Список имён переменных
				vector <string> names;
				// Список шаблонов переменных
				unordered_multimap <string, regex_t> patterns;
			} cache_t;
		private:
			/**
			 * Mutex структура рабочих мютексов
			 */
			typedef struct Mutex {
				std::mutex cache;    // Мютекс контроля кэша
				std::mutex mapping;  // Мютекс контроля собранных соответствий
				std::mutex patterns; // Мютекс контроля собранных шаблонов
			} mtx_t;
			/**
			 * Var Класс работы с переменными
			 */
			typedef class Var {
				private:
					/**
					 * Grok Устанавливаем дружбу с родительским модулем
					 */
					friend class Grok;
				private:
					// Мютекс для блокировки потока
					std::mutex _mtx;
				private:
					// Список имён переменных
					vector <string> _names;
					// Список шаблонов переменных
					unordered_multimap <string, regex_t> _patterns;
				private:
					// Объект работы с логами
					const log_t * _log;
				public:
					/**
					 * reset Метод сброса параметров объекта
					 */
					void reset() noexcept;
				public:
					/**
					 * count Метод получения количество добавленных переменных
					 * @return количество добавленных переменных
					 */
					uint8_t count() const noexcept;
				public:
					/**
					 * get Метод извлечения названия переменной которой соответствует текст
					 * @param text  для получения переменной передан
					 * @param index индекс запрашиваемой переменной
					 * @return      название переменной, которой соответствует текст
					 */
					string get(const string & text, const uint8_t index) noexcept;
				public:
					/**
					 * push Метод добавления переменной
					 * @param name    название переменной
					 * @param pattern шаблон регулярного выражения переменной
					 */
					void push(const string & name, const string & pattern) noexcept;
				public:
					/**
					 * Var конструктор
					 * @param log объект для работы с логами
					 */
					Var(const log_t * log) noexcept : _log(log) {}
			} var_t;
		private:
			// Флаг инициализации
			bool _mode;
		private:
			// Объект контекста регулярного выражения для исправления скобок
			regex_t _reg1;
			// Объект контекста регулярного выражения для формирования групп
			regex_t _reg2;
		private:
			// Мютекс для блокировки потока
			mutable mtx_t _mtx;
		private:
			// Параметры собранных переменных
			mutable var_t _variables;
		private:
			// Список ключей добавленных шаблонов
			std::set <string> _keys;
		private:
			// Схема соответствий ключей
			std::unordered_map <string, string> _mapping;
			// Список шаблонов для работы
			std::unordered_map <string, string> _patterns;
		private:
			// Объект кэша работы модуля
			std::map <uint64_t, std::unique_ptr <cache_t>> _cache;
		private:
			// Объект фреймворка
			const fmk_t * _fmk;
			// Объект работы с логами
			const log_t * _log;
		public:
			/**
			 * init Метод инициализации шаблонов парсинга
			 */
			void init() noexcept;
			/**
			 * clear Метод очистки параметров модуля
			 */
			void clear() noexcept;
			/**
			 * reset Метод сброса собранных данных
			 */
			void reset() noexcept;
		public:
			/**
			 * clearPatterns Метод очистки списка добавленных шаблонов
			 */
			void clearPatterns() noexcept;
		public:
			/**
			 * pattern Метод добавления шаблона
			 * @param key название переменной
			 * @param val регуляреное выражение соответствующее переменной
			 */
			void pattern(const string & key, const string & val) noexcept;
		private:
			/**
			 * generatePattern Метод генерации шаблона
			 * @param key название шаблона в виде <name>
			 * @param val значение шиблок (Регулярное выражение или Grok-шаблон)
			 * @return    сгенерированный шаблон
			 */
			string generatePattern(const string & key, const string & val) noexcept;
		public:
			/**
			 * build Метод сборки регулярного выражения
			 * @param text текст регулярного выражения для сборки
			 * @param pure флаг выполнения сборки чистого регулярного выражения
			 * @param init флаг инициализации сборки
			 * @param pos  начальная позиция в тексте
			 * @return     идентификатор записи в кэше
			 */
			uint64_t build(string & text, const bool pure = false, const bool init = true, const size_t pos = 0) const noexcept;
		private:
			/**
			 * prepare Метод обработки полученной переменной Grok
			 * @param text  текст в котором найдена переменная Grok
			 * @param pure  флаг выполнения сборки чистого регулярного выражения
			 * @param begin начальная позиция переменной в тексте
			 * @param end   конечная позиция переменной в тексте
			 * @return      результат обработанного текста
			 */
			string & prepare(string & text, const bool pure, const size_t begin, const size_t end) const noexcept;
		public:
			/**
			 * parse Метод выполнения парсинга текста
			 * @param text текст для парсинга
			 * @param cid  идентификатор записи в кэше
			 * @return     результат выполнения регулярного выражения
			 */
			bool parse(const string & text, const uint64_t cid) noexcept;
			/**
			 * parse Метод выполнения парсинга текста
			 * @param text текст для парсинга
			 * @param rule правило парсинга текста
			 * @return     результат выполнения регулярного выражения
			 */
			bool parse(const string & text, const string & rule) noexcept;
		public:
			/**
			 * dump Метод извлечения данных в виде JSON
			 * @return json объект дампа данных
			 */
			json dump() const noexcept;
		public:
			/**
			 * mapping Метод извлечения карты полученных значений
			 * @return карта полученных значений 
			 */
			const std::unordered_map <string, string> & mapping() const noexcept;
		public:
			/**
			 * get Метод извлечения записи по ключу
			 * @param key ключ записи для извлечения
			 * @return    значение записи ключа
			 */
			string get(const string & key) const noexcept;
		public:
			/**
			 * Оператор вывода данные контейнера в качестве строки
			 * @return данные контейнера в качестве строки
			 */
			operator std::string() const noexcept;
		public:
			/**
			 * Grok Конструктор
			 * @param fmk объект фреймворка
			 * @param log объект для работы с логами
			 */
			Grok(const fmk_t * fmk, const log_t * log) noexcept;
			/**
			 * ~Grok Деструктор
			 */
			~Grok() noexcept;
	} grok_t;
	/**
	 * Оператор [<<] вывода в поток Grok контейнера
	 * @param os   поток куда нужно вывести данные
	 * @param grok контенер для присвоения
	 */
	ACUSHARED_EXPORT ostream & operator << (ostream & os, const grok_t & grok) noexcept;
};

#endif // __ANYKS_ACU_GROK__
