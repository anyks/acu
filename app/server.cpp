/**
 * @file: server.cpp
 * @date: 2024-10-06
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
 * Подключаем модуль сервера
 */
#include <env.hpp>
#include <server.hpp>

// Подключаем пространство имён
using namespace anyks;

/**
 * config Функция загрузки данных конфигурационного файла
 * @param log объект для работы с логами
 * @param env объект для работы с переменными окружения
 * @param fs  объект работы с файловой системой
 */
static void config(const log_t * log, const env_t * env, const fs_t * fs) noexcept {
	// Если входящие данные переданы
	if((log != nullptr) && (env != nullptr) && (fs != nullptr)){
		// Значение параметра
		string configFile = env->get("config");
		// Если файл конфигурации не получен, ищем конфигурационный файл в системных каталогах
		if(configFile.empty()){
			// Получаем список системных каталогов
			vector <string> configs = ACU_CONFIG;
			// Если список системных каталогов получен
			if(!configs.empty()){
				// Переходим по всему списку системных каталогов
				for(auto & config : configs){
					// Устанавливаем адрес конфигурационного файла
					configFile = fs->realPath(config);
					// Если файл найден
					if(fs->isFile(config))
						// Выходим из цикла
						break;
				}
			}
		// Иначе получаем полный путь к файлу
		} else configFile = fs->realPath(configFile);
		// Если JSON файл указан
		if(!configFile.empty() && fs->isFile(configFile)){
			// Выполняем чтение конфигурационного файла
			const auto & config = fs->read(configFile);
			// Если данные получены
			if(!config.empty())
				// Устанавливаем собранный конфиг
				const_cast <env_t *> (env)->config(string(config.begin(), config.end()));
			// Выводим сообщение об ошибке
			else {
				// Выводим в лог сообщение
				log->print("%s", log_t::flag_t::CRITICAL, "Config file is not read");
				// Выходим из приложения
				::exit(EXIT_FAILURE);
			}
		}
	}
}
/**
 * help Функция вывода справки
 * @param name название приложения
 * @param fmk  объект фреймворка
 * @param env  объект для работы с переменными окружения
 * @param fs   объект работы с файловой системой
 */
void help(const string & name, const fmk_t * fmk, const env_t * env, const fs_t * fs) noexcept {
	// Если входящие данные переданы
	if(!name.empty() && (fmk != nullptr) && (env != nullptr)){
		// Строка адресов конфигурационных файлов и файловых путей
		string config = "";
		// Списки адресов конфигурационных файлов и файловых путей
		vector <string> configs = ACU_CONFIG;
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Если адрес конфигурационного файла указан
			if(env->is("config")){
				// Получаем адрес файла
				const string & filename = fs->realPath(env->get("config"));
				// Определяем существует ли файл по указанному адресу
				const bool exist = fs->isFile(filename);
				// Добавляем полученный файл
				config.append(fmk->format("\x1B[%sm\x1B[1m  %s\x1B[0m %s\r\n", (exist ? "32" : "35"), (exist ? "+" : "-"), filename.c_str()));
			}
		/**
		 * Если возникает ошибка
		 */
		} catch(const exception &) {
			/**
			 * Все возможные ошибки мы пропускаем
			 */
		}
		// Переходим по всему списку конфигурационных файлов
		for(auto & item : configs){
			// Получаем адрес файла
			const string & filename = fs->realPath(item);
			// Если адрес каталога со скриптами торговых ботов указан
			if(!env->is("config", true) || (fs->realPath(env->get("config", true)).compare(filename) != 0)){
				// Определяем существует ли файл по указанному адресу
				const bool exist = fs->isFile(filename);
				// Добавляем полученный файл
				config.append(fmk->format("\x1B[%sm\x1B[1m  %s\x1B[0m %s\r\n", (exist ? "32" : "35"), (exist ? "+" : "-"), filename.c_str()));
			}
		}
		// Формируем строку справки
		const string msg = "\r\n\x1B[32m\x1B[1musage:\x1B[0m %s [-V | --version] [-H | --info] [<args>]\r\n\r\n\r\n"
		"\x1B[34m\x1B[1m[ARGS]\x1B[0m\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m Format date: \x1B[1m[-formatDate \"<value>\" | --formatDate=\"<value>\"]\x1B[0m\r\n"
		"\x1B[32m\x1B[1m  -\x1B[0m ( %%m/%%d/%%Y %%H:%%M:%%S | %%H:%%M:%%S %%d.%%m.%%Y | ... )\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m File address for writing logs: \x1B[1m[-log <value> | --log=<value>]\x1B[0m\r\n\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m Logging level: \x1B[1m[-logLevel <value> | --logLevel=<value>]\x1B[0m\r\n"
		"\x1B[32m\x1B[1m  -\x1B[0m ( 0 = NONE | 1 = INFO | 2 = WARNING | 3 = CRITICAL | 4 = INFO and WARNING | 5 = INFO and CRITICAL | 6 = WARNING CRITICAL | 7 = ALL)\r\n\r\n"
		"\x1B[34m\x1B[1m[PATHS]\x1B[0m\r\n"
		"\x1B[33m\x1B[1m+\x1B[0m \x1B[1mConfiguration file address:\x1B[0m\r\n"
		"%s\r\n";
		// Выводим сообщение справки
		printf(msg.c_str(), name.c_str(), config.c_str());
	}
}
/**
 * daemon Функция создания демона
 * @param fmk объект фреймворка
 * @param log объект для работы с логами
 * @param env объект для работы с переменными окружения
 * @param fs  объект работы с файловой системой
 */
static void daemon(const fmk_t * fmk, const log_t * log, const env_t * env, const fs_t * fs) noexcept {
	// Если операционная система является *Nix-подобной
	#if defined(__APPLE__) || defined(__MACH__) || defined(__linux__) || defined(__FreeBSD__)
		// Если входящие данные переданы
		if((fmk != nullptr) && (log != nullptr) && (env != nullptr) && (fs != nullptr)){
			// Получаем название PID файла
			const string & pidfile = env->get("pidfile", true);
			// Если адрес PID файла получен
			if(!pidfile.empty()){
				// Получаем адрес файла PID
				const string & filename = fmk->format("%s%s%s", ACU_PID_PATH, FS_SEPARATOR, pidfile.c_str());
				// Если PID файл существует
				if(fs->isFile(filename))
					// Удаляем PID файл
					::unlink(filename.c_str());
				// Если сервер должен быть запущен в виде демона
				if(env->is("daemon", true) && env->get <bool> ("daemon", true)){
					// Ответвляемся от родительского процесса
					const pid_t pid = ::fork();
					// Если пид не создан тогда выходим
					if(pid < 0){
						// Выводим в лог сообщение
						log->print("%s", log_t::flag_t::CRITICAL, "Unable to detach from parent process");
						// Выходим из приложения
						::exit(EXIT_FAILURE);
					// Если с PID'ом все получилось, то родительский процесс можно завершить.
					} else if(pid > 0) {
						// Выходим из родительского процесса
						::exit(EXIT_SUCCESS);
						// Изменяем файловую маску
						::umask(0);
						// Здесь можно открывать любые журналы
						// Создание нового SID для дочернего процесса
						const pid_t sid = ::setsid();
						// Если идентификатор сессии дочернего процесса не существует
						if(sid < 0){
							// Выводим в лог сообщение
							log->print("%s", log_t::flag_t::CRITICAL, "Unable to create child process");
							// Выходим из приложения
							::exit(EXIT_FAILURE);
						}
						// Изменяем текущий рабочий каталог
						if((::chdir("/")) < 0){
							// Выводим в лог сообщение
							log->print("%s", log_t::flag_t::CRITICAL, "Unable to get root directory");
							// Выходим из приложения
							::exit(EXIT_FAILURE);
						}
						// Закрываем стандартные файловые дескрипторы
						::close(STDIN_FILENO);
						::close(STDOUT_FILENO);
						::close(STDERR_FILENO);
					}
				}
				// Открываем файл на запись
				std::ofstream file(filename, ios::out);
				// Если файл открыт
				if(file.is_open()){
					// Записываем PID идентификатор в файл
					file << std::to_string(::getpid());
					// Закрываем файл
					file.close();
				}
			}
		}
	#endif
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
		// Название приложения
		string name = "";
		// Создаём формат даты
		string formatDate = DATE_FORMAT;
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
		// Если адрес файла лога передан
		if(env.isString("log")){
			// Получаем адрес файла лога
			const string & filename = env.get("log");
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
		if(!env.size() || (env.is("info") || env.is("H"))){
			// Выполняем установку конфига
			config(&log, &env, &fs);
			// Выводим справочную информацию
			help(name, &fmk, &env, &fs);
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
		// Выполняем подключение конфигурационного файла
		config(&log, &env, &fs);
		// Выполняем инициализацию объекта сервера
		server_t server(&fmk, &log);
		// Выполняем установку конфигурационных параметров
		server.config(env.config());
		// Флаг удачной загрузки конфигурационных данных
		bool successConfig = (env.is("config") && fs.isFile(fs.realPath(env.get("config"))));
		// Если конфигурационный файл не загружен
		if(!successConfig){
			// Переходим по всему списку конфигурационных файлов
			for(auto & item : vector <string> (ACU_CONFIG)){
				// Проверяем существует ли конфигурационный файл по стандартным путям
				successConfig = fs.isFile(fs.realPath(item));
				// Если конфигурационный файл существует
				if(successConfig)
					// Выходим из цикла
					break;
			}
		}
		// Если конфигурационный файл загружен удачно
		if(successConfig){
			// Если уровень логирования передан
			if(env.isNumber("logLevel"))
				// Выполняем установку уровня логирования
				log.level(static_cast <log_t::level_t> (env.get <uint8_t> ("logLevel")));
			// Если уровень логирования прописан в конфигурационном файле
			else if(env.isNumber("logLevel", true))
				// Выполняем установку уровня логирования из конфигурационного файла
				log.level(static_cast <log_t::level_t> (env.get <uint8_t> ("logLevel", true)));
			// Выполняем создание демона
			daemon(&fmk, &log, &env, &fs);
			// Выполняем запуск сервера
			server.start();
			// Выводим удачное завершение работы
			return EXIT_SUCCESS;
		}
		// Выводим сообщение об ошибке
		log.print("Configuration file is not found", log_t::flag_t::CRITICAL);
		// Выводим удачное завершение работы
		return EXIT_FAILURE;
}
