/**
 * @file: cef.cpp
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
#include <cef.hpp>

/**
 * Подписываемся на пространство имён awh
 */
using namespace awh;
/**
 * Подписываемся на пространство имён rapidjson
 */
using namespace rapidjson;

/**
 * clear Метод очистки данных
 */
void anyks::Cef::clear() noexcept {
	// Выполняем очистку заголовка CEF
	this->_header.clear();
	// Выполняем очитску списка расширений
	this->_extensions.clear();
	// Выполняем сброс блока событий
	this->_event = event_t();
	// Выполняем сброс версии CEF
	this->_version = 1.2;
	// Выполняем установку формата даты по умолчанию
	this->_format = FORMAT;
	// Выполняем сброс режима парсинга
	this->_mode = mode_t::STRONG;
}
/**
 * parse Метод парсинга строки в формате CEF
 * @param cef строка в формате CEF
 */
void anyks::Cef::parse(const string & cef) noexcept {
	// Если строка в формате CEF передана
	if(!cef.empty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем поиск CEF заголовка
			size_t stop = cef.find("CEF:");
			// Если заголовок получен
			if(stop != string::npos){
				// Получаем позицию начала строки
				size_t start = (stop + 4);
				// Если заголовок не найден
				if(stop > 0){
					// Получаем заголовок контейнера
					this->_header = cef.substr(0, stop);
					// Удаляем лишние пробелы
					this->_fmk->transform(this->_header, fmk_t::transform_t::TRIM);
				}
				// Если версия контейнера найдена
				if((stop = cef.find("|", start)) != string::npos){
					// Получаем данные строки
					string value = cef.substr(start, stop - start);
					// Выполняем проверку полученного значения
					if(this->_fmk->is(value, fmk_t::check_t::NUMBER)){
						// Получаем позицию начала строки
						start = (stop + 1);
						// Получаем версию протокола
						this->_version = ::stod(value);
						// Выполняем поиск вендора
						if((stop = cef.find("|", start)) != string::npos){
							// Получаем данные строки
							value = cef.substr(start, stop - start);
							// Если вендор получен
							if(!value.empty()){
								// Получаем позицию начала строки
								start = (stop + 1);
								// Выполняем копирование вендора
								::memcpy(this->_event.devVendor, value.data(), sizeof(this->_event.devVendor));
								// Выполняем поиск продукта
								if((stop = cef.find("|", start)) != string::npos){
									// Получаем данные строки
									value = cef.substr(start, stop - start);
									// Если продукт получен
									if(!value.empty()){
										// Получаем позицию начала строки
										start = (stop + 1);
										// Выполняем копирование продукта
										::memcpy(this->_event.devProduct, value.data(), sizeof(this->_event.devProduct));
										// Выполняем поиск версии
										if((stop = cef.find("|", start)) != string::npos){
											// Получаем данные строки
											value = cef.substr(start, stop - start);
											// Если версия получена
											if(!value.empty()){
												// Получаем позицию начала строки
												start = (stop + 1);
												// Выполняем копирование версии
												::memcpy(this->_event.devVersion, value.data(), sizeof(this->_event.devVersion));
												// Выполняем поиск подписи
												if((stop = cef.find("|", start)) != string::npos){
													// Получаем данные строки
													value = cef.substr(start, stop - start);
													// Если версия получена
													if(!value.empty()){
														// Получаем позицию начала строки
														start = (stop + 1);
														// Выполняем копирование подписи
														::memcpy(this->_event.signatureId, value.data(), sizeof(this->_event.signatureId));
														// Выполняем поиск названия события
														if((stop = cef.find("|", start)) != string::npos){
															// Получаем данные строки
															value = cef.substr(start, stop - start);
															// Если название события получено
															if(!value.empty()){
																// Получаем позицию начала строки
																start = (stop + 1);
																// Выполняем копирование названия события
																::memcpy(this->_event.name, value.data(), sizeof(this->_event.name));
																// Выполняем поиск важности события
																if((stop = cef.find("|", start)) != string::npos){
																	// Получаем данные строки
																	value = cef.substr(start, stop - start);
																	// Если важность события получено
																	if(!value.empty()){
																		// Получаем позицию начала строки
																		start = (stop + 1);
																		// Если важность события получено в виде числа
																		if(this->_fmk->is(value, fmk_t::check_t::NUMBER)){
																			// Выполняем получение числа
																			this->_event.severity.level = static_cast <uint8_t> (::stoi(value));
																			// Заполняем нулями буфер названия важности
																			::memset(this->_event.severity.name, 0, sizeof(this->_event.severity.name));
																			// Если событие не больше 4-х
																			if((this->_event.severity.level >= 0) && (this->_event.severity.level <= 3))
																				// Устанавливаем уровень важности
																				::memcpy(this->_event.severity.name, "Low", 3);
																			// Если событие не больше 6-и
																			else if((this->_event.severity.level >= 4) && (this->_event.severity.level <= 6))
																				// Устанавливаем уровень важности
																				::memcpy(this->_event.severity.name, "Medium", 6);
																			// Если событие не больше 8-и
																			else if((this->_event.severity.level >= 7) && (this->_event.severity.level <= 8))
																				// Устанавливаем уровень важности
																				::memcpy(this->_event.severity.name, "High", 4);
																			// Если событие не больше 10-и
																			else if((this->_event.severity.level >= 9) && (this->_event.severity.level <= 10))
																				// Устанавливаем уровень важности
																				::memcpy(this->_event.severity.name, "Very-High", 9);
																		// Если важность события получено в виде строки
																		} else {
																			// Выполняем копирование важность события
																			::memcpy(this->_event.severity.name, value.data(), sizeof(this->_event.severity.name));
																			// Преобразуем важность в верхний регистр
																			this->_fmk->transform(value, fmk_t::transform_t::UPPER);
																			// Если сложность является низкой
																			if(value.compare("LOW") == 0)
																				// Устанавливаем низкое значение сложности
																				this->_event.severity.level = 0;
																			// Если сложность является средней
																			else if(value.compare("MEDIUM") == 0)
																				// Устанавливаем среднее значение сложности
																				this->_event.severity.level = 4;
																			// Если сложность является высокой
																			else if(value.compare("HIGH") == 0)
																				// Устанавливаем высокое значение сложности
																				this->_event.severity.level = 7;
																			// Если сложность является очень-высокой
																			else if(value.compare("VERY-HIGH") == 0)
																				// Устанавливаем очень-высокое значение сложности
																				this->_event.severity.level = 9;
																		}
																		// Получаем данные строки
																		value = cef.substr(start);
																		// Если важность события получено
																		if(!value.empty())
																			// Выполняем препарирование расширений
																			this->prepare(value);
																		// Добавляем в список полученные ошибки
																		else {
																			/**
																			 * Если включён режим отладки
																			 */
																			#if defined(DEBUG_MODE)
																				// Выводим сообщение об ошибке
																				this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(cef), log_t::flag_t::WARNING, "Extensions is not found");
																			/**
																			* Если режим отладки не включён
																			*/
																			#else
																				// Выводим сообщение об ошибке
																				this->_log->print("%s", log_t::flag_t::WARNING, "Extensions is not found");
																			#endif
																		}
																	// Выводим сообщение об ошибке
																	} else {
																		/**
																		 * Если включён режим отладки
																		 */
																		#if defined(DEBUG_MODE)
																			// Выводим сообщение об ошибке
																			this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(cef), log_t::flag_t::WARNING, "Severity is not found");
																		/**
																		* Если режим отладки не включён
																		*/
																		#else
																			// Выводим сообщение об ошибке
																			this->_log->print("%s", log_t::flag_t::WARNING, "Severity is not found");
																		#endif
																	}
																// Выводим сообщение об ошибке
																} else {
																	/**
																	 * Если включён режим отладки
																	 */
																	#if defined(DEBUG_MODE)
																		// Выводим сообщение об ошибке
																		this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(cef), log_t::flag_t::WARNING, "Severity is not found");
																	/**
																	* Если режим отладки не включён
																	*/
																	#else
																		// Выводим сообщение об ошибке
																		this->_log->print("%s", log_t::flag_t::WARNING, "Severity is not found");
																	#endif
																}
															// Выводим сообщение об ошибке
															} else {
																/**
																 * Если включён режим отладки
																 */
																#if defined(DEBUG_MODE)
																	// Выводим сообщение об ошибке
																	this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(cef), log_t::flag_t::WARNING, "Event name is not found");
																/**
																* Если режим отладки не включён
																*/
																#else
																	// Выводим сообщение об ошибке
																	this->_log->print("%s", log_t::flag_t::WARNING, "Event name is not found");
																#endif
															}
														// Выводим сообщение об ошибке
														} else {
															/**
															 * Если включён режим отладки
															 */
															#if defined(DEBUG_MODE)
																// Выводим сообщение об ошибке
																this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(cef), log_t::flag_t::WARNING, "Event name is not found");
															/**
															* Если режим отладки не включён
															*/
															#else
																// Выводим сообщение об ошибке
																this->_log->print("%s", log_t::flag_t::WARNING, "Event name is not found");
															#endif
														}
													// Выводим сообщение об ошибке
													} else {
														/**
														 * Если включён режим отладки
														 */
														#if defined(DEBUG_MODE)
															// Выводим сообщение об ошибке
															this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(cef), log_t::flag_t::WARNING, "Signature ID is not found");
														/**
														* Если режим отладки не включён
														*/
														#else
															// Выводим сообщение об ошибке
															this->_log->print("%s", log_t::flag_t::WARNING, "Signature ID is not found");
														#endif
													}
												// Выводим сообщение об ошибке
												} else {
													/**
													 * Если включён режим отладки
													 */
													#if defined(DEBUG_MODE)
														// Выводим сообщение об ошибке
														this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(cef), log_t::flag_t::WARNING, "Signature ID is not found");
													/**
													* Если режим отладки не включён
													*/
													#else
														// Выводим сообщение об ошибке
														this->_log->print("%s", log_t::flag_t::WARNING, "Signature ID is not found");
													#endif
												}
											// Выводим сообщение об ошибке
											} else {
												/**
												 * Если включён режим отладки
												 */
												#if defined(DEBUG_MODE)
													// Выводим сообщение об ошибке
													this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(cef), log_t::flag_t::WARNING, "Device version is not found");
												/**
												* Если режим отладки не включён
												*/
												#else
													// Выводим сообщение об ошибке
													this->_log->print("%s", log_t::flag_t::WARNING, "Device version is not found");
												#endif
											}
										// Выводим сообщение об ошибке
										} else {
											/**
											 * Если включён режим отладки
											 */
											#if defined(DEBUG_MODE)
												// Выводим сообщение об ошибке
												this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(cef), log_t::flag_t::WARNING, "Device version is not found");
											/**
											* Если режим отладки не включён
											*/
											#else
												// Выводим сообщение об ошибке
												this->_log->print("%s", log_t::flag_t::WARNING, "Device version is not found");
											#endif
										}
									// Выводим сообщение об ошибке
									} else {
										/**
										 * Если включён режим отладки
										 */
										#if defined(DEBUG_MODE)
											// Выводим сообщение об ошибке
											this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(cef), log_t::flag_t::WARNING, "Device product is not found");
										/**
										* Если режим отладки не включён
										*/
										#else
											// Выводим сообщение об ошибке
											this->_log->print("%s", log_t::flag_t::WARNING, "Device product is not found");
										#endif
									}
								// Выводим сообщение об ошибке
								} else {
									/**
									 * Если включён режим отладки
									 */
									#if defined(DEBUG_MODE)
										// Выводим сообщение об ошибке
										this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(cef), log_t::flag_t::WARNING, "Device product is not found");
									/**
									* Если режим отладки не включён
									*/
									#else
										// Выводим сообщение об ошибке
										this->_log->print("%s", log_t::flag_t::WARNING, "Device product is not found");
									#endif
								}
							// Выводим сообщение об ошибке
							} else {
								/**
								 * Если включён режим отладки
								 */
								#if defined(DEBUG_MODE)
									// Выводим сообщение об ошибке
									this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(cef), log_t::flag_t::WARNING, "Device vendor is not found");
								/**
								* Если режим отладки не включён
								*/
								#else
									// Выводим сообщение об ошибке
									this->_log->print("%s", log_t::flag_t::WARNING, "Device vendor is not found");
								#endif
							}
						// Выводим сообщение об ошибке
						} else {
							/**
							 * Если включён режим отладки
							 */
							#if defined(DEBUG_MODE)
								// Выводим сообщение об ошибке
								this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(cef), log_t::flag_t::WARNING, "Device vendor is not found");
							/**
							* Если режим отладки не включён
							*/
							#else
								// Выводим сообщение об ошибке
								this->_log->print("%s", log_t::flag_t::WARNING, "Device vendor is not found");
							#endif
						}
					// Выводим сообщение об ошибке
					} else {
						/**
						 * Если включён режим отладки
						 */
						#if defined(DEBUG_MODE)
							// Выводим сообщение об ошибке
							this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(cef), log_t::flag_t::WARNING, "Container version must be a number");
						/**
						* Если режим отладки не включён
						*/
						#else
							// Выводим сообщение об ошибке
							this->_log->print("%s", log_t::flag_t::WARNING, "Container version must be a number");
						#endif
					}
				// Если получена ошибка
				} else {
					/**
					 * Если включён режим отладки
					 */
					#if defined(DEBUG_MODE)
						// Выводим сообщение об ошибке
						this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(cef), log_t::flag_t::WARNING, "Version is not found");
					/**
					* Если режим отладки не включён
					*/
					#else
						// Выводим сообщение об ошибке
						this->_log->print("%s", log_t::flag_t::WARNING, "Version is not found");
					#endif
				}
			// Если получена ошибка
			} else {
				/**
				 * Если включён режим отладки
				 */
				#if defined(DEBUG_MODE)
					// Выводим сообщение об ошибке
					this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(cef), log_t::flag_t::WARNING, "An invalid data format was passed");
				/**
				* Если режим отладки не включён
				*/
				#else
					// Выводим сообщение об ошибке
					this->_log->print("%s", log_t::flag_t::WARNING, "An invalid data format was passed");
				#endif
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
				this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(cef), log_t::flag_t::CRITICAL, error.what());
			/**
			* Если режим отладки не включён
			*/
			#else
				// Выводим сообщение об ошибке
				this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
			#endif
		}
	// Если получена ошибка
	} else {
		/**
		 * Если включён режим отладки
		 */
		#if defined(DEBUG_MODE)
			// Выводим сообщение об ошибке
			this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(cef), log_t::flag_t::WARNING, "No parsing data passed");
		/**
		* Если режим отладки не включён
		*/
		#else
			// Выводим сообщение об ошибке
			this->_log->print("%s", log_t::flag_t::WARNING, "No parsing data passed");
		#endif
	}
}
/**
 * prepare Метод препарирования расширений
 * @param extensions строка с расширениями
 */
void anyks::Cef::prepare(const string & extensions) noexcept {
	// Если срока расширений передана
	if(!extensions.empty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Смещение в сообщении
			size_t offset = 0;
			// Если включён строгий режим парсинга
			if(this->_mode == mode_t::STRONG){
				/**
				 * matchFn Функция матчинга ключа расширения для CEF
				 * @param key ключ для матчинга
				 * @return    результат сравнения
				 */
				auto matchFn = [this](const string & key) noexcept -> bool {
					// Результат работы функции
					bool result = false;
					// Если ключё передан
					if(!key.empty()){
						// Выполняем поиск нашего ключа
						auto i = this->_extensionSEFv0.find(key);
						// Выполняем поиск ключа в схеме для версии CEF:0
						result = (i != this->_extensionSEFv0.end());
						// Если результат не получен но версия CEF:1
						if(!result && (this->_version == 1.0))
							// Выполняем поиск ключа в схеме для версии CEF:1
							result = (i != this->_extensionSEFv1.end());
					}
					// Выводим результат
					return result;
				};
				// Выполняем извлечение всех сообщений
				for(;;){
					// Выполняем извлечение всего списка установленных параметров
					const auto & items = this->_reg.match(extensions.c_str() + offset, this->_exp);
					// Если список параметров получен
					if(!items.empty()){
						// Если элементов параметров получены 4 штуки
						if(items.size() >= 4){
							// Получаем ключ расширения
							const string & key = extensions.substr(items.at(1).first + offset, items.at(1).second);
							// Выполняем матчинг ключа
							if(matchFn(key))
								// Выполняем создание расширения
								this->extension(key, (items.back().first == 0 ? extensions.substr(items.at(2).first + offset, items.at(2).second) : extensions.substr(items.back().first + offset, items.back().second)));
							// Если ключ не интерпретирован
							else {
								// Формируем текст ошибки
								string error = "";
								// Добавляем экранирование
								error.append(1, '\"');
								// Добавляем брокированное значение ключа
								error.append(key);
								// Добавляем основной текст сообщения
								error.append("\" key does not comply with CEF standard");
								/**
								 * Если включён режим отладки
								 */
								#if defined(DEBUG_MODE)
									// Выводим сообщение об ошибке
									this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(extensions), log_t::flag_t::WARNING, error.c_str());
								/**
								* Если режим отладки не включён
								*/
								#else
									// Выводим сообщение об ошибке
									this->_log->print("%s", log_t::flag_t::WARNING, error.c_str());
								#endif
							}
						}
						// Увеличиваем длину сообщения
						offset += (items.at(2).first + items.at(2).second + items.back().first + items.back().second);
					// Выходим из цикла
					} else break;
				}
			// Если строгий режим не активирован
			} else {
				// Выполняем извлечение всех сообщений
				for(;;){
					// Выполняем извлечение всего списка установленных параметров
					const auto & items = this->_reg.match(extensions.c_str() + offset, this->_exp);
					// Если список параметров получен
					if(!items.empty()){
						// Если элементов параметров получены 4 штуки
						if(items.size() >= 4){
							// Выполняем создание расширения
							this->extension(
								extensions.substr(items.at(1).first + offset, items.at(1).second),
								(items.back().first == 0 ? extensions.substr(items.at(2).first + offset, items.at(2).second) : extensions.substr(items.back().first + offset, items.back().second))
							);
						}
						// Увеличиваем длину сообщения
						offset += (items.at(2).first + items.at(2).second + items.back().first + items.back().second);
					// Выходим из цикла
					} else break;
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
				this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(extensions), log_t::flag_t::CRITICAL, error.what());
			/**
			* Если режим отладки не включён
			*/
			#else
				// Выводим сообщение об ошибке
				this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
			#endif
		}
	// Если получена ошибка
	} else {
		/**
		 * Если включён режим отладки
		 */
		#if defined(DEBUG_MODE)
			// Выводим сообщение об ошибке
			this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(extensions), log_t::flag_t::CRITICAL, "No parsing extensions passed");
		/**
		* Если режим отладки не включён
		*/
		#else
			// Выводим сообщение об ошибке
			this->_log->print("%s", log_t::flag_t::CRITICAL, "No parsing extensions passed");
		#endif
	}
}
/**
 * mode Метод установки режима парсинга
 * @param mode режим парсинга для установки
 */
void anyks::Cef::mode(const mode_t mode) noexcept {
	// Выполняем установку режим парсинга
	this->_mode = mode;
}
/**
 * version Метод извлечения версии контейнера
 * @return версия контейнера
 */
double anyks::Cef::version() const noexcept {
	// Выводим версию контейнера CEF
	return this->_version;
}
/**
 * version Метод установки версии контейнера
 * @param version версия контейнера для установки
 */
void anyks::Cef::version(const double version) noexcept {
	// Устанавливаем версию CEF
	this->_version = version;
}
/**
 * cef Метод получения данных в формате CEF
 * @return данные в формате CEF
 */
string anyks::Cef::cef() const noexcept {
	// Результат работы функции
	string result = "";
	// Если данные заполнены
	if(!this->_extensions.empty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Если заголовок установлен
			if(!this->_header.empty()){
				// Добавляем заголовок события
				result.append(this->_header);
				// Добавляем формат CEF
				result.append(" CEF:");
			// Добавляем формат CEF
			} else result.append("CEF:");
			// Добавляем версию контейнера
			result.append(this->_fmk->noexp(this->_version, true));
			// Добавляем разделитель
			result.append(1, '|');
			// Добавляем поставщика данных
			result.append(this->_event.devVendor);
			// Добавляем разделитель
			result.append(1, '|');
			// Добавляем тип устройства поставщика данных
			result.append(this->_event.devProduct);
			// Добавляем разделитель
			result.append(1, '|');
			// Добавляем версию поставщика данных
			result.append(this->_event.devVersion);
			// Добавляем разделитель
			result.append(1, '|');
			// Добавляем подпись события поставщика данных
			result.append(this->_event.signatureId);
			// Добавляем разделитель
			result.append(1, '|');
			// Добавляем название события
			result.append(this->_event.name);
			// Добавляем разделитель
			result.append(1, '|');
			// Если требуется установить важность в числовом виде
			if(((this->_event.severity.level >= 1) && (this->_event.severity.level <= 3)) ||
			   ((this->_event.severity.level >= 5) && (this->_event.severity.level <= 6)) ||
				(this->_event.severity.level == 8) || (this->_event.severity.level == 10))
				// Добавляем важность события
				result.append(std::to_string(static_cast <uint16_t> (this->_event.severity.level)));
			// Иначе добавляем важность события в текстовом виде
			else result.append(this->_event.severity.name);
			// Добавляем разделитель
			result.append(1, '|');
			// Создаём объект параметров расширения
			const ext_t * params = nullptr;
			// Переходим по всему списку расширений
			for(auto & extension : this->_extensions){
				// Если режим парсинга установлен
				if(this->_mode != mode_t::NONE){
					// Выполняем поиск параметров ключа
					auto i = this->_extensionSEFv0.find(extension.first);
					// Если параметры ключа найдены
					if(i != this->_extensionSEFv0.end())
						// Получаем параметры ключа
						params = &i->second;
					// Если параметры ключа не найдены и версия протокола №1
					else if(this->_version > .0) {
						// Выполняем поиск параметров ключа
						auto i = this->_extensionSEFv1.find(extension.first);
						// Если параметры ключа найдены
						if(i != this->_extensionSEFv0.end())
							// Получаем параметры ключа
							params = &i->second;
					}
					// Если параметры ключа получены
					if(params != nullptr){
						// Если нужно установить разделитель
						if(result.back() != '|')
							// Устанавливаем разделитель расширений
							result.append(1, ' ');
						// Определяем тип ключа
						switch(static_cast <uint8_t> (params->type)){
							// Если тип ключа является IP-адресом
							case static_cast <uint8_t> (type_t::IP): {
								// Если включён строгий режим парсинга
								if(this->_mode == mode_t::STRONG){
									// Создаём объект сети
									net_t net(this->_log);
									// Если количество байт в буфере 4
									if(extension.second.size() == 4){
										// Формируем число из бинарного буфера
										uint32_t value = 0;
										// Копируем в бинарный буфер данные IP адреса
										::memcpy(&value, extension.second.data(), extension.second.size());
										// Устанавливаем данные адреса в объект сети
										net.v4(value);
									// Если количество байт в буфере 16
									} else if(extension.second.size() == 16){
										// Формируем бинарный буфер данных
										array <uint64_t, 2> buffer;
										// Копируем в бинарный буфер данные IP адреса
										::memcpy(buffer.data(), extension.second.data(), extension.second.size());
										// Устанавливаем данные адреса в объект сети
										net.v6(buffer);
									}
									// Добавляем ключ расширения
									result.append(extension.first);
									// Добавляем разделитель
									result.append(1, '=');
									// Добавляем значение ключа
									result.append(net);
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else {
									// Добавляем ключ расширения
									result.append(extension.first);
									// Добавляем разделитель
									result.append(1, '=');
									// Добавляем значение ключа
									result.append(extension.second.begin(), extension.second.end());
								}
							} break;
							// Если тип ключа является MAC-адресом
							case static_cast <uint8_t> (type_t::MAC): {
								// Если включён строгий режим парсинга
								if(this->_mode == mode_t::STRONG){
									// Создаём объект сети
									net_t net(this->_log);
									// Формируем число из бинарного буфера
									uint64_t value = 0;
									// Копируем в бинарный буфер данные IP адреса
									::memcpy(&value, extension.second.data(), extension.second.size());
									// Устанавливаем данные адреса в объект сети
									net.mac(value);
									// Добавляем ключ расширения
									result.append(extension.first);
									// Добавляем разделитель
									result.append(1, '=');
									// Добавляем значение ключа
									result.append(net);
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else {
									// Добавляем ключ расширения
									result.append(extension.first);
									// Добавляем разделитель
									result.append(1, '=');
									// Добавляем значение ключа
									result.append(extension.second.begin(), extension.second.end());
								}
							} break;
							// Если тип ключа является IPV4-адресом
							case static_cast <uint8_t> (type_t::IPV4): {
								// Если включён строгий режим парсинга
								if(this->_mode == mode_t::STRONG){
									// Создаём объект сети
									net_t net(this->_log);
									// Формируем число из бинарного буфера
									uint32_t value = 0;
									// Копируем в бинарный буфер данные IP адреса
									::memcpy(&value, extension.second.data(), extension.second.size());
									// Устанавливаем данные адреса в объект сети
									net.v4(value);
									// Добавляем ключ расширения
									result.append(extension.first);
									// Добавляем разделитель
									result.append(1, '=');
									// Добавляем значение ключа
									result.append(net);
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else {
									// Добавляем ключ расширения
									result.append(extension.first);
									// Добавляем разделитель
									result.append(1, '=');
									// Добавляем значение ключа
									result.append(extension.second.begin(), extension.second.end());
								}
							} break;
							// Если тип ключа является IPV6-адресом
							case static_cast <uint8_t> (type_t::IPV6): {
								// Если включён строгий режим парсинга
								if(this->_mode == mode_t::STRONG){
									// Создаём объект сети
									net_t net(this->_log);
									// Формируем бинарный буфер данных
									array <uint64_t, 2> buffer;
									// Копируем в бинарный буфер данные IP адреса
									::memcpy(buffer.data(), extension.second.data(), extension.second.size());
									// Устанавливаем данные адреса в объект сети
									net.v6(buffer);
									// Добавляем ключ расширения
									result.append(extension.first);
									// Добавляем разделитель
									result.append(1, '=');
									// Добавляем значение ключа
									result.append(net);
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else {
									// Добавляем ключ расширения
									result.append(extension.first);
									// Добавляем разделитель
									result.append(1, '=');
									// Добавляем значение ключа
									result.append(extension.second.begin(), extension.second.end());
								}
							} break;
							// Если тип ключа является LONG
							case static_cast <uint8_t> (type_t::LONG): {
								// Если режим парсинга установлен
								if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
									// Формируем число из бинарного буфера
									long value = 0;
									// Извлекаем из буфера данные числа
									::memcpy(&value, extension.second.data(), extension.second.size());
									// Добавляем ключ расширения
									result.append(extension.first);
									// Добавляем разделитель
									result.append(1, '=');
									// Добавляем значение ключа
									result.append(std::to_string(value));
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else {
									// Добавляем ключ расширения
									result.append(extension.first);
									// Добавляем разделитель
									result.append(1, '=');
									// Добавляем значение ключа
									result.append(extension.second.begin(), extension.second.end());
								}
							} break;
							// Если тип ключа является INT32
							case static_cast <uint8_t> (type_t::INT32): {
								// Если режим парсинга установлен
								if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
									// Формируем число из бинарного буфера
									int32_t value = 0;
									// Извлекаем из буфера данные числа
									::memcpy(&value, extension.second.data(), extension.second.size());
									// Добавляем ключ расширения
									result.append(extension.first);
									// Добавляем разделитель
									result.append(1, '=');
									// Добавляем значение ключа
									result.append(std::to_string(value));
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else {
									// Добавляем ключ расширения
									result.append(extension.first);
									// Добавляем разделитель
									result.append(1, '=');
									// Добавляем значение ключа
									result.append(extension.second.begin(), extension.second.end());
								}
							} break;
							// Если тип ключа является INT64
							case static_cast <uint8_t> (type_t::INT64): {
								// Если режим парсинга установлен
								if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
									// Формируем число из бинарного буфера
									int64_t value = 0;
									// Извлекаем из буфера данные числа
									::memcpy(&value, extension.second.data(), extension.second.size());
									// Добавляем ключ расширения
									result.append(extension.first);
									// Добавляем разделитель
									result.append(1, '=');
									// Добавляем значение ключа
									result.append(std::to_string(value));
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else {
									// Добавляем ключ расширения
									result.append(extension.first);
									// Добавляем разделитель
									result.append(1, '=');
									// Добавляем значение ключа
									result.append(extension.second.begin(), extension.second.end());
								}
							} break;
							// Если тип ключа является FLOAT
							case static_cast <uint8_t> (type_t::FLOAT): {
								// Если режим парсинга установлен
								if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
									// Формируем число из бинарного буфера
									float value = .0f;
									// Извлекаем из буфера данные числа
									::memcpy(&value, extension.second.data(), extension.second.size());
									// Добавляем ключ расширения
									result.append(extension.first);
									// Добавляем разделитель
									result.append(1, '=');
									// Добавляем значение ключа
									result.append(this->_fmk->noexp(value, true));
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else {
									// Добавляем ключ расширения
									result.append(extension.first);
									// Добавляем разделитель
									result.append(1, '=');
									// Добавляем значение ключа
									result.append(extension.second.begin(), extension.second.end());
								}
							} break;
							// Если тип ключа является DOUBLE
							case static_cast <uint8_t> (type_t::DOUBLE): {
								// Если режим парсинга установлен
								if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
									// Формируем число из бинарного буфера
									double value = .0;
									// Извлекаем из буфера данные числа
									::memcpy(&value, extension.second.data(), extension.second.size());
									// Добавляем ключ расширения
									result.append(extension.first);
									// Добавляем разделитель
									result.append(1, '=');
									// Добавляем значение ключа
									result.append(this->_fmk->noexp(value, true));
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else {
									// Добавляем ключ расширения
									result.append(extension.first);
									// Добавляем разделитель
									result.append(1, '=');
									// Добавляем значение ключа
									result.append(extension.second.begin(), extension.second.end());
								}
							} break;
							// Если тип ключа является STRING
							case static_cast <uint8_t> (type_t::STRING):
								// Добавляем ключ расширения
								result.append(extension.first);
								// Добавляем разделитель
								result.append(1, '=');
								// Добавляем значение ключа
								result.append(extension.second.begin(), extension.second.end());
							break;
							// Если тип ключа является TIMESTAMP
							case static_cast <uint8_t> (type_t::TIMESTAMP): {
								// Если режим парсинга установлен
								if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
									// Формируем число из бинарного буфера
									time_t date = 0;
									// Извлекаем из буфера данные числа
									::memcpy(&date, extension.second.data(), extension.second.size());
									// Если формат даты установлен
									if(!this->_format.empty()){
										// Создаём объект потока
										stringstream transTime;
										// Создаем структуру времени
										tm * tm = ::localtime(&date);
										// Выполняем извлечение даты
										transTime << put_time(tm, this->_format.c_str());
										// Добавляем ключ расширения
										result.append(extension.first);
										// Добавляем разделитель
										result.append(1, '=');
										// Добавляем значение ключа
										result.append(transTime.str());
									// Если формат даты не установлен
									} else {
										// Добавляем ключ расширения
										result.append(extension.first);
										// Добавляем разделитель
										result.append(1, '=');
										// Добавляем значение ключа
										result.append(std::to_string(date));
									}
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else {
									// Добавляем ключ расширения
									result.append(extension.first);
									// Добавляем разделитель
									result.append(1, '=');
									// Добавляем значение ключа
									result.append(extension.second.begin(), extension.second.end());
								}
							} break;
						}
					}
				// Если режим парсинга не установлен
				} else {
					// Добавляем ключ расширения
					result.append(extension.first);
					// Добавляем разделитель
					result.append(1, '=');
					// Добавляем значение ключа
					result.append(extension.second.begin(), extension.second.end());
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
	// Выводим заглушку
	return result;
}
/**
 * dump Метод извлечения данных в виде JSON
 * @return json объект дампа данных
 */
anyks::json anyks::Cef::dump() const noexcept {
	// Результат работы функции
	json result(kObjectType);
	// Если данные заполнены
	if(!this->_extensions.empty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Создаём объект параметров расширения
			const ext_t * params = nullptr;
			// Определяем активный режим парсинга
			switch(static_cast <uint8_t> (this->_mode)){
				// Если режим парсинга не активирован
				case static_cast <uint8_t> (mode_t::NONE):
					// Устанавливаем режим парсинга
					result.AddMember(Value("mode", result.GetAllocator()).Move(), Value("NONE", result.GetAllocator()).Move(), result.GetAllocator());
				break;
				// Если включён простой режим парсинга
				case static_cast <uint8_t> (mode_t::LOW):
					// Устанавливаем режим парсинга
					result.AddMember(Value("mode", result.GetAllocator()).Move(), Value("LOW", result.GetAllocator()).Move(), result.GetAllocator());
				break;
				// Если включён средний режим парсинга
				case static_cast <uint8_t> (mode_t::MEDIUM):
					// Устанавливаем режим парсинга
					result.AddMember(Value("mode", result.GetAllocator()).Move(), Value("MEDIUM", result.GetAllocator()).Move(), result.GetAllocator());
				break;
				// Если включён строгий режим парсинга
				case static_cast <uint8_t> (mode_t::STRONG):
					// Устанавливаем режим парсинга
					result.AddMember(Value("mode", result.GetAllocator()).Move(), Value("STRONG", result.GetAllocator()).Move(), result.GetAllocator());
				break;
			}{
				// Временное значение переменной
				double intpart = 0;
				// Выполняем проверку есть ли дробная часть у числа
				if(::modf(this->_version, &intpart) == 0){
					// Если число является положительным
					if(this->_version > 0.)
						// Устанавливаем новые структурированные данные
						result.AddMember(Value("version", result.GetAllocator()).Move(), Value(static_cast <uint32_t> (this->_version)).Move(), result.GetAllocator());
					// Если число является отрицательным
					else result.AddMember(Value("version", result.GetAllocator()).Move(), Value(static_cast <int32_t> (this->_version)).Move(), result.GetAllocator());
				// Устанавливаем версию контейнера
				} else result.AddMember(Value("version", result.GetAllocator()).Move(), Value(this->_version).Move(), result.GetAllocator());
			}
			// Если заголовок получен
			if(!this->_header.empty())
				// Устанавливаем заголовок события
				result.AddMember(Value("header", result.GetAllocator()).Move(), Value(this->_header.c_str(), this->_header.length(), result.GetAllocator()).Move(), result.GetAllocator());
			// Формируем параметры полезной нагрузки
			result.AddMember(Value("event", result.GetAllocator()).Move(), Value(kObjectType).Move(), result.GetAllocator());
			// Устанавливаем название события
			result["event"].AddMember(Value("name", result.GetAllocator()).Move(), Value(this->_event.name, ::strlen(this->_event.name), result.GetAllocator()).Move(), result.GetAllocator());
			// Устанавливаем поставщика данных
			result["event"].AddMember(Value("vendor", result.GetAllocator()).Move(), Value(this->_event.devVendor, ::strlen(this->_event.devVendor), result.GetAllocator()).Move(), result.GetAllocator());
			// Устанавливаем версию поставщика данных
			result["event"].AddMember(Value("version", result.GetAllocator()).Move(), Value(this->_event.devVersion, ::strlen(this->_event.devVersion), result.GetAllocator()).Move(), result.GetAllocator());
			// Устанавливаем тип устройства поставщика данных
			result["event"].AddMember(Value("product", result.GetAllocator()).Move(), Value(this->_event.devProduct, ::strlen(this->_event.devProduct), result.GetAllocator()).Move(), result.GetAllocator());
			// Устанавливаем подпись события поставщика данных
			result["event"].AddMember(Value("signature", result.GetAllocator()).Move(), Value(this->_event.signatureId, ::strlen(this->_event.signatureId), result.GetAllocator()).Move(), result.GetAllocator());
			// Формируем важность события
			result["event"].AddMember(Value("severity", result.GetAllocator()).Move(), Value(kObjectType).Move(), result.GetAllocator());
			// Устанавливаем числовое значение важности события
			result["event"]["severity"].AddMember(Value("level", result.GetAllocator()).Move(), Value(static_cast <uint32_t> (this->_event.severity.level)).Move(), result.GetAllocator());
			// Устанавливаем строковое значение важности события
			result["event"]["severity"].AddMember(Value("text", result.GetAllocator()).Move(), Value(this->_event.severity.name, ::strlen(this->_event.severity.name), result.GetAllocator()).Move(), result.GetAllocator());
			// Формируем список разрешений
			result.AddMember(Value("extensions", result.GetAllocator()).Move(), Value(kObjectType).Move(), result.GetAllocator());
			// Переходим по всему списку расширений
			for(auto & extension : this->_extensions){
				// Если режим парсинга установлен
				if(this->_mode != mode_t::NONE){
					// Выполняем поиск параметров ключа
					auto i = this->_extensionSEFv0.find(extension.first);
					// Если параметры ключа найдены
					if(i != this->_extensionSEFv0.end())
						// Получаем параметры ключа
						params = &i->second;
					// Если параметры ключа не найдены и версия протокола №1
					else if(this->_version > .0) {
						// Выполняем поиск параметров ключа
						auto i = this->_extensionSEFv1.find(extension.first);
						// Если параметры ключа найдены
						if(i != this->_extensionSEFv0.end())
							// Получаем параметры ключа
							params = &i->second;
					}
					// Если параметры ключа получены
					if(params != nullptr){
						// Определяем тип ключа
						switch(static_cast <uint8_t> (params->type)){
							// Если тип ключа является IP-адресом
							case static_cast <uint8_t> (type_t::IP): {
								// Если включён строгий режим парсинга
								if(this->_mode == mode_t::STRONG){
									// Создаём объект сети
									net_t net(this->_log);
									// Если количество байт в буфере 4
									if(extension.second.size() == 4){
										// Формируем число из бинарного буфера
										uint32_t value = 0;
										// Копируем в бинарный буфер данные IP адреса
										::memcpy(&value, extension.second.data(), extension.second.size());
										// Устанавливаем данные адреса в объект сети
										net.v4(value);
									// Если количество байт в буфере 16
									} else if(extension.second.size() == 16){
										// Формируем бинарный буфер данных
										array <uint64_t, 2> buffer;
										// Копируем в бинарный буфер данные IP адреса
										::memcpy(buffer.data(), extension.second.data(), extension.second.size());
										// Устанавливаем данные адреса в объект сети
										net.v6(buffer);
									}
									// Получаем IP-адрес
									const string & addr = net;
									// Устанавливаем значение ключа
									result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(addr.c_str(), addr.length(), result.GetAllocator()).Move(), result.GetAllocator());
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(extension.second.data(), extension.second.size(), result.GetAllocator()).Move(), result.GetAllocator());
							} break;
							// Если тип ключа является MAC-адресом
							case static_cast <uint8_t> (type_t::MAC): {
								// Если включён строгий режим парсинга
								if(this->_mode == mode_t::STRONG){
									// Формируем число из бинарного буфера
									uint64_t value = 0;
									// Копируем в бинарный буфер данные IP адреса
									::memcpy(&value, extension.second.data(), extension.second.size());
									// Создаём объект сети
									net_t net(this->_log);
									// Устанавливаем данные адреса в объект сети
									net.mac(value);
									// Получаем IP-адрес
									const string & addr = net;
									// Устанавливаем значение ключа
									result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(addr.c_str(), addr.length(), result.GetAllocator()).Move(), result.GetAllocator());
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(extension.second.data(), extension.second.size(), result.GetAllocator()).Move(), result.GetAllocator());
							} break;
							// Если тип ключа является IPV4-адресом
							case static_cast <uint8_t> (type_t::IPV4): {
								// Если включён строгий режим парсинга
								if(this->_mode == mode_t::STRONG){
									// Формируем число из бинарного буфера
									uint32_t value = 0;
									// Копируем в бинарный буфер данные IP адреса
									::memcpy(&value, extension.second.data(), extension.second.size());
									// Создаём объект сети
									net_t net(this->_log);
									// Устанавливаем данные адреса в объект сети
									net.v4(value);
									// Получаем IP-адрес
									const string & addr = net;
									// Устанавливаем значение ключа
									result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(addr.c_str(), addr.length(), result.GetAllocator()).Move(), result.GetAllocator());
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(extension.second.data(), extension.second.size(), result.GetAllocator()).Move(), result.GetAllocator());
							} break;
							// Если тип ключа является IPV6-адресом
							case static_cast <uint8_t> (type_t::IPV6): {
								// Если включён строгий режим парсинга
								if(this->_mode == mode_t::STRONG){
									// Формируем бинарный буфер данных
									array <uint64_t, 2> buffer;
									// Копируем в бинарный буфер данные IP адреса
									::memcpy(buffer.data(), extension.second.data(), extension.second.size());
									// Создаём объект сети
									net_t net(this->_log);
									// Устанавливаем данные адреса в объект сети
									net.v6(buffer);
									// Получаем IP-адрес
									const string & addr = net;
									// Устанавливаем значение ключа
									result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(addr.c_str(), addr.length(), result.GetAllocator()).Move(), result.GetAllocator());
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(extension.second.data(), extension.second.size(), result.GetAllocator()).Move(), result.GetAllocator());
							} break;
							// Если тип ключа является LONG
							case static_cast <uint8_t> (type_t::LONG): {
								// Если режим парсинга установлен
								if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
									// Формируем число из бинарного буфера
									long value = 0;
									// Извлекаем из буфера данные числа
									::memcpy(&value, extension.second.data(), extension.second.size());
									// Устанавливаем значение ключа
									result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(static_cast <int64_t> (value)).Move(), result.GetAllocator());
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else {
									// Получаем значение числа
									const string value(extension.second.begin(), extension.second.end());
									/**
									 * Выполняем отлов ошибок
									 */
									try {
										// Если число положительное
										if(value.front() != '-')
											// Добавляем полученное значения расширения
											result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(static_cast <uint64_t> (::stoul(value))).Move(), result.GetAllocator());
										// Добавляем полученное значения расширения
										else result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(static_cast <int64_t> (::stol(value))).Move(), result.GetAllocator());
									/**
									 * Если возникает ошибка
									 */
									} catch(const exception &) {
										// Добавляем полученное значения расширения
										result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(value.c_str(), value.length(), result.GetAllocator()).Move(), result.GetAllocator());
									}
								}
							} break;
							// Если тип ключа является INT32
							case static_cast <uint8_t> (type_t::INT32): {
								// Если режим парсинга установлен
								if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
									// Формируем число из бинарного буфера
									int32_t value = 0;
									// Извлекаем из буфера данные числа
									::memcpy(&value, extension.second.data(), extension.second.size());
									// Устанавливаем значение ключа
									result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(value).Move(), result.GetAllocator());
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else {
									// Получаем значение числа
									const string value(extension.second.begin(), extension.second.end());
									/**
									 * Выполняем отлов ошибок
									 */
									try {
										// Если число положительное
										if(value.front() != '-')
											// Добавляем полученное значения расширения
											result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(static_cast <uint32_t> (::stoul(value))).Move(), result.GetAllocator());
										// Добавляем полученное значения расширения
										else result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(static_cast <int32_t> (::stoi(value))).Move(), result.GetAllocator());
									/**
									 * Если возникает ошибка
									 */
									} catch(const exception &) {
										// Добавляем полученное значения расширения
										result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(value.c_str(), value.length(), result.GetAllocator()).Move(), result.GetAllocator());
									}
								}
							} break;
							// Если тип ключа является INT64
							case static_cast <uint8_t> (type_t::INT64): {
								// Если режим парсинга установлен
								if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
									// Формируем число из бинарного буфера
									int64_t value = 0;
									// Извлекаем из буфера данные числа
									::memcpy(&value, extension.second.data(), extension.second.size());
									// Устанавливаем значение ключа
									result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(value).Move(), result.GetAllocator());
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else {
									// Получаем значение числа
									const string value(extension.second.begin(), extension.second.end());
									/**
									 * Выполняем отлов ошибок
									 */
									try {
										// Если число положительное
										if(value.front() != '-')
											// Добавляем полученное значения расширения
											result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(static_cast <uint64_t> (::stoull(value))).Move(), result.GetAllocator());
										// Добавляем полученное значения расширения
										else result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(static_cast <int64_t> (::stoll(value))).Move(), result.GetAllocator());
									/**
									 * Если возникает ошибка
									 */
									} catch(const exception &) {
										// Добавляем полученное значения расширения
										result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(value.c_str(), value.length(), result.GetAllocator()).Move(), result.GetAllocator());
									}
								}
							} break;
							// Если тип ключа является FLOAT
							case static_cast <uint8_t> (type_t::FLOAT): {
								// Если режим парсинга установлен
								if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
									// Формируем число из бинарного буфера
									float value = .0f;
									// Извлекаем из буфера данные числа
									::memcpy(&value, extension.second.data(), extension.second.size());
									// Устанавливаем значение ключа
									result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(value).Move(), result.GetAllocator());
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else {
									/**
									 * Выполняем отлов ошибок
									 */
									try {
										// Добавляем полученное значения расширения
										result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(::stof(string(extension.second.begin(), extension.second.end()))).Move(), result.GetAllocator());
									/**
									 * Если возникает ошибка
									 */
									} catch(const exception &) {
										// Добавляем полученное значения расширения
										result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(extension.second.data(), extension.second.size(), result.GetAllocator()).Move(), result.GetAllocator());
									}
								}
							} break;
							// Если тип ключа является DOUBLE
							case static_cast <uint8_t> (type_t::DOUBLE): {
								// Если режим парсинга установлен
								if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
									// Формируем число из бинарного буфера
									double value = .0;
									// Извлекаем из буфера данные числа
									::memcpy(&value, extension.second.data(), extension.second.size());
									// Устанавливаем значение ключа
									result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(value).Move(), result.GetAllocator());
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else {
									/**
									 * Выполняем отлов ошибок
									 */
									try {
										// Добавляем полученное значения расширения
										result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(::stod(string(extension.second.begin(), extension.second.end()))).Move(), result.GetAllocator());
									/**
									 * Если возникает ошибка
									 */
									} catch(const exception &) {
										// Добавляем полученное значения расширения
										result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(extension.second.data(), extension.second.size(), result.GetAllocator()).Move(), result.GetAllocator());
									}
								}
							} break;
							// Если тип ключа является STRING
							case static_cast <uint8_t> (type_t::STRING):
								// Устанавливаем значение ключа
								result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(extension.second.data(), extension.second.size(), result.GetAllocator()).Move(), result.GetAllocator());
							break;
							// Если тип ключа является TIMESTAMP
							case static_cast <uint8_t> (type_t::TIMESTAMP): {
								// Если режим парсинга установлен
								if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
									// Формируем число из бинарного буфера
									time_t date = 0;
									// Извлекаем из буфера данные числа
									::memcpy(&date, extension.second.data(), extension.second.size());
									// Если формат даты установлен
									if(!this->_format.empty()){
										// Создаём объект потока
										stringstream transTime;
										// Создаем структуру времени
										tm * tm = ::localtime(&date);
										// Выполняем извлечение даты
										transTime << put_time(tm, this->_format.c_str());
										// Получаем строку штампа времени
										const string & stamp = transTime.str();
										// Устанавливаем значение ключа
										result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(stamp.c_str(), stamp.length(), result.GetAllocator()).Move(), result.GetAllocator());
									// Устанавливаем значение ключа
									} else result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(static_cast <uint64_t> (date)).Move(), result.GetAllocator());
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(extension.second.data(), extension.second.size(), result.GetAllocator()).Move(), result.GetAllocator());
							} break;
						}
					}
				// Если режим парсинга не установлен
				} else {
					// Получаем значение для вывода
					string value(extension.second.begin(), extension.second.end());
					// Если запись является числом
					if(this->_fmk->is(value, fmk_t::check_t::NUMBER)){
						/**
						 * Выполняем отлов ошибок
						 */
						try {
							// Если число положительное
							if(value.front() != '-')
								// Устанавливаем значение ключа как число без знака
								result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(static_cast <uint64_t> (::stoull(value))).Move(), result.GetAllocator());
							// Устанавливаем значение ключа как число со знаком
							else result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(static_cast <int64_t> (::stoll(value))).Move(), result.GetAllocator());
						/**
						 * Если возникает ошибка
						 */
						} catch(const exception &) {
							// Устанавливаем значение ключа как число без знака
							result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(value.c_str(), value.length(), result.GetAllocator()).Move(), result.GetAllocator());
						}
					// Если запись является числом с плавающей точкой
					} else if(this->_fmk->is(value, fmk_t::check_t::DECIMAL)) {
						/**
						 * Выполняем отлов ошибок
						 */
						try {
							// Устанавливаем значение ключа как число с плавающей точкой
							result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(::stod(value)).Move(), result.GetAllocator());
						/**
						 * Если возникает ошибка
						 */
						} catch(const exception &) {
							// Устанавливаем значение ключа как число без знака
							result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(value.c_str(), value.length(), result.GetAllocator()).Move(), result.GetAllocator());
						}
					// Если число является булевым истинным значением
					} else if(this->_fmk->compare("true", value))
						// Устанавливаем значение ключа как булевое положительное значение
						result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(true).Move(), result.GetAllocator());
					// Если число является булевым ложным значением
					else if(this->_fmk->compare("false", value))
						// Устанавливаем значение ключа как булевое отрицательное значение
						result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(false).Move(), result.GetAllocator());
					// Устанавливаем значение ключа как оно есть
					else result["extensions"].AddMember(Value(extension.first.c_str(), extension.first.length(), result.GetAllocator()).Move(), Value(extension.second.data(), extension.second.size(), result.GetAllocator()).Move(), result.GetAllocator());
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
	// Выводим заглушку
	return result;
}
/**
 * dump Метод установки данных в формате JSON
 * @param dump данные в формате JSON
 */
void anyks::Cef::dump(const json & dump) noexcept {
	// Если данные в формате JSON получены
	if(dump.IsObject() && !dump.ObjectEmpty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем очистку данных контейнера
			this->clear();
			// Если флаг строгого режима передан
			if(dump.HasMember("mode") && dump["mode"].IsString()){
				// Если режим парсинга не активирован
				if(::strcmp(dump["mode"].GetString(), "NONE") == 0)
					// Выполняем установку режима парсинга
					this->_mode = mode_t::NONE;
				// Если включён простой режим парсинга
				else if(::strcmp(dump["mode"].GetString(), "LOW") == 0)
					// Выполняем установку режима парсинга
					this->_mode = mode_t::LOW;
				// Если включён средний режим парсинга
				else if(::strcmp(dump["mode"].GetString(), "MEDIUM") == 0)
					// Выполняем установку режима парсинга
					this->_mode = mode_t::MEDIUM;
				// Если включён строгий режим парсинга
				else if(::strcmp(dump["mode"].GetString(), "STRONG") == 0)
					// Выполняем установку режима парсинга
					this->_mode = mode_t::STRONG;
			}
			// Если заголовок события передан
			if(dump.HasMember("header") && dump["header"].IsString())
				// Устанавливаем заголовок события
				this->_header = dump["header"].GetString();
			// Если версия события передана
			if(dump.HasMember("version") && dump["version"].IsDouble())
				// Устанавливаем версию события
				this->_version = dump["version"].GetDouble();
			// Если версия события передана
			else if(dump.HasMember("version") && dump["version"].IsUint())
				// Устанавливаем версию события
				this->_version = static_cast <double> (dump["version"].GetUint());
			// Если данные события переданы
			if(dump.HasMember("event") && dump["event"].IsObject() && !dump["event"].ObjectEmpty()){
				// Если название события передано
				if(dump["event"].HasMember("name") && dump["event"]["name"].IsString()){
					// Получаем название события
					const string & name = dump["event"]["name"].GetString();
					// Заполняем нулями буфер названия события
					::memset(this->_event.name, 0, sizeof(this->_event.name));
					// Устанавливаем название события
					::memcpy(this->_event.name, name.data(), sizeof(this->_event.name));
				}
				// Если поставщик данных передан
				if(dump["event"].HasMember("vendor") && dump["event"]["vendor"].IsString()){
					// Получаем поставщика данных события
					const string & vendor = dump["event"]["vendor"].GetString();
					// Заполняем нулями буфер поставщика данных события
					::memset(this->_event.devVendor, 0, sizeof(this->_event.devVendor));
					// Устанавливаем поставщика данных события
					::memcpy(this->_event.devVendor, vendor.data(), sizeof(this->_event.devVendor));
				}
				// Если версия поставщика данных передана
				if(dump["event"].HasMember("version") && dump["event"]["version"].IsString()){
					// Получаем версию поставщика данных события
					const string & version = dump["event"]["version"].GetString();
					// Заполняем нулями буфер версии поставщика данных события
					::memset(this->_event.devVersion, 0, sizeof(this->_event.devVersion));
					// Устанавливаем версию поставщика данных данных события
					::memcpy(this->_event.devVersion, version.data(), sizeof(this->_event.devVersion));
				}
				// Если тип устройства поставщика данных передан
				if(dump["event"].HasMember("product") && dump["event"]["product"].IsString()){
					// Получаем тип устройства поставщика данных события
					const string & product = dump["event"]["product"].GetString();
					// Заполняем нулями буфер типа устройства поставщика данных события
					::memset(this->_event.devProduct, 0, sizeof(this->_event.devProduct));
					// Устанавливаем тип устройства поставщика данных события
					::memcpy(this->_event.devProduct, product.data(), sizeof(this->_event.devProduct));
				}
				// Если подпись события поставщика данных передана
				if(dump["event"].HasMember("signature") && dump["event"]["signature"].IsString()){
					// Получаем подпись события поставщика данных
					const string & signature = dump["event"]["signature"].GetString();
					// Заполняем нулями буфер подписи события поставщика данных
					::memset(this->_event.signatureId, 0, sizeof(this->_event.signatureId));
					// Устанавливаем подпись события поставщика данных
					::memcpy(this->_event.signatureId, signature.data(), sizeof(this->_event.signatureId));
				}
				// Если важность события передана
				if(dump["event"].HasMember("severity") && dump["event"]["severity"].IsObject() && !dump["event"]["severity"].ObjectEmpty()){
					// Если важность события в текстовом виде передана
					if(dump["event"]["severity"].HasMember("text") && dump["event"]["severity"]["text"].IsString()){
						// Получаем важность события в текстовом виде
						string severity = dump["event"]["severity"]["text"].GetString();
						// Заполняем нулями буфер важности события в текстовом виде
						::memset(this->_event.severity.name, 0, sizeof(this->_event.severity.name));
						// Устанавливаем важность события в текстовом виде
						::memcpy(this->_event.severity.name, severity.data(), sizeof(this->_event.severity.name));
						// Если важность события в числовом виде не передана
						if(!dump["event"]["severity"].HasMember("level")){
							// Преобразуем важность в верхний регистр
							this->_fmk->transform(severity, fmk_t::transform_t::UPPER);
							// Если сложность является низкой
							if(severity.compare("LOW") == 0)
								// Устанавливаем низкое значение сложности
								this->_event.severity.level = 0;
							// Если сложность является средней
							else if(severity.compare("MEDIUM") == 0)
								// Устанавливаем среднее значение сложности
								this->_event.severity.level = 4;
							// Если сложность является высокой
							else if(severity.compare("HIGH") == 0)
								// Устанавливаем высокое значение сложности
								this->_event.severity.level = 7;
							// Если сложность является очень-высокой
							else if(severity.compare("VERY-HIGH") == 0)
								// Устанавливаем очень-высокое значение сложности
								this->_event.severity.level = 9;
						}
					}
					// Если важность события в числовом виде передана
					if(dump["event"]["severity"].HasMember("level") && dump["event"]["severity"]["level"].IsNumber()){
						// Получаем важность события в текстовом виде
						this->_event.severity.level = static_cast <uint8_t> (dump["event"]["severity"]["level"].GetInt());
						// Если важность события в текстовом виде не передана
						if(!dump["event"]["severity"].HasMember("text")){
							// Заполняем нулями буфер названия важности
							::memset(this->_event.severity.name, 0, sizeof(this->_event.severity.name));
							// Если событие не больше 4-х
							if((this->_event.severity.level >= 0) && (this->_event.severity.level <= 3))
								// Устанавливаем уровень важности
								::memcpy(this->_event.severity.name, "Low", 3);
							// Если событие не больше 6-и
							else if((this->_event.severity.level >= 4) && (this->_event.severity.level <= 6))
								// Устанавливаем уровень важности
								::memcpy(this->_event.severity.name, "Medium", 6);
							// Если событие не больше 8-и
							else if((this->_event.severity.level >= 7) && (this->_event.severity.level <= 8))
								// Устанавливаем уровень важности
								::memcpy(this->_event.severity.name, "High", 4);
							// Если событие не больше 10-и
							else if((this->_event.severity.level >= 9) && (this->_event.severity.level <= 10))
								// Устанавливаем уровень важности
								::memcpy(this->_event.severity.name, "Very-High", 9);
						}
					}
				}
			}
			// Если список доступных расширений передан
			if(dump.HasMember("extensions") && dump["extensions"].IsObject() && !dump["extensions"].ObjectEmpty()){
				// Создаём объект параметров расширения
				const ext_t * params = nullptr;
				// Выполняем перебор всех расширений
				for(auto & m : dump["extensions"].GetObj()){
					// Если значение является строкой
					if(m.value.IsString())
						// Выполняем добавление расширение
						this->extension(m.name.GetString(), m.value.GetString());
					// Если значение является числом
					else if(m.value.IsNumber()) {
						// Выполняем поиск параметров ключа
						auto i = this->_extensionSEFv0.find(m.name.GetString());
						// Если параметры ключа найдены
						if(i != this->_extensionSEFv0.end())
							// Получаем параметры ключа
							params = &i->second;
						// Если параметры ключа не найдены и версия протокола №1
						else if(this->_version > .0) {
							// Выполняем поиск параметров ключа
							auto i = this->_extensionSEFv1.find(m.name.GetString());
							// Если параметры ключа найдены
							if(i != this->_extensionSEFv0.end())
								// Получаем параметры ключа
								params = &i->second;
						}
						// Если параметры ключа получены
						if(params != nullptr){
							// Определяем тип ключа
							switch(static_cast <uint8_t> (params->type)){
								// Если тип ключа является LONG
								case static_cast <uint8_t> (type_t::LONG):
									// Выполняем добавление расширение
									this->extension(m.name.GetString(), std::to_string(m.value.GetInt64()));
								break;
								// Если тип ключа является INT32
								case static_cast <uint8_t> (type_t::INT32):
									// Выполняем добавление расширение
									this->extension(m.name.GetString(), std::to_string(m.value.GetInt()));
								break;
								// Если тип ключа является INT64
								case static_cast <uint8_t> (type_t::INT64):
									// Выполняем добавление расширение
									this->extension(m.name.GetString(), std::to_string(m.value.GetInt64()));
								break;
								// Если тип ключа является FLOAT
								case static_cast <uint8_t> (type_t::FLOAT):
									// Выполняем добавление расширение
									this->extension(m.name.GetString(), this->_fmk->noexp(m.value.GetFloat(), true));
								break;
								// Если тип ключа является DOUBLE
								case static_cast <uint8_t> (type_t::DOUBLE):
									// Выполняем добавление расширение
									this->extension(m.name.GetString(), this->_fmk->noexp(m.value.GetDouble(), true));
								break;
								// Если тип ключа является TIMESTAMP
								case static_cast <uint8_t> (type_t::TIMESTAMP):
									// Если переданное значение является числом
									if(m.value.IsNumber())
										// Выполняем добавление расширение
										this->extension(m.name.GetString(), std::to_string(m.value.GetInt64()));
									// Если значение является текстовой строкой
									else if(m.value.IsString())
										// Выполняем добавление расширение
										this->extension(m.name.GetString(), m.value.GetString());
								break;
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
}
/**
 * header Метод извлечения заголовка
 * @return заголовок контейнера
 */
const string & anyks::Cef::header() const noexcept {
	// Выводим данные полученного заголовка
	return this->_header;
}
/**
 * header Метод установки заголовка контейнера
 * @param header заголовок контейнера
 */
void anyks::Cef::header(const string & header) noexcept {
	// Если заголовок передан
	if(!header.empty())
		// Устанавливаем данные заголовка
		this->_header = header;
	// Выводим сообщение об ошибке
	else {
		/**
		 * Если включён режим отладки
		 */
		#if defined(DEBUG_MODE)
			// Выводим сообщение об ошибке
			this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(header), log_t::flag_t::CRITICAL, "Header not passed");
		/**
		* Если режим отладки не включён
		*/
		#else
			// Выводим сообщение об ошибке
			this->_log->print("%s", log_t::flag_t::CRITICAL, "Header not passed");
		#endif
	}
}
/**
 * event Метод извлечения события
 * @return событие контейнера
 */
const anyks::Cef::event_t & anyks::Cef::event() const noexcept {
	// Выводим данные события
	return this->_event;
}
/**
 * event Метод установки события
 * @param event данные события
 */
void anyks::Cef::event(const event_t & event) noexcept {
	// Выполняем установку данных события
	this->_event = event;
}
/**
 * format Метод установки формата даты
 * @param format формат даты
 */
void anyks::Cef::format(const string & format) noexcept {
	// Устанавливаем формат времени
	this->_format = format;
}
/**
 * type Метод извлечения типа ключа
 * @param key ключ для извлечения типа расширения
 * @return    тип данных которому соответствует ключ
 */
anyks::Cef::type_t anyks::Cef::type(const string & key) const noexcept {
	// Если ключ для извлечения типа расширения передан
	if(!key.empty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем поиск соответствие нашему ключу
			auto i = this->_mapping.find(key);
			// Если соответствие ключу найдено
			if(i != this->_mapping.end()){
				// Выполняем поиск данных нашего ключа
				auto j = this->_extensions.find(i->second);
				// Если данные запрашиваемого ключа получены
				if(j != this->_extensions.end()){
					// Создаём объект параметров расширения
					const ext_t * params = nullptr;
					// Выполняем поиск параметров ключа
					auto i = this->_extensionSEFv0.find(j->first);
					// Если параметры ключа найдены
					if(i != this->_extensionSEFv0.end())
						// Получаем параметры ключа
						params = &i->second;
					// Если параметры ключа не найдены и версия протокола №1
					else if(this->_version > .0) {
						// Выполняем поиск параметров ключа
						auto i = this->_extensionSEFv1.find(j->first);
						// Если параметры ключа найдены
						if(i != this->_extensionSEFv0.end())
							// Получаем параметры ключа
							params = &i->second;
					}
					// Если параметры ключа получены
					if(params != nullptr)
						// Выводим тип записи
						return params->type;
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
				this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(key), log_t::flag_t::CRITICAL, error.what());
			/**
			* Если режим отладки не включён
			*/
			#else
				// Выводим сообщение об ошибке
				this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
			#endif
		}
	}
	// Выводим результат по умолчанию
	return type_t::NONE;
}
/**
 * events Метод получения списка событий
 * @return список полученных событий
 */
unordered_map <string, string> anyks::Cef::events() const noexcept {
	// Результат работы функции
	unordered_map <string, string> result;
	/**
	 * Выполняем отлов ошибок
	 */
	try {
		// Устанавливаем название события
		result.emplace("name", this->_event.name);
		// Устанавливаем поставщика данных
		result.emplace("vendor", this->_event.devVendor);
		// Устанавливаем версию поставщика данных
		result.emplace("version", this->_event.devVersion);
		// Устанавливаем тип устройства поставщика данных
		result.emplace("product", this->_event.devProduct);
		// Устанавливаем подпись события поставщика данных
		result.emplace("signature", this->_event.signatureId);
		// Устанавливаем числовое значение важности события
		result.emplace("severity", std::to_string(this->_event.severity.level));
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
	// Выводим результат
	return result;
}
/**
 * extensions Метод извлечения списка расширений
 * @return список установленных расширений
 */
unordered_map <string, string> anyks::Cef::extensions() const noexcept {
	// Результат работы функции
	unordered_map <string, string> result;
	// Если список расширений запомнен
	if(!this->_extensions.empty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Создаём объект параметров расширения
			const ext_t * params = nullptr;
			// Переходим по всему списку расширений
			for(auto & extension : this->_extensions){
				// Если режим парсинга установлен
				if(this->_mode != mode_t::NONE){
					// Выполняем поиск параметров ключа
					auto i = this->_extensionSEFv0.find(extension.first);
					// Если параметры ключа найдены
					if(i != this->_extensionSEFv0.end())
						// Получаем параметры ключа
						params = &i->second;
					// Если параметры ключа не найдены и версия протокола №1
					else if(this->_version > .0) {
						// Выполняем поиск параметров ключа
						auto i = this->_extensionSEFv1.find(extension.first);
						// Если параметры ключа найдены
						if(i != this->_extensionSEFv0.end())
							// Получаем параметры ключа
							params = &i->second;
					}
					// Если параметры ключа получены
					if(params != nullptr){
						// Определяем тип ключа
						switch(static_cast <uint8_t> (params->type)){
							// Если тип ключа является IP-адресом
							case static_cast <uint8_t> (type_t::IP): {
								// Если включён строгий режим парсинга
								if(this->_mode == mode_t::STRONG){
									// Создаём объект сети
									net_t net(this->_log);
									// Если количество байт в буфере 4
									if(extension.second.size() == 4){
										// Формируем число из бинарного буфера
										uint32_t value = 0;
										// Копируем в бинарный буфер данные IP адреса
										::memcpy(&value, extension.second.data(), extension.second.size());
										// Устанавливаем данные адреса в объект сети
										net.v4(value);
									// Если количество байт в буфере 16
									} else if(extension.second.size() == 16){
										// Формируем бинарный буфер данных
										array <uint64_t, 2> buffer;
										// Копируем в бинарный буфер данные IP адреса
										::memcpy(buffer.data(), extension.second.data(), extension.second.size());
										// Устанавливаем данные адреса в объект сети
										net.v6(buffer);
									}
									// Устанавливаем значение ключа
									result.emplace(extension.first, net);
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else result.emplace(extension.first, string(extension.second.begin(), extension.second.end()));
							} break;
							// Если тип ключа является MAC-адресом
							case static_cast <uint8_t> (type_t::MAC): {
								// Если включён строгий режим парсинга
								if(this->_mode == mode_t::STRONG){
									// Создаём объект сети
									net_t net(this->_log);
									// Формируем число из бинарного буфера
									uint64_t value = 0;
									// Копируем в бинарный буфер данные IP адреса
									::memcpy(&value, extension.second.data(), extension.second.size());
									// Устанавливаем данные адреса в объект сети
									net.mac(value);
									// Устанавливаем значение ключа
									result.emplace(extension.first, net);
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else result.emplace(extension.first, string(extension.second.begin(), extension.second.end()));
							} break;
							// Если тип ключа является IPV4-адресом
							case static_cast <uint8_t> (type_t::IPV4): {
								// Если включён строгий режим парсинга
								if(this->_mode == mode_t::STRONG){
									// Создаём объект сети
									net_t net(this->_log);
									// Формируем число из бинарного буфера
									uint32_t value = 0;
									// Копируем в бинарный буфер данные IP адреса
									::memcpy(&value, extension.second.data(), extension.second.size());
									// Устанавливаем данные адреса в объект сети
									net.v4(value);
									// Устанавливаем значение ключа
									result.emplace(extension.first, net);
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else result.emplace(extension.first, string(extension.second.begin(), extension.second.end()));
							} break;
							// Если тип ключа является IPV6-адресом
							case static_cast <uint8_t> (type_t::IPV6): {
								// Если включён строгий режим парсинга
								if(this->_mode == mode_t::STRONG){
									// Создаём объект сети
									net_t net(this->_log);
									// Формируем бинарный буфер данных
									array <uint64_t, 2> buffer;
									// Копируем в бинарный буфер данные IP адреса
									::memcpy(buffer.data(), extension.second.data(), extension.second.size());
									// Устанавливаем данные адреса в объект сети
									net.v6(buffer);
									// Устанавливаем значение ключа
									result.emplace(extension.first, net);
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else result.emplace(extension.first, string(extension.second.begin(), extension.second.end()));
							} break;
							// Если тип ключа является LONG
							case static_cast <uint8_t> (type_t::LONG): {
								// Если режим парсинга установлен
								if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
									// Формируем число из бинарного буфера
									long value = 0;
									// Извлекаем из буфера данные числа
									::memcpy(&value, extension.second.data(), extension.second.size());
									// Устанавливаем значение ключа
									result.emplace(extension.first, std::to_string(value));
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else {
									// Получаем значение числа
									const string value(extension.second.begin(), extension.second.end());
									/**
									 * Выполняем отлов ошибок
									 */
									try {
										// Если число положительное
										if(value.front() != '-')
											// Добавляем полученное значения расширения
											result.emplace(extension.first, std::to_string(::stoul(value)));
										// Добавляем полученное значения расширения
										else result.emplace(extension.first, std::to_string(::stol(value)));
									/**
									 * Если возникает ошибка
									 */
									} catch(const exception &) {
										// Добавляем полученное значения расширения
										result.emplace(extension.first, value);
									}
								}
							} break;
							// Если тип ключа является INT32
							case static_cast <uint8_t> (type_t::INT32): {
								// Если режим парсинга установлен
								if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
									// Формируем число из бинарного буфера
									int32_t value = 0;
									// Извлекаем из буфера данные числа
									::memcpy(&value, extension.second.data(), extension.second.size());
									// Устанавливаем значение ключа
									result.emplace(extension.first, std::to_string(value));
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else {
									// Получаем значение числа
									const string value(extension.second.begin(), extension.second.end());
									/**
									 * Выполняем отлов ошибок
									 */
									try {
										// Если число положительное
										if(value.front() != '-')
											// Добавляем полученное значения расширения
											result.emplace(extension.first, std::to_string(::stoul(value)));
										// Добавляем полученное значения расширения
										else result.emplace(extension.first, std::to_string(::stoi(value)));
									/**
									 * Если возникает ошибка
									 */
									} catch(const exception &) {
										// Добавляем полученное значения расширения
										result.emplace(extension.first, value);
									}
								}
							} break;
							// Если тип ключа является INT64
							case static_cast <uint8_t> (type_t::INT64): {
								// Если режим парсинга установлен
								if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
									// Формируем число из бинарного буфера
									int64_t value = 0;
									// Извлекаем из буфера данные числа
									::memcpy(&value, extension.second.data(), extension.second.size());
									// Устанавливаем значение ключа
									result.emplace(extension.first, std::to_string(value));
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else {
									// Получаем значение числа
									const string value(extension.second.begin(), extension.second.end());
									/**
									 * Выполняем отлов ошибок
									 */
									try {
										// Если число положительное
										if(value.front() != '-')
											// Добавляем полученное значения расширения
											result.emplace(extension.first, std::to_string(::stoull(value)));
										// Добавляем полученное значения расширения
										else result.emplace(extension.first, std::to_string(::stoll(value)));
									/**
									 * Если возникает ошибка
									 */
									} catch(const exception &) {
										// Добавляем полученное значения расширения
										result.emplace(extension.first, value);
									}
								}
							} break;
							// Если тип ключа является FLOAT
							case static_cast <uint8_t> (type_t::FLOAT): {
								// Если режим парсинга установлен
								if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
									// Формируем число из бинарного буфера
									float value = .0f;
									// Извлекаем из буфера данные числа
									::memcpy(&value, extension.second.data(), extension.second.size());
									// Устанавливаем значение ключа
									result.emplace(extension.first, this->_fmk->noexp(value, true));
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else {
									/**
									 * Выполняем отлов ошибок
									 */
									try {
										// Устанавливаем значение ключа
										result.emplace(extension.first, this->_fmk->noexp(::stof(string(extension.second.begin(), extension.second.end())), true));
									/**
									 * Если возникает ошибка
									 */
									} catch(const exception &) {
										// Устанавливаем значение ключа
										result.emplace(extension.first, string(extension.second.begin(), extension.second.end()));
									}
								}
							} break;
							// Если тип ключа является DOUBLE
							case static_cast <uint8_t> (type_t::DOUBLE): {
								// Если режим парсинга установлен
								if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
									// Формируем число из бинарного буфера
									double value = .0;
									// Извлекаем из буфера данные числа
									::memcpy(&value, extension.second.data(), extension.second.size());
									// Устанавливаем значение ключа
									result.emplace(extension.first, this->_fmk->noexp(value, true));
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else {
									/**
									 * Выполняем отлов ошибок
									 */
									try {
										// Устанавливаем значение ключа
										result.emplace(extension.first, this->_fmk->noexp(::stold(string(extension.second.begin(), extension.second.end())), true));
									/**
									 * Если возникает ошибка
									 */
									} catch(const exception &) {
										// Устанавливаем значение ключа
										result.emplace(extension.first, string(extension.second.begin(), extension.second.end()));
									}
								}
							} break;
							// Если тип ключа является STRING
							case static_cast <uint8_t> (type_t::STRING):
								// Устанавливаем значение ключа
								result.emplace(
									extension.first,
									string(extension.second.begin(), extension.second.end())
								);
							break;
							// Если тип ключа является TIMESTAMP
							case static_cast <uint8_t> (type_t::TIMESTAMP): {
								// Если режим парсинга установлен
								if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
									// Формируем число из бинарного буфера
									time_t date = 0;
									// Извлекаем из буфера данные числа
									::memcpy(&date, extension.second.data(), extension.second.size());
									// Если формат даты установлен
									if(!this->_format.empty()){
										// Создаём объект потока
										stringstream transTime;
										// Создаем структуру времени
										tm * tm = ::localtime(&date);
										// Выполняем извлечение даты
										transTime << put_time(tm, this->_format.c_str());
										// Устанавливаем значение ключа
										result.emplace(extension.first, transTime.str());
									// Устанавливаем значение ключа
									} else result.emplace(extension.first, std::to_string(date));
								// Если строгий режим парсинга не активирован, устанавливаем значение ключа
								} else result.emplace(extension.first, string(extension.second.begin(), extension.second.end()));
							} break;
						}
					}
				// Если режим парсинга не установлен
				} else {
					// Получаем значение для вывода
					string value(extension.second.begin(), extension.second.end());
					// Если запись является числом
					if(this->_fmk->is(value, fmk_t::check_t::NUMBER)){
						/**
						 * Выполняем отлов ошибок
						 */
						try {
							// Если число положительное
							if(value.front() != '-')
								// Устанавливаем значение ключа как число без знака
								result.emplace(extension.first, std::to_string(::stoull(value)));
							// Устанавливаем значение ключа как число со знаком
							else result.emplace(extension.first, std::to_string(::stoll(value)));
						/**
						 * Если возникает ошибка
						 */
						} catch(const exception &) {
							// Устанавливаем значение ключа как число без знака
							result.emplace(extension.first, value);
						}
					// Если запись является числом с плавающей точкой
					} else if(this->_fmk->is(value, fmk_t::check_t::DECIMAL)) {
						/**
						 * Выполняем отлов ошибок
						 */
						try {
							// Устанавливаем значение ключа как число с плавающей точкой
							result.emplace(extension.first, this->_fmk->noexp(::stold(value), true));
						/**
						 * Если возникает ошибка
						 */
						} catch(const exception &) {
							// Устанавливаем значение ключа как число без знака
							result.emplace(extension.first, value);
						}
					// Если число является булевым истинным значением
					} else if(this->_fmk->compare("true", value))
						// Устанавливаем значение ключа как булевое положительное значение
						result.emplace(extension.first, "true");
					// Если число является булевым ложным значением
					else if(this->_fmk->compare("false", value))
						// Устанавливаем значение ключа как булевое отрицательное значение
						result.emplace(extension.first, "false");
					// Устанавливаем значение ключа как оно есть
					else result.emplace(extension.first, ::move(value));
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
	// Выводим результат
	return result;
}
/**
 * extension Метод извлечения расширения в бинарном виде
 * @param key ключ для извлечения расширения
 * @return    данные расширения в бинарном виде
 */
const vector <char> & anyks::Cef::extension(const string & key) const noexcept {
	// Результат работы функции
	static const vector <char> result;
	// Если ключ передан
	if(!key.empty()){
		// Выполняем поиск ключа среди расширений
		auto i = this->_extensions.find(key);
		// Если ключ расширения найден
		if(i != this->_extensions.end())
			// Выводим результат
			return i->second;
	}
	// Выводим результат
	return result;
}
/**
 * extension Метод установки расширения в бинарном виде
 * @param key   ключ расширения
 * @param value значение расширения
 */
void anyks::Cef::extension(const string & key, const string & value) noexcept {
	// Если ключ и значение переданы
	if(!key.empty() && !value.empty()){
		/**
		 * Выполняем отлов ошибок
		 */
		try {
			// Выполняем удаление лишних записей значения
			this->_fmk->transform(value, fmk_t::transform_t::TRIM);
			// Если режим парсинга установлен
			if(this->_mode != mode_t::NONE){
				// Создаём объект параметров расширения
				const ext_t * params = nullptr;
				// Выполняем поиск параметров ключа
				auto i = this->_extensionSEFv0.find(key);
				// Если параметры ключа найдены
				if(i != this->_extensionSEFv0.end())
					// Получаем параметры ключа
					params = &i->second;
				// Если параметры ключа не найдены и версия протокола №1
				else if(this->_version > .0) {
					// Выполняем поиск параметров ключа
					auto i = this->_extensionSEFv1.find(key);
					// Если параметры ключа найдены
					if(i != this->_extensionSEFv0.end())
						// Получаем параметры ключа
						params = &i->second;
				}
				// Если параметры ключа не получены но включён не строгий режим работы
				if((params == nullptr) && (this->_mode != mode_t::STRONG)){
					// Выполняем определение ключа, если вместо него пришло название
					auto i = this->_mapping.find(key);
					// Если параметры ключа получены
					if(i != this->_mapping.end()){
						// Выполняем очередную попытку получать данные расширения
						this->extension(i->second, value);
						// Выходим из функции
						return;
					}
				// Если же параметры ключа не получены и включён строгий режим
				} else if((params == nullptr) && (this->_mode == mode_t::STRONG)) {
					// Создаём текст ошибки
					string error = "The \"";
					// Добавляем ключ который не найден
					error.append(key);
					// Добавляем оставшийся текст ошибки
					error.append("\" key is not defined in the specification");
					/**
					 * Если включён режим отладки
					 */
					#if defined(DEBUG_MODE)
						// Выводим сообщение об ошибке
						this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(key, value), log_t::flag_t::WARNING, error.c_str());
					/**
					* Если режим отладки не включён
					*/
					#else
						// Выводим сообщение об ошибке
						this->_log->print("%s", log_t::flag_t::WARNING, error.c_str());
					#endif
					// Выходим из функции
					return;
				}
				// Если параметры ключа получены
				if(params != nullptr){
					// Определяем тип ключа
					switch(static_cast <uint8_t> (params->type)){
						// Если тип ключа является IP-адресом
						case static_cast <uint8_t> (type_t::IP): {
							// Если включён строгий режим парсинга
							if(this->_mode == mode_t::STRONG){
								// Создаём объект сети
								net_t net(this->_log);
								// Выполняем парсинг сетевого адреса
								if(net.parse(value)){
									// Выполняем определение типа IP адреса
									switch(static_cast <uint8_t> (net.type())){
										// Если IP адрес определён как IPv4
										case static_cast <uint8_t> (net_t::type_t::IPV4): {
											// Извлекаем данные IP адреса
											const uint32_t data = net.v4();
											// Добавляем полученное расширение в базу
											this->extension(key, vector <char> (
												reinterpret_cast <const char *> (&data),
												reinterpret_cast <const char *> (&data) + sizeof(data)
											));
										} break;
										// Если IP адрес определён как IPv6
										case static_cast <uint8_t> (net_t::type_t::IPV6): {
											// Извлекаем данные IP адреса
											const auto & data = net.v6();
											// Добавляем полученное расширение в базу
											this->extension(key, vector <char> (data.begin(), data.end()));
										} break;
									}
								// Выводим сообщение об ошибке
								} else {
									/**
									 * Если включён режим отладки
									 */
									#if defined(DEBUG_MODE)
										// Выводим сообщение об ошибке
										this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(key, value), log_t::flag_t::WARNING, params->desc.c_str());
									/**
									* Если режим отладки не включён
									*/
									#else
										// Выводим сообщение об ошибке
										this->_log->print("%s", log_t::flag_t::WARNING, params->desc.c_str());
									#endif
								}
							// Если строгий режим парсинга не активирован, устанавливаем значение ключа
							} else this->extension(key, vector <char> (value.begin(), value.end()));
						} break;
						// Если тип ключа является MAC-адресом
						case static_cast <uint8_t> (type_t::MAC): {
							// Если включён строгий режим парсинга
							if(this->_mode == mode_t::STRONG){
								// Создаём объект сети
								net_t net(this->_log);
								// Выполняем парсинг аппаратного адреса
								if(net.parse(value, net_t::type_t::MAC)){
									// Извлекаем данные MAC адреса
									const uint64_t data = net.mac();
									// Добавляем полученное расширение в базу
									this->extension(key, vector <char> (
										reinterpret_cast <const char *> (&data),
										reinterpret_cast <const char *> (&data) + sizeof(data)
									));
								// Выводим сообщение об ошибке
								} else {
									/**
									 * Если включён режим отладки
									 */
									#if defined(DEBUG_MODE)
										// Выводим сообщение об ошибке
										this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(key, value), log_t::flag_t::WARNING, params->desc.c_str());
									/**
									* Если режим отладки не включён
									*/
									#else
										// Выводим сообщение об ошибке
										this->_log->print("%s", log_t::flag_t::WARNING, params->desc.c_str());
									#endif
								}
							// Если строгий режим парсинга не активирован, устанавливаем значение ключа
							} else this->extension(key, vector <char> (value.begin(), value.end()));
						} break;
						// Если тип ключа является IPV4-адресом
						case static_cast <uint8_t> (type_t::IPV4): {
							// Если включён строгий режим парсинга
							if(this->_mode == mode_t::STRONG){
								// Создаём объект сети
								net_t net(this->_log);
								// Выполняем парсинг сетевого адреса
								if(net.parse(value, net_t::type_t::IPV4)){
									// Извлекаем данные IP адреса
									const uint32_t data = net.v4();
									// Добавляем полученное расширение в базу
									this->extension(key, vector <char> (
										reinterpret_cast <const char *> (&data),
										reinterpret_cast <const char *> (&data) + sizeof(data)
									));
								// Выводим сообщение об ошибке
								} else {
									/**
									 * Если включён режим отладки
									 */
									#if defined(DEBUG_MODE)
										// Выводим сообщение об ошибке
										this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(key, value), log_t::flag_t::WARNING, params->desc.c_str());
									/**
									* Если режим отладки не включён
									*/
									#else
										// Выводим сообщение об ошибке
										this->_log->print("%s", log_t::flag_t::WARNING, params->desc.c_str());
									#endif
								}
							// Если строгий режим парсинга не активирован, устанавливаем значение ключа
							} else this->extension(key, vector <char> (value.begin(), value.end()));
						} break;
						// Если тип ключа является IPV6-адресом
						case static_cast <uint8_t> (type_t::IPV6): {
							// Если включён строгий режим парсинга
							if(this->_mode == mode_t::STRONG){
								// Создаём объект сети
								net_t net(this->_log);
								// Выполняем парсинг сетевого адреса
								if(net.parse(value, net_t::type_t::IPV6)){
									// Извлекаем данные IP адреса
									const auto & data = net.v6();
									// Добавляем полученное расширение в базу
									this->extension(key, vector <char> (data.begin(), data.end()));
								// Выводим сообщение об ошибке
								} else {
									/**
									 * Если включён режим отладки
									 */
									#if defined(DEBUG_MODE)
										// Выводим сообщение об ошибке
										this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(key, value), log_t::flag_t::WARNING, params->desc.c_str());
									/**
									* Если режим отладки не включён
									*/
									#else
										// Выводим сообщение об ошибке
										this->_log->print("%s", log_t::flag_t::WARNING, params->desc.c_str());
									#endif
								}
							// Если строгий режим парсинга не активирован, устанавливаем значение ключа
							} else this->extension(key, vector <char> (value.begin(), value.end()));
						} break;
						// Если тип ключа является LONG
						case static_cast <uint8_t> (type_t::LONG): {
							// Если режим парсинга установлен
							if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
								// Выполняем проверку, является ли значение числом
								if(this->_fmk->is(value, fmk_t::check_t::NUMBER)){
									/**
									 * Выполняем отлов ошибок
									 */
									try {
										// Преобразуем строку в число
										const long data = ::stol(value);
										// Добавляем полученное расширение в базу
										this->extension(key, vector <char> (
											reinterpret_cast <const char *> (&data),
											reinterpret_cast <const char *> (&data) + sizeof(data)
										));
									/**
									 * Если возникает ошибка
									 */
									} catch(const exception &) {
										// Добавляем полученное расширение в базу
										this->extension(key, vector <char> (value.begin(), value.end()));
									}
								// Выводим сообщение об ошибке
								} else {
									/**
									 * Если включён режим отладки
									 */
									#if defined(DEBUG_MODE)
										// Выводим сообщение об ошибке
										this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(key, value), log_t::flag_t::WARNING, params->desc.c_str());
									/**
									* Если режим отладки не включён
									*/
									#else
										// Выводим сообщение об ошибке
										this->_log->print("%s", log_t::flag_t::WARNING, params->desc.c_str());
									#endif
								}
							// Если строгий режим парсинга не активирован, устанавливаем значение ключа
							} else this->extension(key, vector <char> (value.begin(), value.end()));
						} break;
						// Если тип ключа является INT32
						case static_cast <uint8_t> (type_t::INT32): {
							// Если режим парсинга установлен
							if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
								// Выполняем проверку, является ли значение числом
								if(this->_fmk->is(value, fmk_t::check_t::NUMBER)){
									/**
									 * Выполняем отлов ошибок
									 */
									try {
										// Преобразуем строку в число
										const int32_t data = ::stoi(value);
										// Добавляем полученное расширение в базу
										this->extension(key, vector <char> (
											reinterpret_cast <const char *> (&data),
											reinterpret_cast <const char *> (&data) + sizeof(data)
										));
									/**
									 * Если возникает ошибка
									 */
									} catch(const exception &) {
										// Добавляем полученное расширение в базу
										this->extension(key, vector <char> (value.begin(), value.end()));
									}
								// Выводим сообщение об ошибке
								} else {
									/**
									 * Если включён режим отладки
									 */
									#if defined(DEBUG_MODE)
										// Выводим сообщение об ошибке
										this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(key, value), log_t::flag_t::WARNING, params->desc.c_str());
									/**
									* Если режим отладки не включён
									*/
									#else
										// Выводим сообщение об ошибке
										this->_log->print("%s", log_t::flag_t::WARNING, params->desc.c_str());
									#endif
								}
							// Если строгий режим парсинга не активирован, устанавливаем значение ключа
							} else this->extension(key, vector <char> (value.begin(), value.end()));
						} break;
						// Если тип ключа является INT64
						case static_cast <uint8_t> (type_t::INT64): {
							// Если режим парсинга установлен
							if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
								// Выполняем проверку, является ли значение числом
								if(this->_fmk->is(value, fmk_t::check_t::NUMBER)){
									/**
									 * Выполняем отлов ошибок
									 */
									try {
										// Преобразуем строку в число
										const int64_t data = ::stoll(value);
										// Добавляем полученное расширение в базу
										this->extension(key, vector <char> (
											reinterpret_cast <const char *> (&data),
											reinterpret_cast <const char *> (&data) + sizeof(data)
										));
									/**
									 * Если возникает ошибка
									 */
									} catch(const exception &) {
										// Добавляем полученное расширение в базу
										this->extension(key, vector <char> (value.begin(), value.end()));
									}
								// Выводим сообщение об ошибке
								} else {
									/**
									 * Если включён режим отладки
									 */
									#if defined(DEBUG_MODE)
										// Выводим сообщение об ошибке
										this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(key, value), log_t::flag_t::WARNING, params->desc.c_str());
									/**
									* Если режим отладки не включён
									*/
									#else
										// Выводим сообщение об ошибке
										this->_log->print("%s", log_t::flag_t::WARNING, params->desc.c_str());
									#endif
								}
							// Если строгий режим парсинга не активирован, устанавливаем значение ключа
							} else this->extension(key, vector <char> (value.begin(), value.end()));
						} break;
						// Если тип ключа является FLOAT
						case static_cast <uint8_t> (type_t::FLOAT): {
							// Если режим парсинга установлен
							if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
								// Выполняем проверку, является ли значение числом
								if(this->_fmk->is(value, fmk_t::check_t::DECIMAL)){
									/**
									 * Выполняем отлов ошибок
									 */
									try {
										// Преобразуем строку в число
										const float data = ::stof(value);
										// Добавляем полученное расширение в базу
										this->extension(key, vector <char> (
											reinterpret_cast <const char *> (&data),
											reinterpret_cast <const char *> (&data) + sizeof(data)
										));
									/**
									 * Если возникает ошибка
									 */
									} catch(const exception &) {
										// Добавляем полученное расширение в базу
										this->extension(key, vector <char> (value.begin(), value.end()));
									}
								// Выводим сообщение об ошибке
								} else {
									/**
									 * Если включён режим отладки
									 */
									#if defined(DEBUG_MODE)
										// Выводим сообщение об ошибке
										this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(key, value), log_t::flag_t::WARNING, params->desc.c_str());
									/**
									* Если режим отладки не включён
									*/
									#else
										// Выводим сообщение об ошибке
										this->_log->print("%s", log_t::flag_t::WARNING, params->desc.c_str());
									#endif
								}
							// Если строгий режим парсинга не активирован, устанавливаем значение ключа
							} else this->extension(key, vector <char> (value.begin(), value.end()));
						} break;
						// Если тип ключа является DOUBLE
						case static_cast <uint8_t> (type_t::DOUBLE): {
							// Если режим парсинга установлен
							if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
								// Выполняем проверку, является ли значение числом
								if(this->_fmk->is(value, fmk_t::check_t::DECIMAL)){
									/**
									 * Выполняем отлов ошибок
									 */
									try {
										// Преобразуем строку в число
										const double data = ::stold(value);
										// Добавляем полученное расширение в базу
										this->extension(key, vector <char> (
											reinterpret_cast <const char *> (&data),
											reinterpret_cast <const char *> (&data) + sizeof(data)
										));
									/**
									 * Если возникает ошибка
									 */
									} catch(const exception &) {
										// Добавляем полученное расширение в базу
										this->extension(key, vector <char> (value.begin(), value.end()));
									}
								// Выводим сообщение об ошибке
								} else {
									/**
									 * Если включён режим отладки
									 */
									#if defined(DEBUG_MODE)
										// Выводим сообщение об ошибке
										this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(key, value), log_t::flag_t::WARNING, params->desc.c_str());
									/**
									* Если режим отладки не включён
									*/
									#else
										// Выводим сообщение об ошибке
										this->_log->print("%s", log_t::flag_t::WARNING, params->desc.c_str());
									#endif
								}
							// Если строгий режим парсинга не активирован, устанавливаем значение ключа
							} else this->extension(key, vector <char> (value.begin(), value.end()));
						} break;
						// Если тип ключа является STRING
						case static_cast <uint8_t> (type_t::STRING): {
							// Если режим парсинга установлен
							if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
								// Если размер соответствует или включён не строгий режим
								if((params->size >= value.size()))
									// Добавляем полученное расширение в базу
									this->extension(key, vector <char> (value.begin(), value.end()));
								// Выводим сообщение об ошибке
								else {
									/**
									 * Если включён режим отладки
									 */
									#if defined(DEBUG_MODE)
										// Выводим сообщение об ошибке
										this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(key, value), log_t::flag_t::WARNING, params->desc.c_str());
									/**
									* Если режим отладки не включён
									*/
									#else
										// Выводим сообщение об ошибке
										this->_log->print("%s", log_t::flag_t::WARNING, params->desc.c_str());
									#endif
								}
							// Если строгий режим парсинга не активирован, устанавливаем значение ключа
							} else this->extension(key, vector <char> (value.begin(), value.end()));
						} break;
						// Если тип ключа является TIMESTAMP
						case static_cast <uint8_t> (type_t::TIMESTAMP): {
							// Если режим парсинга установлен
							if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
								// Получаем флаг проверки на число
								const bool number = this->_fmk->is(value, fmk_t::check_t::NUMBER);
								// Получаем флаг проверки на число плавающей точкой
								const bool decimal = (!number ? this->_fmk->is(value, fmk_t::check_t::DECIMAL) : false);
								// Выполняем проверку, является ли значение числом
								if(number || decimal){
									/**
									 * Выполняем отлов ошибок
									 */
									try {
										// Преобразуем строку в число
										const time_t date = static_cast <time_t> (number ? ::stoull(value) : ::stold(value));
										// Добавляем полученное расширение в базу
										this->extension(key, vector <char> (
											reinterpret_cast <const char *> (&date),
											reinterpret_cast <const char *> (&date) + sizeof(date)
										));
									/**
									 * Если возникает ошибка
									 */
									} catch(const exception &) {
										// Добавляем полученное расширение в базу
										this->extension(key, vector <char> (value.begin(), value.end()));
									}
								// Если режим работы не строгий
								} else if(!this->_format.empty()) {
									// Выполняем парсинг даты
									const time_t date = this->_fmk->str2time(value.c_str(), this->_format.c_str());
									// Добавляем полученное расширение в базу
									this->extension(key, vector <char> (
										reinterpret_cast <const char *> (&date),
										reinterpret_cast <const char *> (&date) + sizeof(date)
									));
								// Выводим сообщение об ошибке
								} else {
									/**
									 * Если включён режим отладки
									 */
									#if defined(DEBUG_MODE)
										// Выводим сообщение об ошибке
										this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(key, value), log_t::flag_t::WARNING, params->desc.c_str());
									/**
									* Если режим отладки не включён
									*/
									#else
										// Выводим сообщение об ошибке
										this->_log->print("%s", log_t::flag_t::WARNING, params->desc.c_str());
									#endif
								}
							// Если строгий режим парсинга не активирован, устанавливаем значение ключа
							} else this->extension(key, vector <char> (value.begin(), value.end()));
						} break;
					}
				// Если ключ не получен, выводим сообщение об ошибке
				} else {
					// Создаём текст ошибки
					string error = "Extension \"";
					// Добавляем ключ который не найден
					error.append(key);
					// Добавляем оставшийся текст ошибки
					error.append("\" does not exist");
					/**
					 * Если включён режим отладки
					 */
					#if defined(DEBUG_MODE)
						// Выводим сообщение об ошибке
						this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(key, value), log_t::flag_t::WARNING, error.c_str());
					/**
					* Если режим отладки не включён
					*/
					#else
						// Выводим сообщение об ошибке
						this->_log->print("%s", log_t::flag_t::WARNING, error.c_str());
					#endif
				}
			// Если режим парсинга не установлен, добавляем расширение так-как оно было передано
			} else this->extension(key, vector <char> (value.begin(), value.end()));
		/**
		 * Если возникает ошибка
		 */
		} catch(const exception & error) {
			/**
			 * Если включён режим отладки
			 */
			#if defined(DEBUG_MODE)
				// Выводим сообщение об ошибке
				this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(key, value), log_t::flag_t::CRITICAL, error.what());
			/**
			* Если режим отладки не включён
			*/
			#else
				// Выводим сообщение об ошибке
				this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
			#endif
		}
	}
}
/**
 * extension Метод установки расширения в бинарном виде
 * @param key   ключ расширения
 * @param value значение расширения
 */
void anyks::Cef::extension(const string & key, const vector <char> & value) noexcept {
	// Если данные расширения переданы
	if(!key.empty() && !value.empty())
		// Выполняем добавление данных расширения
		this->_extensions.emplace(key, value);
}
/**
 * Оператор вывода данные контейнера в качестве строки
 * @return данные контейнера в качестве строки
 */
anyks::Cef::operator string() const noexcept {
	// Выводим данные контейнера
	return this->cef();
}
/**
 * Оператор [!=] сравнения контейнеров
 * @param cef контенер для сравнения
 * @return    результат сравнения
 */
bool anyks::Cef::operator != (const cef_t & cef) const noexcept {
	// Результат работы функции
	bool result = false;
	// Выполняем сравнение версий контейнера
	if((result = ((this->_version == cef._version) &&
	   (this->_header.compare(cef._header) == 0) &&
	   (::memcmp(&this->_event, &cef._event, sizeof(this->_event)) == 0)))){
		// Если количество расширений совпадает
		if((result = (this->_extensions.size() == cef._extensions.size()))){
			/**
			 * Выполняем отлов ошибок
			 */
			try {
				// Выполняем перебор всех расширений
				for(auto & extension : this->_extensions){
					// Выполняем поиск указанного расширения
					auto i = cef._extensions.find(extension.first);
					// Если ключ расширения найден
					if((result = (i != cef._extensions.end()))){
						// Выполняем сравнение бинарных данных расширения
						result = (::memcmp(extension.second.data(), i->second.data(), i->second.size()) == 0);
						// Если сравнение не выполнено
						if(!result)
							// Выходим из цикла
							break;
					// Иначе выходим из цикла
					} else break;
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
	}
	// Выводим результат
	return !result;
}
/**
 * Оператор [==] сравнения контейнеров
 * @param cef контенер для сравнения
 * @return    результат сравнения
 */
bool anyks::Cef::operator == (const cef_t & cef) const noexcept {
	// Результат работы функции
	bool result = false;
	// Выполняем сравнение версий контейнера
	if((result = ((this->_version == cef._version) &&
	   (this->_header.compare(cef._header) == 0) &&
	   (::memcmp(&this->_event, &cef._event, sizeof(this->_event)) == 0)))){
		// Если количество расширений совпадает
		if((result = (this->_extensions.size() == cef._extensions.size()))){
			/**
			 * Выполняем отлов ошибок
			 */
			try {
				// Выполняем перебор всех расширений
				for(auto & extension : this->_extensions){
					// Выполняем поиск указанного расширения
					auto i = cef._extensions.find(extension.first);
					// Если ключ расширения найден
					if((result = (i != cef._extensions.end()))){
						// Выполняем сравнение бинарных данных расширения
						result = (::memcmp(extension.second.data(), i->second.data(), i->second.size()) == 0);
						// Если сравнение не выполнено
						if(!result)
							// Выходим из цикла
							break;
					// Иначе выходим из цикла
					} else break;
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
	}
	// Выводим результат
	return result;
}
/**
 * Оператор [=] присвоения контейнеров
 * @param cef контенер для присвоения
 * @return    текущий объект
 */
anyks::Cef & anyks::Cef::operator = (const cef_t & cef) noexcept {
	/**
	 * Выполняем отлов ошибок
	 */
	try {
		// Выполняем копирование режима парсинга
		this->_mode = cef._mode;
		// Выполняем копирование заголовка контейнера
		this->_header = cef._header;
		// Выполняем копирование версии контейнера
		this->_version = cef._version;
		// Выполняем получение список расширений
		this->_extensions = cef._extensions;
		// Копируем данные события
		::memcpy(&this->_event, &cef._event, sizeof(cef._event));
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
	// Выводим текущий объект
	return (* this);
}
/**
 * Оператор [=] присвоения режима парсинга
 * @param mode режим парсинга для установки
 * @return     текущий объект
 */
anyks::Cef & anyks::Cef::operator = (const mode_t mode) noexcept {
	// Выполняем копирование режима парсинга
	this->_mode = mode;
	// Выводим текущий объект
	return (* this);
}
/**
 * Оператор [=] присвоения контейнеров
 * @param cef контенер для присвоения
 * @return    текущий объект
 */
anyks::Cef & anyks::Cef::operator = (const string & cef) noexcept {
	// Если данные CEF контейнера получены
	if(!cef.empty())
		// Устанавливаем данные CEF контейнера
		this->parse(cef);
	// Выводим текущий объект
	return (* this);
}
/**
 * Cef Конструктор
 * @param fmk объект фреймворка
 * @param log объект для работы с логами
 */
anyks::Cef::Cef(const fmk_t * fmk, const log_t * log) noexcept :
 _mode(mode_t::STRONG), _version(1.2), _header{""}, _format{FORMAT}, _fmk(fmk), _log(log) {
	/**
	 * Формируем схему расширений для SEFv0
	 */
	this->_extensionSEFv0 = {
		{"slat", {"sourceGeoLatitude", "", type_t::DOUBLE}},
		{"rawEvent", {"rawEvent", "", type_t::STRING, 4000}},
		{"slong", {"sourceGeoLongitude", "", type_t::DOUBLE}},
		{"customerURI", {"customerURI", "", type_t::STRING, 2048}},
		{"fsize", {"fileSize", "Size of the file.", type_t::INT32}},
		{"agentZoneURI", {"agentZoneURI", "", type_t::STRING, 2048}},
		{"fileHash", {"fileHash", "Hash of a file.", type_t::STRING, 255}},
		{"customer ExternalID", {"customerExternalID", "", type_t::STRING, 200}},
		{"oldFileSize", {"oldFileSize", "Size of the old file.", type_t::INT32}},
		{"agentZone ExternalID", {"agentZoneExternalID", "", type_t::STRING, 200}},
		{"deviceZone ExternalID", {"deviceZoneExternalID", "", type_t::STRING, 200}},
		{"act", {"deviceAction", "Action taken by the device.", type_t::STRING, 63}},
		{"sourceZone ExternalID", {"sourceZoneExternalID", "", type_t::STRING, 200}},
		{"oldFileHash", {"oldFileHash", "Hash of the old file.", type_t::STRING, 255}},
		{"oldFileName", {"oldFileName", "Name of the old file.", type_t::STRING, 1023}},
		{"spt", {"sourcePort", "The valid port numbers are 0 to 65535.", type_t::INT32}},
		{"agentTranslated Zone URI", {"agentTranslatedZoneURI", "", type_t::STRING, 2048}},
		{"fileType", {"fileType", "Type of file (pipe, socket, etc.)", type_t::STRING, 1023}},
		{"destinationZone ExternalID", {"destinationZoneExternalID", "", type_t::STRING, 200}},
		{"filePermission", {"filePermission", "Permissions of the file.", type_t::STRING, 1023}},
		{"fname", {"filename", "Name of the file only (without its path).", type_t::STRING, 128}},
		{"fileCreateTime", {"fileCreateTime", "Time when the file was created.", type_t::TIMESTAMP}},
		{"agentTranslated ZoneExternalID", {"agentTranslatedZoneExternalID", "", type_t::STRING, 200}},
		{"dpt", {"destinationPort", "The valid port numbers are between 0 and 65535.", type_t::INT32}},
		{"fileId", {"fileId", "An ID associated with a file could be the inode.", type_t::STRING, 1023}},
		{"device TranslatedZoneExternalID", {"deviceTranslatedZoneExternalID", "", type_t::STRING, 200}},
		{"source TranslatedZoneExternalID", {"sourceTranslatedZoneExternalID", "", type_t::STRING, 200}},
		{"sproc", {"sourceProcessName", "The name of the event’s source process.", type_t::STRING, 1023}},
		{"eventId", {"eventId", "This is a unique ID that ArcSight assigns to each event.", type_t::LONG}},
		{"oldFileCreate Time", {"oldFileCreateTime", "Time when old file was created.", type_t::TIMESTAMP}},
		{"oldFile Permission", {"oldFilePermission", "Permissions of the old file.", type_t::STRING, 1023}},
		{"oldFileType", {"oldFileType", "Type of the old file (pipe, socket, etc.)", type_t::STRING, 1023}},
		{"requestCookies", {"requestCookies", "Cookies associated with the reuest.", type_t::STRING, 1023}},
		{"dtz", {"deviceTimeZone", "The timezone for the device generating the event.", type_t::STRING, 255}},
		{"sntdom", {"sourceNtDomain", "The Windows domain name for the source address.", type_t::STRING, 255}},
		{"spid", {"sourceProcessId", "The ID of the source process associated with the event.", type_t::INT32}},
		{"destination TranslatedZoneExternalID", {"destinationTranslatedZoneExternalID", "", type_t::STRING, 200}},
		{"agt", {"agentAddress", "The IP address of the ArcSight connector that processed the event.", type_t::IP}},
		{"aid", {"agentId", "The agent ID of the ArcSight connector that processed the event.", type_t::STRING, 40}},
		{"oldFileId", {"oldFileId", "An ID associated with the old file could be the inode.", type_t::STRING, 1023}},
		{"outcome", {"eventOutcome", "Displays the outcome, usually as ‘success’ or ‘failure’.", type_t::STRING, 63}},
		{"at", {"agentType", "The agent type of the ArcSight connector that processed the event", type_t::STRING, 63}},
		{"deviceNt Domain", {"deviceNtDomain", "The Windows domain name of the device address.", type_t::STRING, 255}},
		{"dntdom", {"destinationNtDomain", "The Windows domain name of the destination address.", type_t::STRING, 255}},
		{"fileModification Time", {"fileModificationTime", "Time when the file was last modified.", type_t::TIMESTAMP}},
		{"av", {"agentVersion", "The version of the ArcSight connector that processed the event.", type_t::STRING, 31}},
		{"amac", {"agentMacAddress", "The MAC address of the ArcSight connector that processed the event.", type_t::MAC}},
		{"dvcpid", {"deviceProcessId", "Provides the ID of the process on the device generating the event.", type_t::INT32}},
		{"dmac", {"deviceMacAddress", "Six colon-seperated hexadecimal numbers. Example: “00:0D:60:AF:1B:61”", type_t::MAC}},
		{"smac", {"sourceMacAddress", "Six colon-separated hexadecimal numbers. Example: “00:0D:60:AF:1B:61”", type_t::MAC}},
		{"ahost", {"agentHostName", "The hostname of the ArcSight connector that processed the event.", type_t::STRING, 1023}},
		{"oldFile Modification Time", {"oldFileModificationTime", "Time when old file was last modified.", type_t::TIMESTAMP}},
		{"atz", {"agentTimeZone", "The agent time zone of the ArcSight connector that processed the event.", type_t::STRING, 255}},
		{"dlat", {"destinationGeoLatitude", "The latitudinal value from which the destination’s IP address belongs.", type_t::DOUBLE}},
		{"Device eventId", {"deviceeventId", "Unique identifier for the event associated with the event.", type_t::STRING, 128}},
		{"requestClient Application", {"requestClientApplication", "The User-Agent associated with the request.", type_t::STRING, 1023}},
		{"dlong", {"destinationGeoLongitude", "The longitudinal value from which the destination’s IP address belongs.", type_t::DOUBLE}},
		{"source ServiceName", {"sourceServiceName", "The service that is responsible for generating this event.", type_t::STRING, 1023}},
		{"requestMethod", {"requestMethod", "The method used to access a URL. Possible values: “POST”, “GET”, etc.", type_t::STRING, 1023}},
		{"destination ServiceName", {"destinationServiceName", "The service targeted by this event. Example: “sshd”", type_t::STRING, 1023}},
		{"agentNtDomain", {"agentNtDomain", "The DNS domain name of the ArcSight connector that processed the event.", type_t::STRING, 255}},
		{"device ExternalId", {"deviceExternalId", "A name that uniquely identifies the device generating this event.", type_t::STRING, 255}},
		{"flexDate1Label", {"flexDate1Label", "The label field is a string and describes the purpose of the flex field.", type_t::STRING, 128}},
		{"agentDns Domain", {"agentDnsDomain", "The DNS domain name of the ArcSight connector that processed the event.", type_t::STRING, 255}},
		{"deviceDns Domain", {"deviceDnsDomain", "The DNS domain part of the complete fully qualified domain name (FQDN).", type_t::STRING, 255}},
		{"sourceDns Domain", {"sourceDnsDomain", "The DNS domain part of the complete fully qualified domain name (FQDN).", type_t::STRING, 255}},
		{"deviceInbound Interface", {"deviceInboundInterface", "Interface on which the packet or data entered the device.", type_t::STRING, 128}},
		{"Device Outbound Interface", {"deviceOutboundInterface", "Interface on which the packet or data left the device.", type_t::STRING, 128}},
		{"dproc", {"destinationProcessName", "The name of the event’s destination process. Example: “telnetd” or “sshd”.", type_t::STRING, 1023}},
		{"agentTranslated Address", {"agentTranslatedAddress", "The IP address of the ArcSight connector that processed the event.", type_t::IP}},
		{"art", {"agentReceiptTime", "The time at which information about the event was received by the ArcSight connector.", type_t::TIMESTAMP}},
		{"sourceZoneURI", {"sourceZoneURI", "The URI for the Zone that the source asset has been assigned to in ArcSight.", type_t::STRING, 2048}},
		{"flexString1Label", {"flexString2Label", "The label field is a string and describes the purpose of the flex field.", type_t::STRING, 128}},
		{"deviceZoneURI", {"deviceZoneURI", "Thee URI for the Zone that the device asset has been assigned to in ArcSight.", type_t::STRING, 2048}},
		{"flex String2Label", {"flexString2Label", "The label field is a string and describes the purpose of the flex field.", type_t::STRING, 128}},
		{"destination DnsDomain", {"destinationDnsDomain", "The DNS domain part of the complete fully qualified domain name (FQDN).", type_t::STRING, 255}},
		{"proto", {"transportProtocol", "Identifies the Layer-4 protocol used. The possible values are protocols such as TCP or UDP.",  type_t::STRING, 31}},
		{"app", {"applicationProtocol", "Application level protocol, example: HTTP, HTTPS, SSHv2, Telnet, POP, IMPA, IMAPS, and so on.", type_t::STRING, 31}},
		{"externalId", {"externalId", "The ID used by an originating device. They are usually increasing numbers, associated with events.", type_t::STRING, 40}},
		{"requestContext", {"requestContext", "Description of the content from which the request originated (for example, HTTP Referrer)", type_t::STRING, 2048}},
		{"destinationZone URI", {"destinationZoneURI", "The URI for the Zone that the destination asset has been assigned to in ArcSight.", type_t::STRING, 2048}},
		{"cnt", {"baseEventCount", "A count associated with this event. How many times was this same event observed? Count can be omitted if i is 1.", type_t::INT32}},
		{"src", {"sourceAddress", "Identifies the source that an event refers to in an IP network. The format is an IPv4 address. Example: “192.168.10.1”.", type_t::IPV4}},
		{"cfp1", {"deviceCustomFloatingPoint1", "One of our floating point fields available to map fields that do not apply to any other in this dictionary.", type_t::FLOAT}},
		{"c6a1", {"deviceCustomIPv6Address1", "One of the four IPv6 address fields available to map fields that do not apply to any other in this dictionary.", type_t::IPV6}},
		{"c6a2", {"deviceCustomIPv6Address2", "One of the four IPv6 address fields available to map fields that do not apply to any other in this dictionary.", type_t::IPV6}},
		{"c6a3", {"deviceCustomIPv6Address3", "One of the four IPv6 address fields available to map fields that do not apply to any other in this dictionary.", type_t::IPV6}},
		{"c6a4", {"deviceCustomIPv6Address4", "One of the four IPv6 address fields available to map fields that do not apply to any other in this dictionary.", type_t::IPV6}},
		{"type", {"type", "0 means base event, 1 means aggregated, 2 means correlation, and 3 means action. This field can be omitted for base events (type 0).", type_t::INT32}},
		{"destination TranslatedPort", {"destinationTranslatedPort", "Port after i was translated; for example, a firewall. Valid port numbers are 0 to 65535.", type_t::INT32}},
		{"source TranslatedPort", {"sourceTranslatedPort", "A port number after being translated by, for example, a firewall. Valid port numbers are 0 to 65535.", type_t::INT32}},
		{"duid", {"destinationUserId", "Identifies the destination user by ID. For example, in UNIX, the root user is generally associated with user ID 0.", type_t::STRING, 1023}},
		{"cfp2", {"deviceCustomFloatingPoint2", "One of the four floating point fields available to map fields that do not apply to any other in this dictionary.", type_t::FLOAT}},
		{"cfp3", {"deviceCustomFloatingPoint3", "One of the four floating point fields available to map fields that do not apply to any other in this dictionary.", type_t::FLOAT}},
		{"cfp4", {"deviceCustomFloatingPoint4", "One of the four floating point fields available to map fields that do not apply to any other in this dictionary.", type_t::FLOAT}},
		{"dvc", {"deviceAddress", "Identifies the device address that an event refers to in an IP network. The format is an IPv4 address. Example: “192.168.10.1”.", type_t::IPV4}},
		{"device TranslatedZone URI", {"deviceTranslatedZoneURI", "The URI for the Translated Zone that the device asset has been assigned to in ArcSight.", type_t::STRING, 2048}},
		{"deviceFacility", {"deviceFacility", "The facility generating this event. For example, Syslog has an explicit facility associated with every event.", type_t::STRING, 1023}},
		{"source TranslatedZone URI", {"sourceTranslatedZoneURI", "The URI for the Translated Zone that the destination asset has been assigned to in ArcSight.", type_t::STRING, 2048}},
		{"msg", {"message", "An arbitrary message giving more details about the event. Multiline entries can be produced by using \\n as the new line separator.", type_t::STRING, 1023}},
		{"deviceProcess Name", {"deviceProcessName", "Process name associated with the event. An example might be the process generating the syslog entry in UNIX.", type_t::STRING, 1023}},
		{"dst", {"destinationAddress", "Identifies the destination address that the event refers to in an IP network. The format is an IPv4 address. Example: “192.168.10.1”", type_t::IPV4}},
		{"filePath", {"filePath", "Full path to the file, including file name itself. Example: C:\\Program Files\\WindowsNT\\Accessories\\wordpad.exe or /usr/bin/zip", type_t::STRING, 1023}},
		{"start", {"startTime", "The time when the activity the event referred to started. The format is MMM dd yyyy HH:mm:ss or milliseconds since epoch (Jan 1 st 1970)", type_t::TIMESTAMP}},
		{"in", {"bytesIn", "Number of bytes transferred inbound, relative to the source to destination relationship, meaning that data was flowing from source to destination.", type_t::INT32}},
		{"destination TranslatedZoneURI", {"destinationTranslatedZoneURI", "The URI for the Translated Zone that the destination asset has been assigned to in ArcSight.", type_t::STRING, 2048}},
		{"rt", {"deviceReceiptTime", "The time at which the event related to the activity was received. The format is MMM dd yyyy HH:mm:ss or milliseconds since epoch (Jan 1 st 1970)", type_t::TIMESTAMP}},
		{"reason", {"Reason", "The reason an audit event was generated. For example “badd password” or “unknown user”. This could also be an error or return code. Example: “0x1234”", type_t::STRING, 1023}},
		{"oldFilePath", {"oldFilePath", "Full path to the old file, including the file name itself. Examples: c:\\Program Files\\WindowsNT\\Accessories\\wordpad.exe or /usr/bin/zip", type_t::STRING, 1023}},
		{"dpid", {"destinationProcessId", "Provides the ID of the destination process associated with the event. For example, if an event contains process ID 105, “105” is the process ID.", type_t::INT32}},
		{"suser", {"sourceUserName", "Identifies the source user by name. Email addresses are also mapped into the UserName fields. The sender is a candidate to put into this field.", type_t::STRING, 1023}},
		{"cn1Label", {"deviceCustomNumber1Label", "All custom fields have a corresponding label field. Each of these fields is a string and describes the purpose of the custom field.", type_t::STRING, 1023}},
		{"cn2Label", {"deviceCustomNumber2Label", "All custom fields have a corresponding label field. Each of these fields is a string and describes the purpose of the custom field.", type_t::STRING, 1023}},
		{"cn3Label", {"deviceCustomNumber3Label", "All custom fields have a corresponding label field. Each of these fields is a string and describes the purpose of the custom field.", type_t::STRING, 1023}},
		{"cs1Label", {"deviceCustomString1Label", "All custom fields have a corresponding label field. Each of these fields is a string and describes the purpose of the custom field.", type_t::STRING, 1023}},
		{"cs2Label", {"deviceCustomString2Label", "All custom fields have a corresponding label field. Each of these fields is a string and describes the purpose of the custom field.", type_t::STRING, 1023}},
		{"cs3Label", {"deviceCustomString3Label", "All custom fields have a corresponding label field. Each of these fields is a string and describes the purpose of the custom field.", type_t::STRING, 1023}},
		{"cs4Label", {"deviceCustomString4Label", "All custom fields have a corresponding label field. Each of these fields is a string and describes the purpose of the custom field.", type_t::STRING, 1023}},
		{"cs5Label", {"deviceCustomString5Label", "All custom fields have a corresponding label field. Each of these fields is a string and describes the purpose of the custom field.", type_t::STRING, 1023}},
		{"cs6Label", {"deviceCustomString6Label", "All custom fields have a corresponding label field. Each of these fields is a string and describes the purpose of the custom field.", type_t::STRING, 1023}},
		{"request", {"requestUrl", "In the case of an HTTP request, this field contains the URL accessed. The URL should contain the protocol as well. Example: “http://www/secure.com”", type_t::STRING, 1023}},
		{"deviceDirection", {"deviceDirection", "Any information about what direction the observed communication has taken. The following values are supported: “0” for inbound or “1” for outbound", type_t::INT32}},
		{"c6a4Label", {"deviceCustomIPv6Address4Label", "All custom fields have a corresponding label field. Each of these fields is a string and describes the purpose of the custom field.", type_t::STRING, 1023}},
		{"c6a1Label", {"deviceCustomIPv6Address1Label", "All custom fields have a corresponding label field. Each of these fields is a string and describes the purpose of the custom field.", type_t::STRING, 1023}},
		{"c6a3Label", {"deviceCustomIPv6Address3Label", "All custom fields have a corresponding label field. Each of these fields is a string and describes the purpose of the custom field.", type_t::STRING, 1023}},
		{"source Translated Address", {"sourceTranslatedAddress", "Identifies the translated source that the event refers to in an IP network. The format is an IPv4 address. Example: “192.168.10.1”.", type_t::IPV4}},
		{"cfp1Label", {"deviceCustomFloatingPoint1Label", "All custom fields have a corresponding label field. Each of these fields is a string and describes the purpose of the custom field.", type_t::STRING, 1023}},
		{"cfp3Label", {"deviceCustomFloatingPoint3Label", "All custom fields have a corresponding label field. Each of these fields is a string and describes the purpose of the custom field.", type_t::STRING, 1023}},
		{"cfp4Label", {"deviceCustomFloatingPoint4Label", "All custom fields have a corresponding label field. Each of these fields is a string and describes the purpose of the custom field.", type_t::STRING, 1023}},
		{"cfp2Label", {"deviceCustomFloatingPoint2 Label", "All custom fields have a corresponding label field. Each of these fields is a string and describes the purpose of the custom field.", type_t::STRING, 1023}},
		{"out", {"bytesOut", "Number of bytes transferred outbound relative to the source to destination relationship. For example, the byte number of data flowing from the destination to the source.", type_t::INT32}},
		{"dvchost", {"deviceHostName", "The format should be a fully qualified domain name (FQDN) associated with the device node, when a node is available. Example: “host.domain.com” or “host”.", type_t::STRING, 100}},
		{"device Translated Address", {"deviceTranslatedAddress", "Identifies the translated destination that the event refers to in an IP network. The format is an IPv4 address. Example: “192.168.10.1”", type_t::IPV4}},
		{"deviceCustom Date1Label", {"deviceCustomDate1Label", "All custom fields have a corresponding label field. Each of these fields is a string and describes the purpose of the custom field.", type_t::STRING, 1023}},
		{"deviceCustom Date2Label", {"deviceCustomDate2Label", "All custom fields have a corresponding label field. Each of these fields is a string and describes the purpose of the custom field.", type_t::STRING, 1023}},
		{"cat", {"deviceEventCategory", "Represents the category assigned by the originating device. Devices often use their own categorization schema to classify event. Example: “/Monitor/Disk/Read”", type_t::STRING, 1023}},
		{"suid", {"sourceUserId", "Identifies the source user by ID. This is the user associated with the source of the event. For example, in UNIX, the root user is generally associated with user ID 0.", type_t::STRING, 1023}},
		{"end", {"endTime", "The time at which the activity related to the event ended. The format is MMM dd yyyy HH:mm:ss or milliseconds since epoch (Jan 1 st1970). An example would be reporting the end of a session.", type_t::TIMESTAMP}},
		{"cs5", {"deviceCustomString5", "One of six strings available to map fields that do not apply to any other in this dictionary. Use sparingly and seek a more specific, dictionary supplied field when possible.", type_t::STRING, 4000}},
		{"cs6", {"deviceCustomString6", "One of six strings available to map fields that do not apply to any other in this dictionary. Use sparingly and seek a more specific, dictionary supplied field when possible.", type_t::STRING, 4000}},
		{"cs1", {"deviceCustomString1", "One of the six strings available to map fields that do not apply to any other in this dictionary. Use sparingly and seek a more specific, dictionary supplied field when possible.", type_t::STRING, 4000}},
		{"cs2", {"deviceCustomString2", "One of the six strings available to map fields that do not apply to any other in this dictionary. Use sparingly and seek a more specific, dictionary supplied field when possible.", type_t::STRING, 4000}},
		{"cs3", {"deviceCustomString3", "One of the six strings available to map fields that do not apply to any other in this dictionary. Use sparingly and seek a more specific, dictionary supplied field when possible.", type_t::STRING, 4000}},
		{"cs4", {"deviceCustomString4", "One of the six strings available to map fields that do not apply to any other in this dictionary. Use sparingly and seek a more specific, dictionary supplied field when possible.", type_t::STRING, 4000}},
		{"cn1", {"deviceCustomNumber1", "One of the three number fields available to map fields that do not apply to any other in this dictionary. Use sparingly and seek a more specific, dictionary supplied field when possible.", type_t::LONG}},
		{"cn2", {"deviceCustomNumber2", "One of the three number fields available to map fields that do not apply to any other in this dictionary. Use sparingly and seek a more specific, dictionary supplied field when possible.", type_t::LONG}},
		{"cn3", {"deviceCustomNumber3", "One of the three number fields available to map fields that do not apply to any other in this dictionary. Use sparingly and seek a more specific, dictionary supplied field when possible.", type_t::LONG}},
		{"deviceCustom Date1", {"deviceCustomDate1", "One of two timestamp fields available to map fields that do not apply to any other in this dictionary. Use sparingly and seek a more specific, dictionary supplied field when possible.", type_t::TIMESTAMP}},
		{"deviceCustom Date2", {"deviceCustomDate2", "One of the two timestamp fields available to map fields that do not apply to any other in this dictionary. Use sparingly and seek a more specific, dictionary supplied field when possible.", type_t::TIMESTAMP}},
		{"spriv", {"sourceUserPrivileges", "The typical values are “Administrator”, “User”, and “Guest”. It identifies the source user’s privileges. In UNIX, for example, activity executed by the root user would be identified with “Administrator”.", type_t::STRING, 1023}},
		{"duser", {"destinationUserName", "Identifies the destination user by name. This is the user associated with the event’s destination. Email addresses are often mapped into the UserName fields. The recipient is a candidate to put into this field.", type_t::STRING, 1023}},
		{"shost", {"sourceHostName", "Identifies the source that an event refers to in an IP network. The format should be a fully qualified domain name (DQDN) associated with the source node, when a mode is available. Examples: “host” or “host.domain.com”.", type_t::STRING, 1023}},
		{"dhost", {"destinationHostName", "Identifies the destination that an event refers to in an IP network. The format must be a fully qualified domain name (FQDN) associated with the destination node, when a node is available. Examples: “host.domain.com” or “host”.", type_t::STRING, 1023}},
		{"dpriv", {"destinationUserPrivileges", "The typical values are “Administrator”, “User”, and “Guest”. This identifies the destination user’s privileges. In UNIX, for example, activity executed on the root user would be identified with destinationUser Privileges of “Administrator”.", type_t::STRING, 1023}},
		{"flexString1", {"flexString1", "One of four floating point fields available to map fields that do not apply to any other in this dictionary. Use sparingly and seek a more specific, dictionary supplied field when possible. These fields are typically reserved for customer use and should not be set by vendors unless necessary.", type_t::STRING, 1023}},
		{"flexString2", {"flexString2", "One of four floating point fields available to map fields that do not apply to any other in this dictionary. Use sparingly and seek a more specific, dictionary supplied field when possible. These fields are typically reserved for customer use and should not be set by vendors unless necessary.", type_t::STRING, 1023}},
		{"flexDate1", {"flexDate1", "A timestamp field available to map a timestamp that does not apply to any other defined timestamp field in this dictionary. Use all flex fields sparingly and seek a more specific, dictionary supplied field when possible. These fields are typically reserved for customer use and should not be set by vendors unless necessary.", type_t::TIMESTAMP}}
	};
	/**
	 * Формируем схему расширений для SEFv1
	 */
	this->_extensionSEFv1 = {
		{"customerKey", {"Customer Key", "ID of a customer resource reference.", type_t::INT64}},
		{"sZoneKey", {"Source Zone Key", "ID of a sourceZone resource reference.", type_t::INT64}},
		{"agentZoneKey", {"Agent Zone Key", "ID of an agentZone resource reference.", type_t::INT64}},
		{"deviceZoneKey", {"Device Zone Key", "ID of a deviceZone resource reference.", type_t::INT64}},
		{"threatActor", {"Threat actor", "Threat actor associated with the event.", type_t::STRING, 40}},
		{"dZoneKey", {"Destination Zone Key", "ID of a destinationZone resource reference.", type_t::INT64}},
		{"frameworkName", {"Framework Name", "The name of the framework used for threatAttackID.", type_t::STRING, 256}},
		{"sTranslatedZoneKey", {"Source Translated Zone Key", "ID of a sourceTranslatedZone resource reference.", type_t::INT64}},
		{"agentTranslatedZoneKey", {"Agent Translated Zone Key", "ID of an agentTranslatedZone resource reference.", type_t::INT64}},
		{"deviceTranslatedZoneKey", {"Device Translated Zone Key", "ID of a deviceTranslatedZone resource reference.", type_t::INT64}},
		{"dTranslatedZoneKey", {"Destination Translated Zone Key", "ID of a destinationTranslatedZ one resource reference.", type_t::INT64}},
		{"reportedDuration", {"Reported Duration", "Elapsed time in milliseconds of the action or entity the event represents.", type_t::INT64}},
		{"reportedResourceName", {"Reported Resource Name", "Name of the affected resource in the system that sent the event.", type_t::STRING, 64}},
		{"reportedResourceType", {"Reported Resource Type", "Type of the affected resource in the system that sent the event.", type_t::STRING, 64}},
		{"reportedResourceID", {"Reported Resource ID", "Name of a group containing the resource in the system that sent the event.", type_t::STRING, 256}},
		{"threatAttackID", {"Threat Attack ID", "A full ID of a threat or attack as defined in the security framework in frameworkName.", type_t::STRING, 32}},
		{"reportedResourceGroupName", {"Reported ResourceGroup Name", "Name of a group containing the resource in the system that sent the event.", type_t::STRING, 128}}
	};
	/**
	 * Формируем схему соответствия ключей расширений
	 */
	this->_mapping = {
		{"type", "type"},
		{"bytesIn", "in"},
		{"message", "msg"},
		{"agentId", "aid"},
		{"endTime", "end"},
		{"agentType", "at"},
		{"bytesOut", "out"},
		{"Reason", "reason"},
		{"fileId", "fileId"},
		{"fileSize", "fsize"},
		{"sourcePort", "spt"},
		{"filename", "fname"},
		{"eventId", "eventId"},
		{"agentVersion", "av"},
		{"startTime", "start"},
		{"agentAddress", "agt"},
		{"deviceAction", "act"},
		{"sourceUserId", "suid"},
		{"filePath", "filePath"},
		{"deviceAddress", "dvc"},
		{"sourceAddress", "src"},
		{"rawEvent", "rawEvent"},
		{"fileHash", "fileHash"},
		{"fileType", "fileType"},
		{"agentTimeZone", "atz"},
		{"requestUrl", "request"},
		{"baseEventCount", "cnt"},
		{"deviceTimeZone", "dtz"},
		{"flexDate1", "flexDate1"},
		{"agentHostName", "ahost"},
		{"oldFileId", "oldFileId"},
		{"destinationPort", "dpt"},
		{"sourceUserName", "suser"},
		{"agentMacAddress", "amac"},
		{"eventOutcome", "outcome"},
		{"sourceProcessId", "spid"},
		{"agentReceiptTime", "art"},
		{"deviceReceiptTime", "rt"},
		{"sourceHostName", "shost"},
		{"deviceMacAddress", "dmac"},
		{"sourceMacAddress", "smac"},
		{"sourceNtDomain", "sntdom"},
		{"externalId", "externalId"},
		{"destinationAddress", "dst"},
		{"destinationUserId", "duid"},
		{"deviceProcessId", "dvcpid"},
		{"sourceGeoLatitude", "slat"},
		{"deviceHostName", "dvchost"},
		{"deviceEventCategory", "cat"},
		{"oldFileType", "oldFileType"},
		{"sourceProcessName", "sproc"},
		{"oldFileHash", "oldFileHash"},
		{"oldFileName", "oldFileName"},
		{"oldFileSize", "oldFileSize"},
		{"customerURI", "customerURI"},
		{"transportProtocol", "proto"},
		{"applicationProtocol", "app"},
		{"deviceCustomString5", "cs5"},
		{"deviceCustomString6", "cs6"},
		{"deviceCustomString1", "cs1"},
		{"deviceCustomString2", "cs2"},
		{"deviceCustomString3", "cs3"},
		{"deviceCustomString4", "cs4"},
		{"deviceCustomNumber1", "cn1"},
		{"deviceCustomNumber2", "cn2"},
		{"deviceCustomNumber3", "cn3"},
		{"oldFilePath", "oldFilePath"},
		{"flexString1", "flexString1"},
		{"flexString2", "flexString2"},
		{"sourceGeoLongitude", "slong"},
		{"Customer Key", "customerKey"},
		{"Source Zone Key", "sZoneKey"},
		{"Threat actor", "threatActor"},
		{"destinationProcessId", "dpid"},
		{"agentZoneURI", "agentZoneURI"},
		{"destinationUserName", "duser"},
		{"destinationHostName", "dhost"},
		{"destinationNtDomain", "dntdom"},
		{"sourceUserPrivileges", "spriv"},
		{"sourceZoneURI", "sourceZoneURI"},
		{"destinationGeoLatitude", "dlat"},
		{"Agent Zone Key", "agentZoneKey"},
		{"requestMethod", "requestMethod"},
		{"agentNtDomain", "agentNtDomain"},
		{"deviceZoneURI", "deviceZoneURI"},
		{"Framework Name", "frameworkName"},
		{"destinationProcessName", "dproc"},
		{"deviceFacility", "deviceFacility"},
		{"requestContext", "requestContext"},
		{"requestCookies", "requestCookies"},
		{"fileCreateTime", "fileCreateTime"},
		{"filePermission", "filePermission"},
		{"Device Zone Key", "deviceZoneKey"},
		{"Destination Zone Key", "dZoneKey"},
		{"destinationGeoLongitude", "dlong"},
		{"flexDate1Label", "flexDate1Label"},
		{"deviceCustomIPv6Address1", "c6a1"},
		{"deviceCustomIPv6Address2", "c6a2"},
		{"deviceCustomIPv6Address3", "c6a3"},
		{"deviceCustomIPv6Address4", "c6a4"},
		{"agentDnsDomain", "agentDns Domain"},
		{"deviceNtDomain", "deviceNt Domain"},
		{"destinationUserPrivileges", "dpriv"},
		{"deviceDirection", "deviceDirection"},
		{"deviceCustomFloatingPoint2", "cfp2"},
		{"deviceCustomFloatingPoint3", "cfp3"},
		{"deviceCustomFloatingPoint4", "cfp4"},
		{"deviceCustomFloatingPoint1", "cfp1"},
		{"Threat Attack ID", "threatAttackID"},
		{"deviceDnsDomain", "deviceDns Domain"},
		{"sourceDnsDomain", "sourceDns Domain"},
		{"deviceeventId", "Device eventId"},
		{"deviceCustomNumber1Label", "cn1Label"},
		{"deviceCustomNumber2Label", "cn2Label"},
		{"deviceCustomNumber3Label", "cn3Label"},
		{"deviceCustomString1Label", "cs1Label"},
		{"deviceCustomString2Label", "cs2Label"},
		{"deviceCustomString3Label", "cs3Label"},
		{"deviceCustomString4Label", "cs4Label"},
		{"deviceCustomString5Label", "cs5Label"},
		{"deviceCustomString6Label", "cs6Label"},
		{"flexString2Label", "flexString1Label"},
		{"deviceExternalId", "device ExternalId"},
		{"Reported Duration", "reportedDuration"},
		{"flexString2Label", "flex String2Label"},
		{"deviceProcessName", "deviceProcess Name"},
		{"sourceServiceName", "source ServiceName"},
		{"oldFileCreateTime", "oldFileCreate Time"},
		{"oldFilePermission", "oldFile Permission"},
		{"deviceCustomDate1", "deviceCustom Date1"},
		{"deviceCustomDate2", "deviceCustom Date2"},
		{"destinationZoneURI", "destinationZone URI"},
		{"customerExternalID", "customer ExternalID"},
		{"deviceCustomIPv6Address4Label", "c6a4Label"},
		{"deviceCustomIPv6Address1Label", "c6a1Label"},
		{"deviceCustomIPv6Address3Label", "c6a3Label"},
		{"Reported Resource ID", "reportedResourceID"},
		{"agentZoneExternalID", "agentZone ExternalID"},
		{"deviceCustomFloatingPoint1Label", "cfp1Label"},
		{"deviceCustomFloatingPoint3Label", "cfp3Label"},
		{"deviceCustomFloatingPoint4Label", "cfp4Label"},
		{"fileModificationTime", "fileModification Time"},
		{"sourceZoneExternalID", "sourceZone ExternalID"},
		{"deviceZoneExternalID", "deviceZone ExternalID"},
		{"destinationDnsDomain", "destination DnsDomain"},
		{"sourceTranslatedPort", "source TranslatedPort"},
		{"deviceCustomFloatingPoint2 Label", "cfp2Label"},
		{"Reported Resource Type", "reportedResourceType"},
		{"Reported Resource Name", "reportedResourceName"},
		{"Source Translated Zone Key", "sTranslatedZoneKey"},
		{"deviceCustomDate1Label", "deviceCustom Date1Label"},
		{"deviceCustomDate2Label", "deviceCustom Date2Label"},
		{"deviceInboundInterface", "deviceInbound Interface"},
		{"destinationServiceName", "destination ServiceName"},
		{"agentTranslatedAddress", "agentTranslated Address"},
		{"agentTranslatedZoneURI", "agentTranslated Zone URI"},
		{"Agent Translated Zone Key", "agentTranslatedZoneKey"},
		{"deviceTranslatedAddress", "device Translated Address"},
		{"sourceTranslatedAddress", "source Translated Address"},
		{"deviceTranslatedZoneURI", "device TranslatedZone URI"},
		{"sourceTranslatedZoneURI", "source TranslatedZone URI"},
		{"deviceOutboundInterface", "Device Outbound Interface"},
		{"oldFileModificationTime", "oldFile Modification Time"},
		{"Device Translated Zone Key", "deviceTranslatedZoneKey"},
		{"Destination Translated Zone Key", "dTranslatedZoneKey"},
		{"requestClientApplication", "requestClient Application"},
		{"destinationTranslatedPort", "destination TranslatedPort"},
		{"destinationZoneExternalID", "destinationZone ExternalID"},
		{"Reported ResourceGroup Name", "reportedResourceGroupName"},
		{"destinationTranslatedZoneURI", "destination TranslatedZoneURI"},
		{"agentTranslatedZoneExternalID", "agentTranslated ZoneExternalID"},
		{"deviceTranslatedZoneExternalID", "device TranslatedZoneExternalID"},
		{"sourceTranslatedZoneExternalID", "source TranslatedZoneExternalID"},
		{"destinationTranslatedZoneExternalID", "destination TranslatedZoneExternalID"}
	};
	// Выполняем сборку регулярных выражений для извлечения параметров расширений
	this->_exp = this->_reg.build(R"(([\w\-]+)=(?:\"([^\"]+)\"|([^=]*|(?:[^=]+\\\=[^=]+)+))(?:\s+[\w\-]+=|\s*$))", {
		regexp_t::option_t::UTF8,
		regexp_t::option_t::UCP
	});
};
/**
 * Оператор [>>] чтения из потока CEF контейнера
 * @param is  поток для чтения
 * @param cef контенер для присвоения
 */
istream & anyks::operator >> (istream & is, cef_t & cef) noexcept {
	// Данные CEF контейнера
	string data = "";
	// Считываем данные CEF контейнера
	is >> data;
	// Если данные CEF контейнера получены
	if(!data.empty())
		// Устанавливаем данные CEF контейнера
		cef.parse(data);
	// Выводим результат
	return is;
}
/**
 * Оператор [<<] вывода в поток CEF контейнера
 * @param os  поток куда нужно вывести данные
 * @param cef контенер для присвоения
 */
ostream & anyks::operator << (ostream & os, const cef_t & cef) noexcept {
	// Записываем в поток CEF сообщение
	os << cef.cef();
	// Выводим результат
	return os;
}
