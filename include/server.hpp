/**
 * @file: server.hpp
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

#ifndef __ACU_SERVER__
#define __ACU_SERVER__

/**
 * Стандартные модули
 */
#include <map>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

/**
 * Модули AWH
 */
#include <client/awh.hpp>
#include <server/awh.hpp>
#include <nlohmann/json.hpp>

/**
 * Наши модули
 */
#include <lib.hpp>
#include <parser.hpp>

// Подписываемся на стандартное пространство имён
using namespace std;
// Подписываемся на пространство имён awh
using namespace awh;

// Активируем json в качестве объекта пространства имён
using json = nlohmann::json;

/**
 * anyks пространство имён
 */
namespace anyks {
	/**
	 * Server Класс сервера
	 */
	typedef class ACUSHARED_EXPORT Server {
		private:
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
		private:
			/**
			 * Bandwidth Структура параметров пропускной способности сети
			 */
			typedef struct Bandwidth {
				string read;  // Ширина канала на чтение
				string write; // Ширина канала на запись
				/**
				 * Bandwidth Конструктор
				 */
				Bandwidth() noexcept : read{"100Mbps"}, write{"100Mbps"} {}
			} bandwidth_t;
		private:
			// Объект работы с файловой системой
			fs_t _fs;
			// Объект работы с URI параметрами
			uri_t _uri;
		private:
			// Адрес корневого каталога с сайтом
			string _root;
			// Адрес файла по умолчанию
			string _index;
			// Адрес сайта которому разрешён доступ к ресурсу
			string _origin;
			// Адрес хранения favicon.ico
			string _favicon;
		private:
			// Максимальное количество запросов на одного пользователя в сутки
			uint16_t _maxRequests;
		private:
			// Объект сетевого ядра сервера
			server::core_t _core;
			// Объект WEB-сервера
			server::awh_t _awh;
		private:
			// Объект ограничения скорости сети доступа к серверу
			bandwidth_t _bandwidth;
		private:
			// Чёрный список для IP-адресов 
			unordered_set <string> _ipBlack;
			// Белый список для IP-адресов
			unordered_set <string> _ipWhite;
		private:
			// Чёрный список для MAC адресов
			unordered_set <string> _macBlack;
			// Белый список для MAC адресов
			unordered_set <string> _macWhite;
		private:
			// Список пользователей для авторизации клиента
			unordered_map <string, string> _users;
		private:
			// Список запрещённых IP-адресов для DNS
			unordered_map <string, string> _dnsBlack;
		private:
			// Кэш контента содержимого сайта
			unordered_map <string, vector <char>> _cache;
		private:
			// Каунтеры запросов клиентов
			unordered_map <string, pair <uint16_t, time_t>> _counts;
		private:
			// Объект фреймворка
			const fmk_t * _fmk;
			// Объект работы с логами
			const log_t * _log;
		private:
			/**
			 * crash Метод обработки вызова крашей в приложении
			 * @param sig номер сигнала операционной системы
			 */
			void crash(const int sig) noexcept;
			/**
			 * error Метод генерации ошибки
			 * @param sid  идентификатор потока
			 * @param bid  идентификатор брокера
			 * @param code код ответа сервера
			 * @param mess сообщение ответа клиенту
			 */
			void error(const int32_t sid, const uint64_t bid, const uint16_t code, const string & mess) noexcept;
		private:
			/**
			 * password Метод извлечения пароля (для авторизации методом Digest)
			 * @param bid   идентификатор брокера (клиента)
			 * @param login логин пользователя
			 * @return      пароль пользователя хранящийся в базе данных
			 */
			string password(const uint64_t bid, const string & login) noexcept;
			/**
			 * auth Метод проверки авторизации пользователя (для авторизации методом Basic)
			 * @param bid      идентификатор брокера (клиента)
			 * @param login    логин пользователя (от клиента)
			 * @param password пароль пользователя (от клиента)
			 * @return         результат авторизации
			 */
			bool auth(const uint64_t bid, const string & login, const string & password) noexcept;
		private:
			/**
			 * accept Метод активации клиента на сервере
			 * @param ip   адрес интернет подключения
			 * @param mac  аппаратный адрес подключения
			 * @param port порт подключения
			 * @return     результат проверки
			 */
			bool accept(const string & ip, const string & mac, const uint32_t port) noexcept;
		private:
			/**
			 * active Метод вывода статуса работы сетевого ядра
			 * @param status флаг запуска сетевого ядра
			 */
			void active(const awh::core_t::status_t status) noexcept;
			/**
			 * active Метод идентификации активности на Web сервере
			 * @param bid  идентификатор брокера (клиента)
			 * @param mode режим события подключения
			 */
			void active(const uint64_t bid, const server::web_t::mode_t mode) noexcept;
		private:
			/**
			 * handshake Метод получения удачного запроса
			 * @param sid   идентификатор потока
			 * @param bid   идентификатор брокера
			 * @param agent идентификатор агента клиента
			 */
			void handshake(const int32_t sid, const uint64_t bid, const server::web_t::agent_t agent) noexcept;
			/**
			 * request Метод вывода входящего запроса
			 * @param sid     идентификатор входящего потока
			 * @param bid     идентификатор брокера (клиента)
			 * @param method  метод входящего запроса
			 * @param url     адрес входящего запроса
			 * @param headers заголовки запроса
			 */
			void headers(const int32_t sid, const uint64_t bid, const awh::web_t::method_t method, const uri_t::url_t & url, const unordered_multimap <string, string> & headers) noexcept;
			/**
			 * complete Метод завершения получения запроса клиента
			 * @param sid     идентификатор потока
			 * @param bid     идентификатор брокера
			 * @param method  метод запроса
			 * @param url     url-адрес запроса
			 * @param entity  тело запроса
			 * @param headers заголовки запроса
			 */
			void complete(const int32_t sid, const uint64_t bid, const awh::web_t::method_t method, const uri_t::url_t & url, const vector <char> & entity, const unordered_multimap <string, string> & headers) noexcept;
		public:
			/**
			 * config Метод установки конфигурационных параметров в формате JSON
			 * @param config объект конфигурационных параметров в формате JSON
			 */
			void config(const json & config) noexcept;
		public:
			/**
			 * stop Метод остановки работы сервера
			 */
			void stop() noexcept;
			/**
			 * start Метод запуска работы сервера
			 */
			void start() noexcept;
		public:
			/**
			 * Server конструктор
			 * @param fmk объект фреймворка
			 * @param log объект для работы с логами
			 */
			Server(const fmk_t * fmk, const log_t * log) noexcept;
			/**
			 * ~Server деструктор
			 */
			~Server() noexcept;
	} server_t;
};

#endif // __ACU_SERVER__
