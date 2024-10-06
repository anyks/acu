
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

// Подключаем пространство имён
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
		"\x1B[33m\x1B[1m+\x1B[0m Flag for generating headers when parsing CSV files: \x1B[1m[-header | --header]\x1B[0m\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m Flag for generating a readable file format (XML or JSON): \x1B[1m[-prettify | --prettify]\x1B[0m\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m Display application version: \x1B[1m[-version | --version | -V]\x1B[0m\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m Display information about available application functions: \x1B[1m[-info | --info | -H]\x1B[0m\r\n\r\n"
		"\x1B[34m\x1B[1m[ARGS]\x1B[0m\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m Separator for parsing CSV files (default: \";\"): \x1B[1m[-delim <value> | --delim=<value>]\x1B[0m\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m Format date: \x1B[1m[-formatDate \"<value>\" | --formatDate=\"<value>\"]\x1B[0m\r\n"
		"\x1B[32m\x1B[1m  -\x1B[0m ( %%m/%%d/%%Y %%H:%%M:%%S | %%H:%%M:%%S %%d.%%m.%%Y | ... )\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m CEF file parsing mode: \x1B[1m[-cef <value> | --cef=<value>]\x1B[0m\r\n"
		"\x1B[32m\x1B[1m  -\x1B[0m (LOW | MEDIUM | STRONG)\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m File format to which the writing is made: \x1B[1m[-to <value> | --to=<value>]\x1B[0m\r\n"
		"\x1B[32m\x1B[1m  -\x1B[0m (XML | JSON | INI | YAML | CSV | CEF | SYSLOG)\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m Format of the file from which reading is performed: \x1B[1m[-from <value> | --from=<value>]\x1B[0m\r\n"
		"\x1B[32m\x1B[1m  -\x1B[0m (XML | JSON | INI | YAML | CSV | CEF | SYSLOG | GROK)\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m File or directory address for saving converted files: \x1B[1m[-dest <value> | --dest=<value>]\x1B[0m\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m File address for writing trade logs (if required): \x1B[1m[-log <value> | --log=<value>]\x1B[0m\r\n\r\n"
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
				if(!result.empty())
					// Добавляем разделитель строки
					result.append(1, '\n');
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
		if(env.is("formatDate", true))
			// Получаем формат вывода даты
			formatDate = env.get("formatDate", true);
		// Устанавливаем формат вывода даты
		log.format(formatDate);
		// Если нужно вывести справочную помощь
		if(!env.size() || (env.is("info") || env.is("H"))){
			// Выводим справочную информацию
			help(name);
			// Выходим из приложения
			::exit(EXIT_SUCCESS);
		// Если версия получена
		} else if(env.is("version") || env.is("V")) {
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
		// Если указанны форматы конвертирования
		if(env.isString("from") && env.isString("to")){
			// Регулярное выражение в формате GROK
			string express = "";
			// Выполняем инициализацию объекта парсера
			parser_t parser(&fmk, &log);
			// Тип обрабатываемого файла
			enum class type_t : uint8_t {
				NONE   = 0x00, // Тип файла не обнаружен
				XML    = 0x01, // Тип файла XML
				INI    = 0x02, // Тип файла INI
				CEF    = 0x03, // Тип файла CEF
				CSV    = 0x04, // Тип файла CSV
				JSON   = 0x05, // Тип файла JSON
				YAML   = 0x06, // Тип файла YAML
				GROK   = 0x07, // Тип файла GROK
				SYSLOG = 0x08  // Тип файла SYSLOG
			};
			// Тип обрабатываемого файла
			type_t from = type_t::NONE, to = type_t::NONE;
			// Если формат входящих данных указан как XML
			if(fmk.compare("xml", env.get("from")))
				// Определяем тип файла
				from = type_t::XML;
			// Если формат входящих данных указан как JSON
			else if(fmk.compare("json", env.get("from")))
				// Определяем тип файла
				from = type_t::JSON;
			// Если формат входящих данных указан как INI
			else if(fmk.compare("ini", env.get("from")))
				// Определяем тип файла
				from = type_t::INI;
			// Если формат входящих данных указан как YAML
			else if(fmk.compare("yaml", env.get("from")))
				// Определяем тип файла
				from = type_t::YAML;
			// Если формат входящих данных указан как CEF
			else if(fmk.compare("cef", env.get("from")))
				// Определяем тип файла
				from = type_t::CEF;
			// Если формат входящих данных указан как CSV
			else if(fmk.compare("csv", env.get("from")))
				// Определяем тип файла
				from = type_t::CSV;
			// Если формат входящих данных указан как GROK
			else if(fmk.compare("grok", env.get("from"))) {
				// Определяем тип файла
				from = type_t::GROK;
				// Выполняем очистку списка ранее установленных шаблонов
				parser.clearPatterns();
				// Если файл шаблона указан
				if(env.isString("patterns")){
					// Выполняем чтение объекта шаблонов
					json patterns = json::parse(fs.read(env.get("patterns")));
					// Если данные шаблонов получены
					if(patterns.is_object() && !patterns.empty()){
						// Выполняем перебор всего списка значений
						for(auto & el : patterns.items()){
							// Если значение является строкой
							if(el.value().is_string())
								// Выполняем добавление шаблона
								parser.pattern(el.key(), el.value().get <string> ());
						}
					}
				}
				// Если регулярное выражение передано
				if(env.isString("express")){
					// Выполняем чтение данных регулярного выражения
					const auto & buffer = fs.read(env.get("express"));
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
			} else if(fmk.compare("syslog", env.get("from")))
				// Определяем тип файла
				from = type_t::SYSLOG;
			// Если формат не определён
			else {
				// Выводим сообщение об ошибке
				log.print("From file format is not recognized", log_t::flag_t::CRITICAL);
				// Выводим удачное завершение работы
				return EXIT_FAILURE;
			}
			// Если формат исходящих данных указан как XML
			if(fmk.compare("xml", env.get("to")))
				// Определяем тип файла
				to = type_t::XML;
			// Если формат исходящих данных указан как JSON
			else if(fmk.compare("json", env.get("to")))
				// Определяем тип файла
				to = type_t::JSON;
			// Если формат исходящих данных указан как INI
			else if(fmk.compare("ini", env.get("to")))
				// Определяем тип файла
				to = type_t::INI;
			// Если формат исходящих данных указан как YAML
			else if(fmk.compare("yaml", env.get("to")))
				// Определяем тип файла
				to = type_t::YAML;
			// Если формат исходящих данных указан как CEF
			else if(fmk.compare("cef", env.get("to")))
				// Определяем тип файла
				to = type_t::CEF;
			// Если формат исходящих данных указан как CSV
			else if(fmk.compare("csv", env.get("to")))
				// Определяем тип файла
				to = type_t::CSV;
			// Если формат исходящих данных указан как SysLog
			else if(fmk.compare("syslog", env.get("to")))
				// Определяем тип файла
				to = type_t::SYSLOG;
			// Если формат не определён
			else {
				// Выводим сообщение об ошибке
				log.print("To file format is not recognized", log_t::flag_t::CRITICAL);
				// Выводим удачное завершение работы
				return EXIT_FAILURE;
			}
			// Если данные прочитаны из потока
			if(!text.empty()){
				// Объект в формате JSON
				json result = json::object();
				// Определяем тип файла
				switch(static_cast <uint8_t> (from)){
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
						if(env.isString("cef")){
							// Получаем режим парсинга
							const string & cef = env.get("cef");
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
						result = parser.csv(text, env.is("header"));
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
				}
				// Если результат получен
				if(!result.empty()){
					// Если указан адрес каталога назначения
					if(env.isString("dest")){
						// Получаем адрес каталога
						const string & addr = env.get("dest");
						// Если адрес каталога получен
						if(!addr.empty()){
							// Адрес файла для сохранения
							string filename = addr;
							// Если каталог не существует
							if(fs.isDir(addr)){
								// Получаем расширение файла
								const string extension = env.get("to");
								// Переводим расширение в нижний регистр
								fmk.transform(extension, fmk_t::transform_t::LOWER);
								// Выполняем создание адреса файла для сохранения
								filename = fmk.format("%s%s%s.%s", addr.c_str(), FS_SEPARATOR, "result", extension.c_str());
							}
							// Буфер данных для записи
							string buffer = "";
							// Определяем тип файла
							switch(static_cast <uint8_t> (to)){
								// Если формат входящих данных указан как XML
								case static_cast <uint8_t> (type_t::XML):
									// Выполняем конвертирование в формат XML
									buffer = parser.xml(result, env.is("prettify"));
								break;
								// Если формат входящих данных указан как JSON
								case static_cast <uint8_t> (type_t::JSON):
									// Выполняем конвертирование в формат JSON
									buffer = parser.json(result, env.is("prettify"));
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
									if(env.isString("cef")){
										// Получаем режим парсинга
										const string & cef = env.get("cef");
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
									buffer = parser.csv(result, (env.isString("delim") ? env.get("delim").front() : ';'));
								break;
								// Если формат входящих данных указан как SysLog
								case static_cast <uint8_t> (type_t::SYSLOG):
									// Выполняем конвертирование в формат SysLog
									buffer = parser.syslog(result);
								break;
							}
							// Если данные для записи получены
							if(!buffer.empty())
								// Выполняем запись данных в файл
								fs.write(filename, buffer.data(), buffer.size());
							// Выводим сообщение об ошибке
							else log.print("Conversion from \"%s\" format to \"%s\" format is failed", log_t::flag_t::WARNING, fmk.transform(env.get("from"), fmk_t::transform_t::UPPER).c_str(), fmk.transform(env.get("to"), fmk_t::transform_t::UPPER).c_str());
							// Выводим удачное завершение работы
							return EXIT_SUCCESS;
						}
						// Выводим сообщение об ошибке
						log.print("Directory address is not set", log_t::flag_t::CRITICAL);
						// Выводим удачное завершение работы
						return EXIT_FAILURE;
					// Если адрес для сохранения файла не указан
					} else {
						// Определяем тип файла
						switch(static_cast <uint8_t> (to)){
							// Если формат входящих данных указан как XML
							case static_cast <uint8_t> (type_t::XML):
								// Выполняем конвертирование в формат XML
								cout << parser.xml(result, env.is("prettify")) << endl;
							break;
							// Если формат входящих данных указан как JSON
							case static_cast <uint8_t> (type_t::JSON):
								// Выполняем конвертирование в формат JSON
								cout << parser.json(result, env.is("prettify")) << endl;
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
								if(env.isString("cef")){
									// Получаем режим парсинга
									const string & cef = env.get("cef");
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
								cout << parser.csv(result, (env.isString("delim") ? env.get("delim").front() : ';')) << endl;
							break;
							// Если формат входящих данных указан как SysLog
							case static_cast <uint8_t> (type_t::SYSLOG):
								// Выполняем конвертирование в формат SysLog
								cout << parser.syslog(result) << endl;
							break;
						}
					}
				}
			// Если адрес файла хранения указан
			} else if(env.isString("src")) {
				// Получаем адрес файла или каталога
				const string & addr = env.get("src");
				// Если адрес является каталогом
				if(fs.isDir(addr)){
					// Получаем расширение файла
					const string extension = env.get("from");
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
							json result = json::object();
							// Формируем полученный текст
							const string text(buffer.begin(), buffer.end());
							// Определяем тип файла
							switch(static_cast <uint8_t> (from)){
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
									if(env.isString("cef")){
										// Получаем режим парсинга
										const string & cef = env.get("cef");
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
									result = parser.csv(text, env.is("header"));
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
							}
							// Если результат получен
							if(!result.empty()){
								// Если указан адрес каталога назначения
								if(env.isString("dest")){
									// Получаем адрес каталога
									const string & addr = env.get("dest");
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
										// Определяем тип файла
										switch(static_cast <uint8_t> (to)){
											// Если формат входящих данных указан как XML
											case static_cast <uint8_t> (type_t::XML):
												// Выполняем конвертирование в формат XML
												buffer = parser.xml(result, env.is("prettify"));
											break;
											// Если формат входящих данных указан как JSON
											case static_cast <uint8_t> (type_t::JSON):
												// Выполняем конвертирование в формат JSON
												buffer = parser.json(result, env.is("prettify"));
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
												if(env.isString("cef")){
													// Получаем режим парсинга
													const string & cef = env.get("cef");
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
												buffer = parser.csv(result, (env.isString("delim") ? env.get("delim").front() : ';'));
											break;
											// Если формат входящих данных указан как SysLog
											case static_cast <uint8_t> (type_t::SYSLOG):
												// Выполняем конвертирование в формат SysLog
												buffer = parser.syslog(result);
											break;
										}
										// Если данные для записи получены
										if(!buffer.empty()){
											// Получаем расширение файла
											const string extension = env.get("to");
											// Переводим расширение в нижний регистр
											fmk.transform(extension, fmk_t::transform_t::LOWER);
											// Выполняем запись данных в файл
											fs.write(fmk.format("%s%s%s.%s", addr.c_str(), FS_SEPARATOR, name.c_str(), extension.c_str()), buffer.data(), buffer.size());
										// Выводим сообщение об ошибке
										} else log.print("Conversion from \"%s\" format to \"%s\" format is failed", log_t::flag_t::WARNING, fmk.transform(env.get("from"), fmk_t::transform_t::UPPER).c_str(), fmk.transform(env.get("to"), fmk_t::transform_t::UPPER).c_str());
										// Выводим удачное завершение работы
										return;
									}
									// Выводим сообщение об ошибке
									log.print("Directory address is not set", log_t::flag_t::CRITICAL);
									// Выводим удачное завершение работы
									return;
								// Если адрес для сохранения файла не указан
								} else {
									// Определяем тип файла
									switch(static_cast <uint8_t> (to)){
										// Если формат входящих данных указан как XML
										case static_cast <uint8_t> (type_t::XML):
											// Выполняем конвертирование в формат XML
											cout << parser.xml(result, env.is("prettify")) << endl;
										break;
										// Если формат входящих данных указан как JSON
										case static_cast <uint8_t> (type_t::JSON):
											// Выполняем конвертирование в формат JSON
											cout << parser.json(result, env.is("prettify")) << endl;
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
											if(env.isString("cef")){
												// Получаем режим парсинга
												const string & cef = env.get("cef");
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
											cout << parser.csv(result, (env.isString("delim") ? env.get("delim").front() : ';')) << endl;
										break;
										// Если формат входящих данных указан как SysLog
										case static_cast <uint8_t> (type_t::SYSLOG):
											// Выполняем конвертирование в формат SysLog
											cout << parser.syslog(result) << endl;
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
						json result = json::object();
						// Формируем полученный текст
						const string text(buffer.begin(), buffer.end());
						// Определяем тип файла
						switch(static_cast <uint8_t> (from)){
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
								if(env.isString("cef")){
									// Получаем режим парсинга
									const string & cef = env.get("cef");
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
								result = parser.csv(text, env.is("header"));
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
						}
						// Если результат получен
						if(!result.empty()){
							// Если указан адрес каталога назначения
							if(env.isString("dest")){
								// Получаем адрес каталога
								const string & addr = env.get("dest");
								// Если адрес каталога получен
								if(!addr.empty()){
									// Адрес файла для сохранения
									string filename = addr;
									// Если каталог не существует
									if(fs.isDir(addr)){
										// Получаем расширение файла
										const string extension = env.get("to");
										// Переводим расширение в нижний регистр
										fmk.transform(extension, fmk_t::transform_t::LOWER);
										// Выполняем создание адреса файла для сохранения
										filename = fmk.format("%s%s%s.%s", addr.c_str(), FS_SEPARATOR, name.c_str(), extension.c_str());
									}
									// Буфер данных для записи
									string buffer = "";
									// Определяем тип файла
									switch(static_cast <uint8_t> (to)){
										// Если формат входящих данных указан как XML
										case static_cast <uint8_t> (type_t::XML):
											// Выполняем конвертирование в формат XML
											buffer = parser.xml(result, env.is("prettify"));
										break;
										// Если формат входящих данных указан как JSON
										case static_cast <uint8_t> (type_t::JSON):
											// Выполняем конвертирование в формат JSON
											buffer = parser.json(result, env.is("prettify"));
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
											if(env.isString("cef")){
												// Получаем режим парсинга
												const string & cef = env.get("cef");
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
											buffer = parser.csv(result, (env.isString("delim") ? env.get("delim").front() : ';'));
										break;
										// Если формат входящих данных указан как SysLog
										case static_cast <uint8_t> (type_t::SYSLOG):
											// Выполняем конвертирование в формат SysLog
											buffer = parser.syslog(result);
										break;
									}
									// Если данные для записи получены
									if(!buffer.empty())
										// Выполняем запись данных в файл
										fs.write(filename, buffer.data(), buffer.size());
									// Выводим сообщение об ошибке
									else log.print("Conversion from \"%s\" format to \"%s\" format is failed", log_t::flag_t::WARNING, fmk.transform(env.get("from"), fmk_t::transform_t::UPPER).c_str(), fmk.transform(env.get("to"), fmk_t::transform_t::UPPER).c_str());
									// Выводим удачное завершение работы
									return EXIT_SUCCESS;
								}
								// Выводим сообщение об ошибке
								log.print("Directory address is not set", log_t::flag_t::CRITICAL);
								// Выводим удачное завершение работы
								return EXIT_FAILURE;
							// Если адрес для сохранения файла не указан
							} else {
								// Определяем тип файла
								switch(static_cast <uint8_t> (to)){
									// Если формат входящих данных указан как XML
									case static_cast <uint8_t> (type_t::XML):
										// Выполняем конвертирование в формат XML
										cout << parser.xml(result, env.is("prettify")) << endl;
									break;
									// Если формат входящих данных указан как JSON
									case static_cast <uint8_t> (type_t::JSON):
										// Выполняем конвертирование в формат JSON
										cout << parser.json(result, env.is("prettify")) << endl;
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
										if(env.isString("cef")){
											// Получаем режим парсинга
											const string & cef = env.get("cef");
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
										cout << parser.csv(result, (env.isString("delim") ? env.get("delim").front() : ';')) << endl;
									break;
									// Если формат входящих данных указан как SysLog
									case static_cast <uint8_t> (type_t::SYSLOG):
										// Выполняем конвертирование в формат SysLog
										cout << parser.syslog(result) << endl;
									break;
								}
							}
						}
					}
				}
			// Выводим сообщение, что файл или каталог не указан
			} else log.print("Address of the file or directory for conversion is not specified", log_t::flag_t::CRITICAL);
		}
		// Выводим удачное завершение работы
		return EXIT_SUCCESS;
}
