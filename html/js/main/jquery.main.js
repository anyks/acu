(function($){
	// Активируем событие загрузки
	$(document).ready(function(){
		// Блок подсветки синтаксиса шаблонов GROK
		let grokEditor = null;
		// Выполняем инициализацию блока подсветки синтаксиса для редактора формата из которого производится конвертация
		let fromEditor = CodeMirror(document.getElementById('code-editor-from'), {
			tabSize: 4,
			mode: 'xml',
			theme: 'eclipse',
			lineNumbers: true
		});
		// Выполняем инициализацию блока подсветки синтаксиса для редактора формата в который производится конвертация
		const toEditor = CodeMirror(document.getElementById('code-editor-to'), {
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
		// Устанавливаем событие на изменение поля из которого следует выполнять конвертацию
		$("> .CodeMirror", "#code-editor-from")
		.unbind("keyup")
		.bind("keyup", function(){
			// Получаем идентификатор кнопки
			const button = $(".btn-close", $(this).parent().parent());
			// Если поле содержит данные
			if(fromEditor.getValue().length > 0)
				// Выполняем отображение кнопки очистки поля
				button.removeClass("hidden");
			// Выполняем скрытие кнопки очистки
			else button.addClass("hidden");
			// Устанавливаем событие на клик по кнопке
			button
			.unbind('click')
			.bind('click', function(){
				// Выполняем очистку текстового поля
				fromEditor.setValue("");
				// Выполняем скрытие кнопки
				$(this).addClass("hidden");
				// Запрещаем дальнейшее событие для кнопки
				return false;
			});
			// Запрещаем выполнение дальнейшего события
			return false;
		});
		// Устанавливаем событие на кнопку отправки запроса
		$("#submit")
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
					const text = fromEditor.getValue();
					// Если текст для конвертации указан
					if(text.length > 0){
						// Название конвертера из которого и в который производится конвертация
						let from = "", to = "";
						// Определяем формат в который производится конвертация
						switch($("a.active", "#to").attr("href")){
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
						switch($("a.active", "#from").attr("href")){
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
								toEditor.setValue(answer.result);
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
					alert("Ошибка синтаксиса", error);
				}
			})();
			// Выключаем индикатор загрузки
			$("#spinner").addClass("hidden");
			// Запрещаем дальнейшие действия для ссылки
			return false;
		});
		// Устанавливаем событие на перехват событий выбора формата из чего нужно конвертировать
		$("#from > li > a")
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
				fromEditor.setValue("");
				// Выполняем скрытие кнопки очитски
				$(".btn-close", $("#code-editor-from").parent()).addClass("hidden");
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
					fromEditor.setOption("mode", "shell");
					// Активируем вывод номеров строк
					fromEditor.setOption("lineNumbers", false);
					// Выполняем подключение переноса строк
					fromEditor.setOption("lineWrapping", true);
					// Если заголовок CSV не активен
					if($("#to > li > a[href=#CSV]").hasClass("active"))
						// Отображаем переключатель вывода заголовков
						$("#csv-header").parent().removeClass("hidden");
					// Скрываем переключатель формирования заголовков
					else $("#csv-header").parent().addClass("hidden");
					// Блокируем вкладку конвертера
					$("#to > li > a[href=#INI]").addClass("disabled");
					$("#to > li > a[href=#CSV]").addClass("disabled");
					$("#to > li > a[href=#CEF]").addClass("disabled");
					$("#to > li > a[href=#SYSLOG]").addClass("disabled");
					// Получаем активный элемент
					switch($("#to > li > a.active").text()){
						// Если ковертер выбран INI
						case "INI":
						// Если ковертер выбран CSV
						case "CSV":
						// Если ковертер выбран CEF
						case "CEF":
						// Если ковертер выбран SYSLOG
						case "SYSLOG":
							// Переключаем на вкладку по умолчанию
							$("#to > li > a[href=#JSON]").click();
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
							fromEditor.setOption("mode", "xml");
							// Активируем вывод номеров строк
							fromEditor.setOption("lineNumbers", true);
							// Выполняем отключение переноса строк
							fromEditor.setOption("lineWrapping", false);
							// Если заголовок CSV не активен
							if($("#to > li > a[href=#CSV]").hasClass("active"))
								// Отображаем переключатель вывода заголовков
								$("#csv-header").parent().removeClass("hidden");
							// Скрываем переключатель формирования заголовков
							else $("#csv-header").parent().addClass("hidden");
							// Блокируем вкладку конвертера
							$("#to > li > a[href=#CSV]").addClass("disabled");
							$("#to > li > a[href=#CEF]").addClass("disabled");
							$("#to > li > a[href=#SYSLOG]").addClass("disabled");
							// Снимаем блокировку вкладки конвертера
							$("#to > li > a[href=#INI]").removeClass("disabled");
						} break;
						// Если ковертер выбран JSON
						case "JSON": {
							// Активируем подсветку синтаксиса
							fromEditor.setOption("mode", "javascript");
							// Активируем вывод номеров строк
							fromEditor.setOption("lineNumbers", true);
							// Выполняем отключение переноса строк
							fromEditor.setOption("lineWrapping", false);
							// Если заголовок CSV не активен
							if($("#to > li > a[href=#CSV]").hasClass("active"))
								// Отображаем переключатель вывода заголовков
								$("#csv-header").parent().removeClass("hidden");
							// Скрываем переключатель формирования заголовков
							else $("#csv-header").parent().addClass("hidden");
							// Снимаем блокировку вкладки конвертера
							$("#to > li > a[href=#INI]").removeClass("disabled");
							$("#to > li > a[href=#CSV]").removeClass("disabled");
							$("#to > li > a[href=#CEF]").removeClass("disabled");
							$("#to > li > a[href=#SYSLOG]").removeClass("disabled");
						} break;
						// Если ковертер выбран YAML
						case "YAML": {
							// Активируем подсветку синтаксиса
							fromEditor.setOption("mode", "yaml");
							// Активируем вывод номеров строк
							fromEditor.setOption("lineNumbers", true);
							// Выполняем отключение переноса строк
							fromEditor.setOption("lineWrapping", false);
							// Если заголовок CSV не активен
							if($("#to > li > a[href=#CSV]").hasClass("active"))
								// Отображаем переключатель вывода заголовков
								$("#csv-header").parent().removeClass("hidden");
							// Скрываем переключатель формирования заголовков
							else $("#csv-header").parent().addClass("hidden");
							// Снимаем блокировку вкладки конвертера
							$("#to > li > a[href=#INI]").removeClass("disabled");
							$("#to > li > a[href=#CSV]").removeClass("disabled");
							$("#to > li > a[href=#CEF]").removeClass("disabled");
							$("#to > li > a[href=#SYSLOG]").removeClass("disabled");
						} break;
						// Если ковертер выбран INI
						case "INI": {
							// Активируем подсветку синтаксиса
							fromEditor.setOption("mode", "toml");
							// Активируем вывод номеров строк
							fromEditor.setOption("lineNumbers", true);
							// Выполняем подключение переноса строк
							fromEditor.setOption("lineWrapping", true);
							// Если заголовок CSV не активен
							if($("#to > li > a[href=#CSV]").hasClass("active"))
								// Отображаем переключатель вывода заголовков
								$("#csv-header").parent().removeClass("hidden");
							// Скрываем переключатель формирования заголовков
							else $("#csv-header").parent().addClass("hidden");
							// Снимаем блокировку вкладки конвертера
							$("#to > li > a[href=#INI]").removeClass("disabled");
							// Блокируем вкладку конвертера
							$("#to > li > a[href=#CSV]").addClass("disabled");
							$("#to > li > a[href=#CEF]").addClass("disabled");
							$("#to > li > a[href=#SYSLOG]").addClass("disabled");
							// Получаем активный элемент
							switch($("#to > li > a.active").text()){
								// Если ковертер выбран CSV
								case "CSV":
								// Если ковертер выбран CEF
								case "CEF":
								// Если ковертер выбран SYSLOG
								case "SYSLOG":
									// Переключаем на вкладку по умолчанию
									$("#to > li > a[href=#JSON]").click();
								break;
							}
						} break;
						// Если ковертер выбран CSV
						case "CSV": {
							// Активируем подсветку синтаксиса
							fromEditor.setOption("mode", "shell");
							// Активируем вывод номеров строк
							fromEditor.setOption("lineNumbers", true);
							// Выполняем отключение переноса строк
							fromEditor.setOption("lineWrapping", false);
							// Отображаем переключатель вывода заголовков
							$("#csv-header").parent().removeClass("hidden");
							// Снимаем блокировку вкладки конвертера
							$("#to > li > a[href=#CSV]").removeClass("disabled");
							// Блокируем вкладку конвертера
							$("#to > li > a[href=#INI]").addClass("disabled");
							$("#to > li > a[href=#CEF]").addClass("disabled");
							$("#to > li > a[href=#SYSLOG]").addClass("disabled");
							// Получаем активный элемент
							switch($("#to > li > a.active").text()){
								// Если ковертер выбран INI
								case "INI":
								// Если ковертер выбран CEF
								case "CEF":
								// Если ковертер выбран SYSLOG
								case "SYSLOG":
									// Переключаем на вкладку по умолчанию
									$("#to > li > a[href=#JSON]").click();
								break;
							}
						} break;
						// Если ковертер выбран любой другой
						default: {
							// Активируем подсветку синтаксиса
							fromEditor.setOption("mode", "shell");
							// Активируем вывод номеров строк
							fromEditor.setOption("lineNumbers", false);
							// Выполняем подключение переноса строк
							fromEditor.setOption("lineWrapping", true);
							// Если заголовок CSV не активен
							if($("#to > li > a[href=#CSV]").hasClass("active"))
								// Отображаем переключатель вывода заголовков
								$("#csv-header").parent().removeClass("hidden");
							// Скрываем переключатель формирования заголовков
							else $("#csv-header").parent().addClass("hidden");
							// Блокируем вкладку конвертера
							$("#to > li > a[href=#INI]").addClass("disabled");
							$("#to > li > a[href=#CSV]").addClass("disabled");
							$("#to > li > a[href=#CEF]").addClass("disabled");
							$("#to > li > a[href=#SYSLOG]").addClass("disabled");
							// Получаем активный элемент
							switch($("#to > li > a.active").text()){
								// Если ковертер выбран INI
								case "INI":
								// Если ковертер выбран CSV
								case "CSV":
								// Если ковертер выбран CEF
								case "CEF":
								// Если ковертер выбран SYSLOG
								case "SYSLOG":
									// Переключаем на вкладку по умолчанию
									$("#to > li > a[href=#JSON]").click();
								break;
							}
						} break;
					}
				}
			}			
			// Запрещаем дальнейшие действия для ссылки
			return false;
		});
		// Устанавливаем событие на перехват событий выбора формата в который нужно конвертировать
		$("#to > li > a")
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
			if(fromEditor.getValue().length > 0)
				// Выполняем загрузку новых данных
				$("#submit").click();
			// Удаляем текст в текстовом окне
			else toEditor.setValue("");
			// Определяем тип конвертера
			switch($(this).text()){
				// Если ковертер выбран XML
				case "XML": {
					// Активируем подсветку синтаксиса
					toEditor.setOption("mode", "xml");
					// Активируем вывод номеров строк
					toEditor.setOption("lineNumbers", true);
					// Отображаем переключатель вывода сформированного кода в читаемом виде
					$("#prettify").parent().removeClass("hidden");
					// Если заголовок CSV не активен
					if($("#from > li > a[href=#CSV]").hasClass("active"))
						// Отображаем переключатель вывода заголовков
						$("#csv-header").parent().removeClass("hidden");
					// Скрываем переключатель формирования заголовков
					else $("#csv-header").parent().addClass("hidden");
				} break;
				// Если ковертер выбран JSON
				case "JSON": {
					// Активируем подсветку синтаксиса
					toEditor.setOption("mode", "javascript");
					// Активируем вывод номеров строк
					toEditor.setOption("lineNumbers", true);
					// Отображаем переключатель вывода сформированного кода в читаемом виде
					$("#prettify").parent().removeClass("hidden");
					// Если заголовок CSV не активен
					if($("#from > li > a[href=#CSV]").hasClass("active"))
						// Отображаем переключатель вывода заголовков
						$("#csv-header").parent().removeClass("hidden");
					// Скрываем переключатель формирования заголовков
					else $("#csv-header").parent().addClass("hidden");
				} break;
				// Если ковертер выбран YAML
				case "YAML": {
					// Активируем подсветку синтаксиса
					toEditor.setOption("mode", "yaml");
					// Активируем вывод номеров строк
					toEditor.setOption("lineNumbers", true);
					// Скрываем переключатель вывода сформированного кода в читаемом виде
					$("#prettify").parent().addClass("hidden");
					// Если заголовок CSV не активен
					if($("#from > li >a[href=#CSV]").hasClass("active"))
						// Отображаем переключатель вывода заголовков
						$("#csv-header").parent().removeClass("hidden");
					// Скрываем переключатель формирования заголовков
					else $("#csv-header").parent().addClass("hidden");
				} break;
				// Если ковертер выбран INI
				case "INI": {
					// Активируем подсветку синтаксиса
					toEditor.setOption("mode", "toml");
					// Активируем вывод номеров строк
					toEditor.setOption("lineNumbers", true);
					// Выполняем подключение переноса строк
					toEditor.setOption("lineWrapping", true);
					// Скрываем переключатель вывода сформированного кода в читаемом виде
					$("#prettify").parent().addClass("hidden");
					// Если заголовок CSV не активен
					if($("#from > li > a[href=#CSV]").hasClass("active"))
						// Отображаем переключатель вывода заголовков
						$("#csv-header").parent().removeClass("hidden");
					// Скрываем переключатель формирования заголовков
					else $("#csv-header").parent().addClass("hidden");
				} break;
				// Если ковертер выбран CSV
				case "CSV": {
					// Активируем подсветку синтаксиса
					toEditor.setOption("mode", "shell");
					// Активируем вывод номеров строк
					toEditor.setOption("lineNumbers", true);
					// Скрываем переключатель вывода сформированного кода в читаемом виде
					$("#prettify").parent().addClass("hidden");
					// Отображаем переключатель вывода заголовков
					$("#csv-header").parent().removeClass("hidden");
				} break;
				// Если ковертер выбран любой другой
				default: {
					// Активируем подсветку синтаксиса
					toEditor.setOption("mode", "shell");
					// Активируем вывод номеров строк
					toEditor.setOption("lineNumbers", false);
					// Выполняем подключение переноса строк
					toEditor.setOption("lineWrapping", true);
					// Скрываем переключатель вывода сформированного кода в читаемом виде
					$("#prettify").parent().addClass("hidden");
					// Если заголовок CSV не активен
					if($("#from > li > a[href=#CSV]").hasClass("active"))
						// Отображаем переключатель вывода заголовков
						$("#csv-header").parent().removeClass("hidden");
					// Скрываем переключатель формирования заголовков
					else $("#csv-header").parent().addClass("hidden");
				} break;
			}
			// Запрещаем дальнейшие действия для ссылки
			return false;
		});
	});
})(jQuery);
