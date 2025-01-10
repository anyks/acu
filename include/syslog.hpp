/**
 * @file: syslog.hpp
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

#ifndef __ANYKS_ACU_SYSLOG__
#define __ANYKS_ACU_SYSLOG__

/**
 * Разрешаем сборку под Windows
 */
#include <global.hpp>

/**
 * Подключаем зависимые заголовки
 */
#include <ctime>
#include <cmath>
#include <stack>
#include <mutex>
#include <vector>
#include <string>
#include <iomanip>
#include <cstring>
#include <sstream>
#include <iostream>
#include <unordered_map>

/**
 * Модули AWH
 */
#include <sys/fmk.hpp>
#include <sys/log.hpp>
#include <sys/reg.hpp>

/**
 * Подключаем заголовочные файлы JSON
 */
#include <rapidjson/document.h>

/**
 * anyks пространство имён
 */
namespace anyks {
	/**
	 * Активируем пространство имён json
	 */
	using json = rapidjson::Document;
	/**
	 * SysLog Класс модуля парсера SysLog (RFC3164/RFC5424)
	 */
	typedef class ACUSHARED_EXPORT SysLog {
		public:
			/**
			 * Поддерживаемые стандарты парсинга
			 */
			enum class std_t : std::uint8_t {
				AUTO    = 0x00, // Стандарт определяется автоматически
				RFC3164 = 0x01, // Стандарт RFC 3164
				RFC5424 = 0x02  // Стандарт RFC 5424
			};
			/**
			 * Поддерживаемые режимы парсинга
			 */
			enum class mode_t : std::uint8_t {
				NONE   = 0x00, // Режим парсинга не установлен
				NATIVE = 0x01, // Режим парсинга установлен как нативный
				REGEXP = 0x02  // Режим парсинга установлен как регулярные выражения
			};
		private:
			/**
			 * RegExp Структура регулярных выражений
			 */
			typedef struct RegExp {
				awh::regexp_t::exp_t date1;   // Регулярное выражение для распознавания формат даты (Sat Jan  8 20:07:41 2011)
				awh::regexp_t::exp_t date2;   // Регулярное выражение для распознавания формат даты (2024-10-04 13:29:47)
				awh::regexp_t::exp_t date3;   // Регулярное выражение для распознавания формат даты (2003-10-11T22:14:15.003Z)
				awh::regexp_t::exp_t rfc3164; // Регулярное выражение для парсинга всего сообщения RFC3164
				awh::regexp_t::exp_t rfc5424; // Регулярное выражение для парсинга части сообщения RFC5424
			} exp_t;
		private:
			// Поддерживаемый стандарт
			std_t _std;
		private:
			// Версия сообщения
			std::uint8_t _ver;
			// Приоритет сообщения
			std::uint16_t _pri;
		private:
			// Режим парсинга
			mode_t _mode;
		private:
			// Название сообщения
			std::string _app;
			// Хост сообщения
			std::string _host;
		private:
			// Идентификатор процесса
			pid_t _pid;
			// Идентификатор сообщения
			std::string _mid;
		private:
			// Текст сообщения
			std::string _message;
		private:
			// Формат даты сообщения
			std::string _format;
			// Штамп времени сообщения
			std::time_t _timestamp;
		private:
			// Объект собранных регулярных выражений
			exp_t _exp;
			// Объект работы с регулярными выражениями
			awh::regexp_t _reg;
		private:
			// Мютекс для блокировки потока
			std::recursive_mutex _mtx;
		private:
			// Список структурированных данных
			std::unordered_map <string, std::unordered_map <string, string>> _sd;
		private:
			// Объект фреймворка
			const awh::fmk_t * _fmk;
			// Объект работы с логами
			const awh::log_t * _log;
		private:
			// Максимальный размер буфера данных на чтение из файла
			static constexpr const char FORMAT[] = "%Y-%m-%dT%H:%M:%S.000Z";
		public:
			/**
			 * clear Метод очистки данных
			 */
			void clear() noexcept;
		public:
			/**
			 * parse Метод парсинга строки в формате SysLog
			 * @param syslog строка в формате SysLog
			 * @param std    стандарт SysLog
			 */
			void parse(const std::string & syslog, const std_t std = std_t::AUTO) noexcept;
		public:
			/**
			 * has Метод проверки существования идентификатора структурированных данных
			 * @param id идентификатор структурированных данных для проверки
			 * @return   результат проверки существования идентификатора структурированных данных
			 */
			bool has(const std::string & id) const noexcept;
			/**
			 * has Метод проверки существования ключа структурированных данных
			 * @param id  идентификатор структурированных данных
			 * @param key ключ структурированных данных для проверки
			 * @return    результат проверки существования структурированных данных
			 */
			bool has(const std::string & id, const std::string & key) const noexcept;
		public:
			/**
			 * sd Метод получения структурированных данных
			 * @param id  идентификатор структурированных данных
			 * @param key ключ структурированных данных для извлечения
			 * @return    структурированные данные
			 */
			const std::string & sd(const std::string & id, const std::string & key) const noexcept;
			/**
			 * sd Метод получения списка структурированных данных
			 * @param id идентификатор структурированных данных
			 * @return   список структурированных данных
			 */
			const std::unordered_map <std::string, std::string> & sd(const std::string & id) const noexcept;
			/**
			 * sd Метод установки структурированных данных
			 * @param id идентификатор структурированных данных
			 * @param sd список структурированных данных
			 */
			void sd(const std::string & id, const std::unordered_map <std::string, std::string> & sd) noexcept;
		public:
			/**
			 * std Метод получения стандарта сообщения
			 * @return стандарт сообщения
			 */
			std_t std() const noexcept;
		public:
			/**
			 * version Метод извлечения версии сообщения
			 * @return версия сообщения
			 */
			std::uint8_t version() const noexcept;
			/**
			 * version Метод установки версии сообщения
			 * @param version версия сообщения для установки
			 */
			void version(const std::uint8_t version) noexcept;
		public:
			/**
			 * category Метод извлечения категории сообщения
			 * @return категория сообщения
			 */
			std::uint8_t category() const noexcept;
			/**
			 * importance Метод получения важности сообщения
			 * @return важность сообщения
			 */
			std::uint8_t importance() const noexcept;
			/**
			 * pri Метод установки приоритета
			 * @param category   категория сообщения для установки
			 * @param importance важность сообщения для установки
			 */
			void pri(const std::uint8_t category, const std::uint8_t importance) noexcept;
		public:
			/**
			 * host Метод получения хоста сообщения
			 * @return хост сообщения
			 */
			std::string host() const noexcept;
			/**
			 * host Метод установки хоста сообщения
			 * @param host хост сообщения для установки
			 */
			void host(const std::string & host) noexcept;
		public:
			/**
			 * application Метод получения названия приложения сообщения
			 * @return название приложения сообщения
			 */
			std::string application() const noexcept;
			/**
			 * application Метод установки названия приложения сообщения
			 * @param app назование приложения для установки
			 */
			void application(const std::string & app) noexcept;
		public:
			/**
			 * pid Метод получения идентификатора процесса сообщения
			 * @return идентификатор процесса сообщения
			 */
			pid_t pid() const noexcept;
			/**
			 * pid Метод установки идентификатора процесса
			 * @param pid идентификатор процесса для установки
			 */
			void pid(const pid_t pid) noexcept;
		public:
			/**
			 * mid Метод получения идентификатора сообщения
			 * @return идентификатор полученного сообщения 
			 */
			std::string mid() const noexcept;
			/**
			 * mid Метод установки идентификатора сообщения
			 * @param mid идентификатор сообщения для установки
			 */
			void mid(const std::string & mid) noexcept;
		public:
			/**
			 * message Метод получения сообщения
			 * @return полученное сообщение
			 */
			std::string message() const noexcept;
			/**
			 * message Метод установки сообщения
			 * @param message сообщение для установки
			 */
			void message(const std::string & message) noexcept;
		public:
			/**
			 * format Метод получения установленного формата даты
			 * @return установленный формат даты
			 */
			std::string format() const noexcept;
			/**
			 * format Метод установки формата даты
			 * @param format формат даты для установки
			 */
			void format(const std::string & format) noexcept;
		public:
			/**
			 * date Метод получения даты сообщения
			 * @param format формат даты сообщения
			 * @return       дата сообщения в указанном формате
			 */
			std::string date(const std::string & format = FORMAT) const noexcept;
			/**
			 * date Метод установки даты сообщения
			 * @param date   дата сообщения для установки
			 * @param format формат даты сообщения для установки
			 */
			void date(const std::string & date, const std::string & format) noexcept;
		public:
			/**
			 * syslog Метод получения данных в формате SysLog
			 * @return данные в формате SysLog
			 */
			std::string syslog() const noexcept;
		public:
			/**
			 * dump Метод извлечения данных в виде JSON
			 * @return json объект дампа данных
			 */
			json dump() const noexcept;
			/**
			 * dump Метод установки данных в формате JSON
			 * @param dump данные в формате JSON
			 */
			void dump(const json & dump) noexcept;
		public:
			/**
			 * mode Метод получения установленного режима парсинга
			 * @return установленный режим парсинга
			 */
			mode_t mode() const noexcept;
			/**
			 * mode Метод установки режима парсинга
			 * @param mode режим парсинга для установки
			 */
			void mode(const mode_t mode) noexcept;
		public:
			/**
			 * Оператор вывода данные контейнера в качестве строки
			 * @return данные контейнера в качестве строки
			 */
			operator std::string() const noexcept;
		public:
			/**
			 * Оператор [!=] сравнения контейнеров
			 * @param syslog контенер для сравнения
			 * @return       результат сравнения
			 */
			bool operator != (const SysLog & syslog) const noexcept;
			/**
			 * Оператор [==] сравнения контейнеров
			 * @param syslog контенер для сравнения
			 * @return       результат сравнения
			 */
			bool operator == (const SysLog & syslog) const noexcept;
		public:
			/**
			 * Оператор [=] присвоения контейнеров
			 * @param syslog контенер для присвоения
			 * @return       текущий объект
			 */
			SysLog & operator = (const SysLog & syslog) noexcept;
			/**
			 * Оператор [=] присвоения контейнеров
			 * @param syslog контенер для присвоения
			 * @return       текущий объект
			 */
			SysLog & operator = (const std::string & syslog) noexcept;
		public:
			/**
			 * SysLog Конструктор
			 * @param fmk объект фреймворка
			 * @param log объект для работы с логами
			 */
			SysLog(const awh::fmk_t * fmk, const awh::log_t * log) noexcept;
	} syslog_t;
	/**
	 * Оператор [>>] чтения из потока SysLog контейнера
	 * @param is     поток для чтения
	 * @param syslog контенер для присвоения
	 */
	ACUSHARED_EXPORT std::istream & operator >> (std::istream & is, syslog_t & syslog) noexcept;
	/**
	 * Оператор [<<] вывода в поток SysLog контейнера
	 * @param os     поток куда нужно вывести данные
	 * @param syslog контенер для присвоения
	 */
	ACUSHARED_EXPORT std::ostream & operator << (std::ostream & os, const syslog_t & syslog) noexcept;
};

#endif // __ANYKS_ACU_SYSLOG__
