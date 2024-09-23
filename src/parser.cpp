/**
 * @file: parser.cpp
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

// Подключаем заголовочный файл
#include <parser.hpp>

/**
 * clearPatterns Метод сброса списка добавленных шаблонов GROK
 */
void anyks::Parser::clearPatterns() noexcept {
	/*
	// Выполняем блокировку потока
	const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
	// Выполняем удаление списка добавленных шаблонов GROK
	this->_grok.clearPatterns();
	*/
}
/**
 * pattern Метод добавления шаблона GROK
 * @param key название переменной
 * @param val регуляреное выражение соответствующее переменной
 */
void anyks::Parser::pattern(const string & key, const string & val) noexcept {
	/*
	// Выполняем блокировку потока
	const std::lock_guard <std::recursive_mutex> lock(this->_mtx);
	// Выполняем добавление шаблона GROK
	this->_grok.pattern(key, val);
	*/
}
/**
 * yaml Метод конвертации текста в формате YAML в объект JSON
 * @param text текст для конвертации
 * @return     объект в формате JSON
 */
nlohmann::json anyks::Parser::yaml(const string & text) noexcept {
	// Результат работы функции
	nlohmann::json result = nlohmann::json::object();
	
	// Выводим результат работы функции
	return result;
}
/**
 * yaml Метод конвертации объекта JSON в текст в формате YAML
 * @param data данные в объекте JSON
 * @return     текст после конвертации
 */
string anyks::Parser::yaml(const nlohmann::json & data) noexcept {
	// Результат работы функции
	string result = "";
	
	// Выводим результат работы функции
	return result;
}
/**
 * ini Метод конвертации текста в формате INI в объект JSON
 * @param text текст для конвертации
 * @return     объект в формате JSON
 */
nlohmann::json anyks::Parser::ini(const string & text) noexcept {
	// Результат работы функции
	nlohmann::json result = nlohmann::json::object();
	
	// Выводим результат работы функции
	return result;
}
/**
 * ini Метод конвертации объекта JSON в текст в формате INI
 * @param data данные в объекте JSON
 * @return     текст после конвертации
 */
string anyks::Parser::ini(const nlohmann::json & data) noexcept {
	// Результат работы функции
	string result = "";
	
	// Выводим результат работы функции
	return result;
}
/**
 * syslog Метод конвертации текста в формате SysLog в объект JSON
 * @param text текст для конвертации
 * @return     объект в формате JSON
 */
nlohmann::json anyks::Parser::syslog(const string & text) noexcept {
	
	// Выводим результат по умолчанию
	return nlohmann::json::object();
}
/**
 * syslog Метод конвертации объекта JSON в текст в формате SysLog
 * @param data данные в объекте JSON
 * @return     текст после конвертации
 */
string anyks::Parser::syslog(const nlohmann::json & data) noexcept {
	
	// Выводим результат по умолчанию
	return "";
}
/**
 * grok Метод конвертации текста в формате GROK в объект JSON
 * @param text    текст для конвертации
 * @param pattern регулярное выражение в формате GROK
 * @return        объект в формате JSON
 */
nlohmann::json anyks::Parser::grok(const string & text, const string & pattern) noexcept {
	
	// Выводим результат по умолчанию
	return nlohmann::json::object();
}
/**
 * csv Метод конвертации объекта JSON в текст в формате CSV
 * @param data  данные в объекте JSON
 * @param delim используемый разделитель
 * @return      текст после конвертации
 */
string anyks::Parser::csv(const nlohmann::json & data, const char delim) noexcept {
	// Результат работы функции
	string result = "";
	
	// Выводим результат по умолчанию
	return result;
}
/**
 * csv Метод конвертации текста в формате CSV в объект JSON
 * @param text   текст для конвертации
 * @param header флаг формирования заголовков
 * @return       объект в формате JSON
 */
nlohmann::json anyks::Parser::csv(const string & text, const bool header) noexcept {
	
	// Выводим результат по умолчанию
	return nlohmann::json::object();
}
/**
 * xml Метод конвертации текста в формате XML в объект JSON
 * @param text текст для конвертации
 * @return     объект в формате JSON
 */
nlohmann::json anyks::Parser::xml(const string & text) noexcept {
	// Результат работы функции
	nlohmann::json result = nlohmann::json::object();
	
	// Выводим результат по умолчанию
	return result;
}
/**
 * xml Метод конвертации объекта JSON в текст в формате XML
 * @param data   данные в объекте JSON
 * @param pretty флаг генерации читаемого формата
 * @return       текст после конвертации
 */
string anyks::Parser::xml(const nlohmann::json & data, const bool pretty) noexcept {
	// Результат работы функции
	string result = "";
	
	// Выводим результат по умолчанию
	return result;
}
/**
 * json Метод конвертации текста в формате JSON в объект JSON
 * @param text текст для конвертации
 * @return     объект в формате JSON
 */
nlohmann::json anyks::Parser::json(const string & text) noexcept {
	
	// Выводим результат по умолчанию
	return nlohmann::json::object();
}
/**
 * json Метод конвертации объекта JSON в текст в формате JSON
 * @param data   данные в объекте JSON
 * @param pretty флаг генерации читаемого формата
 * @return       текст после конвертации
 */
string anyks::Parser::json(const nlohmann::json & data, const bool pretty) noexcept {
	
	// Выводим результат по умолчанию
	return "";
}
/**
 * cef Метод конвертации текста в формате CEF в объект JSON
 * @param text текст для конвертации
 * @param mode режим парсинга
 * @return     объект в формате JSON
 */
nlohmann::json anyks::Parser::cef(const string & text, const cef_t::mode_t mode) noexcept {
	
	// Выводим результат по умолчанию
	return nlohmann::json::object();
}
/**
 * cef Метод конвертации объекта JSON в текст в формате CEF
 * @param data данные в объекте JSON
 * @param mode режим парсинга
 * @return     текст после конвертации
 */
string anyks::Parser::cef(const nlohmann::json & data, const cef_t::mode_t mode) noexcept {
	
	// Выводим результат по умолчанию
	return "";
}
/**
 * Parser Конструктор
 * @param fmk объект фреймворка
 * @param log объект для работы с логами
 */
anyks::Parser::Parser(const fmk_t * fmk, const log_t * log) noexcept :
 _cef(fmk, log), _csv(fmk, log), _grok(fmk, log), _syslog(fmk, log), _fmk(fmk), _log(log) {
	// Выполняем инициализацию Grok
	this->_grok.init();
}
