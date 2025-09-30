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
 * @brief пространство имён
 *
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
	 * @brief Класс модуля GROK
	 *
	 */
	typedef class ACU_SHARED_EXPORT Grok {
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
			 * @brief Структура кэша регулярных выражений
			 *
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
					 * @brief Конструктор
					 *
					 */
					Express() noexcept : mode(false) {}
					/**
					 * @brief Деструктор
					 *
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
			 * @brief структура рабочих мютексов
			 *
			 */
			typedef struct Mutex {
				// Мютекс контроля кэша
				std::mutex cache;
				// Мютекс контроля собранных соответствий
				std::mutex mapping;
				// Мютекс контроля собранных шаблонов
				std::mutex patterns;
			} mtx_t;
			/**
			 * @brief Класс работы с блочными переменными
			 *
			 */
			typedef struct Let {
				size_t pos;   // Начальная позиция переменной
				size_t size;  // Размер переменной
				size_t delim; // Позиция разделителя
				/**
				 * @brief Конструктор
				 *
				 */
				Let() noexcept : pos(0), size(0), delim(0) {}
			} __attribute__((packed)) let_t;
			/**
			 * @brief Класс работы с переменными
			 *
			 */
			typedef class ACU_SHARED_EXPORT Variables {
				private:
					/**
					 * @brief Устанавливаем дружбу с родительским модулем
					 *
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
					 * @brief Метод сброса параметров объекта
					 *
					 */
					void reset() noexcept;
				public:
					/**
					 * @brief Метод получения количество добавленных переменных
					 *
					 * @return количество добавленных переменных
					 */
					uint8_t count() const noexcept;
				public:
					/**
					 * @brief Метод извлечения названия переменной которой соответствует текст
					 *
					 * @param text  для получения переменной передан
					 * @param index индекс запрашиваемой переменной
					 * @return      название переменной, которой соответствует текст
					 */
					string get(const string & text, const uint8_t index) noexcept;
				public:
					/**
					 * @brief Метод добавления переменной
					 *
					 * @param name    название переменной
					 * @param pattern шаблон регулярного выражения переменной
					 */
					void push(const string & name, const string & pattern) noexcept;
				public:
					/**
					 * @brief конструктор
					 *
					 * @param log объект для работы с логами
					 */
					Variables(const log_t * log) noexcept : _log(log) {}
					/**
					 * @brief деструктор
					 *
					 */
					~Variables() noexcept;
			} vars_t;
		private:
			/**
			 * @brief Структура кэша
			 *
			 */
			typedef struct Cache {
				// Переменные регулярного выражения
				vars_t vars;
				// Регулярные выражения
				express_t express;
				// Схема соответствий ключей
				std::unordered_map <string, string> mapping;
				/**
				 * @brief конструктор
				 *
				 * @param log объект для работы с логами
				 */
				Cache(const log_t * log) noexcept : vars(log) {}
			} cache_t;
		private:
			// Мютекс для блокировки потока
			mtx_t _mtx;
		private:
			// Список именованных групп
			std::map <uint64_t, string> _nameGroups;
		private:
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
			 * @brief Метод очистки параметров модуля
			 *
			 */
			void clear() noexcept;
			/**
			 * @brief Метод сброса собранных данных
			 *
			 * @param cid идентификатор записи в кэше
			 */
			void reset(const uint64_t cid) noexcept;
		public:
			/**
			 * @brief Метод очистки списка добавленных шаблонов
			 *
			 */
			void clearPatterns() noexcept;
		public:
			/**
			 * @brief Метод удаления добавленного шаблона
			 *
			 * @param name название шаблона для удаления
			 */
			void removePattern(const string & name) noexcept;
		private:
			/**
			 * @brief Метод извлечения именованных групп
			 *
			 * @param text текст для извлечения именованных групп
			 */
			void namedGroups(string & text) const noexcept;
		private:
			/**
			 * @brief Метод извлечения первой блоковой переменной в тексте
			 *
			 * @param text текст из которого следует извлечь переменные
			 * @return     первая блоковая переменная
			 */
			let_t variable(const string & text) const noexcept;
		private:
			/**
			 * @brief Метод удаления скобок
			 *
			 * @param text текст в котором следует удалить скобки
			 */
			void removeBrackets(string & text) const noexcept;
		private:
			/**
			 * @brief Метод поиска скобки для замены
			 *
			 * @param text текст для поиска
			 * @param pos  начальная позиция для поиска
			 * @return     позиция найденной скобки
			 */
			ssize_t bracket(const string & text, const size_t pos = 0) const noexcept;
		private:
			/**
			 * @brief Метод получения позиции именованной группы
			 *
			 * @param text текст для поиска
			 * @param pos  начальная позиция для поиска
			 * @return     позиция найденной именованной группы
			 */
			std::pair <ssize_t, ssize_t> namedGroup(const string & text, const size_t pos = 0) const noexcept;
		private:
			/**
			 * @brief Метод обработки полученной переменной Grok
			 *
			 * @param text текст в котором найдена переменная Grok
			 * @param lets разрешить обработку блочных переменных
			 * @return     список извлечённых переменных
			 */
			vector <std::pair <string, string>> prepare(string & text, const bool lets = true) const noexcept;
		public:
			/**
			 * @brief Метод добавления списка поддерживаемых шаблонов
			 *
			 * @param patterns список поддерживаемых шаблонов
			 */
			void patterns(const json & patterns) noexcept;
			/**
			 * @brief Метод добавления шаблона
			 *
			 * @param key название переменной
			 * @param val регуляреное выражение соответствующее переменной
			 */
			void pattern(const string & key, const string & val) noexcept;
		private:
			/**
			 * @brief Метод добавления шаблона
			 *
			 * @param key   название переменной
			 * @param val   регуляреное выражение соответствующее переменной
			 * @param event тип выполняемого события
			 */
			void pattern(const string & key, const string & val, const event_t event) noexcept;
		private:
			/**
			 * @brief Метод генерации шаблона
			 *
			 * @param key название шаблона в виде <name>
			 * @param val значение шиблок (Регулярное выражение или Grok-шаблон)
			 * @return    сгенерированный шаблон
			 */
			string generatePattern(const string & key, const string & val) noexcept;
		public:
			/**
			 * @brief Метод сборки регулярного выражения
			 *
			 * @param text текст регулярного выражения для сборки
			 * @return     идентификатор записи в кэше
			 */
			uint64_t build(string & text) const noexcept;
		public:
			/**
			 * @brief Метод выполнения парсинга текста
			 *
			 * @param text текст для парсинга
			 * @param cid  идентификатор записи в кэше
			 * @return     результат выполнения регулярного выражения
			 */
			bool parse(const string & text, const uint64_t cid) noexcept;
		public:
			/**
			 * @brief Метод извлечения данных в виде JSON
			 *
			 * @param cid идентификатор записи в кэше
			 * @return    json объект дампа данных
			 */
			json dump(const uint64_t cid) const noexcept;
		public:
			/**
			 * @brief Метод извлечения записи по ключу
			 *
			 * @param key ключ записи для извлечения
			 * @param cid идентификатор записи в кэше
			 * @return    значение записи ключа
			 */
			string get(const string & key, const uint64_t cid) const noexcept;
		public:
			/**
			 * @brief Конструктор
			 *
			 * @param fmk объект фреймворка
			 * @param log объект для работы с логами
			 */
			Grok(const fmk_t * fmk, const log_t * log) noexcept;
			/**
			 * @brief Деструктор
			 *
			 */
			~Grok() noexcept {}
	} grok_t;
};

#endif // __ANYKS_ACU_GROK__
