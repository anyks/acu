/**
 * @file: csv.hpp
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

#ifndef __ANYKS_ACU_CSV__
#define __ANYKS_ACU_CSV__

/**
 * Разрешаем сборку под Windows
 */
#include <global.hpp>

/**
 * Подключаем зависимые заголовки
 */
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <functional>

/**
 * Модули AWH
 */
#include <sys/fs.hpp>
#include <sys/fmk.hpp>
#include <sys/log.hpp>

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
	 * Активируем пространство имён json
	 */
	using json = rapidjson::Document;
	/**
	 * CSV Класс модуля CSV
	 */
	typedef class ACUSHARED_EXPORT CSV {
		private:
			// Флаг разрешения использования заголовков
			bool _header;
		private:
			// Объект работы с файловой системой
			awh::fs_t _fs;
		private:
			// Схема соответствий ключей расширения
			std::vector <std::vector <std::string>> _mapping;
		private:
			// Объект фреймворка
			const awh::fmk_t * _fmk;
			// Объект работы с логами
			const awh::log_t * _log;
		private:
			// Максимальный размер буфера данных на чтение из файла
			static constexpr std::uintmax_t CSV_BUFFER_SIZE = 0x10000;
		private:
			/**
			 * prepare Метод выполнения препарирования полученных данных строки
			 * @param buffer буфер данных для препарирования
			 * @param size   размер буфера данных для препарирования
			 * @param delim  используемый разделитель
			 */
			void prepare(const char * buffer, const std::size_t size, const char delim = ',') noexcept;
			/**
			 * prepare Метод выполнения препарирования полученных данных строки
			 * @param buffer   буфер данных для препарирования
			 * @param size     размер буфера данных для препарирования
			 * @param callback функция обратного вызова
			 * @param delim    используемый разделитель
			 */
			void prepare(const char * buffer, const std::size_t size, std::function <void (const std::vector <std::string> &)> callback, const char delim = ',') noexcept;
		public:
			/**
			 * clear Метод очистки данных
			 */
			void clear() noexcept;
		public:
			/**
			 * header Метод установки флага использования заголовков
			 * @param mode флаг использования заголовков
			 */
			void header(const bool mode) noexcept;
		public:
			/**
			 * parse Метод выполнения парсинга текста
			 * @param text текст для парсинга
			 */
			void parse(const std::string & text) noexcept;
			/**
			 * parse Метод выполнения парсинга текста
			 * @param text  текст для парсинга
			 * @param delim используемый разделитель
			 */
			void parse(const std::string & text, const char delim) noexcept;
		public:
			/**
			 * cols Метод получения количества столбцов
			 * @return количество столбцов
			 */
			std::size_t cols() const noexcept;
			/**
			 * rows Метод получения количества строк
			 * @return количество строк
			 */
			std::size_t rows() const noexcept;
		public:
			/**
			 * row Метод получения строки
			 * @param index индекс строки
			 * @param delim используемый разделитель
			 * @return      сформированная строка
			 */
			std::string row(const std::size_t index, const char delim = ';') noexcept;
		public:
			/**
			 * write Метод записи данных в файл
			 * @param filename адрес файла контейнера CSV для записи
			 * @param delim    используемый разделитель
			 */
			void write(const std::string & filename, const char delim = ';') noexcept;
		public:
			/**
			 * read Метод чтения данных из файла
			 * @param filename адрес файла контейнера CSV для чтения
			 */
			void read(const std::string & filename) noexcept;
			/**
			 * read Метод чтения данных из файла
			 * @param filename адрес файла контейнера CSV для чтения
			 * @param delim    используемый разделитель
			 */
			void read(const std::string & filename, const char delim) noexcept;
			/**
			 * read Метод чтения данных из файла
			 * @param filename адрес файла контейнера CSV для чтения
			 * @param callback функция обратного вызова
			 * @param delim    используемый разделитель
			 */
			void read(const std::string & filename, std::function <void (const std::vector <std::string> &)> callback, const char delim = '0') noexcept;
		public:
			/**
			 * dump Метод создания дампа данных
			 * @return дамп данных в формате JSON
			 */
			json dump() const noexcept;
			/**
			 * dump Метод установки дампа данных
			 * @param dump дамп данных в формате JSON
			 */
			void dump(const json & dump) noexcept;
		public:
			/**
			 * get Метод извлечения данных контейнера
			 * @return собранные данные контейнера
			 */
			const std::vector <std::vector <std::string>> & get() const noexcept;
		public:
			/**
			 * Оператор вывода данные контейнера в качестве строки
			 * @return данные контейнера в качестве строки
			 */
			operator std::string() const noexcept;
		public:
			/**
			 * Оператор [=] присвоения контейнеров
			 * @param csv контенер для присвоения
			 * @return    текущий объект
			 */
			CSV & operator = (const CSV & csv) noexcept;
			/**
			 * Оператор [=] присвоения контейнеров
			 * @param csv контенер для присвоения
			 * @return    текущий объект
			 */
			CSV & operator = (const std::string & csv) noexcept;
		public:
			/**
			 * CSV Конструктор
			 * @param fmk объект фреймворка
			 * @param log объект для работы с логами
			 */
			CSV(const awh::fmk_t * fmk, const awh::log_t * log) noexcept : _header(false), _fs(fmk, log), _fmk(fmk), _log(log) {}
			/**
			 * ~CSV Деструктор
			 */
			~CSV() noexcept {}
	} csv_t;
	/**
	 * Оператор [>>] чтения из потока CSV контейнера
	 * @param is  поток для чтения
	 * @param csv контенер для присвоения
	 */
	ACUSHARED_EXPORT std::istream & operator >> (std::istream & is, csv_t & csv) noexcept;
	/**
	 * Оператор [<<] вывода в поток CSV контейнера
	 * @param os  поток куда нужно вывести данные
	 * @param csv контенер для присвоения
	 */
	ACUSHARED_EXPORT std::ostream & operator << (std::ostream & os, const csv_t & csv) noexcept;
};

#endif // __ANYKS_ACU_CSV__
