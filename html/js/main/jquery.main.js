(function($){
	// Активируем событие загрузки
	$(document).ready(function(){
		// Блок подсветки синтаксиса шаблонов GROK
		let grokEditor = null;
		// Блок подсветки синтаксиса формата в который производится хеширвоание
		let toEditorHashing = null;
		// Блок подсветки синтаксиса формата из которого производится хеширвоание
		let fromEditorHashing = null;
		// Выполняем инициализацию блока подсветки синтаксиса для редактора формата из которого производится конвертация
		let fromEditorContainers = CodeMirror(document.getElementById('code-editor-from-containers'), {
			tabSize: 4,
			mode: 'xml',
			theme: 'eclipse',
			lineNumbers: true
		});
		// Выполняем инициализацию блока подсветки синтаксиса для редактора формата в который производится конвертация
		const toEditorContainers = CodeMirror(document.getElementById('code-editor-to-containers'), {
			tabSize: 4,
			theme: 'eclipse',
			mode: 'javascript',
			readOnly: true,
			lineNumbers: true
		});
		/**
		 * alert Функция вывода всплывающего сообщения
		 * @param {String} caption заголовок сообщения
		 * @param {String} data    текст сообщения
		 */
		const alert = (caption, data) => {
			// Если размер окнша выше 767 пикселей
			if($(window).width() >= 768){
				// Шаблон всплывающего сообщения
				const template = `<div class="modal fade" id="alert" tabindex="-1" aria-labelledby="alertCaption" aria-hidden="true"><div class="modal-dialog"><div class="modal-content"><div class="modal-header"><h1 class="modal-title fs-5" id="alertCaption"></h1><button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button></div><div class="modal-body"></div><div class="modal-footer"><button type="button" class="btn btn-primary" data-bs-dismiss="modal" aria-label="Close">Ok</button></div></div></div></div>`;
				// Выполняем добавление шаблона всплывающего сообщения
				$("body").append(template);
				// Устанавливаем событие на скрытие всплывающего сообщения
				$("#alert")
				.unbind("hidden.bs.modal")
				.bind("hidden.bs.modal", function() {
					// Выполняем удаление всплывающего сообщения
					$(this).remove();
					// Выходим из функции
					return false;
				});
				// Выполняем установку текста заголовка
				$("#alertCaption").text(caption);
				// Выполняем установка текста сообщения
				$(".modal-body", "#alert").html(data);
				// Отображаем всплывающее сообщение
				$("#alert").modal('show');
			// Выводим нативное всплывающее сообщение
			} else window.alert(data);
		};
		// Активируем событие вкладок
		$(".card-header-tabs a.nav-link", "#formatter")
		.unbind('click')
		.bind('click', function(){
			// Если вкладка не является активной
			if(!$(this).hasClass("active")){
				// Выполняем получение активной вкладки
				const active = $("a.active", $(this).parent().parent());
				// Выполняем удаление активности ссылки
				active.removeClass("active");
				// Скрываем блок с данными активной вкладки
				$(active.attr("href")).addClass("hidden");
				// Выполняем установку активной вкладки
				$(this).addClass("active");
				// Отображаем нужный нам блок данных
				$($(this).attr("href")).removeClass("hidden");
				// Если блоки подсветки синтаксиса не созданы
				if((fromEditorHashing === null) || (toEditorHashing === null)){
					// Выполняем инициализацию блока подсветки синтаксиса для редактора формата из которого производится конвертация
					fromEditorHashing = CodeMirror(document.getElementById('code-editor-from-hashing'), {
						tabSize: 4,
						mode: 'shell',
						theme: 'eclipse',
						lineNumbers: false,
						lineWrapping: true
					});
					// Выполняем инициализацию блока подсветки синтаксиса для редактора формата в который производится конвертация
					toEditorHashing = CodeMirror(document.getElementById('code-editor-to-hashing'), {
						tabSize: 4,
						mode: 'shell',
						theme: 'eclipse',
						readOnly: true,
						lineNumbers: false,
						lineWrapping: true
					});
					// Устанавливаем событие на изменение поля из которого следует выполнять хеширования
					$("> .CodeMirror", "#code-editor-from-hashing")
					.unbind("keyup")
					.bind("keyup", function(){
						// Получаем идентификатор кнопки
						const button = $(".btn-close", $(this).parent().parent());
						// Если поле содержит данные
						if(fromEditorHashing.getValue().length > 0)
							// Выполняем отображение кнопки очистки поля
							button.removeClass("hidden");
						// Выполняем скрытие кнопки очистки
						else button.addClass("hidden");
						// Устанавливаем событие на клик по кнопке
						button
						.unbind('click')
						.bind('click', function(){
							// Выполняем очистку текстового поля
							fromEditorHashing.setValue("");
							// Выполняем скрытие кнопки
							$(this).addClass("hidden");
							// Запрещаем дальнейшее событие для кнопки
							return false;
						});
						// Запрещаем выполнение дальнейшего события
						return false;
					});
					// Устанавливаем событие на кнопку отправки запроса
					$("#submit-hashing")
					.unbind('click')
					.bind('click', function(){
						// Включаем индикатор загрузки
						$("#spinner").removeClass("hidden");
						/**
						 * Формируем функцию выполнения запроса на удалённый сервер
						 */
						(async () => {
							/**
							 * Выполняем перехват ошибки
							 */
							try {
								// Получаем значение текста который необходимо сконвертировать
								const text = fromEditorHashing.getValue();
								// Если текст для конвертации указан
								if(text.length > 0){
									// Название конвертера из которого и в который производится конвертация
									let from = "", to = "";
									// Определяем формат в который производится конвертация
									switch($("a.active", "#to-hashing").attr("href")){
										// Если если формат в который производится конвертация установлен как TEXT
										case "#TEXT": to = "text"; break;
										// Если если формат в который производится конвертация установлен как BASE64
										case "#BASE64": to = "base64"; break;
										// Если если формат в который производится конвертация установлен как MD5
										case "#MD5": to = "md5"; break;
										// Если если формат в который производится конвертация установлен как SHA1
										case "#SHA1": to = "sha1"; break;
										// Если если формат в который производится конвертация установлен как SHA224
										case "#SHA224": to = "sha224"; break;
										// Если если формат в который производится конвертация установлен как SHA256
										case "#SHA256": to = "sha256"; break;
										// Если если формат в который производится конвертация установлен как SHA384
										case "#SHA384": to = "sha384"; break;
										// Если если формат в который производится конвертация установлен как SHA512
										case "#SHA512": to = "sha512"; break;
									}
									// Определяем формат с которого производится конвертация
									switch($("a.active", "#from-hashing").attr("href")){
										// Если если формат в который производится конвертация установлен как TEXT
										case "#TEXT": from = "text"; break;
										// Если если формат в который производится конвертация установлен как BASE64
										case "#BASE64": from = "base64"; break;
									}
									// Получаем данные текстового поля HMAC
									const hmac = (((to !== "text") && (to !== "base64")) ? $("#hmac-key").val() : undefined);
									// Формируем объект тело запроса
									const body = {to, from, text, hmac};
									// Выполняем запрос на сервер
									const response = await fetch("/exec", {
										method: "POST",
										body: JSON.stringify(body),
										headers: {"Content-type": "application/json; charset=UTF-8"}
									});
									// Извлекаем полученный результат
									const answer = await response.json();
									// Если в ответе есть поле результата
									if((answer.result !== null) && (answer.result !== undefined))
										// Выполняем установку результата
										toEditorHashing.setValue(answer.result);
									// Выводим сообщение об ошибке
									else {
										// Если получен текст ошибки
										if((answer.error !== null) && (answer.error !== undefined))
											// Отображаем всплывающее сообщение
											alert("Ошибка запроса", answer.error);
										// Если текст ошибки не получен
										else alert("Ответ не получен", "Ответ от сервера не содержит результата");
									}
									
								// Если текст для хеширования не заполнен
								} else alert("Ошибка хеширования", "Текст для хеширования не заполнен");
							/**
							 * Если ошибка перехвачена
							 */
							} catch(error) {
								// Отображаем всплывающее сообщение
								alert("Ошибка запроса", error);
							}
						})();
						// Выключаем индикатор загрузки
						$("#spinner").addClass("hidden");
						// Запрещаем дальнейшие действия для ссылки
						return false;
					});
					// Устанавливаем событие на перехват событий выбора формата из чего нужно хешировать
					$("#from-hashing > li > a")
					.unbind('click')
					.bind('click', function(){
						// Если флаг активации ещё не установлен
						if(!$(this).hasClass("active")){
							// Снимаем флаг активации у уже активного элемента
							$("li > a.active", $(this).parent().parent())
							.removeClass("active")
							.removeAttr("aria-current");
							// Устанавливаем флаг активации
							$(this)
							.addClass("active")
							.attr("aria-current", "page");
							// Удаляем текст в текстовом окне
							fromEditorHashing.setValue("");
							// Выполняем скрытие кнопки очитски
							$(".btn-close", $("#code-editor-from-hashing").parent()).addClass("hidden");
							// Определяем тип конвертера
							switch($(this).text()){
								// Если ковертер выбран BASE64
								case "BASE64": {
									// Выполняем включение переноса строк
									fromEditorHashing.setOption("lineWrapping", true);
									// Переключаем на вкладку по умолчанию
									$("#to-hashing > li > a[href=#TEXT]").click();
									// Блокируем вкладку конвертера
									$("#to-hashing > li > a[href=#BASE64]").addClass("disabled");
								} break;
								// Если активирован любой другой конвертер
								default: {
									// Выполняем отключение переноса строк
									fromEditorHashing.setOption("lineWrapping", false);
									// Разблокируем вкладку конвертера
									$("#to-hashing > li > a[href=#BASE64]").removeClass("disabled");
								}
							}
						}			
						// Запрещаем дальнейшие действия для ссылки
						return false;
					});
					// Устанавливаем событие на перехват событий выбора формата в который нужно хешировать
					$("#to-hashing > li > a")
					.unbind('click')
					.bind('click', function(){
						// Снимаем флаг активации у уже активного элемента
						$("li > a.active", $(this).parent().parent())
						.removeClass("active")
						.removeAttr("aria-current");
						// Устанавливаем флаг активации
						$(this)
						.addClass("active")
						.attr("aria-current", "page");
						// Если текст для конвертации присутствует
						if(fromEditorHashing.getValue().length > 0)
							// Выполняем загрузку новых данных
							$("#submit-hashing").click();
						// Удаляем текст в текстовом окне
						else toEditorHashing.setValue("");
						// Определяем тип конвертера
						switch($(this).text()){
							// Если ковертер выбран MD5
							case "MD5":
							// Если ковертер выбран SHA1
							case "SHA1":
							// Если ковертер выбран SHA224
							case "SHA224":
							// Если ковертер выбран SHA256
							case "SHA256":
							// Если ковертер выбран SHA384
							case "SHA384":
							// Если ковертер выбран SHA512
							case "SHA512": {
								// Отображаем текстовое поле HMAC ключа
								$("#hmac-key").parent().removeClass("hidden");
								// Выполняем включение переноса строк
								toEditorHashing.setOption("lineWrapping", true);
							} break;
							// Если ковертер выбран BASE64
							case "BASE64": {
								// Скрываем текстовое поле HMAC ключа
								$("#hmac-key").parent().addClass("hidden");
								// Выполняем включение переноса строк
								toEditorHashing.setOption("lineWrapping", true);
							} break;
							// Если ковертер выбран любой другой
							default: {
								// Скрываем текстовое поле HMAC ключа
								$("#hmac-key").parent().addClass("hidden");
								// Выполняем отключение переноса строк
								toEditorHashing.setOption("lineWrapping", false);
							}
						}
						// Запрещаем дальнейшие действия для ссылки
						return false;
					});
					// Устанавливаем событие на текстовое поле ввода HMAC ключа
					$("#hmac-key")
					.unbind("keyup")
					.bind("keyup", function(){
						// Получаем объект кнопки очистки текстового поля
						const button = $(".btn-close", $(this).parent());
						// Если текст введён в текстовое поле
						if($(this).val().length > 0)
							// Выполняем отображение кнопки очистки текстового поля
							button.removeClass("hidden");
						// Скрываем кнопку очистки текстового поля
						else button.addClass("hidden");
						// Запрещаем дальнейшее событие для кнопки
						return false;
					})
					// Устанавливаем событие на клик по кнопке
					.parent()
					.find(".btn-close")
					.unbind('click')
					.bind('click', function(){
						// Скрываем кнопку очистки текстового поля
						$(this).addClass("hidden");
						// Выполняем очистку текстового поля
						$("#hmac-key").val("");
						// Запрещаем дальнейшее событие для кнопки
						return false;
					});
				}
			}
			// Запрещаем дальнейшие действия для ссылки
			return false;
		});
		// Устанавливаем событие на изменение поля из которого следует выполнять конвертацию
		$("> .CodeMirror", "#code-editor-from-containers")
		.unbind("keyup")
		.bind("keyup", function(){
			// Получаем идентификатор кнопки
			const button = $(".btn-close", $(this).parent().parent());
			// Если поле содержит данные
			if(fromEditorContainers.getValue().length > 0)
				// Выполняем отображение кнопки очистки поля
				button.removeClass("hidden");
			// Выполняем скрытие кнопки очистки
			else button.addClass("hidden");
			// Устанавливаем событие на клик по кнопке
			button
			.unbind('click')
			.bind('click', function(){
				// Выполняем очистку текстового поля
				fromEditorContainers.setValue("");
				// Выполняем скрытие кнопки
				$(this).addClass("hidden");
				// Запрещаем дальнейшее событие для кнопки
				return false;
			});
			// Запрещаем выполнение дальнейшего события
			return false;
		});
		// Устанавливаем событие на кнопку отправки запроса
		$("#submit-containers")
		.unbind('click')
		.bind('click', function(){
			// Включаем индикатор загрузки
			$("#spinner").removeClass("hidden");
			/**
			 * Формируем функцию выполнения запроса на удалённый сервер
			 */
			(async () => {
				/**
				 * Выполняем перехват ошибки
				 */
				try {
					// Получаем значение текста который необходимо сконвертировать
					const text = fromEditorContainers.getValue();
					// Если текст для конвертации указан
					if(text.length > 0){
						// Название конвертера из которого и в который производится конвертация
						let from = "", to = "";
						// Определяем формат в который производится конвертация
						switch($("a.active", "#to-containers").attr("href")){
							// Если если формат в который производится конвертация установлен как INI
							case "#INI": to = "ini"; break;
							// Если если формат в который производится конвертация установлен как XML
							case "#XML": to = "xml"; break;
							// Если если формат в который производится конвертация установлен как CSV
							case "#CSV": to = "csv"; break;
							// Если если формат в который производится конвертация установлен как CEF
							case "#CEF": to = "cef"; break;
							// Если если формат в который производится конвертация установлен как JSON
							case "#JSON": to = "json"; break;
							// Если если формат в который производится конвертация установлен как YAML
							case "#YAML": to = "yaml"; break;
							// Если если формат в который производится конвертация установлен как SYSLOG
							case "#SYSLOG": to = "syslog"; break;
						}
						// Определяем формат с которого производится конвертация
						switch($("a.active", "#from-containers").attr("href")){
							// Если если формат из которого производится конвертация установлен как INI
							case "#INI": from = "ini"; break;
							// Если если формат из которого производится конвертация установлен как XML
							case "#XML": from = "xml"; break;
							// Если если формат из которого производится конвертация установлен как CSV
							case "#CSV": from = "csv"; break;
							// Если если формат из которого производится конвертация установлен как CEF
							case "#CEF": from = "cef"; break;
							// Если если формат из которого производится конвертация установлен как JSON
							case "#JSON": from = "json"; break;
							// Если если формат из которого производится конвертация установлен как YAML
							case "#YAML": from = "yaml"; break;
							// Если если формат из которого производится конвертация установлен как GROK
							case "#GROK": from = "grok"; break;
							// Если если формат из которого производится конвертация установлен как SYSLOG
							case "#SYSLOG": from = "syslog"; break;
						}
						// Получаем данные текстового поля регулярного выражения в формате GROK
						const express = ((from === "grok") ? $("#grok-expression").val() : undefined);
						// Если выбран формат GROK но регулярное выражение не запущено
						if((from === "grok") && (express.length === 0))
							// Отображаем всплывающее сообщение
							alert("Ошибка конвертации", "Регулярное выражение <strong>в формате GROK</strong> не заполнено");
						// Если всё хорошо то продолжаем дальше
						else {
							// Получаем данные шаблонов
							let patterns = ((from === "grok") && (grokEditor !== null) ? grokEditor.getValue() : undefined);
							// Если выбран формат GROK и указаны шаблоны
							if((from === "grok") && (patterns.length > 0))
								// Выполняем парсинг блока шаблонов
								patterns = JSON.parse(patterns);
							// Если шаблон пустой
							else if((from === "grok") && (patterns.length === 0))
								// Выполняем удаление шаблонов
								patterns = undefined;
							// Формируем объект тело запроса
							const body = {
								to, from, text, express, patterns,
								prettify: $("#prettify").is(":checked"),
								header: (((from === "csv") || (to === "csv")) ? $("#csv-header").is(":checked") : undefined)
							};
							// Выполняем запрос на сервер
							const response = await fetch("/exec", {
								method: "POST",
								body: JSON.stringify(body),
								headers: {"Content-type": "application/json; charset=UTF-8"}
							});
							// Извлекаем полученный результат
							const answer = await response.json();
							// Если в ответе есть поле результата
							if((answer.result !== null) && (answer.result !== undefined))
								// Выполняем установку результата
								toEditorContainers.setValue(answer.result);
							// Выводим сообщение об ошибке
							else {
								// Если получен текст ошибки
								if((answer.error !== null) && (answer.error !== undefined))
									// Отображаем всплывающее сообщение
									alert("Ошибка запроса", answer.error);
								// Если текст ошибки не получен
								else alert("Ответ не получен", "Ответ от сервера не содержит результата");
							}
						}
					// Если текст для конвертации не заполнен
					} else alert("Ошибка конвертации", "Текст для конвертации не заполнен");
				/**
				 * Если ошибка перехвачена
				 */
				} catch(error) {
					// Отображаем всплывающее сообщение
					alert("Ошибка запроса", error);
				}
			})();
			// Выключаем индикатор загрузки
			$("#spinner").addClass("hidden");
			// Запрещаем дальнейшие действия для ссылки
			return false;
		});
		// Устанавливаем событие на перехват событий выбора формата из чего нужно конвертировать
		$("#from-containers > li > a")
		.unbind('click')
		.bind('click', function(){
			// Если флаг активации ещё не установлен
			if(!$(this).hasClass("active")){
				// Снимаем флаг активации у уже активного элемента
				$("li > a.active", $(this).parent().parent())
				.removeClass("active")
				.removeAttr("aria-current");
				// Устанавливаем флаг активации
				$(this)
				.addClass("active")
				.attr("aria-current", "page");
				// Удаляем текст в текстовом окне
				fromEditorContainers.setValue("");
				// Выполняем скрытие кнопки очитски
				$(".btn-close", $("#code-editor-from-containers").parent()).addClass("hidden");
				// Получаем название парсера
				const name = $(this).text();
				// Если мы работаем с парсером GROK
				if(name.localeCompare("GROK") == 0){
					// Отображаем текстовое поле формирования шаблонов GROK
					$("#grok-template").removeClass("hidden");
					// Отображаем текстовое поле регулярного выражения
					$("#grok-expression").parent().removeClass("hidden");
					// Формируем блок подсветки синтаксиса
					grokEditor = CodeMirror(document.getElementById('code-editor-grok'), {
						tabSize: 4,
						theme: 'eclipse',
						mode: 'javascript',
						lineNumbers: false
					});
					// Активируем подсветку синтаксиса
					fromEditorContainers.setOption("mode", "shell");
					// Активируем вывод номеров строк
					fromEditorContainers.setOption("lineNumbers", false);
					// Выполняем подключение переноса строк
					fromEditorContainers.setOption("lineWrapping", true);
					// Если заголовок CSV не активен
					if($("#to-containers > li > a[href=#CSV]").hasClass("active"))
						// Отображаем переключатель вывода заголовков
						$("#csv-header").parent().removeClass("hidden");
					// Скрываем переключатель формирования заголовков
					else $("#csv-header").parent().addClass("hidden");
					// Блокируем вкладку конвертера
					$("#to-containers > li > a[href=#INI]").addClass("disabled");
					$("#to-containers > li > a[href=#CSV]").addClass("disabled");
					$("#to-containers > li > a[href=#CEF]").addClass("disabled");
					$("#to-containers > li > a[href=#SYSLOG]").addClass("disabled");
					// Получаем активный элемент
					switch($("#to-containers > li > a.active").text()){
						// Если ковертер выбран INI
						case "INI":
						// Если ковертер выбран CSV
						case "CSV":
						// Если ковертер выбран CEF
						case "CEF":
						// Если ковертер выбран SYSLOG
						case "SYSLOG":
							// Переключаем на вкладку по умолчанию
							$("#to-containers > li > a[href=#JSON]").click();
						break;
					}
					// Устанавливаем событие на текстовое поле ввода регулярного адреса
					$("#grok-expression")
					.unbind("keyup")
					.bind("keyup", function(){
						// Получаем объект кнопки очистки текстового поля
						const button = $(".btn-close", $(this).parent());
						// Если текст введён в текстовое поле
						if($(this).val().length > 0)
							// Выполняем отображение кнопки очистки текстового поля
							button.removeClass("hidden");
						// Скрываем кнопку очистки текстового поля
						else button.addClass("hidden");
						// Запрещаем дальнейшее событие для кнопки
						return false;
					})
					// Устанавливаем событие на клик по кнопке
					.parent()
					.find(".btn-close")
					.unbind('click')
					.bind('click', function(){
						// Скрываем кнопку очистки текстового поля
						$(this).addClass("hidden");
						// Выполняем очистку текстового поля
						$("#grok-expression").val("");
						// Запрещаем дальнейшее событие для кнопки
						return false;
					})
					// Устанавливаем событие на изменение поля для ввода шаблонов GROK
					$("> .CodeMirror", "#code-editor-grok")
					.unbind("keyup")
					.bind("keyup", function(){
						// Получаем идентификатор кнопки
						const button = $(".btn-close", $(this).parent().parent());
						// Если поле содержит данные
						if((grokEditor !== null) && (grokEditor.getValue().length > 0))
							// Выполняем отображение кнопки очистки поля
							button.removeClass("hidden");
						// Выполняем скрытие кнопки очистки
						else button.addClass("hidden");
						// Устанавливаем событие на клик по кнопке
						button
						.unbind('click')
						.bind('click', function(){
							// Выполняем очистку текстового поля
							grokEditor.setValue("");
							// Выполняем скрытие кнопки
							$(this).addClass("hidden");
							// Запрещаем дальнейшее событие для кнопки
							return false;
						});
						// Запрещаем выполнение дальнейшего события
						return false;
					});
				// Если мы работаем с другими парсерами
				} else {
					// Если блок ввода шаблонов GROK не скрыт
					if(!$("#grok-template").hasClass("hidden")){
						// Скрываем текстовое поле формирования шаблонов GROK
						$("#grok-template").addClass("hidden");
						// Скрываем текстовое поле регулярного выражения
						$("#grok-expression").parent().addClass("hidden");
						// Выполняем удаление блока подсветки синтаксиса
						grokEditor.getWrapperElement().parentNode.removeChild(grokEditor.getWrapperElement());
						// Зануляем объект блока подсветки синтаксиса
						grokEditor = null;
					}
					// Определяем тип конвертера
					switch(name){
						// Если ковертер выбран XML
						case "XML": {
							// Активируем подсветку синтаксиса
							fromEditorContainers.setOption("mode", "xml");
							// Активируем вывод номеров строк
							fromEditorContainers.setOption("lineNumbers", true);
							// Выполняем отключение переноса строк
							fromEditorContainers.setOption("lineWrapping", false);
							// Если заголовок CSV не активен
							if($("#to-containers > li > a[href=#CSV]").hasClass("active"))
								// Отображаем переключатель вывода заголовков
								$("#csv-header").parent().removeClass("hidden");
							// Скрываем переключатель формирования заголовков
							else $("#csv-header").parent().addClass("hidden");
							// Блокируем вкладку конвертера
							$("#to-containers > li > a[href=#CSV]").addClass("disabled");
							$("#to-containers > li > a[href=#CEF]").addClass("disabled");
							$("#to-containers > li > a[href=#SYSLOG]").addClass("disabled");
							// Снимаем блокировку вкладки конвертера
							$("#to-containers > li > a[href=#INI]").removeClass("disabled");
						} break;
						// Если ковертер выбран JSON
						case "JSON": {
							// Активируем подсветку синтаксиса
							fromEditorContainers.setOption("mode", "javascript");
							// Активируем вывод номеров строк
							fromEditorContainers.setOption("lineNumbers", true);
							// Выполняем отключение переноса строк
							fromEditorContainers.setOption("lineWrapping", false);
							// Если заголовок CSV не активен
							if($("#to-containers > li > a[href=#CSV]").hasClass("active"))
								// Отображаем переключатель вывода заголовков
								$("#csv-header").parent().removeClass("hidden");
							// Скрываем переключатель формирования заголовков
							else $("#csv-header").parent().addClass("hidden");
							// Снимаем блокировку вкладки конвертера
							$("#to-containers > li > a[href=#INI]").removeClass("disabled");
							$("#to-containers > li > a[href=#CSV]").removeClass("disabled");
							$("#to-containers > li > a[href=#CEF]").removeClass("disabled");
							$("#to-containers > li > a[href=#SYSLOG]").removeClass("disabled");
						} break;
						// Если ковертер выбран YAML
						case "YAML": {
							// Активируем подсветку синтаксиса
							fromEditorContainers.setOption("mode", "yaml");
							// Активируем вывод номеров строк
							fromEditorContainers.setOption("lineNumbers", true);
							// Выполняем отключение переноса строк
							fromEditorContainers.setOption("lineWrapping", false);
							// Если заголовок CSV не активен
							if($("#to-containers > li > a[href=#CSV]").hasClass("active"))
								// Отображаем переключатель вывода заголовков
								$("#csv-header").parent().removeClass("hidden");
							// Скрываем переключатель формирования заголовков
							else $("#csv-header").parent().addClass("hidden");
							// Снимаем блокировку вкладки конвертера
							$("#to-containers > li > a[href=#INI]").removeClass("disabled");
							$("#to-containers > li > a[href=#CSV]").removeClass("disabled");
							$("#to-containers > li > a[href=#CEF]").removeClass("disabled");
							$("#to-containers > li > a[href=#SYSLOG]").removeClass("disabled");
						} break;
						// Если ковертер выбран INI
						case "INI": {
							// Активируем подсветку синтаксиса
							fromEditorContainers.setOption("mode", "toml");
							// Активируем вывод номеров строк
							fromEditorContainers.setOption("lineNumbers", true);
							// Выполняем подключение переноса строк
							fromEditorContainers.setOption("lineWrapping", true);
							// Если заголовок CSV не активен
							if($("#to-containers > li > a[href=#CSV]").hasClass("active"))
								// Отображаем переключатель вывода заголовков
								$("#csv-header").parent().removeClass("hidden");
							// Скрываем переключатель формирования заголовков
							else $("#csv-header").parent().addClass("hidden");
							// Снимаем блокировку вкладки конвертера
							$("#to-containers > li > a[href=#INI]").removeClass("disabled");
							// Блокируем вкладку конвертера
							$("#to-containers > li > a[href=#CSV]").addClass("disabled");
							$("#to-containers > li > a[href=#CEF]").addClass("disabled");
							$("#to-containers > li > a[href=#SYSLOG]").addClass("disabled");
							// Получаем активный элемент
							switch($("#to-containers > li > a.active").text()){
								// Если ковертер выбран CSV
								case "CSV":
								// Если ковертер выбран CEF
								case "CEF":
								// Если ковертер выбран SYSLOG
								case "SYSLOG":
									// Переключаем на вкладку по умолчанию
									$("#to-containers > li > a[href=#JSON]").click();
								break;
							}
						} break;
						// Если ковертер выбран CSV
						case "CSV": {
							// Активируем подсветку синтаксиса
							fromEditorContainers.setOption("mode", "shell");
							// Активируем вывод номеров строк
							fromEditorContainers.setOption("lineNumbers", true);
							// Выполняем отключение переноса строк
							fromEditorContainers.setOption("lineWrapping", false);
							// Отображаем переключатель вывода заголовков
							$("#csv-header").parent().removeClass("hidden");
							// Снимаем блокировку вкладки конвертера
							$("#to-containers > li > a[href=#CSV]").removeClass("disabled");
							// Блокируем вкладку конвертера
							$("#to-containers > li > a[href=#INI]").addClass("disabled");
							$("#to-containers > li > a[href=#CEF]").addClass("disabled");
							$("#to-containers > li > a[href=#SYSLOG]").addClass("disabled");
							// Получаем активный элемент
							switch($("#to-containers > li > a.active").text()){
								// Если ковертер выбран INI
								case "INI":
								// Если ковертер выбран CEF
								case "CEF":
								// Если ковертер выбран SYSLOG
								case "SYSLOG":
									// Переключаем на вкладку по умолчанию
									$("#to-containers > li > a[href=#JSON]").click();
								break;
							}
						} break;
						// Если ковертер выбран любой другой
						default: {
							// Активируем подсветку синтаксиса
							fromEditorContainers.setOption("mode", "shell");
							// Активируем вывод номеров строк
							fromEditorContainers.setOption("lineNumbers", false);
							// Выполняем подключение переноса строк
							fromEditorContainers.setOption("lineWrapping", true);
							// Если заголовок CSV не активен
							if($("#to-containers > li > a[href=#CSV]").hasClass("active"))
								// Отображаем переключатель вывода заголовков
								$("#csv-header").parent().removeClass("hidden");
							// Скрываем переключатель формирования заголовков
							else $("#csv-header").parent().addClass("hidden");
							// Блокируем вкладку конвертера
							$("#to-containers > li > a[href=#INI]").addClass("disabled");
							$("#to-containers > li > a[href=#CSV]").addClass("disabled");
							$("#to-containers > li > a[href=#CEF]").addClass("disabled");
							$("#to-containers > li > a[href=#SYSLOG]").addClass("disabled");
							// Получаем активный элемент
							switch($("#to-containers > li > a.active").text()){
								// Если ковертер выбран INI
								case "INI":
								// Если ковертер выбран CSV
								case "CSV":
								// Если ковертер выбран CEF
								case "CEF":
								// Если ковертер выбран SYSLOG
								case "SYSLOG":
									// Переключаем на вкладку по умолчанию
									$("#to-containers > li > a[href=#JSON]").click();
								break;
							}
						}
					}
				}
			}			
			// Запрещаем дальнейшие действия для ссылки
			return false;
		});
		// Устанавливаем событие на перехват событий выбора формата в который нужно конвертировать
		$("#to-containers > li > a")
		.unbind('click')
		.bind('click', function(){
			// Снимаем флаг активации у уже активного элемента
			$("li > a.active", $(this).parent().parent())
			.removeClass("active")
			.removeAttr("aria-current");
			// Устанавливаем флаг активации
			$(this)
			.addClass("active")
			.attr("aria-current", "page");
			// Если текст для конвертации присутствует
			if(fromEditorContainers.getValue().length > 0)
				// Выполняем загрузку новых данных
				$("#submit-containers").click();
			// Удаляем текст в текстовом окне
			else toEditorContainers.setValue("");
			// Определяем тип конвертера
			switch($(this).text()){
				// Если ковертер выбран XML
				case "XML": {
					// Активируем подсветку синтаксиса
					toEditorContainers.setOption("mode", "xml");
					// Активируем вывод номеров строк
					toEditorContainers.setOption("lineNumbers", true);
					// Отображаем переключатель вывода сформированного кода в читаемом виде
					$("#prettify").parent().removeClass("hidden");
					// Если заголовок CSV не активен
					if($("#from-containers > li > a[href=#CSV]").hasClass("active"))
						// Отображаем переключатель вывода заголовков
						$("#csv-header").parent().removeClass("hidden");
					// Скрываем переключатель формирования заголовков
					else $("#csv-header").parent().addClass("hidden");
				} break;
				// Если ковертер выбран JSON
				case "JSON": {
					// Активируем подсветку синтаксиса
					toEditorContainers.setOption("mode", "javascript");
					// Активируем вывод номеров строк
					toEditorContainers.setOption("lineNumbers", true);
					// Отображаем переключатель вывода сформированного кода в читаемом виде
					$("#prettify").parent().removeClass("hidden");
					// Если заголовок CSV не активен
					if($("#from-containers > li > a[href=#CSV]").hasClass("active"))
						// Отображаем переключатель вывода заголовков
						$("#csv-header").parent().removeClass("hidden");
					// Скрываем переключатель формирования заголовков
					else $("#csv-header").parent().addClass("hidden");
				} break;
				// Если ковертер выбран YAML
				case "YAML": {
					// Активируем подсветку синтаксиса
					toEditorContainers.setOption("mode", "yaml");
					// Активируем вывод номеров строк
					toEditorContainers.setOption("lineNumbers", true);
					// Скрываем переключатель вывода сформированного кода в читаемом виде
					$("#prettify").parent().addClass("hidden");
					// Если заголовок CSV не активен
					if($("#from-containers > li >a[href=#CSV]").hasClass("active"))
						// Отображаем переключатель вывода заголовков
						$("#csv-header").parent().removeClass("hidden");
					// Скрываем переключатель формирования заголовков
					else $("#csv-header").parent().addClass("hidden");
				} break;
				// Если ковертер выбран INI
				case "INI": {
					// Активируем подсветку синтаксиса
					toEditorContainers.setOption("mode", "toml");
					// Активируем вывод номеров строк
					toEditorContainers.setOption("lineNumbers", true);
					// Выполняем подключение переноса строк
					toEditorContainers.setOption("lineWrapping", true);
					// Скрываем переключатель вывода сформированного кода в читаемом виде
					$("#prettify").parent().addClass("hidden");
					// Если заголовок CSV не активен
					if($("#from-containers > li > a[href=#CSV]").hasClass("active"))
						// Отображаем переключатель вывода заголовков
						$("#csv-header").parent().removeClass("hidden");
					// Скрываем переключатель формирования заголовков
					else $("#csv-header").parent().addClass("hidden");
				} break;
				// Если ковертер выбран CSV
				case "CSV": {
					// Активируем подсветку синтаксиса
					toEditorContainers.setOption("mode", "shell");
					// Активируем вывод номеров строк
					toEditorContainers.setOption("lineNumbers", true);
					// Скрываем переключатель вывода сформированного кода в читаемом виде
					$("#prettify").parent().addClass("hidden");
					// Отображаем переключатель вывода заголовков
					$("#csv-header").parent().removeClass("hidden");
				} break;
				// Если ковертер выбран любой другой
				default: {
					// Активируем подсветку синтаксиса
					toEditorContainers.setOption("mode", "shell");
					// Активируем вывод номеров строк
					toEditorContainers.setOption("lineNumbers", false);
					// Выполняем подключение переноса строк
					toEditorContainers.setOption("lineWrapping", true);
					// Скрываем переключатель вывода сформированного кода в читаемом виде
					$("#prettify").parent().addClass("hidden");
					// Если заголовок CSV не активен
					if($("#from-containers > li > a[href=#CSV]").hasClass("active"))
						// Отображаем переключатель вывода заголовков
						$("#csv-header").parent().removeClass("hidden");
					// Скрываем переключатель формирования заголовков
					else $("#csv-header").parent().addClass("hidden");
				}
			}
			// Запрещаем дальнейшие действия для ссылки
			return false;
		});
	});
})(jQuery);
