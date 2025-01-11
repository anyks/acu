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
 * Подключаем заголовочный файл
 */
#include <server.hpp>

// Подписываемся на стандартное пространство имён
using namespace std;
// Подписываемся на пространство имён awh
using namespace awh;
// Подписываемся на пространство имён rapidjson
using namespace rapidjson;

/**
 * crash Метод обработки вызова крашей в приложении
 * @param sig номер сигнала операционной системы
 */
void anyks::Server::crash(const int32_t sig) noexcept {
	// Если мы получили сигнал завершения работы
	if(sig == 2)
		// Выводим сообщение о заверщении работы
		this->_log->print("%s finishing work, goodbye!", log_t::flag_t::INFO, ACU_NAME);
	// Выводим сообщение об ошибке
	else this->_log->print("%s cannot be continued, signal: [%u]. Finishing work, goodbye!", log_t::flag_t::CRITICAL, ACU_NAME, sig);
	// Выполняем отключение вывода лога
	const_cast <awh::log_t *> (this->_log)->level(awh::log_t::level_t::NONE);
	// Выполняем остановку работы сервера
	this->stop();
	// Завершаем работу приложения
	::exit(sig);
}
/**
 * error Метод генерации ошибки
 * @param sid  идентификатор потока
 * @param bid  идентификатор брокера
 * @param code код ответа сервера
 * @param mess сообщение ответа клиенту
 */
void anyks::Server::error(const int32_t sid, const uint64_t bid, const uint16_t code, const string & mess) noexcept {
	// Если текст и код ошибки переданы
	if((code > 400) && !mess.empty()){
		// Формируем адрес файла
		const string & filename = this->_fs.realPath(this->_fmk->format("%s/%u.html", this->_root.c_str(), code), false);
		// Выполняем поиск запрошенного файла в кэше
		auto i = this->_cache.find(filename);
		// Если запрошенный файл в кэше найден
		if(i != this->_cache.end()){
			// Отправляем сообщение клиенту
			this->_awh.send(sid, bid, code, mess, i->second.second, {
				{"Accept-Ranges", "bytes"},
				{"Vary", "Accept-Encoding"},
				{"Content-Type", "text/html; charset=utf-8"},
				{"Access-Control-Request-Headers", "Content-Type"},
				{"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
				{"Access-Control-Allow-Origin", !this->_origin.empty() ? this->_origin : "*"}
			});
		// Если в кэше данные не обнаружены, но есть в файловой системе
		} else if(this->_fs.isFile(filename)) {
			// Выполняем чтение запрошенного файла
			const auto & buffer = this->_fs.read(filename);
			// Выполняем добавление файла в кэш
			this->_cache.emplace(filename, make_pair(13, buffer));
			// Отправляем сообщение клиенту
			this->_awh.send(sid, bid, code, mess, buffer, {
				{"Accept-Ranges", "bytes"},
				{"Vary", "Accept-Encoding"},
				{"Content-Type", "text/html; charset=utf-8"},
				{"Access-Control-Request-Headers", "Content-Type"},
				{"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
				{"Access-Control-Allow-Origin", !this->_origin.empty() ? this->_origin : "*"}
			});
		}
		// Выводим сообщение инициализации метода класса скрипта сервера
		this->_log->print("Response: %s", log_t::flag_t::CRITICAL, mess.c_str());
	// Если ошибка передана и текст сообщения тоже
	} else if((code == 400) && !mess.empty()) {
		// Выполняем формирование результата ответа
		json answer(kObjectType);
		// Выполняем формирование результата
		answer.AddMember(Value("error", answer.GetAllocator()).Move(), Value(mess.c_str(), mess.length(), answer.GetAllocator()).Move(), answer.GetAllocator());
		// Создаём результьрующий буфер
		rapidjson::StringBuffer result;
		// Выполняем создание объекта писателя
		Writer <StringBuffer> writer(result);
		// Передаем данные объекта JSON писателю
		answer.Accept(writer);
		// Выполняем получение буфера данных для отправки
		const string & buffer = result.GetString();
		// Отправляем сообщение клиенту
		this->_awh.send(sid, bid, code, mess, vector <char> (buffer.begin(), buffer.end()), {
			{"Accept-Ranges", "bytes"},
			{"Vary", "Accept-Encoding"},
			{"Content-Type", "application/json"},
			{"Access-Control-Request-Headers", "Content-Type"},
			{"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
			{"Access-Control-Allow-Origin", !this->_origin.empty() ? this->_origin : "*"}
		});
		// Выводим сообщение инициализации метода класса скрипта сервера
		this->_log->print("Response: %s", log_t::flag_t::CRITICAL, mess.c_str());
	}
}
/**
 * password Метод извлечения пароля (для авторизации методом Digest)
 * @param bid   идентификатор брокера (клиента)
 * @param login логин пользователя
 * @return      пароль пользователя хранящийся в базе данных
 */
string anyks::Server::password(const uint64_t bid, const string & login) noexcept {
	// Выполняем поиск пользователя в списке
	auto i = this->_users.find(login);
	// Если пользователь найден
	if(i != this->_users.end())
		// Выводим пароль пользователя
		return i->second;
	// Сообщаем, что пользователь не найден
	return "";
}
/**
 * auth Метод проверки авторизации пользователя (для авторизации методом Basic)
 * @param bid      идентификатор брокера (клиента)
 * @param login    логин пользователя (от клиента)
 * @param password пароль пользователя (от клиента)
 * @return         результат авторизации
 */
bool anyks::Server::auth(const uint64_t bid, const string & login, const string & password) noexcept {
	// Выполняем поиск пользователя в списке
	auto i = this->_users.find(login);
	// Если пользователь найден
	if(i != this->_users.end())
		// Выводим результат проверки
		return (i->second.compare(password) == 0);
	// Сообщаем, что пользователь неверный
	return false;
}
/**
 * accept Метод активации клиента на сервере
 * @param ip   адрес интернет подключения
 * @param mac  аппаратный адрес подключения
 * @param port порт подключения
 * @return     результат проверки
 */
bool anyks::Server::accept(const string & ip, const string & mac, const uint32_t port) noexcept {
	// Выводим сообщение в лог
	this->_log->print("ACCEPT: IP=%s, MAC=%s, PORT=%u", log_t::flag_t::INFO, ip.c_str(), mac.c_str(), port);
	// Если хоть один список не является пустым
	if(!this->_ipWhite.empty() || !this->_macWhite.empty() || !this->_ipBlack.empty() || !this->_macBlack.empty()){
		// Переводим IP-адрес в нижний регистр
		this->_fmk->transform(ip, fmk_t::transform_t::LOWER);
		// Переводим MAC адрес в нижний регистр
		this->_fmk->transform(mac, fmk_t::transform_t::LOWER);
		// Определяем разрешено ли подключение к серверу клиента
		return (
			((this->_ipWhite.find(ip) != this->_ipWhite.end()) ||
			(this->_macWhite.find(mac) != this->_macWhite.end())) ||
			((this->_ipBlack.find(ip) == this->_ipBlack.end()) &&
			(this->_macBlack.find(mac) == this->_macBlack.end()))
		);
	}
	// Разрешаем подключение клиенту
	return true;
}
/**
 * active Метод вывода статуса работы сетевого ядра
 * @param status флаг запуска сетевого ядра
 */
void anyks::Server::active(const awh::core_t::status_t status) noexcept {
	// Определяем статус активности сетевого ядра
	switch(static_cast <uint8_t> (status)){
		// Если система запущена
		case static_cast <uint8_t> (awh::core_t::status_t::START):
			// Выполняем инициализацию сервера
			// this->init();
		break;
		// Если система остановлена
		case static_cast <uint8_t> (awh::core_t::status_t::STOP): {
			/*
			// Очищаем список активных связей
			this->_bonds.clear();
			// Очищаем список активных брокеров
			this->_brokers.clear();
			// Очищаем список активных воркеров
			this->_workers.clear();
			*/
		} break;
	}
}
/**
 * active Метод идентификации активности на Web сервере
 * @param bid  идентификатор брокера (клиента)
 * @param mode режим события подключения
 */
void anyks::Server::active(const uint64_t bid, const server::web_t::mode_t mode) noexcept {
	// Определяем тип события
	switch(static_cast <uint8_t> (mode)){
		// Если произведено подключение клиента к серверу
		case static_cast <uint8_t> (server::web_t::mode_t::CONNECT): {
			// Выполняем установку ограничения пропускной способности сети
			this->_core.bandwidth(bid, this->_bandwidth.read, this->_bandwidth.write);
			// Выводим информацию в лог
			this->_log->print("%s client", log_t::flag_t::INFO, "Connect");
		} break;
		// Если произведено отключение клиента от сервера
		case static_cast <uint8_t> (server::web_t::mode_t::DISCONNECT):
			// Выводим информацию в лог
			this->_log->print("%s client", log_t::flag_t::INFO, "Disconnect");
		break;
	}
}
/**
 * handshake Метод получения удачного запроса
 * @param sid   идентификатор потока
 * @param bid   идентификатор брокера
 * @param agent идентификатор агента клиента
 */
void anyks::Server::handshake(const int32_t sid, const uint64_t bid, const server::web_t::agent_t agent) noexcept {
	// Если агентом не является HTTP-клиент
	if(agent != server::web_t::agent_t::HTTP){
		// Выпоолняем генерацию ошибки запроса
		this->error(sid, bid, 403, "Unauthorized connection attempt");
		// Выполняем закрытие подключения
		this->_awh.close(bid);
	}
}
/**
 * request Метод вывода входящего запроса
 * @param sid     идентификатор входящего потока
 * @param bid     идентификатор брокера (клиента)
 * @param method  метод входящего запроса
 * @param url     адрес входящего запроса
 * @param headers заголовки запроса
 */
void anyks::Server::headers(const int32_t sid, const uint64_t bid, const awh::web_t::method_t method, const uri_t::url_t & url, const unordered_multimap <string, string> & headers) noexcept {
	// Если выполняем поиска заголовка Origin
	auto i = headers.find("origin");
	// Если заголовок не получен
	if(!this->_origin.empty() && (method == awh::web_t::method_t::POST) && ((i != headers.end()) && !this->_fmk->compare(this->_origin, i->second))){
		// Выпоолняем генерацию ошибки запроса
		this->error(sid, bid, 403, "This resource is denied access to the API trading platform");
		// Выполняем закрытие подключения
		this->_awh.close(bid);
	}
}
/**
 * complete Метод завершения получения запроса клиента
 * @param sid     идентификатор потока
 * @param bid     идентификатор брокера
 * @param method  метод запроса
 * @param url     url-адрес запроса
 * @param entity  тело запроса
 * @param headers заголовки запроса
 */
void anyks::Server::complete(const int32_t sid, const uint64_t bid, const awh::web_t::method_t method, const uri_t::url_t & url, const vector <char> & entity, const unordered_multimap <string, string> & headers) noexcept {
	/**
	 * Выполняем перехват ошибок
	 */
	try {
		// Результат отправки ответа
		bool result = false;
		// Формируем адрес запроса
		string addr = this->_uri.joinPath(url.path);
		// Определяем метод входящего запроса
		switch(static_cast <uint8_t> (method)){
			// Если мы получили GET запрос
			case static_cast <uint8_t> (awh::web_t::method_t::GET): {
				// Если браузер запрашивает иконку приложения
				if(!this->_favicon.empty() && this->_fmk->compare("/favicon.ico", addr)){
					// Если файл существует в файловой системе
					if(this->_fs.isFile(this->_favicon)){
						// Выполняем чтение favicon
						const auto & buffer = this->_fs.read(this->_favicon);
						// Если буфер бинарных данных сформирован
						if((result = !buffer.empty())){
							// Выполняем получение штампа времени даты компиляции приложения
							const time_t date = this->_fmk->str2time(this->_fmk->format("%s %s", __DATE__, __TIME__), "%b %d %Y %H:%M:%S");
							// Отправляем сообщение клиенту
							this->_awh.send(sid, bid, 200, "OK", buffer, {
								{"Etag", "65d46266-47e"},
								{"Accept-Ranges", "bytes"},
								{"Vary", "Accept-Encoding"},
								{"Content-Type", "image/x-icon"},
								{"Last-Modified", this->_http.date(date)},
								{"Access-Control-Request-Headers", "Content-Type"},
								{"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
								{"Access-Control-Allow-Origin", !this->_origin.empty() ? this->_origin : "*"}
							});
						}
					}
				// Если был запрошен другой файл
				} else {
					// Тип отдаваемого контента
					string contentType = "text/plain";
					// Если адрес указан как корень
					if(this->_fmk->compare("/", addr))
						// Устанавливаем адрес файла по умолчанию
						addr = this->_fmk->format("/%s", this->_index.c_str());
					// Определяем расширение запрашиваемого файла
					const string & extension = this->_fs.components(addr, false).second;
					// Формируем адрес файла
					const string & filename = this->_fs.realPath(this->_fmk->format("%s%s", this->_root.c_str(), addr.c_str()), false);
					// Если запрашиваемый файл является HTML
					if(this->_fmk->compare("html", extension))
						// Выполняем установку типа контента
						contentType = "text/html; charset=utf-8";
					// Если запрашиваемый файл является CSS
					else if(this->_fmk->compare("css", extension))
						// Выполняем установку типа контента
						contentType = "text/css; charset=utf-8";
					// Если запрашиваемый файл является JS
					else if(this->_fmk->compare("js", extension))
						// Выполняем установку типа контента
						contentType = "application/javascript; charset=utf-8";
					// Если запрашиваемый файл является WOFF2
					else if(this->_fmk->compare("woff2", extension))
						// Выполняем установку типа контента
						contentType = "font/woff2";
					// Если запрашиваемый файл является PNG
					else if(this->_fmk->compare("png", extension))
						// Выполняем установку типа контента
						contentType = "image/png";
					// Если запрашиваемый файл является GIF
					else if(this->_fmk->compare("gif", extension))
						// Выполняем установку типа контента
						contentType = "image/gif";
					// Если запрашиваемый файл является WEBP
					else if(this->_fmk->compare("webp", extension))
						// Выполняем установку типа контента
						contentType = "image/webp";
					// Если запрашиваемый файл является SVG
					else if(this->_fmk->compare("svg", extension))
						// Выполняем установку типа контента
						contentType = "image/svg+xml";
					// Если запрашиваемый файл является ICO
					else if(this->_fmk->compare("ico", extension))
						// Выполняем установку типа контента
						contentType = "image/vnd.microsoft.icon";
					// Если запрашиваемый файл является ZIP
					else if(this->_fmk->compare("zip", extension))
						// Выполняем установку типа контента
						contentType = "application/zip";
					// Если запрашиваемый файл является XML
					else if(this->_fmk->compare("xml", extension))
						// Выполняем установку типа контента
						contentType = "application/xml";
					// Если запрашиваемый файл является JSON
					else if(this->_fmk->compare("json", extension))
						// Выполняем установку типа контента
						contentType = "application/json";
					// Если запрашиваемый файл является YAML
					else if(this->_fmk->compare("yaml", extension))
						// Выполняем установку типа контента
						contentType = "application/x-yaml";
					// Если запрашиваемый файл является RPM
					else if(this->_fmk->compare("rpm", extension))
						// Выполняем установку типа контента
						contentType = "application/x-rpm";
					// Если запрашиваемый файл является BIN
					else if(this->_fmk->compare("bin", extension))
						// Выполняем установку типа контента
						contentType = "application/octet-stream";
					// Если запрашиваемый файл является DEB
					else if(this->_fmk->compare("deb", extension))
						// Выполняем установку типа контента
						contentType = "application/x-debian-package";
					// Если запрашиваемый файл является DMG
					else if(this->_fmk->compare("dmg", extension))
						// Выполняем установку типа контента
						contentType = "application/x-apple-diskimage";
					// Если запрашиваемый файл является TAR.GZ
					else if(this->_fmk->compare("gz", extension))
						// Выполняем установку типа контента
						contentType = "application/tar+gzip";
					// Если запрашиваемый файл является MPKG
					else if(this->_fmk->compare("mpkg", extension))
						// Выполняем установку типа контента
						contentType = "application/vnd.apple.installer+xml";
					// Если запрашиваемый файл является EXE
					else if(this->_fmk->compare("exe", extension))
						// Выполняем установку типа контента
						contentType = "application/vnd.microsoft.portable-executable";
					// Если запрашиваемый файл является JPG
					else if(this->_fmk->compare("jpg", extension) || this->_fmk->compare("jpeg", extension))
						// Выполняем установку типа контента
						contentType = "image/jpeg";
					// Выполняем получение штампа времени даты компиляции приложения
					const time_t date = this->_fmk->str2time(this->_fmk->format("%s %s", __DATE__, __TIME__), "%b %d %Y %H:%M:%S");
					// Выполняем поиск запрошенного файла в кэше
					auto i = this->_cache.find(filename);
					// Если запрошенный файл в кэше найден
					if((result = (i != this->_cache.end()))){
						// Выполняем поиск заголовка проверки ETag
						auto j = headers.find("if-none-match");
						// Если заголовок с хештегом найден
						if(j != headers.end()){
							/**
							 * Выполняем отлов ошибок
							 */
							try {
								// Получаем заголовко запроса
								const string header(j->second.begin() + 1, j->second.end() - 1);
								// Выводим значение заголовка
								if(i->second.first == ::stoull(header)){
									// Отправляем сообщение клиенту
									this->_awh.send(sid, bid, 304, "Not Modified", {}, {
										{"Vary", "Accept-Encoding"},
										{"Last-Modified", this->_http.date(date)},
										{"Date", this->_http.date(::time(nullptr))},
										{"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
										{"ETag", this->_fmk->format("\"%llu\"", i->second.first)},
										{"Access-Control-Allow-Origin", !this->_origin.empty() ? this->_origin : "*"}
									});
									// Выходим из функции
									return;
								}
							/**
							 * Если возникает ошибка
							 */
							} catch(const exception &) {
								/* Ничего не делаем */
							}
						}
						// Отправляем сообщение клиенту
						this->_awh.send(sid, bid, 200, "OK", i->second.second, {
							{"Accept-Ranges", "bytes"},
							{"Vary", "Accept-Encoding"},
							{"Content-Type", contentType},
							{"Last-Modified", this->_http.date(date)},
							{"Date", this->_http.date(::time(nullptr))},
							{"Access-Control-Request-Headers", "Content-Type"},
							{"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
							{"ETag", this->_fmk->format("\"%llu\"", i->second.first)},
							{"Access-Control-Allow-Origin", !this->_origin.empty() ? this->_origin : "*"}
						});
					// Если в кэше данные не обнаружены, но есть в файловой системе
					} else if((result = this->_fs.isFile(filename))) {
						// Выполняем чтение запрошенного файла
						const auto & buffer = this->_fs.read(filename);
						// Получаем хэш буфера данных
						const uint64_t etag = CityHash64(buffer.data(), buffer.size());
						// Выполняем добавление файла в кэш
						this->_cache.emplace(filename, make_pair(etag, buffer));
						// Выполняем поиск заголовка проверки ETag
						auto j = headers.find("if-none-match");
						// Если заголовок с хештегом найден
						if(j != headers.end()){
							/**
							 * Выполняем отлов ошибок
							 */
							try {
								// Получаем заголовко запроса
								const string header(j->second.begin() + 1, j->second.end() - 1);
								// Выводим значение заголовка
								if(etag == ::stoull(header)){
									// Отправляем сообщение клиенту
									this->_awh.send(sid, bid, 304, "Not Modified", {}, {
										{"Vary", "Accept-Encoding"},
										{"Last-Modified", this->_http.date(date)},
										{"Date", this->_http.date(::time(nullptr))},
										{"ETag", this->_fmk->format("\"%llu\"", etag)},
										{"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
										{"Access-Control-Allow-Origin", !this->_origin.empty() ? this->_origin : "*"}
									});
									// Выходим из функции
									return;
								}
							/**
							 * Если возникает ошибка
							 */
							} catch(const exception &) {
								/* Ничего не делаем */
							}
						}
						// Отправляем сообщение клиенту
						this->_awh.send(sid, bid, 200, "OK", buffer, {
							{"Accept-Ranges", "bytes"},
							{"Vary", "Accept-Encoding"},
							{"Content-Type", contentType},
							{"Last-Modified", this->_http.date(date)},
							{"Date", this->_http.date(::time(nullptr))},
							{"ETag", this->_fmk->format("\"%llu\"", etag)},
							{"Access-Control-Request-Headers", "Content-Type"},
							{"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
							{"Access-Control-Allow-Origin", !this->_origin.empty() ? this->_origin : "*"}
						});
					}
				}
			} break;
			// Если мы получили POST запрос
			case static_cast <uint8_t> (awh::web_t::method_t::POST): {
				// Получаем значение IP-адреса
				const string & ip = this->_awh.ip(bid);
				// Выполняем получение каунтеров запроса клиента
				auto i = this->_counts.find(ip);
				// Если каунтер клиента получен
				if(i != this->_counts.end()){
					// Получаем текущее значение даты
					const time_t date = this->_fmk->timestamp(fmk_t::stamp_t::MILLISECONDS);
					// Выполняем проверку прошли ли сутки с момента предыдущего запроса
					if((date - i->second.second) >= 86400000){
						// Выполняем сброс количества выполненных запросов
						i->second.first = 0;
						// Выполняем обновление значение даты
						i->second.second = date;
					}
					// Если количество запросов вышло за пределы
					if(i->second.first >= this->_maxRequests){
						// Выпоолняем генерацию ошибки запроса
						this->error(sid, bid, 400, "Your daily request limit has been reached");
						// Выводим удачное завершение работы
						return;
					// Увеличиваем количество выполненных запросов
					} else i->second.first++;
				// Выполняем заполнение списка количества запросов
				} else this->_counts.emplace(ip, make_pair(1, this->_fmk->timestamp(fmk_t::stamp_t::MILLISECONDS)));
				// Если производится вызов метода /exec
				if(this->_fmk->compare("/exec", addr)){
					// Объект запроса
					json request;
					// Выполняем парсинг полученных текстовых данных
					if(request.Parse(entity.data(), entity.size()).HasParseError()){
						/**
						 * Если включён режим отладки
						 */
						#if defined(DEBUG_MODE)
							// Выводим сообщение об ошибке
							this->_log->debug("Request JSON: (offset %d): %s", __PRETTY_FUNCTION__, make_tuple(sid, bid, static_cast <uint16_t> (method), url, entity.size(), headers.size()), log_t::flag_t::CRITICAL, request.GetErrorOffset(), GetParseError_En(request.GetParseError()));
						/**
						* Если режим отладки не включён
						*/
						#else
							// Выводим сообщение об ошибке
							this->_log->print("Request JSON: (offset %d): %s", log_t::flag_t::CRITICAL, request.GetErrorOffset(), GetParseError_En(request.GetParseError()));
						#endif
					}
					// Если результат получен
					if(request.IsObject() && !request.ObjectEmpty()){
						// Если поля указаны правильно
						if(request.HasMember("text") && request.HasMember("from") && request.HasMember("to") &&
						   request["text"].IsString() && request["from"].IsString() && request["to"].IsString()){
							// Регулярное выражение в формате GROK
							string express = "";
							// Выполняем инициализацию объекта парсера
							parser_t parser(this->_fmk, this->_log);
							// Тип конвертируемого формата данных и тип формата для конвертации
							type_t from = type_t::TEXT, to = type_t::TEXT;
							// Если формат входящих данных указан как Text
							if(this->_fmk->compare("text", request["from"].GetString()))
								// Определяем формат данных
								from = type_t::TEXT;
							// Если формат входящих данных указан как XML
							else if(this->_fmk->compare("xml", request["from"].GetString()))
								// Определяем формат данных
								from = type_t::XML;
							// Если формат входящих данных указан как JSON
							else if(this->_fmk->compare("json", request["from"].GetString()))
								// Определяем формат данных
								from = type_t::JSON;
							// Если формат входящих данных указан как INI
							else if(this->_fmk->compare("ini", request["from"].GetString()))
								// Определяем формат данных
								from = type_t::INI;
							// Если формат входящих данных указан как YAML
							else if(this->_fmk->compare("yaml", request["from"].GetString()))
								// Определяем формат данных
								from = type_t::YAML;
							// Если формат входящих данных указан как CEF
							else if(this->_fmk->compare("cef", request["from"].GetString()))
								// Определяем формат данных
								from = type_t::CEF;
							// Если формат входящих данных указан как CSV
							else if(this->_fmk->compare("csv", request["from"].GetString()))
								// Определяем формат данных
								from = type_t::CSV;
							// Если формат входящих данных указан как GROK
							else if(this->_fmk->compare("grok", request["from"].GetString())) {
								// Определяем формат данных
								from = type_t::GROK;
								// Если файл шаблона указан
								if(request.HasMember("patterns") && request["patterns"].IsObject()){
									// Создаём объект JSON шаблонов
									json patterns;
									// Выполняем копирование полученных данных
									patterns.CopyFrom(request["patterns"], patterns.GetAllocator());
									// Выполняем добавление поддерживаемых шаблонов
									parser.patterns(::move(patterns));
								}
								// Если регулярное выражение передано
								if(request.HasMember("express") && request["express"].IsString())
									// Устанавливаем полученное регулярное выражение
									express = request["express"].GetString();
								// Выводим сообщение об ошибке
								else {
									// Выпоолняем генерацию ошибки запроса
									this->error(sid, bid, 400, "Regular expression in GROK format is not set");
									// Выводим удачное завершение работы
									return;
								}
							// Если формат входящих данных указан как SysLog
							} else if(this->_fmk->compare("syslog", request["from"].GetString()))
								// Определяем формат данных
								from = type_t::SYSLOG;
							// Если формат входящих данных указан как Base64
							else if(this->_fmk->compare("base64", request["from"].GetString()))
								// Определяем формат данных
								from = type_t::BASE64;
							// Если формат не определён
							else {
								// Выпоолняем генерацию ошибки запроса
								this->error(sid, bid, 400, "From format is not recognized");
								// Выводим удачное завершение работы
								return;
							}
							// Если формат исходящих данных указан как Text
							if(this->_fmk->compare("text", request["to"].GetString()))
								// Определяем формат данных
								to = type_t::TEXT;
							// Если формат исходящих данных указан как XML
							else if(this->_fmk->compare("xml", request["to"].GetString()))
								// Определяем формат данных
								to = type_t::XML;
							// Если формат исходящих данных указан как JSON
							else if(this->_fmk->compare("json", request["to"].GetString()))
								// Определяем формат данных
								to = type_t::JSON;
							// Если формат исходящих данных указан как INI
							else if(this->_fmk->compare("ini", request["to"].GetString()))
								// Определяем формат данных
								to = type_t::INI;
							// Если формат исходящих данных указан как YAML
							else if(this->_fmk->compare("yaml", request["to"].GetString()))
								// Определяем формат данных
								to = type_t::YAML;
							// Если формат исходящих данных указан как CEF
							else if(this->_fmk->compare("cef", request["to"].GetString()))
								// Определяем формат данных
								to = type_t::CEF;
							// Если формат исходящих данных указан как CSV
							else if(this->_fmk->compare("csv", request["to"].GetString()))
								// Определяем формат данных
								to = type_t::CSV;
							// Если формат исходящих данных указан как SysLog
							else if(this->_fmk->compare("syslog", request["to"].GetString()))
								// Определяем формат данных
								to = type_t::SYSLOG;
							// Если формат исходящих данных указан как Base64
							else if(this->_fmk->compare("base64", request["to"].GetString()))
								// Определяем формат данных
								to = type_t::BASE64;
							// Если формат исходящих данных указан как MD5
							else if(this->_fmk->compare("md5", request["to"].GetString()))
								// Определяем формат данных
								to = type_t::MD5;
							// Если формат исходящих данных указан как SHA1
							else if(this->_fmk->compare("sha1", request["to"].GetString()))
								// Определяем формат данных
								to = type_t::SHA1;
							// Если формат исходящих данных указан как SHA224
							else if(this->_fmk->compare("sha224", request["to"].GetString()))
								// Определяем формат данных
								to = type_t::SHA224;
							// Если формат исходящих данных указан как SHA256
							else if(this->_fmk->compare("sha256", request["to"].GetString()))
								// Определяем формат данных
								to = type_t::SHA256;
							// Если формат исходящих данных указан как SHA384
							else if(this->_fmk->compare("sha384", request["to"].GetString()))
								// Определяем формат данных
								to = type_t::SHA384;
							// Если формат исходящих данных указан как SHA512
							else if(this->_fmk->compare("sha512", request["to"].GetString()))
								// Определяем формат данных
								to = type_t::SHA512;
							// Если формат не определён
							else {
								// Выпоолняем генерацию ошибки запроса
								this->error(sid, bid, 400, "To format is not recognized");
								// Выводим удачное завершение работы
								return;
							}
							// Получаем ключ HMAC
							string hmac = "";
							// Если ключ проверки подлинности передан
							if(request.HasMember("hmac") && request["hmac"].IsString()){
								// Получаем ключ подтверждения подлинности
								hmac = request["hmac"].GetString();
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
							// Объект ответа парсера в формате JSON
							json answer(kObjectType);
							// Определяем формат данных
							switch(static_cast <uint8_t> (from)){
								// Если формат входящих данных указан как Text
								case static_cast <uint8_t> (type_t::TEXT):
									// Выполняем передачу данных как она есть
									answer.SetString(request["text"].GetString(), answer.GetAllocator());
								break;
								// Если формат входящих данных указан как XML
								case static_cast <uint8_t> (type_t::XML):
									// Выполняем конвертацию данных
									answer = parser.xml(request["text"].GetString());
								break;
								// Если формат входящих данных указан как JSON
								case static_cast <uint8_t> (type_t::JSON):
									// Выполняем конвертацию данных
									answer = parser.json(request["text"].GetString());
								break;
								// Если формат входящих данных указан как INI
								case static_cast <uint8_t> (type_t::INI):
									// Выполняем конвертацию данных
									answer = parser.ini(request["text"].GetString());
								break;
								// Если формат входящих данных указан как YAML
								case static_cast <uint8_t> (type_t::YAML):
									// Выполняем конвертацию данных
									answer = parser.yaml(request["text"].GetString());
								break;
								// Если формат входящих данных указан как CEF
								case static_cast <uint8_t> (type_t::CEF):
									// Выполняем конвертацию данных
									answer = parser.cef(request["text"].GetString(), cef_t::mode_t::NONE);
								break;
								// Если формат входящих данных указан как CSV
								case static_cast <uint8_t> (type_t::CSV):
									// Выполняем конвертацию данных
									answer = parser.csv(
										request["text"].GetString(),
										request.HasMember("header") &&
										request["header"].IsBool() &&
										request["header"].GetBool()
									);
								break;
								// Если формат входящих данных указан как GROK
								case static_cast <uint8_t> (type_t::GROK):
									// Выполняем конвертацию данных
									answer = parser.grok(request["text"].GetString(), express);
								break;
								// Если формат входящих данных указан как SysLog
								case static_cast <uint8_t> (type_t::SYSLOG):
									// Выполняем конвертацию данных
									answer = parser.syslog(request["text"].GetString());
								break;
								// Если формат входящих данных указан как BASE64
								case static_cast <uint8_t> (type_t::BASE64): {
									// Получаем текстове значение буфера
									const string & text = request["text"].GetString();
									// Выполняем декодирование полученных данных
									const string & hash = this->_hash.decode <string> (text.data(), text.size(), hash_t::cipher_t::BASE64);
									// Выполняем декодирование хэша BASE64
									answer.SetString(hash.c_str(), hash.length(), answer.GetAllocator());
								} break;
							}
							// Если ответ парсера получен
							if(!answer.IsNull()){
								// Текст сформированного ответа
								string text = "";
								// Определяем формат данных
								switch(static_cast <uint8_t> (to)){
									// Если формат входящих данных указан как Text
									case static_cast <uint8_t> (type_t::TEXT):
										// Выполняем вывод текст как он есть
										text = answer.GetString();
									break;
									// Если формат входящих данных указан как XML
									case static_cast <uint8_t> (type_t::XML):
										// Выполняем конвертирование в формат XML
										text = parser.xml(
											answer,
											request.HasMember("prettify") &&
											request["prettify"].IsBool() &&
											request["prettify"].GetBool()
										);
									break;
									// Если формат входящих данных указан как JSON
									case static_cast <uint8_t> (type_t::JSON):
										// Выполняем конвертирование в формат JSON
										text = parser.json(
											answer,
											request.HasMember("prettify") &&
											request["prettify"].IsBool() &&
											request["prettify"].GetBool()
										);
									break;
									// Если формат входящих данных указан как INI
									case static_cast <uint8_t> (type_t::INI):
										// Выполняем конвертирование в формат INI
										text = parser.ini(answer);
									break;
									// Если формат входящих данных указан как YAML
									case static_cast <uint8_t> (type_t::YAML):
										// Выполняем конвертирование в формат YAML
										text = parser.yaml(answer);
									break;
									// Если формат входящих данных указан как CEF
									case static_cast <uint8_t> (type_t::CEF):
										// Выполняем конвертирование в формат CEF
										text = parser.cef(answer, cef_t::mode_t::NONE);
									break;
									// Если формат входящих данных указан как CSV
									case static_cast <uint8_t> (type_t::CSV): {
										// Выполняем конвертирование в формат CSV
										text = parser.csv(
											answer,
											request.HasMember("header") &&
											request["header"].IsBool() &&
											request["header"].GetBool()
										);
									} break;
									// Если формат входящих данных указан как SysLog
									case static_cast <uint8_t> (type_t::SYSLOG):
										// Выполняем конвертирование в формат SysLog
										text = parser.syslog(answer);
									break;
									// Если формат входящих данных указан как BASE64
									case static_cast <uint8_t> (type_t::BASE64): {
										// Выполняем получение текста для шифрования
										const string data = answer.GetString();
										// Выполняем конвертирование в формат BASE64
										this->_hash.encode(data.c_str(), data.size(), hash_t::cipher_t::BASE64, text);
									} break;
									// Если формат входящих данных указан как MD5
									case static_cast <uint8_t> (type_t::MD5):
										// Выполняем конвертирование в формат MD5
										this->_hash.hashing(answer.GetString(), hash_t::type_t::MD5, text);
									break;
									// Если формат входящих данных указан как SHA1
									case static_cast <uint8_t> (type_t::SHA1):
										// Выполняем конвертирование в формат SHA1
										this->_hash.hashing(answer.GetString(), hash_t::type_t::SHA1, text);
									break;
									// Если формат входящих данных указан как SHA224
									case static_cast <uint8_t> (type_t::SHA224):
										// Выполняем конвертирование в формат SHA224
										this->_hash.hashing(answer.GetString(), hash_t::type_t::SHA224, text);
									break;
									// Если формат входящих данных указан как SHA256
									case static_cast <uint8_t> (type_t::SHA256):
										// Выполняем конвертирование в формат SHA256
										this->_hash.hashing(answer.GetString(), hash_t::type_t::SHA256, text);
									break;
									// Если формат входящих данных указан как SHA384
									case static_cast <uint8_t> (type_t::SHA384):
										// Выполняем конвертирование в формат SHA384
										this->_hash.hashing(answer.GetString(), hash_t::type_t::SHA384, text);
									break;
									// Если формат входящих данных указан как SHA512
									case static_cast <uint8_t> (type_t::SHA512):
										// Выполняем конвертирование в формат SHA512
										this->_hash.hashing(answer.GetString(), hash_t::type_t::SHA512, text);
									break;
									// Если формат входящих данных указан как HMAC MD5
									case static_cast <uint8_t> (type_t::HMAC_MD5):
										// Выполняем конвертирование в формат HMAC MD5
										this->_hash.hmac(hmac, answer.GetString(), hash_t::type_t::MD5, text);
									break;
									// Если формат входящих данных указан как HMAC SHA1
									case static_cast <uint8_t> (type_t::HMAC_SHA1):
										// Выполняем конвертирование в формат HMAC SHA1
										this->_hash.hmac(hmac, answer.GetString(), hash_t::type_t::SHA1, text);
									break;
									// Если формат входящих данных указан как HMAC SHA224
									case static_cast <uint8_t> (type_t::HMAC_SHA224):
										// Выполняем конвертирование в формат HMAC SHA224
										this->_hash.hmac(hmac, answer.GetString(), hash_t::type_t::SHA224, text);
									break;
									// Если формат входящих данных указан как HMAC SHA256
									case static_cast <uint8_t> (type_t::HMAC_SHA256):
										// Выполняем конвертирование в формат HMAC SHA256
										this->_hash.hmac(hmac, answer.GetString(), hash_t::type_t::SHA256, text);
									break;
									// Если формат входящих данных указан как HMAC SHA384
									case static_cast <uint8_t> (type_t::HMAC_SHA384):
										// Выполняем конвертирование в формат HMAC SHA384
										this->_hash.hmac(hmac, answer.GetString(), hash_t::type_t::SHA384, text);
									break;
									// Если формат входящих данных указан как HMAC SHA512
									case static_cast <uint8_t> (type_t::HMAC_SHA512):
										// Выполняем конвертирование в формат HMAC SHA512
										this->_hash.hmac(hmac, answer.GetString(), hash_t::type_t::SHA512, text);
									break;
								}
								// Если текст ответа получен
								if((result = !text.empty())){
									// Выполняем формирование результата ответа
									json answer(kObjectType);
									// Выполняем формирование результата
									answer.AddMember(Value("result", answer.GetAllocator()).Move(), Value(text.c_str(), text.length(), answer.GetAllocator()).Move(), answer.GetAllocator());
									// Создаём результьрующий буфер
									rapidjson::StringBuffer result;
									// Выполняем создание объекта писателя
									Writer <StringBuffer> writer(result);
									// Передаем данные объекта JSON писателю
									answer.Accept(writer);
									// Выполняем получение буфера данных для отправки
									const string & buffer = result.GetString();
									// Отправляем сообщение клиенту
									this->_awh.send(sid, bid, 200, "OK", vector <char> (buffer.begin(), buffer.end()), {
										{"Accept-Ranges", "bytes"},
										{"Vary", "Accept-Encoding"},
										{"Content-Type", "application/json"},
										{"Access-Control-Request-Headers", "Content-Type"},
										{"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
										{"Access-Control-Allow-Origin", !this->_origin.empty() ? this->_origin : "*"}
									});
									// Выходим из функции
									return;
								}
							}
						}
					}
				}
				// Выпоолняем генерацию ошибки запроса
				this->error(sid, bid, 400, "Broken request");
				// Выводим удачное завершение работы
				return;
			}
			// Если мы получили OPTIONS запрос
			case static_cast <uint8_t> (awh::web_t::method_t::OPTIONS):
				// Выводим результат со список разрешённых методов
				this->_awh.send(sid, bid, 200, "Supported Methods", {}, {{"Allow", "GET, POST, OPTIONS"}});
			break;
		}
		// Если ответ не отправлен
		if(!result){
			// Если нужно отправить тело графика
			if(method != awh::web_t::method_t::HEAD)
				// Выпоолняем генерацию ошибки запроса
				this->error(sid, bid, 404, this->_fmk->format("Page %s is not found", addr.c_str()));
			// Если тело графика отправлять не нужно
			else this->_awh.send(sid, bid, 404, this->_fmk->format("Page %s is not found", addr.c_str()));
		}
	/**
	 * Если возникает ошибка
	 */
	} catch(const exception & error) {
		// Выпоолняем генерацию ошибки запроса
		this->error(sid, bid, 403, error.what());
	}
}
/**
 * config Метод установки конфигурационных параметров в формате JSON
 * @param config объект конфигурационных параметров в формате JSON
 */
void anyks::Server::config(const json & config) noexcept {
	/**
	 * Выполняем отлов ошибок
	 */
	try {
		// Если объект конфигурационных параметров передан
		if(config.IsObject() && !config.ObjectEmpty()){
			// Если адрес иконки сайта favicon.ico установлен
			if(config.HasMember("favicon") && config["favicon"].IsString())
				// Выполняем установку адреса favicon.ico
				this->_favicon = this->_fs.realPath(config["favicon"].GetString());
			// Если адрес установлен ресурс с которого разрешено выполнять доступ к API-сервера
			if(config.HasMember("origin") && config["origin"].IsString()){
				// Выполняем установку ресурса с которого разрешено выполнять доступ к API-сервера
				this->_origin = config["origin"].GetString();
				// Выполняем установку Origin сервера
				this->_awh.addOrigin(this->_origin);
			}
			// Если установлен адрес расположения файлов сайта
			if(config.HasMember("root") && config["root"].IsString())
				// Выполняем установку корневого адреса
				this->_root = config["root"].GetString();
			// Если файл по умолчанию который должен выводиться по запросу корня
			if(config.HasMember("index") && config["index"].IsString())
				// Выполняем установку название файла по умолчанию
				this->_index = config["index"].GetString();
			// Если максимальное количество запросов на одного пользователя в сутки установлено
			if(config.HasMember("maxRequests") && config["maxRequests"].IsUint())
				// Выполняем установку максимального количества запросов на одного пользователя в сутки
				this->_maxRequests = static_cast <uint16_t> (config["maxRequests"].GetUint());
			// Если количество воркеров получено
			if(config.HasMember("workers") && config["workers"].IsUint()){
				// Получаем количество доступных потоков
				const int16_t count = static_cast <uint16_t> (config["workers"].GetUint());
				// Если количество воркеров установлено
				if(count >= 0){
					// Получаем количество доступных потоков
					const uint16_t threads = std::thread::hardware_concurrency();
					// Если количество потоков больше одного
					if(threads > 1){
						// Разрешаем выполняем автоматический перезапуск упавшего процесса
						this->_core.clusterAutoRestart(true);
						// Активируем максимальное количество рабочих процессов
						this->_core.cluster(awh::scheme_t::mode_t::ENABLED, (count > 0 ? count : (threads / 2)));
					}
				}
			}
			// Если сетевые параметры работы с сервером присутствуют в конфиге
			if(config.HasMember("net") && config["net"].IsObject()){
				// Если максимальное количество подключений указано в конфиге
				if(config["net"].HasMember("total") &&
				   config["net"]["total"].IsUint() &&
				  (config["net"]["total"].GetUint() > 0))
					// Устанавливаем максимальное количество подключений к серверу
					this->_awh.total(static_cast <uint16_t> (config["net"]["total"].GetUint()));
				// Устанавливаем флаг использования только сети IPv6
				this->_core.ipV6only(
					config["net"].HasMember("ipv6") &&
					config["net"]["ipv6"].IsBool() &&
					config["net"]["ipv6"].GetBool()
				);
				// Если параметры ширины канала есть в конфиге
				if(config["net"].HasMember("bandwidth") && config["net"]["bandwidth"].IsObject()){
					// Получаем ширину канала на чтение
					this->_bandwidth.read = (
						config["net"]["bandwidth"].HasMember("read") &&
						config["net"]["bandwidth"]["read"].IsString() ?
						config["net"]["bandwidth"]["read"].GetString() : ""
					);
					// Получаем ширину канала на запись
					this->_bandwidth.write = (
						config["net"]["bandwidth"].HasMember("write") &&
						config["net"]["bandwidth"]["write"].IsString() ?
						config["net"]["bandwidth"]["write"].GetString() : ""
					);
				}
				// Список поддерживаемых компрессоров
				vector <awh::http_t::compressor_t> compressors;
				// Если список компрессоров передан
				if(config["net"].HasMember("compress") && config["net"]["compress"].IsArray()){
					// Выполняем перебор всех компрессоров
					for(auto & v : config["net"]["compress"].GetArray()){
						// Если компрессор является строкой
						if(v.IsString()){
							// Если компрессор соответствует LZ4
							if(this->_fmk->compare("LZ4", v.GetString()))
								// Выполняем добавление нового поддерживаемого компрессора
								compressors.push_back(awh::http_t::compressor_t::LZ4);
							// Если компрессор соответствует LZMA
							else if(this->_fmk->compare("LZMA", v.GetString()))
								// Выполняем добавление нового поддерживаемого компрессора
								compressors.push_back(awh::http_t::compressor_t::LZMA);
							// Если компрессор соответствует ZSTD
							else if(this->_fmk->compare("ZSTD", v.GetString()))
								// Выполняем добавление нового поддерживаемого компрессора
								compressors.push_back(awh::http_t::compressor_t::ZSTD);
							// Если компрессор соответствует GZIP
							else if(this->_fmk->compare("GZIP", v.GetString()))
								// Выполняем добавление нового поддерживаемого компрессора
								compressors.push_back(awh::http_t::compressor_t::GZIP);
							// Если компрессор соответствует BZIP2
							else if(this->_fmk->compare("BZIP2", v.GetString()))
								// Выполняем добавление нового поддерживаемого компрессора
								compressors.push_back(awh::http_t::compressor_t::BZIP2);
							// Если компрессор соответствует BROTLI
							else if(this->_fmk->compare("BROTLI", v.GetString()))
								// Выполняем добавление нового поддерживаемого компрессора
								compressors.push_back(awh::http_t::compressor_t::BROTLI);
							// Если компрессор соответствует DEFLATE
							else if(this->_fmk->compare("DEFLATE", v.GetString()))
								// Выполняем добавление нового поддерживаемого компрессора
								compressors.push_back(awh::http_t::compressor_t::DEFLATE);
						}
					}
				}
				// Если установлен unix-сокет для подклчюения
				if(config["net"].HasMember("unixSocket") &&
				   config["net"]["unixSocket"].IsString() &&
				  (strlen(config["net"]["unixSocket"].GetString()) > 0)){
					// Устанавливаем тип сокета unix-сокет
					this->_core.family(awh::scheme_t::family_t::NIX);
					// Выполняем инициализацию сервера для unix-сокета
					this->_awh.init(config["net"]["unixSocket"].GetString(), ::move(compressors));
				// Если подключение к серверу производится по хосту и порту
				} else {
					// Хост сервера
					string host = SERVER_HOST;
					// Порт сервера
					uint32_t port = SERVER_PORT;
					// Определяем версию IP протокола
					switch(config["net"].HasMember("ipv") && config["net"]["ipv"].IsUint() ? config["net"]["ipv"].GetUint() : 4){
						// Если версия IPv4
						case 4: this->_core.family(awh::scheme_t::family_t::IPV4); break;
						// Если версия IPv6
						case 6: this->_core.family(awh::scheme_t::family_t::IPV6); break;
					}
					// Тип протокола интернета по умолчанию
					awh::scheme_t::sonet_t sonet = awh::scheme_t::sonet_t::TCP;
					// Если передан тип сокета подключения
					if(config["net"].HasMember("sonet") && config["net"]["sonet"].IsString()){
						// Получаем тип сокета подключения
						const string & name = config["net"]["sonet"].GetString();
						// Если тип сокета подключения соответствует TCP
						if(this->_fmk->compare("TCP", name))
							// Выполняем установку тип сокета подключения
							sonet = awh::scheme_t::sonet_t::TCP;
						// Если тип сокета подключения соответствует UDP
						else if(this->_fmk->compare("UDP", name))
							// Выполняем установку тип сокета подключения
							sonet = awh::scheme_t::sonet_t::UDP;
						// Если тип сокета подключения соответствует TLS
						else if(this->_fmk->compare("TLS", name))
							// Выполняем установку тип сокета подключения
							sonet = awh::scheme_t::sonet_t::TLS;
						// Если тип сокета подключения соответствует DTLS
						else if(this->_fmk->compare("DTLS", name))
							// Выполняем установку тип сокета подключения
							sonet = awh::scheme_t::sonet_t::DTLS;
						// Если тип сокета подключения соответствует SCTP
						else if(this->_fmk->compare("SCTP", name))
							// Выполняем установку тип сокета подключения
							sonet = awh::scheme_t::sonet_t::SCTP;
					}
					// Устанавливаем тип сокета
					this->_core.sonet(sonet);
					// Устанавливаем активный протокол подключения
					this->_core.proto(awh::engine_t::proto_t::HTTP1_1);
					// Если хост сервера указан в конфиге
					if(config["net"].HasMember("host") && config["net"]["host"].IsString()){
						// Устанавливаем хост сервера
						host = config["net"]["host"].GetString();
						// Получаем тип передаваемого адреса
						awh::net_t::type_t type = net_t(this->_log).host(host);
						// Если тип переданного адреса не соответствует, выводим сообщение об ошибке
						if((type != awh::net_t::type_t::FQDN) && (type != awh::net_t::type_t::IPV4) && (type != awh::net_t::type_t::IPV6)){
							// Выполняем сброс хоста сервера
							host = SERVER_HOST;
							// Иначе выводим сообщение об ошибке
							this->_log->print("%s", log_t::flag_t::WARNING, "Server host is not correct");
						}
					}
					// Если порт сервера указан в конфиге
					if(config["net"].HasMember("port") && config["net"]["port"].IsUint()){
						// Устанавливаем порт сервера
						port = config["net"]["port"].GetUint();
						// Если порт сервера указан неправильно
						if((port < 80) || (port > 49151)){
							// Выполняем сброс порта сервера
							port = SERVER_PORT;
							// Иначе выводим сообщение об ошибке
							this->_log->print("%s", log_t::flag_t::WARNING, "Server port is not correct");
						}
					}
					// Если параметр работы с SSL сертификатами передан
					if(config.HasMember("ssl") && config["ssl"].IsObject()){
						// Создаём объект параметров SSL-шифрования
						node_t::ssl_t ssl;
						// Получаем флаг выполнения проверки SSL сертификата
						ssl.verify = (
							config["ssl"].HasMember("verify") ? (
							config["ssl"]["verify"].IsBool() &&
							config["ssl"]["verify"].GetBool()
						) : true);
						// Если ключ SSL-сертификата сервера установлен
						if(config["ssl"].HasMember("key") && config["ssl"]["key"].IsString())
							// Устанавливаем ключ SSL-сертификата сервера
							ssl.key = config["ssl"]["key"].GetString();
						// Если SSL-сертификат сервера установлен
						if(config["ssl"].HasMember("cert") && config["ssl"]["cert"].IsString())
							// Устанавливаем SSL-сертификат сервера
							ssl.cert = config["ssl"]["cert"].GetString();
						// Если сертификат получен
						if(!ssl.key.empty() && !ssl.cert.empty()){
							// Если версия HTTP протокола установлена как HTTP/2
							if(config["net"].HasMember("proto") && config["net"]["proto"].IsString() && this->_fmk->compare(config["net"]["proto"].GetString(), "http2"))
								// Выполняем установку поддержку протокола HTTP/2
								this->_core.proto(awh::engine_t::proto_t::HTTP2);
							// Определяем тип установленного сокета
							switch(static_cast <uint8_t> (sonet)){
								// Если тип сокета установлен как TCP
								case static_cast <uint8_t> (awh::scheme_t::sonet_t::TCP):
									// Устанавливаем тип сокета
									this->_core.sonet(awh::scheme_t::sonet_t::TLS);
								break;
								// Если тип сокета установлен как UDP
								case static_cast <uint8_t> (awh::scheme_t::sonet_t::UDP):
									// Устанавливаем тип сокета
									this->_core.sonet(awh::scheme_t::sonet_t::DTLS);
								break;
							}
							// Выполняем установку параметров SSL-шифрования
							this->_core.ssl(::move(ssl));
						}
					}
					// Выполняем инициализацию сервера
					this->_awh.init(port, ::move(host), ::move(compressors));
				}
				// Если ваш сервер требует аутентификации
				if(config["net"].HasMember("authentication") && config["net"]["authentication"].IsObject()){
					// Если авторизация присутствует на сервере
					if(config["net"]["authentication"].HasMember("enabled") &&
					   config["net"]["authentication"]["enabled"].IsBool() &&
					   config["net"]["authentication"]["enabled"].GetBool()){
						// Тип хэша Digest авторизации на вашем сервере
						awh::auth_t::hash_t hash = awh::auth_t::hash_t::MD5;
						// Тип авторизации на вашем сервере
						awh::auth_t::type_t type = awh::auth_t::type_t::BASIC;
						// Если логин и пароль авторизации на сервере установлены
						if(config["net"]["authentication"].HasMember("users") &&
						   config["net"]["authentication"]["users"].IsObject()){
							// Выполняем перебор всего списка пользователей
							for(auto & m : config["net"]["authentication"]["users"].GetObj()){
								// Если значение является строкой
								if(m.value.IsString())
									// Устанавливаем пользователя для авторизации на сервере
									this->_users.emplace(m.name.GetString(), m.value.GetString());
							}
						// Если логин или пароль не переданы, устанавливаем параметры по умолчанию
						} else this->_users.emplace(ACU_SERVER_USERNAME, ACU_SERVER_PASSWORD);
						// Если авторизация у сервера указана
						if(config["net"]["authentication"].HasMember("auth") && config["net"]["authentication"]["auth"].IsString()){
							// Получаем параметры авторизации
							const string & auth = config["net"]["authentication"]["auth"].GetString();
							// Если авторизация Basic требуется для сервера
							if(this->_fmk->compare(auth, "basic"))
								// Устанавливаем тип авторизации Basic сервера
								type = awh::auth_t::type_t::BASIC;
							// Если авторизация Digest требуется для сервера
							else if(this->_fmk->compare(auth, "digest"))
								// Устанавливаем тип авторизации Digest сервера
								type = awh::auth_t::type_t::DIGEST;
						}
						// Если тип Digest авторизации у сервера указан
						if(config["net"]["authentication"].HasMember("digest") && config["net"]["authentication"]["digest"].IsString()){
							// Получаем тип Digest авторизации
							const string & digest = config["net"]["authentication"]["digest"].GetString();
							// Если тип авторизация MD5 требуется для сервера
							if(this->_fmk->compare(digest, "md5"))
								// Устанавливаем тип авторизации Digest MD5 сервера
								hash = awh::auth_t::hash_t::MD5;
							// Если тип авторизация SHA1 требуется для сервера
							else if(this->_fmk->compare(digest, "sha1"))
								// Устанавливаем тип авторизации Digest SHA1 сервера
								hash = awh::auth_t::hash_t::SHA1;
							// Если тип авторизация SHA256 требуется для сервера
							else if(this->_fmk->compare(digest, "sha256"))
								// Устанавливаем тип авторизации Digest SHA256 сервера
								hash = awh::auth_t::hash_t::SHA256;
							// Если тип авторизация SHA512 требуется для сервера
							else if(this->_fmk->compare(digest, "sha512"))
								// Устанавливаем тип авторизации Digest SHA512 сервера
								hash = awh::auth_t::hash_t::SHA512;
						}
						// Устанавливаем тип авторизации на сервере
						this->_awh.authType(type, hash);
						// Устанавливаем название сервера
						this->_awh.realm(AWH_SHORT_NAME);
						// Устанавливаем временный ключ сессии сервера
						this->_awh.opaque(this->_hash.hashing <string> (std::to_string(::time(nullptr)), hash_t::type_t::MD5));
					}
				}
				// Если время ожидания получения сообщения передано
				if(config["net"].HasMember("wait") && config["net"]["wait"].IsUint64())
					// Выполняем установку времени ожидания получения сообщения
					this->_awh.waitMessage(static_cast <time_t> (config["net"]["wait"].GetUint64()));
				// Если фильтры доступа к серверу переданы
				if(config["net"].HasMember("filter") && config["net"]["filter"].IsObject()){
					// Создаём объект для работы с IP-адресами
					net_t net(this->_log);
					/**
					 * Тип фильтрации пользователей
					 */
					enum class filtred_t : uint8_t {
						IP  = 0x001, // IP-адрес
						MAC = 0x002  // MAC-адрес
					};
					// Тип фильтра доступа к серверу
					filtred_t filter = filtred_t::MAC;
					// Тип передаваемого адреса
					awh::net_t::type_t type = awh::net_t::type_t::NONE;
					// Определяем тип фильтра, если он передан
					if(config["net"]["filter"].HasMember("type") && config["net"]["filter"]["type"].IsString()){
						// Получаем тип фильтра доступа к серверу
						const string & type = config["net"]["filter"]["type"].GetString();
						// Если фильтр установлен как MAC-адрес
						if(this->_fmk->compare(type, "mac"))
							// Устанавливаем фильтрацию по MAC-адресу
							filter = filtred_t::MAC;
						// Если фильтр установлен как IP-адрес
						else if(this->_fmk->compare(type, "ip"))
							// Устанавливаем фильтрацию по IP-адресу
							filter = filtred_t::IP;
					}
					// Если передан чёрный список адресов
					if(config["net"]["filter"].HasMember("black") &&
					   config["net"]["filter"]["black"].IsArray() &&
					   !config["net"]["filter"]["black"].Empty()){
						// Переходим по всему массиву адресов
						for(auto & v : config["net"]["filter"]["black"].GetArray()){
							// Если запись в чёрном списке является строкой
							if(v.IsString()){
								// Получаем тип передаваемого адреса
								type = net.host(v.GetString());
								// Определяем тип фильтра
								switch(static_cast <uint8_t> (filter)){
									// Если очищается IP-адрес
									case static_cast <uint8_t> (filtred_t::IP): {
										// Если адрес является IP-адресом
										if((type == awh::net_t::type_t::IPV4) || (type == awh::net_t::type_t::IPV6))
											// Устанавливаем разрешённый адрес подключения в фильтр
											this->_ipBlack.emplace(v.GetString());
										// Иначе выводим сообщение об ошибке
										else this->_log->print("%s [%s]", log_t::flag_t::WARNING, "Address for blacklist filter is not correct", v.GetString());
									} break;
									// Если очищается MAC-адрес
									case static_cast <uint8_t> (filtred_t::MAC): {
										// Если адрес является MAC-адресом
										if(type == awh::net_t::type_t::MAC)
											// Устанавливаем разрешённый адрес подключения в фильтр
											this->_macBlack.emplace(v.GetString());
										// Иначе выводим сообщение об ошибке
										else this->_log->print("%s [%s]", log_t::flag_t::WARNING, "Address for blacklist filter is not correct", v.GetString());
									} break;
								}
							}
						}
					}
					// Если передан белый список адресов
					if(config["net"]["filter"].HasMember("white") &&
					   config["net"]["filter"]["white"].IsArray() &&
					   !config["net"]["filter"]["white"].Empty()){
						// Переходим по всему массиву адресов
						for(auto & v : config["net"]["filter"]["white"].GetArray()){
							// Если запись в белом списке является строкой
							if(v.IsString()){
								// Получаем тип передаваемого адреса
								type = net.host(v.GetString());
								// Определяем тип фильтра
								switch(static_cast <uint8_t> (filter)){
									// Если очищается IP-адрес
									case static_cast <uint8_t> (filtred_t::IP): {
										// Если адрес является IP-адресом
										if((type == awh::net_t::type_t::IPV4) || (type == awh::net_t::type_t::IPV6))
											// Устанавливаем разрешённый адрес подключения в фильтр
											this->_ipWhite.emplace(v.GetString());
										// Иначе выводим сообщение об ошибке
										else this->_log->print("%s [%s]", log_t::flag_t::WARNING, "Address for whitelist filter is not correct", v.GetString());
									} break;
									// Если очищается MAC-адрес
									case static_cast <uint8_t> (filtred_t::MAC): {
										// Если адрес является MAC-адресом
										if(type == awh::net_t::type_t::MAC)
											// Устанавливаем разрешённый адрес подключения в фильтр
											this->_macWhite.emplace(v.GetString());
										// Иначе выводим сообщение об ошибке
										else this->_log->print("%s [%s]", log_t::flag_t::WARNING, "Address for whitelist filter is not correct", v.GetString());
									} break;
								}
							}
						}
					}
				}
			}{
				// Создаём объект работы с операционной системой
				os_t os;
				// Если требуется перенастроить сервер на максимальную производительность
				if(config.HasMember("boost") && config["boost"].IsBool() && config["boost"].GetBool())
					// Выполняем перенастрофку сервера на максимальную производительность
					os.boost();
				// Если пользователь получен
				if(config.HasMember("user")){
					// Если пользователь указан как число или как строка но не строка в виде "auto"
					if(config["user"].IsUint() || (config["user"].IsString() && !this->_fmk->compare("auto", config["user"].GetString()))){
						// Если пользователь указан как число
						if(config["user"].IsUint()){
							// Идентификатор группы
							gid_t gid = 0;
							// Если группа пользователя получена
							if(config.HasMember("group") && config["group"].IsUint())
								// Получаем идентификатор группы
								gid = static_cast <gid_t> (config["group"].GetUint());
							// Выполняем активацию пользователя
							os.chown(static_cast <uid_t> (config["user"].GetUint()), gid);
						// Если пользователь указан как строка
						} else {
							// Название группы
							string group = "";
							// Если группа пользователя получена
							if(config.HasMember("group") && (config["group"].IsString() && !this->_fmk->compare("auto", config["group"].GetString())))
								// Получаем название группы
								group = config["group"].GetString();
							// Выполняем активацию пользователя
							os.chown(config["user"].GetString(), group);
						}
					}
				}
			}
		}
	/**
	 * Если возникает ошибка
	 */
	} catch(const exception & error) {
		/**
		 * Если включён режим отладки
		 */
		#if defined(DEBUG_MODE)
			// Выводим сообщение об ошибке
			this->_log->debug("%s", __PRETTY_FUNCTION__, {}, log_t::flag_t::CRITICAL, error.what());
		/**
		* Если режим отладки не включён
		*/
		#else
			// Выводим сообщение об ошибке
			this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
		#endif
	}
}
/**
 * stop Метод остановки работы сервера
 */
void anyks::Server::stop() noexcept {
	// Очищаем кэш запросов
	this->_cache.clear();
	// Выполняем очищение выделенной ранее памяти
	unordered_map <string, pair <uint64_t, vector <char>>> ().swap(this->_cache);
	// Запрещаем перехват сигналов
	this->_core.signalInterception(awh::scheme_t::mode_t::DISABLED);
	// Выполняем остановку сервера
	this->_awh.stop();
}
/**
 * start Метод запуска работы сервера
 */
void anyks::Server::start() noexcept {
	// Разрешаем перехват сигналов
	this->_core.signalInterception(awh::scheme_t::mode_t::ENABLED);
	// Устанавливаем функцию обработки сигналов завершения работы приложения
	this->_core.callback <void (const int)> ("crash", bind(&server_t::crash, this, _1));
	// Устанавливаем функцию обратного вызова на запуск системы
	this->_core.callback <void (const awh::core_t::status_t)> ("status", bind(static_cast <void (server_t::*)(const awh::core_t::status_t)> (&server_t::active), this, _1));
	// Выполняем запуск сервера
	this->_awh.start();
}
/**
 * Server конструктор
 * @param fmk объект фреймворка
 * @param log объект для работы с логами
 */
anyks::Server::Server(const fmk_t * fmk, const log_t * log) noexcept :
 _fs(fmk, log), _uri(fmk, log), _hash(log), _root{""}, _index{""}, _origin{""}, _favicon{""},
 _http(fmk, log), _maxRequests(100), _core(fmk, log), _awh(&_core, fmk, log), _fmk(fmk), _log(log) {
	// Выполняем установку идентификатора клиента
	this->_awh.ident(AWH_SHORT_NAME, AWH_NAME, AWH_VERSION);
	// Устанавливаем функцию извлечения пароля пользователя для авторизации
	this->_awh.callback <string (const uint64_t, const string &)> ("extractPassword", bind(&server_t::password, this, _1, _2));
	// Устанавливаем функцию проверки авторизации прользователя
	this->_awh.callback <bool (const uint64_t, const string &, const string &)> ("checkPassword", bind(&server_t::auth, this, _1, _2, _3));
	// Установливаем функцию обратного вызова на событие активации клиента на сервере
	this->_awh.callback <bool (const string &, const string &, const uint32_t)> ("accept", bind(&server_t::accept, this, _1, _2, _3));
	// Устанавливаем функцию обратного вызова при выполнении удачного рукопожатия
	this->_awh.callback <void (const int32_t, const uint64_t, const server::web_t::agent_t)> ("handshake", bind(&server_t::handshake, this, _1, _2, _3));
	// Установливаем функцию обратного вызова на событие запуска или остановки подключения
	this->_awh.callback <void (const uint64_t, const server::web_t::mode_t)> ("active", bind(static_cast <void (server_t::*)(const uint64_t, const server::web_t::mode_t)> (&server_t::active), this, _1, _2));
	// Устанавливаем функцию обратного вызова на получение входящих сообщений запросов
	this->_awh.callback <void (const int32_t, const uint64_t, const awh::web_t::method_t, const uri_t::url_t &, const unordered_multimap <string, string> &)> ("headers", bind(&server_t::headers, this, _1, _2, _3, _4, _5));
	// Установливаем функцию обратного вызова на событие получения полного запроса клиента
	this->_awh.callback <void (const int32_t, const uint64_t, const awh::web_t::method_t, const uri_t::url_t &, const vector <char> &, const unordered_multimap <string, string> &)> ("complete", bind(&server_t::complete, this, _1, _2, _3, _4, _5, _6));
}
/**
 * ~Server деструктор
 */
anyks::Server::~Server() noexcept {
	// Запрещаем перехват сигналов
	this->_core.signalInterception(awh::scheme_t::mode_t::DISABLED);
	// Выполняем остановку сервера
	this->_awh.stop();
}
