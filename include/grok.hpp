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
#include <memory>
#include <map>
#include <functional>
#include <utility>
#include <unordered_map>

/**
 * Модули AWH
 */
#include <awh/sys/fmk.hpp>
#include <awh/sys/log.hpp>
#include <awh/regex/grok/grok.hpp>

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
	 * @details Разворот ссылок вида «%{NAME}», «%{NAME:поле}» и «%{NAME:поле:вид}»,
	 *          реестр шаблонов и извлечение полей выполняет модуль «awh::Grok»:
	 *          здесь остаётся только кэш собранных выражений по тексту шаблона и
	 *          схема соответствий, из которой собирается ответ.
	 *
	 */
	typedef class ACU_SHARED_EXPORT Grok {
		private:
			/**
			 * @brief Структура кэша регулярных выражений
			 *
			 */
			typedef struct Cache {
				// Собранный шаблон регулярного выражения
				awh::Grok::exp_t exp;
				// Схема соответствий ключей
				std::unordered_map <string, string> mapping;
			} cache_t;
		private:
			// Мютекс для блокировки потока
			mutable std::mutex _mtx;
		private:
			/**
			 * @brief Метод перевода именованных групп в ссылки шаблонов Grok
			 *
			 * @param text текст регулярного выражения для обработки
			 */
			void namedGroups(string & text) const noexcept;
			/**
			 * @brief Метод поиска именованной группы в тексте выражения
			 *
			 * @param text текст регулярного выражения для поиска
			 * @return     границы найденной группы, либо «-1, -1» при её отсутствии
			 */
			std::pair <ssize_t, ssize_t> namedGroup(const string & text) const noexcept;
		private:
			// Объект реестра и сборки шаблонов Grok: сборка выражения дополняет реестр
			// именованными группами текста, поэтому объект доступен и в const-методе
			mutable awh::Grok _grok;
		private:
			// Объект кэша работы модуля
			mutable std::map <uint64_t, std::unique_ptr <cache_t>> _cache;
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
			/**
			 * @brief Метод удаления добавленного шаблона
			 *
			 * @param name название шаблона для удаления
			 */
			void removePattern(const string & name) noexcept;
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
			 * @brief Метод добавления шаблона в реестр под удержанной блокировкой
			 *
			 * @param key название переменной
			 * @param val регуляреное выражение соответствующее переменной
			 */
			void addPattern(const string & key, const string & val) noexcept;
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
