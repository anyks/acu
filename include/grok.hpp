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
#include <mutex>
#include <ctime>
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
			/**
			 * Тип выполняемого события
			 */
			enum class event_t : uint8_t {
				NONE     = 0x00,
				INTERNAL = 0x01,
				EXTERNAL = 0x02
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
			 * Let Класс работы с блочными переменными
			 */
			typedef struct Let {
				size_t pos;   // Начальная позиция переменной
				size_t size;  // Размер переменной
				size_t delim; // Позиция разделителя
				/**
				 * Let Конструктор
				 */
				Let() noexcept : pos(0), size(0), delim(0) {}
			} __attribute__((packed)) let_t;
			/**
			 * Variable Класс работы с переменными
			 */
			typedef class Variable {
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
					std::unordered_multimap <string, regex_t> _patterns;
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
					 * Variable конструктор
					 * @param log объект для работы с логами
					 */
					Variable(const log_t * log) noexcept : _log(log) {}
			} var_t;
		private:
			// Флаг инициализации
			bool _mode;
		private:
			// Объект контекста регулярного выражения для формирования групп
			regex_t _reg;
		private:
			// Мютекс для блокировки потока
			mutable mtx_t _mtx;
		private:
			// Параметры собранных переменных
			mutable var_t _variables;
		private:
			// Схема соответствий ключей
			std::unordered_map <string, string> _mapping;
			// Список внутренних шаблонов для работы
			std::unordered_map <string, string> _patternsInternal;
			// Список внешних шаблонов для работы
			std::unordered_map <string, string> _patternsExternal;
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
			/**
			 * clearPattern Метод очистки добавленного шаблона
			 * @param name название шаблона для удаления
			 */
			void clearPattern(const string & name) noexcept;
		private:
			/**
			 * variable Метод извлечения первой блоковой переменной в тексте
			 * @param text текст из которого следует извлечь переменные
			 * @return     первая блоковая переменная
			 */
			let_t variable(const string & text) const noexcept;
		private:
			/**
			 * removeBrackets Метод удаления скобок
			 * @param text текст в котором следует удалить скобки
			 */
			void removeBrackets(string & text) const noexcept;
		private:
			/**
			 * bracket Метод поиска скобки для замены
			 * @param text текст для поиска
			 * @param pos  начальная позиция для поиска
			 * @return     позиция найденной скобки
			 */
			ssize_t bracket(const string & text, const size_t pos = 0) const noexcept;
		private:
			/**
			 * prepare Метод обработки полученной переменной Grok
			 * @param text текст в котором найдена переменная Grok
			 * @param lets разрешить обработку блочных переменных
			 * @return     список извлечённых переменных
			 */
			vector <pair <string, string>> prepare(string & text, const bool lets = true) const noexcept;
		public:
			/**
			 * patterns Метод добавления списка поддерживаемых шаблонов
			 * @param patterns список поддерживаемых шаблонов
			 */
			void patterns(const json & patterns) noexcept;
			/**
			 * pattern Метод добавления шаблона
			 * @param name    название шаблона
			 * @param express регуляреное выражение соответствующее переменной
			 */
			void pattern(const string & name, const string & express) noexcept;
		private:
			/**
			 * pattern Метод добавления шаблона
			 * @param name    название шаблона
			 * @param express регуляреное выражение соответствующее переменной
			 * @param event   тип выполняемого события
			 */
			void pattern(const string & name, const string & express, const event_t event) noexcept;
		private:
			/**
			 * generatePattern Метод генерации шаблона
			 * @param name    название шаблона в виде <name>
			 * @param express значение шиблок (Регулярное выражение или Grok-шаблон)
			 * @return        сгенерированный шаблон
			 */
			string generatePattern(const string & name, const string & express) noexcept;
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
			 * build Метод сборки регулярного выражения
			 * @param text текст регулярного выражения для сборки
			 * @param pure флаг выполнения сборки чистого регулярного выражения
			 * @return     идентификатор записи в кэше
			 */
			uint64_t build(string & text, const bool pure = false) const noexcept;
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
