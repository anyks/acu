/**
 * @file: parser.hpp
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

#ifndef __ANYKS_ACU_PARSER__
#define __ANYKS_ACU_PARSER__

/**
 * Разрешаем сборку библиотеки LibXML2
 */
#ifndef LIBXML_STATIC
	#define LIBXML_STATIC
#endif

/**
 * Разрешаем сборку библиотеки YAML
 */
#ifndef yaml_cpp_EXPORTS
	#define YAML_CPP_API
	#define yaml_cpp_EXPORTS
#endif

/**
 * Подключаем зависимые заголовки
 */
#include <mutex>
#include <string>

/**
 * Подключаем INI
*/
#include "ini/ini.h"

/**
 * Подключаем YAML
*/
#include "yaml-cpp/yaml.h"

/**
 * Подключаем LibXML2
*/
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

/**
 * Подключаем заголовочные файлы JSON
 */
#include <rapidjson/writer.h>
#include <rapidjson/error/en.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

/**
 * Наши модули
 */
#include <cef.hpp>
#include <csv.hpp>
#include <grok.hpp>
#include <syslog.hpp>

/**
 * anyks пространство имён
 */
namespace anyks {
	/**
	 * Parser Класс парсера
	 */
	typedef class ACUSHARED_EXPORT Parser {
		private:
			// Объект модуля CEF
			cef_t _cef;
			// Объект модуля CSV
			csv_t _csv;
			// Объект модуля GROK
			grok_t _grok;
			// Объект модуля SysLog
			syslog_t _syslog;
		private:
			// Мютекс для блокировки потока
			std::recursive_mutex _mtx;
		private:
			// Объект фреймворка
			const awh::fmk_t * _fmk;
			// Объект работы с логами
			const awh::log_t * _log;
		public:
			/**
			 * clearPatterns Метод сброса списка добавленных шаблонов GROK
			 */
			void clearPatterns() noexcept;
		public:
			/**
			 * patterns Метод добавления списка поддерживаемых шаблонов
			 * @param patterns список поддерживаемых шаблонов
			 */
			void patterns(const rapidjson::Document & patterns) noexcept;
			/**
			 * pattern Метод добавления шаблона GROK
			 * @param key название переменной
			 * @param val регуляреное выражение соответствующее переменной
			 */
			void pattern(const std::string & key, const std::string & val) noexcept;
		public:
			/**
			 * yaml Метод конвертации текста в формате YAML в объект JSON
			 * @param text текст для конвертации
			 * @return     объект в формате JSON
			 */
			rapidjson::Document yaml(const std::string & text) noexcept;
			/**
			 * yaml Метод конвертации объекта JSON в текст в формате YAML
			 * @param data данные в объекте JSON
			 * @return     текст после конвертации
			 */
			std::string yaml(const rapidjson::Document & data) noexcept;
		public:
			/**
			 * ini Метод конвертации текста в формате INI в объект JSON
			 * @param text текст для конвертации
			 * @return     объект в формате JSON
			 */
			rapidjson::Document ini(const std::string & text) noexcept;
			/**
			 * ini Метод конвертации объекта JSON в текст в формате INI
			 * @param data данные в объекте JSON
			 * @return     текст после конвертации
			 */
			std::string ini(const rapidjson::Document & data) noexcept;
		public:
			/**
			 * syslog Метод конвертации текста в формате SysLog в объект JSON
			 * @param text текст для конвертации
			 * @return     объект в формате JSON
			 */
			rapidjson::Document syslog(const std::string & text) noexcept;
			/**
			 * syslog Метод конвертации объекта JSON в текст в формате SysLog
			 * @param data данные в объекте JSON
			 * @return     текст после конвертации
			 */
			std::string syslog(const rapidjson::Document & data) noexcept;
		public:
			/**
			 * grok Метод конвертации текста в формате GROK в объект JSON
			 * @param text    текст для конвертации
			 * @param pattern регулярное выражение в формате GROK
			 * @return        объект в формате JSON
			 */
			rapidjson::Document grok(const std::string & text, const std::string & pattern) noexcept;
		public:
			/**
			 * csv Метод конвертации текста в формате CSV в объект JSON
			 * @param text   текст для конвертации
			 * @param header флаг формирования заголовков
			 * @return       объект в формате JSON
			 */
			rapidjson::Document csv(const std::string & text, const bool header = true) noexcept;
			/**
			 * csv Метод конвертации объекта JSON в текст в формате CSV
			 * @param data   данные в объекте JSON
			 * @param header флаг формирования заголовков
			 * @param delim  используемый разделитель
			 * @return       текст после конвертации
			 */
			std::string csv(const rapidjson::Document & data, const bool header = true, const char delim = ';') noexcept;
		public:
			/**
			 * xml Метод конвертации текста в формате XML в объект JSON
			 * @param text текст для конвертации
			 * @return     объект в формате JSON
			 */
			rapidjson::Document xml(const std::string & text) noexcept;
			/**
			 * xml Метод конвертации объекта JSON в текст в формате XML
			 * @param data     данные в объекте JSON
			 * @param prettify флаг генерации читаемого формата
			 * @return         текст после конвертации
			 */
			std::string xml(const rapidjson::Document & data, const bool prettify = false) noexcept;
		public:
			/**
			 * json Метод конвертации текста в формате JSON в объект JSON
			 * @param text текст для конвертации
			 * @return     объект в формате JSON
			 */
			rapidjson::Document json(const std::string & text) noexcept;
			/**
			 * json Метод конвертации объекта JSON в текст в формате JSON
			 * @param data     данные в объекте JSON
			 * @param prettify флаг генерации читаемого формата
			 * @return         текст после конвертации
			 */
			std::string json(const rapidjson::Document & data, const bool prettify = false) noexcept;
		public:
			/**
			 * cef Метод конвертации текста в формате CEF в объект JSON
			 * @param text текст для конвертации
			 * @param mode режим парсинга
			 * @return     объект в формате JSON
			 */
			rapidjson::Document cef(const std::string & text, const cef_t::mode_t mode = cef_t::mode_t::STRONG) noexcept;
			/**
			 * cef Метод конвертации объекта JSON в текст в формате CEF
			 * @param data данные в объекте JSON
			 * @param mode режим парсинга
			 * @return     текст после конвертации
			 */
			std::string cef(const rapidjson::Document & data, const cef_t::mode_t mode = cef_t::mode_t::STRONG) noexcept;
		public:
			/**
			 * Parser Конструктор
			 * @param fmk объект фреймворка
			 * @param log объект для работы с логами
			 */
			Parser(const awh::fmk_t * fmk, const awh::log_t * log) noexcept :
			 _cef(fmk, log), _csv(fmk, log), _grok(fmk, log),
			 _syslog(fmk, log), _fmk(fmk), _log(log) {}
			/**
			 * ~Parser Деструктор
			 */
			~Parser() noexcept {}
	} parser_t;
};

#endif // __ANYKS_ACU_PARSER__
