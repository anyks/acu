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
 * @copyright: Copyright © 2025
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
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <pcre2posix.h>
#include <cityhash/city.h>

/**
 * Модули AWH
 */
#include <awh/sys/fmk.hpp>
#include <awh/sys/log.hpp>

/**
 * Подключаем заголовочные файлы JSON
 */
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

/**
 * anyks пространство имён
 */
namespace anyks {
	/**
	 * Подписываемся на пространство имён awh
	 */
	using namespace awh;
	/**
	 * Подписываемся на пространство имён rapidjson
	 */
	using namespace rapidjson;
	/**
	 * Активируем пространство имён json
	 */
	using json = Document;
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
				NONE     = 0x00, // Событие не установленно
				INTERNAL = 0x01, // Внутреннее событие
				EXTERNAL = 0x02  // Внешнее событие
			};
		private:
			/**
			 * Express Структура кэша регулярных выражений
			 */
			typedef class Express {
				public:
					// Флаг инициализации регулярного выражения
					bool mode;
				public:
					// Объект контекста регулярного выражения
					regex_t reg;
				public:
					// Регулярное выражение в текстовом виде
					string expression;
				public:
					/**
					 * Express Конструктор
					 */
					Express() noexcept : mode(false) {}
					/**
					 * ~Express Деструктор
					 */
					~Express() noexcept {
						// Если регулярное выражение скомпилированно
						if(this->mode)
							// Выполняем удаление скомпилированного регулярного выражения
							::pcre2_regfree(&this->reg);
					}
			} express_t;
		private:
			/**
			 * Mutex структура рабочих мютексов
			 */
			typedef struct Mutex {
				mutex cache;    // Мютекс контроля кэша
				mutex mapping;  // Мютекс контроля собранных соответствий
				mutex patterns; // Мютекс контроля собранных шаблонов
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
			 * Variables Класс работы с переменными
			 */
			typedef class Variables {
				private:
					/**
					 * Grok Устанавливаем дружбу с родительским модулем
					 */
					friend class Grok;
				private:
					// Мютекс для блокировки потока
					mutex _mtx;
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
					 * Variables конструктор
					 * @param log объект для работы с логами
					 */
					Variables(const log_t * log) noexcept : _log(log) {}
					/**
					 * ~Variables деструктор
					 */
					~Variables() noexcept;
			} vars_t;
		private:
			/**
			 * Cache Структура кэша
			 */
			typedef struct Cache {
				// Переменные регулярного выражения
				vars_t vars;
				// Регулярные выражения
				express_t express;
				// Схема соответствий ключей
				unordered_map <string, string> mapping;
				/**
				 * Cache конструктор
				 * @param log объект для работы с логами
				 */
				Cache(const log_t * log) noexcept : vars(log) {}
			} cache_t;
		private:
			// Мютекс для блокировки потока
			mtx_t _mtx;
		private:
			// Список именованных групп
			map <uint64_t, string> _nameGroups;
		private:
			// Список внутренних шаблонов для работы
			unordered_map <string, string> _patternsInternal;
			// Список внешних шаблонов для работы
			unordered_map <string, string> _patternsExternal;
		private:
			// Объект кэша работы модуля
			map <uint64_t, unique_ptr <cache_t>> _cache;
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
			 * @param cid идентификатор записи в кэше
			 */
			void reset(const uint64_t cid) noexcept;
		public:
			/**
			 * clearPatterns Метод очистки списка добавленных шаблонов
			 */
			void clearPatterns() noexcept;
		public:
			/**
			 * removePattern Метод удаления добавленного шаблона
			 * @param name название шаблона для удаления
			 */
			void removePattern(const string & name) noexcept;
		private:
			/**
			 * namedGroups Метод извлечения именованных групп
			 * @param text текст для извлечения именованных групп
			 */
			void namedGroups(string & text) const noexcept;
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
			 * namedGroup Метод получения позиции именованной группы
			 * @param text текст для поиска
			 * @param pos  начальная позиция для поиска
			 * @return     позиция найденной именованной группы
			 */
			pair <ssize_t, ssize_t> namedGroup(const string & text, const size_t pos = 0) const noexcept;
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
			 * @param key название переменной
			 * @param val регуляреное выражение соответствующее переменной
			 */
			void pattern(const string & key, const string & val) noexcept;
		private:
			/**
			 * pattern Метод добавления шаблона
			 * @param key   название переменной
			 * @param val   регуляреное выражение соответствующее переменной
			 * @param event тип выполняемого события
			 */
			void pattern(const string & key, const string & val, const event_t event) noexcept;
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
			 * @return     идентификатор записи в кэше
			 */
			uint64_t build(string & text) const noexcept;
		public:
			/**
			 * parse Метод выполнения парсинга текста
			 * @param text текст для парсинга
			 * @param cid  идентификатор записи в кэше
			 * @return     результат выполнения регулярного выражения
			 */
			bool parse(const string & text, const uint64_t cid) noexcept;
		public:
			/**
			 * dump Метод извлечения данных в виде JSON
			 * @param cid идентификатор записи в кэше
			 * @return    json объект дампа данных
			 */
			json dump(const uint64_t cid) const noexcept;
		public:
			/**
			 * get Метод извлечения записи по ключу
			 * @param key ключ записи для извлечения
			 * @param cid идентификатор записи в кэше
			 * @return    значение записи ключа
			 */
			string get(const string & key, const uint64_t cid) const noexcept;
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
			~Grok() noexcept {}
	} grok_t;
};

#endif // __ANYKS_ACU_GROK__
