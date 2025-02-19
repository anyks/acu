
/**
 * @file: acu.cpp
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

/**
 * Подключаем модули парсера
 */
#include <lib.hpp>
#include <env.hpp>
#include <parser.hpp>

/**
 * Подключаем модуль файловой системы
 */
#include <sys/fs.hpp>
#include <sys/hash.hpp>

/**
 * Подписываемся на пространство имён anyks
 */
using namespace anyks;

/**
 * help Функция вывода справки
 * @param name название приложения
 */
static void help(const string & name) noexcept {
	// Если название приложения получено
	if(!name.empty()){
		// Формируем строку справки
		const string msg = "\r\n\x1B[32m\x1B[1musage:\x1B[0m %s [-V | --version] [-H | --info] [<args>]\r\n\r\n\r\n"
		"\x1B[34m\x1B[1m[FLAGS]\x1B[0m\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m Flag for convert notation: \x1B[1m[-notation | --notation]\x1B[0m\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m Flag for generating headers when parsing CSV files: \x1B[1m[-header | --header]\x1B[0m\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m Flag for generating a readable file format (XML or JSON): \x1B[1m[-prettify | --prettify]\x1B[0m\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m Display application version: \x1B[1m[-version | --version | -V]\x1B[0m\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m Display information about available application functions: \x1B[1m[-info | --info | -H]\x1B[0m\r\n\r\n"
		"\x1B[34m\x1B[1m[ARGS]\x1B[0m\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m CEF file parsing mode: \x1B[1m[-cef <value> | --cef=<value>]\x1B[0m\r\n"
		"\x1B[32m\x1B[1m  -\x1B[0m (LOW | MEDIUM | STRONG)\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m Format of the file from which reading is performed: \x1B[1m[-from <value> | --from=<value>]\x1B[0m\r\n"
		"\x1B[32m\x1B[1m  -\x1B[0m (XML | JSON | INI | YAML | CSV | CEF | SYSLOG | GROK | TEXT | BASE64)\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m File format to which the writing is made: \x1B[1m[-to <value> | --to=<value>]\x1B[0m\r\n"
		"\x1B[32m\x1B[1m  -\x1B[0m (XML | JSON | INI | YAML | CSV | CEF | SYSLOG | TEXT | BASE64 | MD5 | SHA1 | SHA224 | SHA256 | SHA384 | SHA512)\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m Format date (if required): \x1B[1m[-formatDate \"<value>\" | --formatDate=\"<value>\"]\x1B[0m\r\n"
		"\x1B[32m\x1B[1m  -\x1B[0m ( %%m/%%d/%%Y %%H:%%M:%%S | %%H:%%M:%%S %%d.%%m.%%Y | ... )\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m Logging level (if required): \x1B[1m[-logLevel <value> | --logLevel=<value>]\x1B[0m\r\n"
		"\x1B[32m\x1B[1m  -\x1B[0m ( 0 = NONE | 1 = INFO | 2 = WARNING | 3 = CRITICAL | 4 = INFO and WARNING | 5 = INFO and CRITICAL | 6 = WARNING CRITICAL | 7 = ALL)\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m Hash-based message authentication code: \x1B[1m[-hmac <value> | --hmac=<value>]\x1B[0m\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m File address for writing logs (if required): \x1B[1m[-log <value> | --log=<value>]\x1B[0m\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m File or directory address for saving converted files: \x1B[1m[-dest <value> | --dest=<value>]\x1B[0m\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m Separator for parsing CSV files (default: \";\"): \x1B[1m[-delim <value> | --delim=<value>]\x1B[0m\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m Address of the file or directory with files to convert: \x1B[1m[-src <value> | --src=<value>]\x1B[0m\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m Address of the file in JSON format with GROK templates: \x1B[1m[-patterns <value> | --patterns=<value>]\x1B[0m\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m Address of the file with the regular expression in GROK format: \x1B[1m[-express <value> | --express=<value>]\x1B[0m\r\n\r\n";
		// Выводим сообщение справки
		printf(msg.c_str(), name.c_str());
	}
}
/**
 * version Функция вывода версии приложения
 * @param fmk     объект фреймворка
 * @param log     объект для работы с логами
 * @param fs      объект работы с файловой системой
 * @param address адрес приложения
 */
static void version(const fmk_t * fmk, const log_t * log, const fs_t * fs, const string & address) noexcept {
	// Если входящие данные переданы
	if((fmk != nullptr) && (log != nullptr) && (fs != nullptr) && !address.empty()){
		// Название операционной системы
		const char * os = nullptr;
		// Определяем название операционной системы
		switch(static_cast <uint8_t> (os_t().type())){
			// Если операционной системой является Unix
			case static_cast <uint8_t> (os_t::type_t::UNIX):
				// Устанавливаем название Операционной Системы
				os = "Unix";
			break;
			// Если операционной системой является Linux
			case static_cast <uint8_t> (os_t::type_t::LINUX):
				// Устанавливаем название Операционной Системы
				os = "Linux";
			break;
			// Если операционной системой является неизвестной
			case static_cast <uint8_t> (os_t::type_t::NONE):
				// Устанавливаем название Операционной Системы
				os = "Unknown";
			break;
			// Если операционной системой является Windows
			case static_cast <uint8_t> (os_t::type_t::WIND32):
			case static_cast <uint8_t> (os_t::type_t::WIND64):
				// Устанавливаем название Операционной Системы
				os = "Windows";
			break;
			// Если операционной системой является MacOS X
			case static_cast <uint8_t> (os_t::type_t::MACOSX):
				// Устанавливаем название Операционной Системы
				os = "MacOS X";
			break;
			// Если операционной системой является FreeBSD
			case static_cast <uint8_t> (os_t::type_t::FREEBSD):
				// Устанавливаем название Операционной Системы
				os = "FreeBSD";
			break;
		}
		// Если операционная система обнаружена
		if(os != nullptr){
			// Позиция в каталоге
			size_t pos = 0;
			// Определяем адрес приложения
			string app = fs->realPath(address);
			// Ищем каталог
			if((pos = app.rfind(FS_SEPARATOR)) != string::npos)
				// Получаем название приложения
				app = app.substr(0, pos);
			// Выводим версию приложения
			printf(
				"\r\n%s %s (built: %s %s)\r\n"
				"awh: %s\r\n"
				"target: %s\r\n"
				"installed dir: %s\r\n\r\n*\r\n"
				"* site:     %s\r\n"
				"* email:    %s\r\n"
				"* telegram: %s\r\n*\r\n\r\n",
				ACU_NAME,
				ACU_VERSION,
				__DATE__,
				__TIME__,
				AWH_VERSION,
				os, app.c_str(),
				ACU_SITE,
				ACU_EMAIL,
				ACU_CONTACT
			);
		// Выводим сообщение об ошибке
		} else log->print("Operating system is not identified", log_t::flag_t::CRITICAL);
	}
}
/**
 * Выполняем работу для Windows
 */
#if defined(_WIN32) || defined(_WIN64)
	/**
	 * main Главная функция приложения
	 * @param count  длина массива параметров
	 * @param params массив параметров
	 * @return       код выхода из приложения
	 */
	int32_t wmain(int32_t count, const wchar_t * params[]) noexcept {
/**
 * Выполняем работу для Unix
 */
#else
	/**
	 * main Главная функция приложения
	 * @param count  длина массива параметров
	 * @param params массив параметров
	 * @return       код выхода из приложения
	 */
	int32_t main(int32_t count, const char * params[]) noexcept {
#endif
		// Создаём объект фреймворка
		fmk_t fmk;
		// Создаём объект для работы с логами
		log_t log(&fmk);
		// Объект работы с файловой системой
		fs_t fs(&fmk, &log);
		// Создаём объект переменных окружения
		env_t env(ACU_SHORT_NAME, "text", &fmk, &log);
		// Устанавливаем название сервиса
		log.name(ACU_SHORT_NAME);
		// Название приложения и текстовое значение полученное из потока
		string name = "", text = "";
		// Создаём формат даты
		string formatDate = DATE_FORMAT;
		// Считываем строку из буфера stdin
		if(!::isatty(STDIN_FILENO)){
			// Результат полученный из потока
			string result = "";
			// Выполняем чтение данных из потока
			while(std::getline(std::cin, result) && !result.empty()){
				// Если результат сформирован
				if(!text.empty())
					// Добавляем разделитель строки
					text.append(1, '\n');
				// Формируем текст полученного результата
				text.append(result);
			}
		}
		/**
		 * Выполняем работу для Windows
		 */
		#if defined(_WIN32) || defined(_WIN64)
			{
				// Получаем компоненты адреса приложения
				const auto & components = fs.components(fmk.convert(wstring{params[0]}), false);
				// Если расширение файла установлено а названия нет, выполняем корректировку
				if(!components.second.empty() && components.first.empty()){
					// Устанавливаем название файла
					name.append(1, '.');
					// Устанавливаем основное название файла
					name.append(components.second);
				// Если название файла и расширение получено
				} else if(!components.first.empty() && !components.second.empty())
					// Формируем название приложения
					name = fmk.format("%s.%s", components.first.c_str(), components.second.c_str());
				// Устанавливаем название файла как оно есть
				else name = components.first;
				// Выполняем инициализацию переданных параметров
				env.init(reinterpret_cast <const wchar_t **> (params), static_cast <uint8_t> (count));
			}
		/**
		 * Выполняем работу для Unix
		 */
		#else
			{
				// Получаем компоненты адреса приложения
				const auto & components = fs.components(string{params[0]}, false);
				// Если расширение файла установлено а названия нет, выполняем корректировку
				if(!components.second.empty() && components.first.empty()){
					// Устанавливаем название файла
					name.append(1, '.');
					// Устанавливаем основное название файла
					name.append(components.second);
				// Если название файла и расширение получено
				} else if(!components.first.empty() && !components.second.empty())
					// Формируем название приложения
					name = fmk.format("%s.%s", components.first.c_str(), components.second.c_str());
				// Устанавливаем название файла как оно есть
				else name = components.first;
				// Выполняем инициализацию переданных параметров
				env.init(reinterpret_cast <const char **> (params), static_cast <uint8_t> (count));
			}
		#endif
		// Если формат вывода лога передан
		if(env.isString(true, "formatDate"))
			// Получаем формат вывода даты
			formatDate = env.get <string> (true, "formatDate");
		// Устанавливаем формат вывода даты
		log.format(formatDate);
		// Если адрес файла лога передан
		if(env.isString(true, "log")){
			// Получаем адрес файла лога
			const string & filename = env.get <string> (true, "log");
			// Если адрес файла лога получен
			if(!filename.empty()){
				// Позиция разделителя каталога
				size_t pos = 0;
				// Выполняем поиск разделителя каталога
				if((pos = string(filename).rfind(FS_SEPARATOR)) != string::npos){
					// Извлекаем путь сохранения файла лога
					const string & path = fs.realPath(filename.substr(0, pos));
					// Если путь для сохранения каталога не существует
					if(!path.empty() && !fs.isDir(path))
						// Выполняем создание адреса каталога
						fs.makePath(path);
				}
				// Устанавливаем адрес файла лога
				log.filename(fs.realPath(filename));
			}
		}
		// Если нужно вывести справочную помощь
		if((env.count(true) == 0) || env.is(false, "info") || env.is(false, "H")){
			// Выполняем загрузку конфигурационного файла
			if(env.isString(false, "config")){
				// Получаем параметр конфигурационного файла
				const string configFile = env.get <string> (false, "config");
				// Если конфигурационный файл получен
				if(!configFile.empty())
					// Выполняем извлечение данных конфигурационного файла
					env.filename(configFile);
			}
			// Выводим справочную информацию
			help(name);
			// Выходим из приложения
			::exit(EXIT_SUCCESS);
		// Если версия получена
		} else if(env.is(false, "version") || env.is(false, "V")) {
			/**
			 * Выполняем работу для Windows
			 */
			#if defined(_WIN32) || defined(_WIN64)
				// Выводим версию приложения
				version(&fmk, &log, &fs, fmk.convert(wstring(params[0])));
			/**
			 * Выполняем работу для Unix
			 */
			#else
				// Выводим версию приложения
				version(&fmk, &log, &fs, params[0]);
			#endif
			// Выходим из приложения
			::exit(EXIT_SUCCESS);
		}
		// Если уровень логирования передан
		if(env.isUint(false, "logLevel"))
			// Выполняем установку уровня логирования
			log.level(static_cast <log_t::level_t> (env.get <uint32_t> (false, "logLevel")));
		// Если уровень логирования прописан в конфигурационном файле
		else if(env.isUint(true, "logLevel"))
			// Выполняем установку уровня логирования из конфигурационного файла
			log.level(static_cast <log_t::level_t> (env.get <uint32_t> (true, "logLevel")));
		// Если указанны форматы конвертирования
		if(env.isString(false, "from") && env.isString(false, "to")){
			// Регулярное выражение в формате GROK
			string express = "";
			// Выполняем инициализацию объекта парсера
			parser_t parser(&fmk, &log);
			// Тип рабочего формата данных
			enum class type_t : uint8_t {
				TEXT        = 0x00, // Тип формата - текстовый
				XML         = 0x01, // Тип формата - XML
				INI         = 0x02, // Тип формата - INI
				CEF         = 0x03, // Тип формата - CEF
				CSV         = 0x04, // Тип формата - CSV
				JSON        = 0x05, // Тип формата - JSON
				YAML        = 0x06, // Тип формата - YAML
				GROK        = 0x07, // Тип формата - GROK
				SYSLOG      = 0x08, // Тип формата - SYSLOG
				BASE64      = 0x09, // Тип формата - BASE64
				MD5         = 0x0A, // Тип формата - MD5
				SHA1        = 0x0B, // Тип формата - SHA1
				SHA224      = 0x0C, // Тип формата - SHA224
				SHA256      = 0x0D, // Тип формата - SHA256
				SHA384      = 0x0E, // Тип формата - SHA384
				SHA512      = 0x0F, // Тип формата - SHA512
				HMAC_MD5    = 0x10, // Тип формата - HMAC MD5
				HMAC_SHA1   = 0x11, // Тип формата - HMAC SHA1
				HMAC_SHA224 = 0x12, // Тип формата - HMAC SHA224
				HMAC_SHA256 = 0x13, // Тип формата - HMAC SHA256
				HMAC_SHA384 = 0x14, // Тип формата - HMAC SHA384
				HMAC_SHA512 = 0x15  // Тип формата - HMAC SHA512
			};
			// Тип конвертируемого формата данных и тип формата для конвертации
			type_t from = type_t::TEXT, to = type_t::TEXT;
			// Если формат входящих данных указан как Text
			if(fmk.compare("text", env.get <string> (false, "from")))
				// Определяем формат данных
				from = type_t::TEXT;
			// Если формат входящих данных указан как XML
			else if(fmk.compare("xml", env.get <string> (false, "from")))
				// Определяем формат данных
				from = type_t::XML;
			// Если формат входящих данных указан как JSON
			else if(fmk.compare("json", env.get <string> (false, "from")))
				// Определяем формат данных
				from = type_t::JSON;
			// Если формат входящих данных указан как INI
			else if(fmk.compare("ini", env.get <string> (false, "from")))
				// Определяем формат данных
				from = type_t::INI;
			// Если формат входящих данных указан как YAML
			else if(fmk.compare("yaml", env.get <string> (false, "from")))
				// Определяем формат данных
				from = type_t::YAML;
			// Если формат входящих данных указан как CEF
			else if(fmk.compare("cef", env.get <string> (false, "from")))
				// Определяем формат данных
				from = type_t::CEF;
			// Если формат входящих данных указан как CSV
			else if(fmk.compare("csv", env.get <string> (false, "from")))
				// Определяем формат данных
				from = type_t::CSV;
			// Если формат входящих данных указан как GROK
			else if(fmk.compare("grok", env.get <string> (false, "from"))) {
				// Определяем формат данных
				from = type_t::GROK;
				// Если файл шаблона указан
				if(env.isString(false, "patterns")){
					// Объект шаблонов
					json patterns;
					// Выполяем получение текста шаблонов
					const auto & buffer = fs.read(env.get <string> (false, "patterns"));
					// Выполняем парсинг полученных текстовых данных
					if(patterns.Parse(buffer.data(), buffer.size()).HasParseError()){
						/**
						 * Если включён режим отладки
						 */
						#if defined(DEBUG_MODE)
							// Выводим сообщение об ошибке
							log.debug("Patterns JSON: (offset %d): %s", __PRETTY_FUNCTION__, {}, log_t::flag_t::CRITICAL, patterns.GetErrorOffset(), GetParseError_En(patterns.GetParseError()));
						/**
						* Если режим отладки не включён
						*/
						#else
							// Выводим сообщение об ошибке
							log.print("Patterns JSON: (offset %d): %s", log_t::flag_t::CRITICAL, patterns.GetErrorOffset(), GetParseError_En(patterns.GetParseError()));
						#endif
					}
					// Если данные шаблонов получены
					if(patterns.IsObject() && !patterns.ObjectEmpty())
						// Выполняем добавление поддерживаемых шаблонов
						parser.patterns(patterns);
				}
				// Если регулярное выражение передано
				if(env.isString(false, "express")){
					// Выполняем чтение данных регулярного выражения
					const auto & buffer = fs.read(env.get <string> (false, "express"));
					// Если регулярное выражение не получено
					if(buffer.empty()){
						// Выводим сообщение об ошибке
						log.print("Regular expression in GROK format is not set", log_t::flag_t::CRITICAL);
						// Выводим удачное завершение работы
						return EXIT_FAILURE;
					// Формируем полученное регулярное выражение
					} else express.assign(buffer.begin(), buffer.end());
				// Выводим сообщение об ошибке
				} else {
					// Выводим сообщение об ошибке
					log.print("Regular expression in GROK format is not set", log_t::flag_t::CRITICAL);
					// Выводим удачное завершение работы
					return EXIT_FAILURE;
				}
			// Если формат входящих данных указан как SysLog
			} else if(fmk.compare("syslog", env.get <string> (false, "from")))
				// Определяем формат данных
				from = type_t::SYSLOG;
			// Если формат входящих данных указан как BASE64
			else if(fmk.compare("base64", env.get <string> (false, "from")))
				// Определяем формат данных
				from = type_t::BASE64;
			// Если формат не определён
			else {
				// Выводим сообщение об ошибке
				log.print("From file format is not recognized", log_t::flag_t::CRITICAL);
				// Выводим удачное завершение работы
				return EXIT_FAILURE;
			}
			// Если формат исходящих данных указан как Text
			if(fmk.compare("text", env.get <string> (false, "to")))
				// Определяем формат данных
				to = type_t::TEXT;
			// Если формат исходящих данных указан как XML
			else if(fmk.compare("xml", env.get <string> (false, "to")))
				// Определяем формат данных
				to = type_t::XML;
			// Если формат исходящих данных указан как JSON
			else if(fmk.compare("json", env.get <string> (false, "to")))
				// Определяем формат данных
				to = type_t::JSON;
			// Если формат исходящих данных указан как INI
			else if(fmk.compare("ini", env.get <string> (false, "to")))
				// Определяем формат данных
				to = type_t::INI;
			// Если формат исходящих данных указан как YAML
			else if(fmk.compare("yaml", env.get <string> (false, "to")))
				// Определяем формат данных
				to = type_t::YAML;
			// Если формат исходящих данных указан как CEF
			else if(fmk.compare("cef", env.get <string> (false, "to")))
				// Определяем формат данных
				to = type_t::CEF;
			// Если формат исходящих данных указан как CSV
			else if(fmk.compare("csv", env.get <string> (false, "to")))
				// Определяем формат данных
				to = type_t::CSV;
			// Если формат исходящих данных указан как SysLog
			else if(fmk.compare("syslog", env.get <string> (false, "to")))
				// Определяем формат данных
				to = type_t::SYSLOG;
			// Если формат исходящих данных указан как BASE64
			else if(fmk.compare("base64", env.get <string> (false, "to")))
				// Определяем формат данных
				to = type_t::BASE64;
			// Если формат исходящих данных указан как MD5
			else if(fmk.compare("md5", env.get <string> (false, "to")))
				// Определяем формат данных
				to = type_t::MD5;
			// Если формат исходящих данных указан как SHA1
			else if(fmk.compare("sha1", env.get <string> (false, "to")))
				// Определяем формат данных
				to = type_t::SHA1;
			// Если формат исходящих данных указан как SHA224
			else if(fmk.compare("sha224", env.get <string> (false, "to")))
				// Определяем формат данных
				to = type_t::SHA224;
			// Если формат исходящих данных указан как SHA256
			else if(fmk.compare("sha256", env.get <string> (false, "to")))
				// Определяем формат данных
				to = type_t::SHA256;
			// Если формат исходящих данных указан как SHA384
			else if(fmk.compare("sha384", env.get <string> (false, "to")))
				// Определяем формат данных
				to = type_t::SHA384;
			// Если формат исходящих данных указан как SHA512
			else if(fmk.compare("sha512", env.get <string> (false, "to")))
				// Определяем формат данных
				to = type_t::SHA512;
			// Если формат не определён
			else {
				// Выводим сообщение об ошибке
				log.print("To file format is not recognized", log_t::flag_t::CRITICAL);
				// Выводим удачное завершение работы
				return EXIT_FAILURE;
			}
			// Получаем ключ HMAC
			string hmac = "";
			// Если ключ проверки подлинности передан
			if(env.isString(false, "hmac")){
				// Получаем ключ подтверждения подлинности
				hmac = env.get <string> (false, "hmac");
				// Если ключ подтверждения подлинности получен
				if(!hmac.empty()){
					// Проверяем формат данных для конвертации
					switch(static_cast <uint8_t> (to)){
						// Если формат исходящих данных указан как MD5
						case static_cast <uint8_t> (type_t::MD5):
							// Определяем формат данных
							to = type_t::HMAC_MD5;
						break;
						// Если формат исходящих данных указан как SHA1
						case static_cast <uint8_t> (type_t::SHA1):
							// Определяем формат данных
							to = type_t::HMAC_SHA1;
						break;
						// Если формат исходящих данных указан как SHA224
						case static_cast <uint8_t> (type_t::SHA224):
							// Определяем формат данных
							to = type_t::HMAC_SHA224;
						break;
						// Если формат исходящих данных указан как SHA256
						case static_cast <uint8_t> (type_t::SHA256):
							// Определяем формат данных
							to = type_t::HMAC_SHA256;
						break;
						// Если формат исходящих данных указан как SHA384
						case static_cast <uint8_t> (type_t::SHA384):
							// Определяем формат данных
							to = type_t::HMAC_SHA384;
						break;
						// Если формат исходящих данных указан как SHA512
						case static_cast <uint8_t> (type_t::SHA512):
							// Определяем формат данных
							to = type_t::HMAC_SHA512;
						break;
					}
				}
			}
			// Если данные прочитаны из потока
			if(!text.empty()){
				// Объект в формате JSON
				json result(kObjectType);
				// Определяем формат данных
				switch(static_cast <uint8_t> (from)){
					// Если формат входящих данных указан как Text
					case static_cast <uint8_t> (type_t::TEXT):
						// Выполняем передачу данных как она есть
						result.SetString(text.c_str(), text.length(), result.GetAllocator());
					break;
					// Если формат входящих данных указан как XML
					case static_cast <uint8_t> (type_t::XML):
						// Выполняем конвертацию данных
						result = parser.xml(text);
					break;
					// Если формат входящих данных указан как JSON
					case static_cast <uint8_t> (type_t::JSON):
						// Выполняем конвертацию данных
						result = parser.json(text);
					break;
					// Если формат входящих данных указан как INI
					case static_cast <uint8_t> (type_t::INI):
						// Выполняем конвертацию данных
						result = parser.ini(text);
					break;
					// Если формат входящих данных указан как YAML
					case static_cast <uint8_t> (type_t::YAML):
						// Выполняем конвертацию данных
						result = parser.yaml(text);
					break;
					// Если формат входящих данных указан как CEF
					case static_cast <uint8_t> (type_t::CEF): {
						// Режим парсинга CEF по умолчанию
						cef_t::mode_t mode = cef_t::mode_t::STRONG;
						// Если режим парсинга CEF передан
						if(env.isString(false, "cef")){
							// Получаем режим парсинга
							const string & cef = env.get <string> (false, "cef");
							// Если режим парсинга установлен как строгий
							if(fmk.compare("strong", cef))
								// Устанавливаем строгий режим парсинга
								mode = cef_t::mode_t::STRONG;
							// Если режим парсинга установлен как простой
							else if(fmk.compare("low", cef))
								// Устанавливаем простой режим парсинга
								mode = cef_t::mode_t::LOW;
							// Если режим парсинга установлен как средний
							else if(fmk.compare("medium", cef))
								// Устанавливаем средний режим парсинга
								mode = cef_t::mode_t::MEDIUM;
						}
						// Выполняем конвертацию данных
						result = parser.cef(text, mode);
					} break;
					// Если формат входящих данных указан как CSV
					case static_cast <uint8_t> (type_t::CSV):
						// Выполняем конвертацию данных
						result = parser.csv(text, env.isBoolean(false, "header"));
					break;
					// Если формат входящих данных указан как GROK
					case static_cast <uint8_t> (type_t::GROK):
						// Выполняем конвертацию данных
						result = parser.grok(text, express);
					break;
					// Если формат входящих данных указан как SysLog
					case static_cast <uint8_t> (type_t::SYSLOG):
						// Выполняем конвертацию данных
						result = parser.syslog(text);
					break;
					// Если формат входящих данных указан как BASE64
					case static_cast <uint8_t> (type_t::BASE64): {
						// Выполняем декодирование хэша BASE64
						const string & hash = hash_t(&log).decode <string> (text.data(), text.size(), hash_t::cipher_t::BASE64);
						// Выполняем установку полученного хэша BASE64
						result.SetString(hash.c_str(), hash.length(), result.GetAllocator());
					} break;
				}
				// Если результат получен
				if(!result.IsNull()){
					// Если указан адрес каталога назначения
					if(env.isString(false, "dest")){
						// Получаем адрес каталога
						const string & addr = env.get <string> (false, "dest");
						// Если адрес каталога получен
						if(!addr.empty()){
							// Адрес файла для сохранения
							string filename = addr;
							// Если каталог не существует
							if(fs.isDir(addr)){
								// Получаем расширение файла
								const string extension = env.get <string> (false, "to");
								// Переводим расширение в нижний регистр
								fmk.transform(extension, fmk_t::transform_t::LOWER);
								// Выполняем создание адреса файла для сохранения
								filename = fmk.format("%s%s%s.%s", addr.c_str(), FS_SEPARATOR, "result", extension.c_str());
							}
							// Буфер данных для записи
							string buffer = "";
							// Определяем формат данных
							switch(static_cast <uint8_t> (to)){
								// Если формат входящих данных указан как Text
								case static_cast <uint8_t> (type_t::TEXT):
									// Выполняем вывод текст как он есть
									buffer = result.GetString();
								break;
								// Если формат входящих данных указан как XML
								case static_cast <uint8_t> (type_t::XML):
									// Выполняем конвертирование в формат XML
									buffer = parser.xml(result, env.isBoolean(false, "prettify"));
								break;
								// Если формат входящих данных указан как JSON
								case static_cast <uint8_t> (type_t::JSON):
									// Выполняем конвертирование в формат JSON
									buffer = parser.json(result, env.isBoolean(false, "prettify"));
								break;
								// Если формат входящих данных указан как INI
								case static_cast <uint8_t> (type_t::INI):
									// Выполняем конвертирование в формат INI
									buffer = parser.ini(result);
								break;
								// Если формат входящих данных указан как YAML
								case static_cast <uint8_t> (type_t::YAML):
									// Выполняем конвертирование в формат YAML
									buffer = parser.yaml(result);
								break;
								// Если формат входящих данных указан как CEF
								case static_cast <uint8_t> (type_t::CEF): {
									// Режим парсинга CEF по умолчанию
									cef_t::mode_t mode = cef_t::mode_t::STRONG;
									// Если режим парсинга CEF передан
									if(env.isString(false, "cef")){
										// Получаем режим парсинга
										const string & cef = env.get <string> (false, "cef");
										// Если режим парсинга установлен как строгий
										if(fmk.compare("strong", cef))
											// Устанавливаем строгий режим парсинга
											mode = cef_t::mode_t::STRONG;
										// Если режим парсинга установлен как простой
										else if(fmk.compare("low", cef))
											// Устанавливаем простой режим парсинга
											mode = cef_t::mode_t::LOW;
										// Если режим парсинга установлен как средний
										else if(fmk.compare("medium", cef))
											// Устанавливаем средний режим парсинга
											mode = cef_t::mode_t::MEDIUM;
									}
									// Выполняем конвертирование в формат CEF
									buffer = parser.cef(result, mode);
								} break;
								// Если формат входящих данных указан как CSV
								case static_cast <uint8_t> (type_t::CSV):
									// Выполняем конвертирование в формат CSV
									buffer = parser.csv(result, env.isBoolean(false, "header"), (env.isString(false, "delim") ? env.get <string> (false, "delim").front() : ';'));
								break;
								// Если формат входящих данных указан как SysLog
								case static_cast <uint8_t> (type_t::SYSLOG):
									// Выполняем конвертирование в формат SysLog
									buffer = parser.syslog(result);
								break;
								// Если формат входящих данных указан как BASE64
								case static_cast <uint8_t> (type_t::BASE64): {
									// Выполняем получение текста для шифрования
									const string data = result.GetString();
									// Выполняем конвертирование в формат BASE64
									hash_t(&log).encode(data.c_str(), data.size(), hash_t::cipher_t::BASE64, buffer);
								} break;
								// Если формат входящих данных указан как MD5
								case static_cast <uint8_t> (type_t::MD5):
									// Выполняем конвертирование в формат MD5
									hash_t(&log).hashing(result.GetString(), hash_t::type_t::MD5, buffer);
								break;
								// Если формат входящих данных указан как SHA1
								case static_cast <uint8_t> (type_t::SHA1):
									// Выполняем конвертирование в формат SHA1
									hash_t(&log).hashing(result.GetString(), hash_t::type_t::SHA1, buffer);
								break;
								// Если формат входящих данных указан как SHA224
								case static_cast <uint8_t> (type_t::SHA224):
									// Выполняем конвертирование в формат SHA224
									hash_t(&log).hashing(result.GetString(), hash_t::type_t::SHA224, buffer);
								break;
								// Если формат входящих данных указан как SHA256
								case static_cast <uint8_t> (type_t::SHA256):
									// Выполняем конвертирование в формат SHA256
									hash_t(&log).hashing(result.GetString(), hash_t::type_t::SHA256, buffer);
								break;
								// Если формат входящих данных указан как SHA384
								case static_cast <uint8_t> (type_t::SHA384):
									// Выполняем конвертирование в формат SHA384
									hash_t(&log).hashing(result.GetString(), hash_t::type_t::SHA384, buffer);
								break;
								// Если формат входящих данных указан как SHA512
								case static_cast <uint8_t> (type_t::SHA512):
									// Выполняем конвертирование в формат SHA512
									hash_t(&log).hashing(result.GetString(), hash_t::type_t::SHA512, buffer);
								break;
								// Если формат входящих данных указан как HMAC MD5
								case static_cast <uint8_t> (type_t::HMAC_MD5):
									// Выполняем конвертирование в формат HMAC MD5
									hash_t(&log).hmac(hmac, result.GetString(), hash_t::type_t::MD5, buffer);
								break;
								// Если формат входящих данных указан как HMAC SHA1
								case static_cast <uint8_t> (type_t::HMAC_SHA1):
									// Выполняем конвертирование в формат HMAC SHA1
									hash_t(&log).hmac(hmac, result.GetString(), hash_t::type_t::SHA1, buffer);
								break;
								// Если формат входящих данных указан как HMAC SHA224
								case static_cast <uint8_t> (type_t::HMAC_SHA224):
									// Выполняем конвертирование в формат HMAC SHA224
									hash_t(&log).hmac(hmac, result.GetString(), hash_t::type_t::SHA224, buffer);
								break;
								// Если формат входящих данных указан как HMAC SHA256
								case static_cast <uint8_t> (type_t::HMAC_SHA256):
									// Выполняем конвертирование в формат HMAC SHA256
									hash_t(&log).hmac(hmac, result.GetString(), hash_t::type_t::SHA256, buffer);
								break;
								// Если формат входящих данных указан как HMAC SHA384
								case static_cast <uint8_t> (type_t::HMAC_SHA384):
									// Выполняем конвертирование в формат HMAC SHA384
									hash_t(&log).hmac(hmac, result.GetString(), hash_t::type_t::SHA384, buffer);
								break;
								// Если формат входящих данных указан как HMAC SHA512
								case static_cast <uint8_t> (type_t::HMAC_SHA512):
									// Выполняем конвертирование в формат HMAC SHA512
									hash_t(&log).hmac(hmac, result.GetString(), hash_t::type_t::SHA512, buffer);
								break;
							}
							// Если данные для записи получены
							if(!buffer.empty())
								// Выполняем запись данных в файл
								fs.write(filename, buffer.data(), buffer.size());
							// Выводим сообщение об ошибке
							else log.print("Conversion from \"%s\" format to \"%s\" format is failed", log_t::flag_t::WARNING, fmk.transform(env.get <string> (false, "from"), fmk_t::transform_t::UPPER).c_str(), fmk.transform(env.get <string> (false, "to"), fmk_t::transform_t::UPPER).c_str());
							// Выводим удачное завершение работы
							return EXIT_SUCCESS;
						}
						// Выводим сообщение об ошибке
						log.print("Directory address is not set", log_t::flag_t::CRITICAL);
						// Выводим удачное завершение работы
						return EXIT_FAILURE;
					// Если адрес для сохранения файла не указан
					} else {
						// Определяем формат данных
						switch(static_cast <uint8_t> (to)){
							// Если формат входящих данных указан как Text
							case static_cast <uint8_t> (type_t::TEXT):
								// Выполняем вывод текст как он есть
								cout << result.GetString() << endl;
							break;
							// Если формат входящих данных указан как XML
							case static_cast <uint8_t> (type_t::XML):
								// Выполняем конвертирование в формат XML
								cout << parser.xml(result, env.isBoolean(false, "prettify")) << endl;
							break;
							// Если формат входящих данных указан как JSON
							case static_cast <uint8_t> (type_t::JSON):
								// Выполняем конвертирование в формат JSON
								cout << parser.json(result, env.isBoolean(false, "prettify")) << endl;
							break;
							// Если формат входящих данных указан как INI
							case static_cast <uint8_t> (type_t::INI):
								// Выполняем конвертирование в формат INI
								cout << parser.ini(result) << endl;
							break;
							// Если формат входящих данных указан как YAML
							case static_cast <uint8_t> (type_t::YAML):
								// Выполняем конвертирование в формат YAML
								cout << parser.yaml(result) << endl;
							break;
							// Если формат входящих данных указан как CEF
							case static_cast <uint8_t> (type_t::CEF): {
								// Режим парсинга CEF по умолчанию
								cef_t::mode_t mode = cef_t::mode_t::STRONG;
								// Если режим парсинга CEF передан
								if(env.isString(false, "cef")){
									// Получаем режим парсинга
									const string & cef = env.get <string> (false, "cef");
									// Если режим парсинга установлен как строгий
									if(fmk.compare("strong", cef))
										// Устанавливаем строгий режим парсинга
										mode = cef_t::mode_t::STRONG;
									// Если режим парсинга установлен как простой
									else if(fmk.compare("low", cef))
										// Устанавливаем простой режим парсинга
										mode = cef_t::mode_t::LOW;
									// Если режим парсинга установлен как средний
									else if(fmk.compare("medium", cef))
										// Устанавливаем средний режим парсинга
										mode = cef_t::mode_t::MEDIUM;
								}
								// Выполняем конвертирование в формат CEF
								cout << parser.cef(result, mode) << endl;
							} break;
							// Если формат входящих данных указан как CSV
							case static_cast <uint8_t> (type_t::CSV):
								// Выполняем конвертирование в формат CSV
								cout << parser.csv(result, (env.isString(false, "delim") ? env.get <string> (false, "delim").front() : ';')) << endl;
							break;
							// Если формат входящих данных указан как SysLog
							case static_cast <uint8_t> (type_t::SYSLOG):
								// Выполняем конвертирование в формат SysLog
								cout << parser.syslog(result) << endl;
							break;
							// Если формат входящих данных указан как BASE64
							case static_cast <uint8_t> (type_t::BASE64): {
								// Выполняем получение текста для шифрования
								const string data = result.GetString();
								// Выполняем конвертирование в формат BASE64
								cout << hash_t(&log).encode <string> (data.c_str(), data.size(), hash_t::cipher_t::BASE64) << endl;
							} break;
							// Если формат входящих данных указан как MD5
							case static_cast <uint8_t> (type_t::MD5):
								// Выполняем конвертирование в формат MD5
								cout << hash_t(&log).hashing <string> (result.GetString(), hash_t::type_t::MD5) << endl;
							break;
							// Если формат входящих данных указан как SHA1
							case static_cast <uint8_t> (type_t::SHA1):
								// Выполняем конвертирование в формат SHA1
								cout << hash_t(&log).hashing <string> (result.GetString(), hash_t::type_t::SHA1) << endl;
							break;
							// Если формат входящих данных указан как SHA224
							case static_cast <uint8_t> (type_t::SHA224):
								// Выполняем конвертирование в формат SHA224
								cout << hash_t(&log).hashing <string> (result.GetString(), hash_t::type_t::SHA224) << endl;
							break;
							// Если формат входящих данных указан как SHA256
							case static_cast <uint8_t> (type_t::SHA256):
								// Выполняем конвертирование в формат SHA256
								cout << hash_t(&log).hashing <string> (result.GetString(), hash_t::type_t::SHA256) << endl;
							break;
							// Если формат входящих данных указан как SHA384
							case static_cast <uint8_t> (type_t::SHA384):
								// Выполняем конвертирование в формат SHA384
								cout << hash_t(&log).hashing <string> (result.GetString(), hash_t::type_t::SHA384) << endl;
							break;
							// Если формат входящих данных указан как SHA512
							case static_cast <uint8_t> (type_t::SHA512):
								// Выполняем конвертирование в формат SHA512
								cout << hash_t(&log).hashing <string> (result.GetString(), hash_t::type_t::SHA512) << endl;
							break;
							// Если формат входящих данных указан как HMAC MD5
							case static_cast <uint8_t> (type_t::HMAC_MD5):
								// Выполняем конвертирование в формат HMAC MD5
								cout << hash_t(&log).hmac <string> (hmac, result.GetString(), hash_t::type_t::MD5) << endl;
							break;
							// Если формат входящих данных указан как HMAC SHA1
							case static_cast <uint8_t> (type_t::HMAC_SHA1):
								// Выполняем конвертирование в формат HMAC SHA1
								cout << hash_t(&log).hmac <string> (hmac, result.GetString(), hash_t::type_t::SHA1) << endl;
							break;
							// Если формат входящих данных указан как HMAC SHA224
							case static_cast <uint8_t> (type_t::HMAC_SHA224):
								// Выполняем конвертирование в формат HMAC SHA224
								cout << hash_t(&log).hmac <string> (hmac, result.GetString(), hash_t::type_t::SHA224) << endl;
							break;
							// Если формат входящих данных указан как HMAC SHA256
							case static_cast <uint8_t> (type_t::HMAC_SHA256):
								// Выполняем конвертирование в формат HMAC SHA256
								cout << hash_t(&log).hmac <string> (hmac, result.GetString(), hash_t::type_t::SHA256) << endl;
							break;
							// Если формат входящих данных указан как HMAC SHA384
							case static_cast <uint8_t> (type_t::HMAC_SHA384):
								// Выполняем конвертирование в формат HMAC SHA384
								cout << hash_t(&log).hmac <string> (hmac, result.GetString(), hash_t::type_t::SHA384) << endl;
							break;
							// Если формат входящих данных указан как HMAC SHA512
							case static_cast <uint8_t> (type_t::HMAC_SHA512):
								// Выполняем конвертирование в формат HMAC SHA512
								cout << hash_t(&log).hmac <string> (hmac, result.GetString(), hash_t::type_t::SHA512) << endl;
							break;
						}
					}
				}
			// Если адрес файла хранения указан
			} else if(env.isString(false, "src")) {
				// Получаем адрес файла или каталога
				const string & addr = env.get <string> (false, "src");
				// Если адрес является каталогом
				if(fs.isDir(addr)){
					// Получаем расширение файла
					const string extension = env.get <string> (false, "from");
					// Переводим расширение в нижний регистр
					fmk.transform(extension, fmk_t::transform_t::LOWER);
					// Выполняем чтение каталога
					fs.readDir(addr, extension, true, [&](const string & filename) noexcept -> void {
						// Выполняем чтение данных файла
						const auto & buffer = fs.read(filename);
						// Получаем название файла
						const string & name = fs.components(filename).first;
						// Если данные файла загружены
						if(!buffer.empty()){
							// Объект в формате JSON
							json result(kObjectType);
							// Формируем полученный текст
							const string text(buffer.begin(), buffer.end());
							// Определяем формат данных
							switch(static_cast <uint8_t> (from)){
								// Если формат входящих данных указан как Text
								case static_cast <uint8_t> (type_t::TEXT):
									// Выполняем передачу данных как она есть
									result.SetString(text.c_str(), text.length(), result.GetAllocator());
								break;
								// Если формат входящих данных указан как XML
								case static_cast <uint8_t> (type_t::XML):
									// Выполняем конвертацию данных
									result = parser.xml(text);
								break;
								// Если формат входящих данных указан как JSON
								case static_cast <uint8_t> (type_t::JSON):
									// Выполняем конвертацию данных
									result = parser.json(text);
								break;
								// Если формат входящих данных указан как INI
								case static_cast <uint8_t> (type_t::INI):
									// Выполняем конвертацию данных
									result = parser.ini(text);
								break;
								// Если формат входящих данных указан как YAML
								case static_cast <uint8_t> (type_t::YAML):
									// Выполняем конвертацию данных
									result = parser.yaml(text);
								break;
								// Если формат входящих данных указан как CEF
								case static_cast <uint8_t> (type_t::CEF): {
									// Режим парсинга CEF по умолчанию
									cef_t::mode_t mode = cef_t::mode_t::STRONG;
									// Если режим парсинга CEF передан
									if(env.isString(false, "cef")){
										// Получаем режим парсинга
										const string & cef = env.get <string> (false, "cef");
										// Если режим парсинга установлен как строгий
										if(fmk.compare("strong", cef))
											// Устанавливаем строгий режим парсинга
											mode = cef_t::mode_t::STRONG;
										// Если режим парсинга установлен как простой
										else if(fmk.compare("low", cef))
											// Устанавливаем простой режим парсинга
											mode = cef_t::mode_t::LOW;
										// Если режим парсинга установлен как средний
										else if(fmk.compare("medium", cef))
											// Устанавливаем средний режим парсинга
											mode = cef_t::mode_t::MEDIUM;
									}
									// Выполняем конвертацию данных
									result = parser.cef(text, mode);
								} break;
								// Если формат входящих данных указан как CSV
								case static_cast <uint8_t> (type_t::CSV):
									// Выполняем конвертацию данных
									result = parser.csv(text, env.isBoolean(false, "header"));
								break;
								// Если формат входящих данных указан как GROK
								case static_cast <uint8_t> (type_t::GROK):
									// Выполняем конвертацию данных
									result = parser.grok(text, express);
								break;
								// Если формат входящих данных указан как SysLog
								case static_cast <uint8_t> (type_t::SYSLOG):
									// Выполняем конвертацию данных
									result = parser.syslog(text);
								break;
								// Если формат входящих данных указан как BASE64
								case static_cast <uint8_t> (type_t::BASE64): {
									// Выполняем декодирование хэша BASE64
									const string & hash = hash_t(&log).decode <string> (text.data(), text.size(), hash_t::cipher_t::BASE64);
									// Выполняем установку полученного хэша BASE64
									result.SetString(hash.c_str(), hash.length(), result.GetAllocator());
								} break;
							}
							// Если результат получен
							if(!result.IsNull()){
								// Если указан адрес каталога назначения
								if(env.isString(false, "dest")){
									// Получаем адрес каталога
									const string & addr = env.get <string> (false, "dest");
									// Если адрес каталога получен
									if(!addr.empty()){
										// Если каталог не существует
										if(!fs.isDir(addr)){
											// Выполняем создание каталога
											fs.makePath(addr);
											// Если каталог не создан, выходим
											if(!fs.isDir(addr)){
												// Выводим сообщение об ошибке
												log.print("Directory at address \"%s\" could not be created", log_t::flag_t::CRITICAL, addr.c_str());
												// Выводим удачное завершение работы
												return;
											}
										// Если адрес передан в качестве файла
										} else if(fs.isFile(addr)) {
											// Выводим сообщение об ошибке
											log.print("Destination address must be a directory and not a file \"%s\"", log_t::flag_t::CRITICAL, addr.c_str());
											// Выводим удачное завершение работы
											return;
										}
										// Буфер данных для записи
										string buffer = "";
										// Определяем формат данных
										switch(static_cast <uint8_t> (to)){
											// Если формат входящих данных указан как Text
											case static_cast <uint8_t> (type_t::TEXT):
												// Выполняем вывод текст как он есть
												buffer = result.GetString();
											break;
											// Если формат входящих данных указан как XML
											case static_cast <uint8_t> (type_t::XML):
												// Выполняем конвертирование в формат XML
												buffer = parser.xml(result, env.isBoolean(false, "prettify"));
											break;
											// Если формат входящих данных указан как JSON
											case static_cast <uint8_t> (type_t::JSON):
												// Выполняем конвертирование в формат JSON
												buffer = parser.json(result, env.isBoolean(false, "prettify"));
											break;
											// Если формат входящих данных указан как INI
											case static_cast <uint8_t> (type_t::INI):
												// Выполняем конвертирование в формат INI
												buffer = parser.ini(result);
											break;
											// Если формат входящих данных указан как YAML
											case static_cast <uint8_t> (type_t::YAML):
												// Выполняем конвертирование в формат YAML
												buffer = parser.yaml(result);
											break;
											// Если формат входящих данных указан как CEF
											case static_cast <uint8_t> (type_t::CEF): {
												// Режим парсинга CEF по умолчанию
												cef_t::mode_t mode = cef_t::mode_t::STRONG;
												// Если режим парсинга CEF передан
												if(env.isString(false, "cef")){
													// Получаем режим парсинга
													const string & cef = env.get <string> (false, "cef");
													// Если режим парсинга установлен как строгий
													if(fmk.compare("strong", cef))
														// Устанавливаем строгий режим парсинга
														mode = cef_t::mode_t::STRONG;
													// Если режим парсинга установлен как простой
													else if(fmk.compare("low", cef))
														// Устанавливаем простой режим парсинга
														mode = cef_t::mode_t::LOW;
													// Если режим парсинга установлен как средний
													else if(fmk.compare("medium", cef))
														// Устанавливаем средний режим парсинга
														mode = cef_t::mode_t::MEDIUM;
												}
												// Выполняем конвертирование в формат CEF
												buffer = parser.cef(result, mode);
											} break;
											// Если формат входящих данных указан как CSV
											case static_cast <uint8_t> (type_t::CSV):
												// Выполняем конвертирование в формат CSV
												buffer = parser.csv(result, (env.isString(false, "delim") ? env.get <string> (false, "delim").front() : ';'));
											break;
											// Если формат входящих данных указан как SysLog
											case static_cast <uint8_t> (type_t::SYSLOG):
												// Выполняем конвертирование в формат SysLog
												buffer = parser.syslog(result);
											break;
											// Если формат входящих данных указан как BASE64
											case static_cast <uint8_t> (type_t::BASE64): {
												// Выполняем получение текста для шифрования
												const string data = result.GetString();
												// Выполняем конвертирование в формат BASE64
												hash_t(&log).encode(data.c_str(), data.size(), hash_t::cipher_t::BASE64, buffer);
											} break;
											// Если формат входящих данных указан как MD5
											case static_cast <uint8_t> (type_t::MD5):
												// Выполняем конвертирование в формат MD5
												hash_t(&log).hashing(result.GetString(), hash_t::type_t::MD5, buffer);
											break;
											// Если формат входящих данных указан как SHA1
											case static_cast <uint8_t> (type_t::SHA1):
												// Выполняем конвертирование в формат SHA1
												hash_t(&log).hashing(result.GetString(), hash_t::type_t::SHA1, buffer);
											break;
											// Если формат входящих данных указан как SHA224
											case static_cast <uint8_t> (type_t::SHA224):
												// Выполняем конвертирование в формат SHA224
												hash_t(&log).hashing(result.GetString(), hash_t::type_t::SHA224, buffer);
											break;
											// Если формат входящих данных указан как SHA256
											case static_cast <uint8_t> (type_t::SHA256):
												// Выполняем конвертирование в формат SHA256
												hash_t(&log).hashing(result.GetString(), hash_t::type_t::SHA256, buffer);
											break;
											// Если формат входящих данных указан как SHA384
											case static_cast <uint8_t> (type_t::SHA384):
												// Выполняем конвертирование в формат SHA384
												hash_t(&log).hashing(result.GetString(), hash_t::type_t::SHA384, buffer);
											break;
											// Если формат входящих данных указан как SHA512
											case static_cast <uint8_t> (type_t::SHA512):
												// Выполняем конвертирование в формат SHA512
												hash_t(&log).hashing(result.GetString(), hash_t::type_t::SHA512, buffer);
											break;
											// Если формат входящих данных указан как HMAC MD5
											case static_cast <uint8_t> (type_t::HMAC_MD5):
												// Выполняем конвертирование в формат HMAC MD5
												hash_t(&log).hmac(hmac, result.GetString(), hash_t::type_t::MD5, buffer);
											break;
											// Если формат входящих данных указан как HMAC SHA1
											case static_cast <uint8_t> (type_t::HMAC_SHA1):
												// Выполняем конвертирование в формат HMAC SHA1
												hash_t(&log).hmac(hmac, result.GetString(), hash_t::type_t::SHA1, buffer);
											break;
											// Если формат входящих данных указан как HMAC SHA224
											case static_cast <uint8_t> (type_t::HMAC_SHA224):
												// Выполняем конвертирование в формат HMAC SHA224
												hash_t(&log).hmac(hmac, result.GetString(), hash_t::type_t::SHA224, buffer);
											break;
											// Если формат входящих данных указан как HMAC SHA256
											case static_cast <uint8_t> (type_t::HMAC_SHA256):
												// Выполняем конвертирование в формат HMAC SHA256
												hash_t(&log).hmac(hmac, result.GetString(), hash_t::type_t::SHA256, buffer);
											break;
											// Если формат входящих данных указан как HMAC SHA384
											case static_cast <uint8_t> (type_t::HMAC_SHA384):
												// Выполняем конвертирование в формат HMAC SHA384
												hash_t(&log).hmac(hmac, result.GetString(), hash_t::type_t::SHA384, buffer);
											break;
											// Если формат входящих данных указан как HMAC SHA512
											case static_cast <uint8_t> (type_t::HMAC_SHA512):
												// Выполняем конвертирование в формат HMAC SHA512
												hash_t(&log).hmac(hmac, result.GetString(), hash_t::type_t::SHA512, buffer);
											break;
										}
										// Если данные для записи получены
										if(!buffer.empty()){
											// Получаем расширение файла
											const string extension = env.get <string> (false, "to");
											// Переводим расширение в нижний регистр
											fmk.transform(extension, fmk_t::transform_t::LOWER);
											// Выполняем запись данных в файл
											fs.write(fmk.format("%s%s%s.%s", addr.c_str(), FS_SEPARATOR, name.c_str(), extension.c_str()), buffer.data(), buffer.size());
										// Выводим сообщение об ошибке
										} else log.print("Conversion from \"%s\" format to \"%s\" format is failed", log_t::flag_t::WARNING, fmk.transform(env.get <string> (false, "from"), fmk_t::transform_t::UPPER).c_str(), fmk.transform(env.get <string> (false, "to"), fmk_t::transform_t::UPPER).c_str());
										// Выводим удачное завершение работы
										return;
									}
									// Выводим сообщение об ошибке
									log.print("Directory address is not set", log_t::flag_t::CRITICAL);
									// Выводим удачное завершение работы
									return;
								// Если адрес для сохранения файла не указан
								} else {
									// Определяем формат данных
									switch(static_cast <uint8_t> (to)){
										// Если формат входящих данных указан как Text
										case static_cast <uint8_t> (type_t::TEXT):
											// Выполняем вывод текст как он есть
											cout << result.GetString() << endl;
										break;
										// Если формат входящих данных указан как XML
										case static_cast <uint8_t> (type_t::XML):
											// Выполняем конвертирование в формат XML
											cout << parser.xml(result, env.isBoolean(false, "prettify")) << endl;
										break;
										// Если формат входящих данных указан как JSON
										case static_cast <uint8_t> (type_t::JSON):
											// Выполняем конвертирование в формат JSON
											cout << parser.json(result, env.isBoolean(false, "prettify")) << endl;
										break;
										// Если формат входящих данных указан как INI
										case static_cast <uint8_t> (type_t::INI):
											// Выполняем конвертирование в формат INI
											cout << parser.ini(result) << endl;
										break;
										// Если формат входящих данных указан как YAML
										case static_cast <uint8_t> (type_t::YAML):
											// Выполняем конвертирование в формат YAML
											cout << parser.yaml(result) << endl;
										break;
										// Если формат входящих данных указан как CEF
										case static_cast <uint8_t> (type_t::CEF): {
											// Режим парсинга CEF по умолчанию
											cef_t::mode_t mode = cef_t::mode_t::STRONG;
											// Если режим парсинга CEF передан
											if(env.isString(false, "cef")){
												// Получаем режим парсинга
												const string & cef = env.get <string> (false, "cef");
												// Если режим парсинга установлен как строгий
												if(fmk.compare("strong", cef))
													// Устанавливаем строгий режим парсинга
													mode = cef_t::mode_t::STRONG;
												// Если режим парсинга установлен как простой
												else if(fmk.compare("low", cef))
													// Устанавливаем простой режим парсинга
													mode = cef_t::mode_t::LOW;
												// Если режим парсинга установлен как средний
												else if(fmk.compare("medium", cef))
													// Устанавливаем средний режим парсинга
													mode = cef_t::mode_t::MEDIUM;
											}
											// Выполняем конвертирование в формат CEF
											cout << parser.cef(result, mode) << endl;
										} break;
										// Если формат входящих данных указан как CSV
										case static_cast <uint8_t> (type_t::CSV):
											// Выполняем конвертирование в формат CSV
											cout << parser.csv(result, (env.isString(false, "delim") ? env.get <string> (false, "delim").front() : ';')) << endl;
										break;
										// Если формат входящих данных указан как SysLog
										case static_cast <uint8_t> (type_t::SYSLOG):
											// Выполняем конвертирование в формат SysLog
											cout << parser.syslog(result) << endl;
										break;
										// Если формат входящих данных указан как BASE64
										case static_cast <uint8_t> (type_t::BASE64): {
											// Выполняем получение текста для шифрования
											const string data = result.GetString();
											// Выполняем конвертирование в формат BASE64
											cout << hash_t(&log).encode <string> (data.c_str(), data.size(), hash_t::cipher_t::BASE64) << endl;
										} break;
										// Если формат входящих данных указан как MD5
										case static_cast <uint8_t> (type_t::MD5):
											// Выполняем конвертирование в формат MD5
											cout << hash_t(&log).hashing <string> (result.GetString(), hash_t::type_t::MD5) << endl;
										break;
										// Если формат входящих данных указан как SHA1
										case static_cast <uint8_t> (type_t::SHA1):
											// Выполняем конвертирование в формат SHA1
											cout << hash_t(&log).hashing <string> (result.GetString(), hash_t::type_t::SHA1) << endl;
										break;
										// Если формат входящих данных указан как SHA224
										case static_cast <uint8_t> (type_t::SHA224):
											// Выполняем конвертирование в формат SHA224
											cout << hash_t(&log).hashing <string> (result.GetString(), hash_t::type_t::SHA224) << endl;
										break;
										// Если формат входящих данных указан как SHA256
										case static_cast <uint8_t> (type_t::SHA256):
											// Выполняем конвертирование в формат SHA256
											cout << hash_t(&log).hashing <string> (result.GetString(), hash_t::type_t::SHA256) << endl;
										break;
										// Если формат входящих данных указан как SHA384
										case static_cast <uint8_t> (type_t::SHA384):
											// Выполняем конвертирование в формат SHA384
											cout << hash_t(&log).hashing <string> (result.GetString(), hash_t::type_t::SHA384) << endl;
										break;
										// Если формат входящих данных указан как SHA512
										case static_cast <uint8_t> (type_t::SHA512):
											// Выполняем конвертирование в формат SHA512
											cout << hash_t(&log).hashing <string> (result.GetString(), hash_t::type_t::SHA512) << endl;
										break;
										// Если формат входящих данных указан как HMAC MD5
										case static_cast <uint8_t> (type_t::HMAC_MD5):
											// Выполняем конвертирование в формат HMAC MD5
											cout << hash_t(&log).hmac <string> (hmac, result.GetString(), hash_t::type_t::MD5) << endl;
										break;
										// Если формат входящих данных указан как HMAC SHA1
										case static_cast <uint8_t> (type_t::HMAC_SHA1):
											// Выполняем конвертирование в формат HMAC SHA1
											cout << hash_t(&log).hmac <string> (hmac, result.GetString(), hash_t::type_t::SHA1) << endl;
										break;
										// Если формат входящих данных указан как HMAC SHA224
										case static_cast <uint8_t> (type_t::HMAC_SHA224):
											// Выполняем конвертирование в формат HMAC SHA224
											cout << hash_t(&log).hmac <string> (hmac, result.GetString(), hash_t::type_t::SHA224) << endl;
										break;
										// Если формат входящих данных указан как HMAC SHA256
										case static_cast <uint8_t> (type_t::HMAC_SHA256):
											// Выполняем конвертирование в формат HMAC SHA256
											cout << hash_t(&log).hmac <string> (hmac, result.GetString(), hash_t::type_t::SHA256) << endl;
										break;
										// Если формат входящих данных указан как HMAC SHA384
										case static_cast <uint8_t> (type_t::HMAC_SHA384):
											// Выполняем конвертирование в формат HMAC SHA384
											cout << hash_t(&log).hmac <string> (hmac, result.GetString(), hash_t::type_t::SHA384) << endl;
										break;
										// Если формат входящих данных указан как HMAC SHA512
										case static_cast <uint8_t> (type_t::HMAC_SHA512):
											// Выполняем конвертирование в формат HMAC SHA512
											cout << hash_t(&log).hmac <string> (hmac, result.GetString(), hash_t::type_t::SHA512) << endl;
										break;
									}
								}
							}
						}
					});
				// Если адрес является файлом
				} else if(fs.isFile(addr)) {
					// Выполняем чтение данных файла
					const auto & buffer = fs.read(addr);
					// Получаем название файла
					const string & name = fs.components(addr).first;
					// Если данные файла загружены
					if(!buffer.empty()){
						// Объект в формате JSON
						json result(kObjectType);
						// Формируем полученный текст
						const string text(buffer.begin(), buffer.end());
						// Определяем формат данных
						switch(static_cast <uint8_t> (from)){
							// Если формат входящих данных указан как Text
							case static_cast <uint8_t> (type_t::TEXT):
								// Выполняем передачу данных как она есть
								result.SetString(text.c_str(), text.length(), result.GetAllocator());
							break;
							// Если формат входящих данных указан как XML
							case static_cast <uint8_t> (type_t::XML):
								// Выполняем конвертацию данных
								result = parser.xml(text);
							break;
							// Если формат входящих данных указан как JSON
							case static_cast <uint8_t> (type_t::JSON):
								// Выполняем конвертацию данных
								result = parser.json(text);
							break;
							// Если формат входящих данных указан как INI
							case static_cast <uint8_t> (type_t::INI):
								// Выполняем конвертацию данных
								result = parser.ini(text);
							break;
							// Если формат входящих данных указан как YAML
							case static_cast <uint8_t> (type_t::YAML):
								// Выполняем конвертацию данных
								result = parser.yaml(text);
							break;
							// Если формат входящих данных указан как CEF
							case static_cast <uint8_t> (type_t::CEF): {
								// Режим парсинга CEF по умолчанию
								cef_t::mode_t mode = cef_t::mode_t::STRONG;
								// Если режим парсинга CEF передан
								if(env.isString(false, "cef")){
									// Получаем режим парсинга
									const string & cef = env.get <string> (false, "cef");
									// Если режим парсинга установлен как строгий
									if(fmk.compare("strong", cef))
										// Устанавливаем строгий режим парсинга
										mode = cef_t::mode_t::STRONG;
									// Если режим парсинга установлен как простой
									else if(fmk.compare("low", cef))
										// Устанавливаем простой режим парсинга
										mode = cef_t::mode_t::LOW;
									// Если режим парсинга установлен как средний
									else if(fmk.compare("medium", cef))
										// Устанавливаем средний режим парсинга
										mode = cef_t::mode_t::MEDIUM;
								}
								// Выполняем конвертацию данных
								result = parser.cef(text, mode);
							} break;
							// Если формат входящих данных указан как CSV
							case static_cast <uint8_t> (type_t::CSV):
								// Выполняем конвертацию данных
								result = parser.csv(text, env.isBoolean(false, "header"));
							break;
							// Если формат входящих данных указан как GROK
							case static_cast <uint8_t> (type_t::GROK):
								// Выполняем конвертацию данных
								result = parser.grok(text, express);
							break;
							// Если формат входящих данных указан как SysLog
							case static_cast <uint8_t> (type_t::SYSLOG):
								// Выполняем конвертацию данных
								result = parser.syslog(text);
							break;
							// Если формат входящих данных указан как BASE64
							case static_cast <uint8_t> (type_t::BASE64): {
								// Выполняем декодирование хэша BASE64
								const string & hash = hash_t(&log).decode <string> (text.data(), text.size(), hash_t::cipher_t::BASE64);
								// Выполняем установку входящих данных в результат
								result.SetString(hash.c_str(), hash.length(), result.GetAllocator());
							} break;
						}
						// Если результат получен
						if(!result.IsNull()){
							// Если указан адрес каталога назначения
							if(env.isString(false, "dest")){
								// Получаем адрес каталога
								const string & addr = env.get <string> (false, "dest");
								// Если адрес каталога получен
								if(!addr.empty()){
									// Адрес файла для сохранения
									string filename = addr;
									// Если каталог не существует
									if(fs.isDir(addr)){
										// Получаем расширение файла
										const string extension = env.get <string> (false, "to");
										// Переводим расширение в нижний регистр
										fmk.transform(extension, fmk_t::transform_t::LOWER);
										// Выполняем создание адреса файла для сохранения
										filename = fmk.format("%s%s%s.%s", addr.c_str(), FS_SEPARATOR, name.c_str(), extension.c_str());
									}
									// Буфер данных для записи
									string buffer = "";
									// Определяем формат данных
									switch(static_cast <uint8_t> (to)){
										// Если формат входящих данных указан как Text
										case static_cast <uint8_t> (type_t::TEXT):
											// Выполняем вывод текст как он есть
											buffer = result.GetString();
										break;
										// Если формат входящих данных указан как XML
										case static_cast <uint8_t> (type_t::XML):
											// Выполняем конвертирование в формат XML
											buffer = parser.xml(result, env.isBoolean(false, "prettify"));
										break;
										// Если формат входящих данных указан как JSON
										case static_cast <uint8_t> (type_t::JSON):
											// Выполняем конвертирование в формат JSON
											buffer = parser.json(result, env.isBoolean(false, "prettify"));
										break;
										// Если формат входящих данных указан как INI
										case static_cast <uint8_t> (type_t::INI):
											// Выполняем конвертирование в формат INI
											buffer = parser.ini(result);
										break;
										// Если формат входящих данных указан как YAML
										case static_cast <uint8_t> (type_t::YAML):
											// Выполняем конвертирование в формат YAML
											buffer = parser.yaml(result);
										break;
										// Если формат входящих данных указан как CEF
										case static_cast <uint8_t> (type_t::CEF): {
											// Режим парсинга CEF по умолчанию
											cef_t::mode_t mode = cef_t::mode_t::STRONG;
											// Если режим парсинга CEF передан
											if(env.isString(false, "cef")){
												// Получаем режим парсинга
												const string & cef = env.get <string> (false, "cef");
												// Если режим парсинга установлен как строгий
												if(fmk.compare("strong", cef))
													// Устанавливаем строгий режим парсинга
													mode = cef_t::mode_t::STRONG;
												// Если режим парсинга установлен как простой
												else if(fmk.compare("low", cef))
													// Устанавливаем простой режим парсинга
													mode = cef_t::mode_t::LOW;
												// Если режим парсинга установлен как средний
												else if(fmk.compare("medium", cef))
													// Устанавливаем средний режим парсинга
													mode = cef_t::mode_t::MEDIUM;
											}
											// Выполняем конвертирование в формат CEF
											buffer = parser.cef(result, mode);
										} break;
										// Если формат входящих данных указан как CSV
										case static_cast <uint8_t> (type_t::CSV):
											// Выполняем конвертирование в формат CSV
											buffer = parser.csv(result, (env.isString(false, "delim") ? env.get <string> (false, "delim").front() : ';'));
										break;
										// Если формат входящих данных указан как SysLog
										case static_cast <uint8_t> (type_t::SYSLOG):
											// Выполняем конвертирование в формат SysLog
											buffer = parser.syslog(result);
										break;
										// Если формат входящих данных указан как BASE64
										case static_cast <uint8_t> (type_t::BASE64): {
											// Выполняем получение текста для шифрования
											const string data = result.GetString();
											// Выполняем конвертирование в формат BASE64
											hash_t(&log).encode(data.c_str(), data.size(), hash_t::cipher_t::BASE64, buffer);
										} break;
										// Если формат входящих данных указан как MD5
										case static_cast <uint8_t> (type_t::MD5):
											// Выполняем конвертирование в формат MD5
											hash_t(&log).hashing(result.GetString(), hash_t::type_t::MD5, buffer);
										break;
										// Если формат входящих данных указан как SHA1
										case static_cast <uint8_t> (type_t::SHA1):
											// Выполняем конвертирование в формат SHA1
											hash_t(&log).hashing(result.GetString(), hash_t::type_t::SHA1, buffer);
										break;
										// Если формат входящих данных указан как SHA224
										case static_cast <uint8_t> (type_t::SHA224):
											// Выполняем конвертирование в формат SHA224
											hash_t(&log).hashing(result.GetString(), hash_t::type_t::SHA224, buffer);
										break;
										// Если формат входящих данных указан как SHA256
										case static_cast <uint8_t> (type_t::SHA256):
											// Выполняем конвертирование в формат SHA256
											hash_t(&log).hashing(result.GetString(), hash_t::type_t::SHA256, buffer);
										break;
										// Если формат входящих данных указан как SHA384
										case static_cast <uint8_t> (type_t::SHA384):
											// Выполняем конвертирование в формат SHA384
											hash_t(&log).hashing(result.GetString(), hash_t::type_t::SHA384, buffer);
										break;
										// Если формат входящих данных указан как SHA512
										case static_cast <uint8_t> (type_t::SHA512):
											// Выполняем конвертирование в формат SHA512
											hash_t(&log).hashing(result.GetString(), hash_t::type_t::SHA512, buffer);
										break;
										// Если формат входящих данных указан как HMAC MD5
										case static_cast <uint8_t> (type_t::HMAC_MD5):
											// Выполняем конвертирование в формат HMAC MD5
											hash_t(&log).hmac(hmac, result.GetString(), hash_t::type_t::MD5, buffer);
										break;
										// Если формат входящих данных указан как HMAC SHA1
										case static_cast <uint8_t> (type_t::HMAC_SHA1):
											// Выполняем конвертирование в формат HMAC SHA1
											hash_t(&log).hmac(hmac, result.GetString(), hash_t::type_t::SHA1, buffer);
										break;
										// Если формат входящих данных указан как HMAC SHA224
										case static_cast <uint8_t> (type_t::HMAC_SHA224):
											// Выполняем конвертирование в формат HMAC SHA224
											hash_t(&log).hmac(hmac, result.GetString(), hash_t::type_t::SHA224, buffer);
										break;
										// Если формат входящих данных указан как HMAC SHA256
										case static_cast <uint8_t> (type_t::HMAC_SHA256):
											// Выполняем конвертирование в формат HMAC SHA256
											hash_t(&log).hmac(hmac, result.GetString(), hash_t::type_t::SHA256, buffer);
										break;
										// Если формат входящих данных указан как HMAC SHA384
										case static_cast <uint8_t> (type_t::HMAC_SHA384):
											// Выполняем конвертирование в формат HMAC SHA384
											hash_t(&log).hmac(hmac, result.GetString(), hash_t::type_t::SHA384, buffer);
										break;
										// Если формат входящих данных указан как HMAC SHA512
										case static_cast <uint8_t> (type_t::HMAC_SHA512):
											// Выполняем конвертирование в формат HMAC SHA512
											hash_t(&log).hmac(hmac, result.GetString(), hash_t::type_t::SHA512, buffer);
										break;
									}
									// Если данные для записи получены
									if(!buffer.empty())
										// Выполняем запись данных в файл
										fs.write(filename, buffer.data(), buffer.size());
									// Выводим сообщение об ошибке
									else log.print("Conversion from \"%s\" format to \"%s\" format is failed", log_t::flag_t::WARNING, fmk.transform(env.get <string> (false, "from"), fmk_t::transform_t::UPPER).c_str(), fmk.transform(env.get <string> (false, "to"), fmk_t::transform_t::UPPER).c_str());
									// Выводим удачное завершение работы
									return EXIT_SUCCESS;
								}
								// Выводим сообщение об ошибке
								log.print("Directory address is not set", log_t::flag_t::CRITICAL);
								// Выводим удачное завершение работы
								return EXIT_FAILURE;
							// Если адрес для сохранения файла не указан
							} else {
								// Определяем формат данных
								switch(static_cast <uint8_t> (to)){
									// Если формат входящих данных указан как Text
									case static_cast <uint8_t> (type_t::TEXT):
										// Выполняем вывод текст как он есть
										cout << result.GetString() << endl;
									break;
									// Если формат входящих данных указан как XML
									case static_cast <uint8_t> (type_t::XML):
										// Выполняем конвертирование в формат XML
										cout << parser.xml(result, env.isBoolean(false, "prettify")) << endl;
									break;
									// Если формат входящих данных указан как JSON
									case static_cast <uint8_t> (type_t::JSON):
										// Выполняем конвертирование в формат JSON
										cout << parser.json(result, env.isBoolean(false, "prettify")) << endl;
									break;
									// Если формат входящих данных указан как INI
									case static_cast <uint8_t> (type_t::INI):
										// Выполняем конвертирование в формат INI
										cout << parser.ini(result) << endl;
									break;
									// Если формат входящих данных указан как YAML
									case static_cast <uint8_t> (type_t::YAML):
										// Выполняем конвертирование в формат YAML
										cout << parser.yaml(result) << endl;
									break;
									// Если формат входящих данных указан как CEF
									case static_cast <uint8_t> (type_t::CEF): {
										// Режим парсинга CEF по умолчанию
										cef_t::mode_t mode = cef_t::mode_t::STRONG;
										// Если режим парсинга CEF передан
										if(env.isString(false, "cef")){
											// Получаем режим парсинга
											const string & cef = env.get <string> (false, "cef");
											// Если режим парсинга установлен как строгий
											if(fmk.compare("strong", cef))
												// Устанавливаем строгий режим парсинга
												mode = cef_t::mode_t::STRONG;
											// Если режим парсинга установлен как простой
											else if(fmk.compare("low", cef))
												// Устанавливаем простой режим парсинга
												mode = cef_t::mode_t::LOW;
											// Если режим парсинга установлен как средний
											else if(fmk.compare("medium", cef))
												// Устанавливаем средний режим парсинга
												mode = cef_t::mode_t::MEDIUM;
										}
										// Выполняем конвертирование в формат CEF
										cout << parser.cef(result, mode) << endl;
									} break;
									// Если формат входящих данных указан как CSV
									case static_cast <uint8_t> (type_t::CSV):
										// Выполняем конвертирование в формат CSV
										cout << parser.csv(result, (env.isString(false, "delim") ? env.get <string> (false, "delim").front() : ';')) << endl;
									break;
									// Если формат входящих данных указан как SysLog
									case static_cast <uint8_t> (type_t::SYSLOG):
										// Выполняем конвертирование в формат SysLog
										cout << parser.syslog(result) << endl;
									break;
									// Если формат входящих данных указан как BASE64
									case static_cast <uint8_t> (type_t::BASE64): {
										// Выполняем получение текста для шифрования
										const string data = result.GetString();
										// Выполняем конвертирование в формат BASE64
										cout << hash_t(&log).encode <string> (data.c_str(), data.size(), hash_t::cipher_t::BASE64) << endl;
									} break;
									// Если формат входящих данных указан как MD5
									case static_cast <uint8_t> (type_t::MD5):
										// Выполняем конвертирование в формат MD5
										cout << hash_t(&log).hashing <string> (result.GetString(), hash_t::type_t::MD5) << endl;
									break;
									// Если формат входящих данных указан как SHA1
									case static_cast <uint8_t> (type_t::SHA1):
										// Выполняем конвертирование в формат SHA1
										cout << hash_t(&log).hashing <string> (result.GetString(), hash_t::type_t::SHA1) << endl;
									break;
									// Если формат входящих данных указан как SHA224
									case static_cast <uint8_t> (type_t::SHA224):
										// Выполняем конвертирование в формат SHA224
										cout << hash_t(&log).hashing <string> (result.GetString(), hash_t::type_t::SHA224) << endl;
									break;
									// Если формат входящих данных указан как SHA256
									case static_cast <uint8_t> (type_t::SHA256):
										// Выполняем конвертирование в формат SHA256
										cout << hash_t(&log).hashing <string> (result.GetString(), hash_t::type_t::SHA256) << endl;
									break;
									// Если формат входящих данных указан как SHA384
									case static_cast <uint8_t> (type_t::SHA384):
										// Выполняем конвертирование в формат SHA384
										cout << hash_t(&log).hashing <string> (result.GetString(), hash_t::type_t::SHA384) << endl;
									break;
									// Если формат входящих данных указан как SHA512
									case static_cast <uint8_t> (type_t::SHA512):
										// Выполняем конвертирование в формат SHA512
										cout << hash_t(&log).hashing <string> (result.GetString(), hash_t::type_t::SHA512) << endl;
									break;
									// Если формат входящих данных указан как HMAC MD5
									case static_cast <uint8_t> (type_t::HMAC_MD5):
										// Выполняем конвертирование в формат HMAC MD5
										cout << hash_t(&log).hmac <string> (hmac, result.GetString(), hash_t::type_t::MD5) << endl;
									break;
									// Если формат входящих данных указан как HMAC SHA1
									case static_cast <uint8_t> (type_t::HMAC_SHA1):
										// Выполняем конвертирование в формат HMAC SHA1
										cout << hash_t(&log).hmac <string> (hmac, result.GetString(), hash_t::type_t::SHA1) << endl;
									break;
									// Если формат входящих данных указан как HMAC SHA224
									case static_cast <uint8_t> (type_t::HMAC_SHA224):
										// Выполняем конвертирование в формат HMAC SHA224
										cout << hash_t(&log).hmac <string> (hmac, result.GetString(), hash_t::type_t::SHA224) << endl;
									break;
									// Если формат входящих данных указан как HMAC SHA256
									case static_cast <uint8_t> (type_t::HMAC_SHA256):
										// Выполняем конвертирование в формат HMAC SHA256
										cout << hash_t(&log).hmac <string> (hmac, result.GetString(), hash_t::type_t::SHA256) << endl;
									break;
									// Если формат входящих данных указан как HMAC SHA384
									case static_cast <uint8_t> (type_t::HMAC_SHA384):
										// Выполняем конвертирование в формат HMAC SHA384
										cout << hash_t(&log).hmac <string> (hmac, result.GetString(), hash_t::type_t::SHA384) << endl;
									break;
									// Если формат входящих данных указан как HMAC SHA512
									case static_cast <uint8_t> (type_t::HMAC_SHA512):
										// Выполняем конвертирование в формат HMAC SHA512
										cout << hash_t(&log).hmac <string> (hmac, result.GetString(), hash_t::type_t::SHA512) << endl;
									break;
								}
							}
						}
					}
				}
			// Выводим сообщение, что файл или каталог не указан
			} else log.print("Address of the file or directory for conversion is not specified", log_t::flag_t::CRITICAL);
		// Если указаны форматы системы счисления
		} else if(env.isNumber(false, "from") && env.isNumber(false, "to") && env.isBoolean(false, "notation")) {
			// Получаем систему счисления в которую необходимо выполнить конвертацию
			const uint8_t to = env.get <uint8_t> (false, "to");
			// Получаем систему счисления из которой необходимо выполнить конвертацию
			const uint8_t from = env.get <uint8_t> (false, "from");
			// Если данные прочитаны из потока
			if(!text.empty()){
				// Если система счиления из которой производится конвертация не указана
				if((from == 0) && (to == 2)){
					/**
					 * Выполняем отлов ошибок
					 */
					try {
						// Определяем размер числа
						const size_t size = fmk.size(text.c_str(), text.size());
						// Создаём бинарный буфер
						uint8_t * buffer = new uint8_t[size];
						// Копируем в буфер наши данные
						::memcpy(buffer, text.c_str(), text.size());
						// Выполняем конвертацию полученного текста
						cout << fmk.itoa(buffer, size, to) << endl;
						// Удаляем выделенную память
						delete [] buffer;
					/**
					 * Если возникает ошибка
					 */
					} catch(const exception & error) {
						// Выводим сообщение об ошибке
						log.print("%s", log_t::flag_t::CRITICAL, error.what());
					}
				// Если система счисления из которой производится конвертация является десятичная
				} else if((from == 10) && ((to > 1) && (to < 37))) {
					// Текст передан в виде числа
					if(fmk.is(text, fmk_t::check_t::NUMBER)){
						/**
						 * Выполняем отлов ошибок
						 */
						try {
							// Получаем число
							const uint64_t num = static_cast <uint64_t> (text.front() == '-' ? (::stoll(text) * -1) : ::stoull(text));
							// Определяем размер числа
							const size_t size = fmk.size(num);
							// Если число помещается в один байт
							if(size == 1)
								// Выполняем конвертацию системы счисления
								cout << fmk.itoa(static_cast <uint8_t> (num), to) << endl;
							// Если число помещается в два байта
							else if(size == 2)
								// Выполняем конвертацию системы счисления
								cout << fmk.itoa(static_cast <uint16_t> (num), to) << endl;
							// Если число помещается в четыре байта
							else if(size <= 4)
								// Выполняем конвертацию системы счисления
								cout << fmk.itoa(static_cast <uint32_t> (num), to) << endl;
							// Если число помещается в 8 байт
							else if(size <= 8)
								// Выполняем конвертацию системы счисления
								cout << fmk.itoa(static_cast <uint64_t> (num), to) << endl;
							// Выводим сообщение об ошибке
							else log.print("Number to convert does not fit into 64 bits", log_t::flag_t::CRITICAL);
						/**
						 * Если возникает ошибка
						 */
						} catch(const exception &) {
							// Выводим сообщение об ошибке
							log.print("Number to convert does not fit into 64 bits", log_t::flag_t::CRITICAL);
						}
					// Текст передан в виде числа с плавающей точкой
					} else if(fmk.is(text, fmk_t::check_t::DECIMAL)) {
						/**
						 * Выполняем отлов ошибок
						 */
						try {
							// Получаем переданное число
							const uint64_t num = static_cast <uint64_t> (text.front() == '-' ? ::round(::stod(text) * -1) : ::round(::stod(text)));
							// Определяем размер числа
							const size_t size = fmk.size(num);
							// Если число помещается в один байт
							if(size == 1)
								// Выполняем конвертацию системы счисления
								cout << fmk.itoa(static_cast <uint8_t> (num), to) << endl;
							// Если число помещается в два байта
							else if(size == 2)
								// Выполняем конвертацию системы счисления
								cout << fmk.itoa(static_cast <uint16_t> (num), to) << endl;
							// Если число помещается в четыре байта
							else if(size <= 4)
								// Выполняем конвертацию системы счисления
								cout << fmk.itoa(static_cast <uint32_t> (num), to) << endl;
							// Если число помещается в 8 байт
							else if(size <= 8)
								// Выполняем конвертацию системы счисления
								cout << fmk.itoa(static_cast <uint64_t> (num), to) << endl;
							// Выводим сообщение об ошибке
							else log.print("Number to convert does not fit into 64 bits", log_t::flag_t::CRITICAL);
						/**
						 * Если возникает ошибка
						 */
						} catch(const exception &) {
							// Выводим сообщение об ошибке
							log.print("Number to convert does not fit into 64 bits", log_t::flag_t::CRITICAL);
						}
					// Если текст передан не в виде числа, то выводим сообщение об ошибке
					} else log.print("Data to be converted must be in the form of a number", log_t::flag_t::CRITICAL);
				// Если система счисления в которую производится конвертация является десятичная
				} else if((to < 37) && ((from > 1) && (from < 37))) {
					// Если система счисления является двоичной
					if(from == 2){
						// Размер бинарного буфера
						size_t size = 0;
						// Получаем количество байт
						const size_t count = (text.length() % 8);
						// Если количество байт умещается в буфер
						if(count == 0)
							// Получам размер бинарного буфера
							size = (text.length() / 8);
						// Получам размер бинарного буфера
						else size = ((text.length() + (8 - count)) / 8);
						// Определяем размер бинарного буфера
						switch(size){
							// Если размер бинарного буфера состоит из одного байта
							case 1: {
								// Если нам необходимо получить число
								if(to > 0)
									// Выполняем конвертацию системы счисления
									cout << fmk.itoa(fmk.atoi <uint8_t> (text, from), to) << endl;
								// Если нам необходимо получить текст
								else {
									// Создаём контейнер для хранения данных
									uint8_t buffer = 0;
									// Выполняем извлечение текста
									fmk.atoi(text, from, &buffer, sizeof(buffer));
									// Выводим полученный текст
									cout << string(reinterpret_cast <const char *> (&buffer), sizeof(buffer)) << endl;
								}
							} break;
							// Если размер бинарного буфера состоит из двух байтов
							case 2: {
								// Если нам необходимо получить число
								if(to > 0)
									// Выполняем конвертацию системы счисления
									cout << fmk.itoa(fmk.atoi <uint16_t> (text, from), to) << endl;
								// Если нам необходимо получить текст
								else {
									// Создаём контейнер для хранения данных
									uint16_t buffer = 0;
									// Выполняем извлечение текста
									fmk.atoi(text, from, &buffer, sizeof(buffer));
									// Выводим полученный текст
									cout << string(reinterpret_cast <const char *> (&buffer), sizeof(buffer)) << endl;
								}
							} break;
							// Если размер бинарного буфера состоит из четырёх байт
							case 4: {
								// Если нам необходимо получить число
								if(to > 0)
									// Выполняем конвертацию системы счисления
									cout << fmk.itoa(fmk.atoi <uint32_t> (text, from), to) << endl;
								// Если нам необходимо получить текст
								else {
									// Создаём контейнер для хранения данных
									uint32_t buffer = 0;
									// Выполняем извлечение текста
									fmk.atoi(text, from, &buffer, sizeof(buffer));
									// Выводим полученный текст
									cout << string(reinterpret_cast <const char *> (&buffer), sizeof(buffer)) << endl;
								}
							} break;
							// Если размер бинарного буфера состоит из восьми байт
							case 8: {
								// Если нам необходимо получить число
								if(to > 0)
									// Выполняем конвертацию системы счисления
									cout << fmk.itoa(fmk.atoi <uint64_t> (text, from), to) << endl;
								// Если нам необходимо получить текст
								else {
									// Создаём контейнер для хранения данных
									uint64_t buffer = 0;
									// Выполняем извлечение текста
									fmk.atoi(text, from, &buffer, sizeof(buffer));
									// Выводим полученный текст
									cout << string(reinterpret_cast <const char *> (&buffer), sizeof(buffer)) << endl;
								}
							} break;
							// Если размер буфера слишком большой
							default: {
								/**
								 * Выполняем отлов ошибок
								 */
								try {
									// Создаём бинарный буфер
									uint8_t * buffer = new uint8_t[size];
									// Выполняем извлечение текста
									fmk.atoi(text, from, buffer, size);
									// Выводим полученный текст
									cout << string(reinterpret_cast <const char *> (buffer), size) << endl;
									// Удаляем выделенную память
									delete [] buffer;
								/**
								 * Если возникает ошибка
								 */
								} catch(const exception &) {
									// Выводим сообщение об ошибке
									log.print("Number to convert does not fit into 64 bits", log_t::flag_t::CRITICAL);
								}
							}
						}
					// Если система счисления выше двоичной
					} else {
						// Получаем число в десятичной системе счисления
						const uint64_t num = fmk.atoi <uint64_t> (text, from);
						// Определяем размер числа
						const size_t size = fmk.size(num);
						// Если число помещается в один байт
						if(size == 1)
							// Выполняем конвертацию системы счисления
							cout << fmk.itoa(static_cast <int8_t> (num), to) << endl;
						// Если число помещается в два байта
						else if(size == 2)
							// Выполняем конвертацию системы счисления
							cout << fmk.itoa(static_cast <int16_t> (num), to) << endl;
						// Если число помещается в четыре байта
						else if(size <= 4)
							// Выполняем конвертацию системы счисления
							cout << fmk.itoa(static_cast <int32_t> (num), to) << endl;
						// Если число помещается в 8 байт
						else if(size <= 8)
							// Выполняем конвертацию системы счисления
							cout << fmk.itoa(static_cast <int64_t> (num), to) << endl;
						// Выводим сообщение об ошибке
						else log.print("Number to convert does not fit into 64 bits", log_t::flag_t::CRITICAL);
					}
				// Выводим сообщение, что параметры для конвертации указаны неправильно
				} else log.print("Parameters for converting the number system are specified incorrectly", log_t::flag_t::CRITICAL);
			// Выводим сообщение, что значение для конвертации не указанно
			} else log.print("No value specified for conversion", log_t::flag_t::CRITICAL);
		}
		// Выводим удачное завершение работы
		return EXIT_SUCCESS;
}
