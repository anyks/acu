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
 * @copyright: Copyright © 2025
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
#include <sys/chrono.hpp>

/**
 * Подключаем заголовочные файлы JSON
 */
#include <rapidjson/document.h>

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
	 * SysLog Класс модуля парсера SysLog (RFC3164/RFC5424)
	 */
	typedef class ACUSHARED_EXPORT SysLog {
		public:
			/**
			 * Поддерживаемые стандарты парсинга
			 */
			enum class std_t : uint8_t {
				AUTO    = 0x00, // Стандарт определяется автоматически
				RFC3164 = 0x01, // Стандарт RFC 3164
				RFC5424 = 0x02  // Стандарт RFC 5424
			};
			/**
			 * Поддерживаемые режимы парсинга
			 */
			enum class mode_t : uint8_t {
				NONE   = 0x00, // Режим парсинга не установлен
				NATIVE = 0x01, // Режим парсинга установлен как нативный
				REGEXP = 0x02  // Режим парсинга установлен как регулярные выражения
			};
		private:
			/**
			 * RegExp Структура регулярных выражений
			 */
			typedef struct RegExp {
				regexp_t::exp_t date1;   // Регулярное выражение для распознавания формат даты (Sat Jan  8 20:07:41 2011)
				regexp_t::exp_t date2;   // Регулярное выражение для распознавания формат даты (2024-10-04 13:29:47)
				regexp_t::exp_t date3;   // Регулярное выражение для распознавания формат даты (2003-10-11T22:14:15.003Z)
				regexp_t::exp_t rfc3164; // Регулярное выражение для парсинга всего сообщения RFC3164
				regexp_t::exp_t rfc5424; // Регулярное выражение для парсинга части сообщения RFC5424
			} exp_t;
		private:
			// Поддерживаемый стандарт
			std_t _std;
		private:
			// Версия сообщения
			uint8_t _ver;
			// Приоритет сообщения
			uint16_t _pri;
		private:
			// Режим парсинга
			mode_t _mode;
		private:
			// Название сообщения
			string _app;
			// Хост сообщения
			string _host;
		private:
			// Идентификатор процесса
			pid_t _pid;
			// Идентификатор сообщения
			string _mid;
		private:
			// Текст сообщения
			string _message;
		private:
			// Формат даты сообщения
			string _format;
			// Штамп времени сообщения
			uint64_t _timestamp;
		private:
			// Объект работы с датой и временем
			chrono_t _chrono;
		private:
			// Объект собранных регулярных выражений
			exp_t _exp;
			// Объект работы с регулярными выражениями
			regexp_t _reg;
		private:
			// Мютекс для блокировки потока
			recursive_mutex _mtx;
		private:
			// Список структурированных данных
			unordered_map <string, unordered_map <string, string>> _sd;
		private:
			// Объект фреймворка
			const fmk_t * _fmk;
			// Объект работы с логами
			const log_t * _log;
		private:
			// Максимальный размер буфера данных на чтение из файла
			static constexpr const char FORMAT[] = "%Y-%m-%dT%H:%M:%S.%sZ";
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
			void parse(const string & syslog, const std_t std = std_t::AUTO) noexcept;
		public:
			/**
			 * has Метод проверки существования идентификатора структурированных данных
			 * @param id идентификатор структурированных данных для проверки
			 * @return   результат проверки существования идентификатора структурированных данных
			 */
			bool has(const string & id) const noexcept;
			/**
			 * has Метод проверки существования ключа структурированных данных
			 * @param id  идентификатор структурированных данных
			 * @param key ключ структурированных данных для проверки
			 * @return    результат проверки существования структурированных данных
			 */
			bool has(const string & id, const string & key) const noexcept;
		public:
			/**
			 * sd Метод получения структурированных данных
			 * @param id  идентификатор структурированных данных
			 * @param key ключ структурированных данных для извлечения
			 * @return    структурированные данные
			 */
			const string & sd(const string & id, const string & key) const noexcept;
			/**
			 * sd Метод получения списка структурированных данных
			 * @param id идентификатор структурированных данных
			 * @return   список структурированных данных
			 */
			const unordered_map <string, string> & sd(const string & id) const noexcept;
			/**
			 * sd Метод установки структурированных данных
			 * @param id идентификатор структурированных данных
			 * @param sd список структурированных данных
			 */
			void sd(const string & id, const unordered_map <string, string> & sd) noexcept;
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
			uint8_t version() const noexcept;
			/**
			 * version Метод установки версии сообщения
			 * @param version версия сообщения для установки
			 */
			void version(const uint8_t version) noexcept;
		public:
			/**
			 * category Метод извлечения категории сообщения
			 * @return категория сообщения
			 */
			uint8_t category() const noexcept;
			/**
			 * importance Метод получения важности сообщения
			 * @return важность сообщения
			 */
			uint8_t importance() const noexcept;
			/**
			 * pri Метод установки приоритета
			 * @param category   категория сообщения для установки
			 * @param importance важность сообщения для установки
			 */
			void pri(const uint8_t category, const uint8_t importance) noexcept;
		public:
			/**
			 * host Метод получения хоста сообщения
			 * @return хост сообщения
			 */
			string host() const noexcept;
			/**
			 * host Метод установки хоста сообщения
			 * @param host хост сообщения для установки
			 */
			void host(const string & host) noexcept;
		public:
			/**
			 * application Метод получения названия приложения сообщения
			 * @return название приложения сообщения
			 */
			string application() const noexcept;
			/**
			 * application Метод установки названия приложения сообщения
			 * @param app назование приложения для установки
			 */
			void application(const string & app) noexcept;
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
			string mid() const noexcept;
			/**
			 * mid Метод установки идентификатора сообщения
			 * @param mid идентификатор сообщения для установки
			 */
			void mid(const string & mid) noexcept;
		public:
			/**
			 * message Метод получения сообщения
			 * @return полученное сообщение
			 */
			string message() const noexcept;
			/**
			 * message Метод установки сообщения
			 * @param message сообщение для установки
			 */
			void message(const string & message) noexcept;
		public:
			/**
			 * format Метод получения установленного формата даты
			 * @return установленный формат даты
			 */
			string format() const noexcept;
			/**
			 * format Метод установки формата даты
			 * @param format формат даты для установки
			 */
			void format(const string & format) noexcept;
		public:
			/**
			 * date Метод получения даты сообщения
			 * @param format формат даты сообщения
			 * @return       дата сообщения в указанном формате
			 */
			string date(const string & format = FORMAT) const noexcept;
			/**
			 * date Метод установки даты сообщения
			 * @param date   дата сообщения для установки
			 * @param format формат даты сообщения для установки
			 */
			void date(const string & date, const string & format) noexcept;
		public:
			/**
			 * syslog Метод получения данных в формате SysLog
			 * @return данные в формате SysLog
			 */
			string syslog() const noexcept;
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
			operator string() const noexcept;
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
			SysLog & operator = (const string & syslog) noexcept;
		public:
			/**
			 * SysLog Конструктор
			 * @param fmk объект фреймворка
			 * @param log объект для работы с логами
			 */
			SysLog(const fmk_t * fmk, const log_t * log) noexcept;
	} syslog_t;
	/**
	 * Оператор [>>] чтения из потока SysLog контейнера
	 * @param is     поток для чтения
	 * @param syslog контенер для присвоения
	 */
	ACUSHARED_EXPORT istream & operator >> (istream & is, syslog_t & syslog) noexcept;
	/**
	 * Оператор [<<] вывода в поток SysLog контейнера
	 * @param os     поток куда нужно вывести данные
	 * @param syslog контенер для присвоения
	 */
	ACUSHARED_EXPORT ostream & operator << (ostream & os, const syslog_t & syslog) noexcept;
};

#endif // __ANYKS_ACU_SYSLOG__
