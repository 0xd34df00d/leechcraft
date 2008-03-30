<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS><TS version="1.1">
<defaultcodec></defaultcodec>
<context>
    <name>ColumnSelector</name>
    <message>
        <location filename="columnselector.ui" line="19"/>
        <source>Column Selector</source>
        <translation>Выбор столбцов</translation>
    </message>
    <message>
        <location filename="columnselector.ui" line="25"/>
        <source>Select columns:</source>
        <translation>Выбери столбцы:</translation>
    </message>
    <message>
        <location filename="columnselector.ui" line="37"/>
        <source>Select all</source>
        <translation>Выбрать все</translation>
    </message>
    <message>
        <location filename="columnselector.ui" line="44"/>
        <source>Deselect all</source>
        <translation>Убрать все</translation>
    </message>
</context>
<context>
    <name>FileExistsDialog</name>
    <message>
        <location filename="fileexistsdialog.ui" line="25"/>
        <source>File exists</source>
        <comment>File already exists</comment>
        <translation>Файл уже существует</translation>
    </message>
    <message>
        <location filename="fileexistsdialog.ui" line="43"/>
        <source>Specified file already exists, what should I do?</source>
        <translation>Указанный файл уже существует, что мне делать?</translation>
    </message>
    <message>
        <location filename="fileexistsdialog.ui" line="54"/>
        <source>Start from scratch</source>
        <translation type="obsolete">Начать сначала</translation>
    </message>
    <message>
        <location filename="fileexistsdialog.ui" line="61"/>
        <source>Continue already started job</source>
        <translation type="obsolete">Продолжить уже начатую работу</translation>
    </message>
    <message>
        <location filename="fileexistsdialog.ui" line="71"/>
        <source>Make the file unique (append a number)</source>
        <translation type="obsolete">Сделать имя скачиваемого файла уникальным (дописать число)</translation>
    </message>
    <message>
        <location filename="fileexistsdialog.ui" line="78"/>
        <source>Cancel, do nothing</source>
        <translation type="obsolete">Ничего не делать</translation>
    </message>
    <message>
        <location filename="fileexistsdialog.ui" line="91"/>
        <source>OK</source>
        <translation>ОК</translation>
    </message>
    <message>
        <location filename="fileexistsdialog.ui" line="51"/>
        <source>Continue the job</source>
        <translation>Продолжить закачку</translation>
    </message>
    <message>
        <location filename="fileexistsdialog.ui" line="56"/>
        <source>Delete old file</source>
        <translation>Удалить старый файл</translation>
    </message>
    <message>
        <location filename="fileexistsdialog.ui" line="61"/>
        <source>Make filename unique</source>
        <translation>Сделать имя скачиваемого файла уникальным</translation>
    </message>
    <message>
        <location filename="fileexistsdialog.ui" line="66"/>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
</context>
<context>
    <name>FinishedJob</name>
    <message>
        <location filename="finishedjob.cpp" line="19"/>
        <source>/s</source>
        <translation type="obsolete">/с</translation>
    </message>
</context>
<context>
    <name>FtpImp</name>
    <message>
        <location filename="ftpimp.cpp" line="99"/>
        <source>Wrong login.</source>
        <translation>Неправильный логин.</translation>
    </message>
    <message>
        <location filename="ftpimp.cpp" line="106"/>
        <source>Wrong password.</source>
        <translation>Неправильный пароль.</translation>
    </message>
    <message>
        <location filename="ftpimp.cpp" line="113"/>
        <source>Wrong path, file not found.</source>
        <translation>Неправильный путь, файл не найден.</translation>
    </message>
    <message>
        <location filename="ftpimp.cpp" line="120"/>
        <source>Wrong restart position.</source>
        <translation>Сервер не принял точку старта.</translation>
    </message>
    <message>
        <location filename="ftpimp.cpp" line="127"/>
        <source>PASV failed</source>
        <translation>Команда PASV не выполнена</translation>
    </message>
    <message>
        <location filename="ftpimp.cpp" line="134"/>
        <source>NLST failed.</source>
        <translation>Команда NLST не выполнена.</translation>
    </message>
    <message>
        <location filename="ftpimp.cpp" line="141"/>
        <source>RETR failed.</source>
        <translation>Команда RETR не выполнена.</translation>
    </message>
    <message>
        <location filename="ftpimp.cpp" line="148"/>
        <source>SIZE failed.</source>
        <translation>Команда SIZE не выполнена.</translation>
    </message>
    <message>
        <location filename="ftpimp.cpp" line="155"/>
        <source>CWD failed.</source>
        <translation>Команда CWD не выполнена.</translation>
    </message>
    <message>
        <location filename="ftpimp.cpp" line="162"/>
        <source>TYPE I failed.</source>
        <translation>Команда TYPE I не выполнена.</translation>
    </message>
</context>
<context>
    <name>HttpImp</name>
    <message>
        <location filename="httpimp.cpp" line="299"/>
        <source>400 Bad request. The request contains bad syntax or cannot be fulfilled.</source>
        <translation>400 Плохой запрос. Запрос не понят сервером из-за наличия синтаксической ошибки. Клиенту следует повторно обратиться к ресурсу с изменённым запросом.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="302"/>
        <source>401 Unauthorized. Authentication is possible but has failed or not yet been provided.</source>
        <translation>401 Неавторизован. Запрос требует идентификации пользователя. Клиент должен запросить имя и пароль у пользователя и передать их в записи WWW-Authenticate заголовка в следующем запросе. В случае ввода ошибочных данных сервер снова вернёт этот же статус.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="305"/>
        <source>403 Forbidden. The request was legal, but server is refusing to respond to it. Authenticating will make no difference.</source>
        <translation>403 Запрещено. Сервер понял запрос, но он отказывается его выполнять из-за каких-то ограничений в доступе. Идентификация через протокол HTTP здесь не поможет. Скорее всего на сервере нужно провести аутентификацию другим способом, сделать запрос с определёнными параметрами или удовлетворить каким-либо условиям.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="308"/>
        <source>404 Resource not found.</source>
        <translation>404 Ресурс не найден. Сервер понял запрос, но не нашёл соответствующего ресурса по указанному URI. Если серверу известно что по этому адресу был документ, то ему желательно использовать код 410 вместо этого. Этот код может использоваться вместо 403 если требуется тщательно скрыть от посторонних глаз определённые ресурсы.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="311"/>
        <source>405 Method not allowed. Request method not supported by the URL.</source>
        <translation>405 Метод не поддерживается. Указанный клиентом метод нельзя применить к ресурсу. Сервер так же должен передать в заголовке ответа поле Allow со списком доступных методов.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="314"/>
        <source>406 Not acceptable.</source>
        <translation>406 Не приемлимо. Запрошенный URI не может удовлетворить переданным в заголовке характеристикам. Если метод был не HEAD, то сервер должен вернуть список допустимых характеристик для данного ресурса.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="317"/>
        <source>407 Proxy authentication required.</source>
        <translation>407 Необходима авторизация на прокси-сервере. Ответ аналогичен коду 401 за исключением того, что аутентификация производится для прокси-сервера. Механизм аналогичен идентификации на обычном сервере.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="320"/>
        <source>408 Request timeout.</source>
        <translation>408 Время ожидания истекло. Время ожидания сервером передачи от клиента истекло. Клиент может повторить аналогичный предыдущему запрос в любое время.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="323"/>
        <source>409 Conflict.</source>
        <translation>409 Конфликт. Запрос не может выполнен из-за конфликтного обращения к ресурсу. Такое возможно, например, когда два клиента пытаются изменить ресурс с помощью метода PUT.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="326"/>
        <source>410 Gone. Resource is not available and will not be available again. Maybe it was intentionally removed.</source>
        <translation>410 Удален. Такой ответ сервер посылает когда ресурс раньше был по указанному URI, но был удалён и теперь не доступен. Серверу в этом случае не известно и местоположение альтернативного документа (например, копии). Если у сервера есть подозрение что документ в ближайшее время может быть восстановлен, то лучше клиенту передать код 404.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="329"/>
        <source>411 Length required.</source>
        <translation>411 Необходима длина. Для указанного ресурса клиент должен указать Content-Length в заголовке запроса. Без указания этого поля не стоит делать повторную попытку запроса к серверу по данному URI.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="332"/>
        <source>412 Precondition failed.</source>
        <translation>412 Предварительное условие не удовлетворено. Возвращается если ни одно из условных полей заголовка запроса не было выполнено.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="335"/>
        <source>413 Request entity too large.</source>
        <translation>413 Запрашиваемые данные слишком большие. Возвращается если сервер по каким-то причинам не может передать запрашиваемый объём информации. Если проблема временная, то сервер может в ответе указать в поле Retry-After через которое можно повторить аналогичный запрос.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="338"/>
        <source>414 Request URI too long.</source>
        <translation>414 Запрашиваемый URI слишком длинный. Сервер не может обработать запрос из-за слишком длинного указанного URI. Такую ошибку можно спровоцировать, например, когда клиент пытается передать длинные параметры через метод GET, а не POST.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="341"/>
        <source>415 Unsupported media type.</source>
        <translation>415 Неподдерживаемый тип данных. По каким-то причинам сервер отказывается работать с указанным типом данных при данном методе.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="344"/>
        <source>417 Expectation failed.</source>
        <translation>417 Ожидаемое ошибочно. По каким-то причинам сервер не может удовлетворить значению поля Except заголовка запроса.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="347"/>
        <source>422 Unprocessable entity (WebDAV). The request was well-formed but was unable to be followed due to semantic errors.</source>
        <translation>422 Необрабатываемый экземпляр (WebDAV). Сервер успешно принял запрос, может работать с указанным видом данных, в теле запроса XML-документ имеет верный синтаксис, но имеется какая-то логическая ошибка из-за которой не возможно произвести операцию над ресурсом.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="350"/>
        <source>423 Locked (WebDAV). The resource that is being accessed is locked.</source>
        <translation>423 Заблокировано (WebDAV). Целевой ресурс из запроса заблокирован от применения к нему указанного метода.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="353"/>
        <source>424 Failed dependency (WebDAV). The request failed due to failure of a previous request.</source>
        <translation>424 Неудовлетворенная зависимость (WebDAV). Реализация текущего запроса может зависеть от успешности выполнения другой операции. Если она провалена и из-за этого нельзя выполнить текущий запрос, то сервер вернёт код 424.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="356"/>
        <source>425 Unordered collection (WebDAV). You really never should see this message.</source>
        <translation>425 Неупорядоченная коллекция (WebDAV). Ты не должен видеть это сообщение, так как этот код ответа нигде не применяется.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="359"/>
        <source>426 Upgrade required. The client should switch to TLS/1.0.</source>
        <translation>426 Необходимо обновление. Сервер указывает клиенту на необходимость обновить протокол (TLS/1.0). Заголовок ответа должен содержать правильно сформированные поля Upgrade и Connection.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="362"/>
        <source>449 Retry with. A Microsoft extension: The request should be retried after doing the appropriate action.</source>
        <translation>449 Попробуй снова. Мелкомягкое расширение: запрос должен быть повторен после выполнения соответствующих действий.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="365"/>
        <source>500 Internal server error. Server failed to fulfil the request due to misconfiguration.</source>
        <translation>500 Внутренняя ошибка сервера. Серверу не удалось выполнить запрос из-за внутренней ошибки, не входящей в другие класса 5xx.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="368"/>
        <source>501 Not implemented.</source>
        <translation>501 Не реализовано. Сервер не поддерживает возможностей, необходимых для обработки запроса. Типичный ответ для случаев, когда сервер не понимает указанный в запросе метод.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="371"/>
        <source>502 Bad gateway.</source>
        <translation>502 Плохой шлюз. Сервер в роли шлюза или прокси получил сообщение о неудачном выполнении промежуточной операции.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="374"/>
        <source>503 Service unavailable.</source>
        <translation>503 Сервис недоступен. Сервер временно не имеет возможности обрабатывать запросы по техническим причинам (обслуживание, перегрузка и прочее). В поле Retry-After заголовка сервер может указать время, через которое клиенту рекомендуется повторить запрос. Хотя во время перегрузки очевидным является сразу разрывать соединение, эффективней может оказаться установка большого значения поля Retry-After для уменьшения частоты избыточных запросов.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="377"/>
        <source>504 Gateway timeout.</source>
        <translation>504 Шлюз не отвечает. Сервер в роли шлюза или прокси не дождался ответа от вышестоящего сервера для завершения текущего запроса.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="380"/>
        <source>505 HTTP version not supported.</source>
        <translation>505 Версия HTTP не поддерживается. Сервер не поддерживает или отказывается поддерживать указанную в запросе версию протокола HTTP.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="383"/>
        <source>506 Variant also negotiates.</source>
        <translation>506 Вариант тоже согласован. В результате ошибочной конфигурации выбранный вариант указывает сам на себя, из-за чего процесс связывания прерывается.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="386"/>
        <source>507 Insufficient storage (WebDAV).</source>
        <translation>507 Не хватает места (WebDAV). Не хватает места для выполнения текущего запроса. Проблема может быть временной.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="389"/>
        <source>509 Bandwidth limit exceeded.</source>
        <translation>509 Превышен предел полосы пропускания. Неофициальный код, тем не менее, довольно часто используемый. Используется для указания израсходованной полосы пропускания.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="392"/>
        <source>510 Not extented.</source>
        <translation>510 Не расширено. На сервере отсутствует расширение, которое планирует использовать клиент. Сервер может дополнительно передать информацию о доступных ему расширениях.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="211"/>
        <source>Failed to read response: socket timeout</source>
        <translation>Не удалось прочитать ответ сервера: таймаут соекта</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="74"/>
        <source>Error while trying to connect to host %1, port %2: %3</source>
        <translation>Ошибка при попытке подключения к хосту %1, порт %2: %3</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="149"/>
        <source>HTTP implementation failed in a very strange way. Please send to developers any .log files you find in application&apos;s directory and it&apos;s subdirectories. Thanks for your help.</source>
        <translation>HTTP-протокол упал очень странным образом. Пожайлуста, отправь разработчикам любые .log-файлы, которые найдешь в директории приложения и ее поддиректориях. Спасибо за помощь.</translation>
    </message>
</context>
<context>
    <name>HttpPlugin</name>
    <message>
        <location filename="httpplugin.cpp" line="47"/>
        <source>HTTP/FTP worker 0.2</source>
        <translation type="obsolete">Скачиватель по HTTP/FTP 0.2</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="82"/>
        <source>&amp;Tools</source>
        <translation type="obsolete">&amp;Инструменты</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="81"/>
        <source>&amp;Jobs</source>
        <translation type="obsolete">&amp;Задания</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="93"/>
        <source>Add...</source>
        <translation type="obsolete">Добавить задание...</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="95"/>
        <source>Add a new job</source>
        <translation type="obsolete">Добавить новое задание задание</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="97"/>
        <source>Start current</source>
        <translation type="obsolete">Запустить выделенное</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="101"/>
        <source>Ctrl+S</source>
        <translation type="obsolete">Ctrl+S</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="103"/>
        <source>Start all</source>
        <translation type="obsolete">Запустить все</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="104"/>
        <source>Ctrl+Shift+S</source>
        <translation type="obsolete">Ctrl+Shift+S</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="101"/>
        <source>Stop current</source>
        <translation type="obsolete">Остановить выделенное</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="107"/>
        <source>Ctrl+I</source>
        <translation type="obsolete">Ctrl+I</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="109"/>
        <source>Stop all</source>
        <translation type="obsolete">Остановить все</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="110"/>
        <source>Ctrl+Shift+I</source>
        <translation type="obsolete">Ctrl+Shift+I</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="108"/>
        <source>Stop selected job(s)</source>
        <translation type="obsolete">Запланировать выбранные задания</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="154"/>
        <source>Autoadjust interface</source>
        <translation type="obsolete">Подстроить интерфейс</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="158"/>
        <source>Preferences...</source>
        <translation type="obsolete">Настройки...</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="119"/>
        <source>Ctrl+P</source>
        <translation type="obsolete">Ctrl+P</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="199"/>
        <source>%</source>
        <translation type="obsolete">%</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="224"/>
        <source>Local name</source>
        <translation type="obsolete">Локальное имя</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="224"/>
        <source>URL</source>
        <translation type="obsolete">URL</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="199"/>
        <source>Speed</source>
        <translation type="obsolete">Скорость</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="177"/>
        <source>Downloaded size</source>
        <translation type="obsolete">Скачено</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="177"/>
        <source>Total size</source>
        <translation type="obsolete">Размер</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="224"/>
        <source>Size</source>
        <translation type="obsolete">Размер</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="295"/>
        <source>0</source>
        <translation type="obsolete">0</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="338"/>
        <source>: Preferences</source>
        <translation>: Настройки</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="343"/>
        <source>Job error</source>
        <translation>Ошибка задания</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="568"/>
        <source>Job with URL %1 signals about following error:&lt;br /&gt;&lt;code&gt;%2&lt;/code&gt;</source>
        <translation type="obsolete">Задание с URL %1 говорит о следующей ошибке:&lt;br /&gt;&lt;code&gt;%2&lt;/code&gt;</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="348"/>
        <source>/s</source>
        <translation>/с</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="104"/>
        <source>Simple HTTP and FTP plugin, providing basic functionality.</source>
        <translation>Простой HTTP/FTP плагин, обеспечивающий базовую функциональность.</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="112"/>
        <source>Get file size</source>
        <translation type="obsolete">Получить размер файла</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="132"/>
        <source>Copy URL to clipboard</source>
        <translation type="obsolete">Скопировать URL в буфер</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="199"/>
        <source>State</source>
        <translation type="obsolete">Статус</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="199"/>
        <source>Remaining time</source>
        <translation type="obsolete">Оставшееся время</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="199"/>
        <source>Downloaded</source>
        <translation type="obsolete">Скачано</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="199"/>
        <source>Total</source>
        <translation type="obsolete">Размер</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="199"/>
        <source>Download time</source>
        <translation type="obsolete">Время загрузки</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="224"/>
        <source>Average speed</source>
        <translation type="obsolete">Средняя скорость</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="224"/>
        <source>Time to complete</source>
        <translation type="obsolete">Время до завершения</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="140"/>
        <source>Select active tasks list columns...</source>
        <translation type="obsolete">Выбрать столбцы списка активных задач...</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="149"/>
        <source>Select finished tasks list columns...</source>
        <translation type="obsolete">Выбрать столбцы списка завершенных задач...</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="343"/>
        <source>Job with URL&lt;br /&gt;%1&lt;br /&gt;signals about following error:&lt;br /&gt;&lt;br /&gt;&lt;em&gt;%2&lt;/em&gt;</source>
        <translation>Задание с URL&lt;br /&gt;%1&lt;br /&gt;сообщает об ошибке:&lt;br /&gt;&lt;em&gt;%2&lt;/em&gt;</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="121"/>
        <source>Delete</source>
        <translation type="obsolete">Удалить</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="98"/>
        <source>Delete selected job(s)</source>
        <translation type="obsolete">Удалить выбранное задание(ия)</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="102"/>
        <source>Start selected job(s)</source>
        <translation type="obsolete">Запустить выбранное задание(ия)</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="105"/>
        <source>Start all jobs</source>
        <translation type="obsolete">Запустить все задания</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="106"/>
        <source>Stop</source>
        <translation type="obsolete">Остановить</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="111"/>
        <source>Stop all jobs</source>
        <translation type="obsolete">Остановить все задания</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="113"/>
        <source>Get file size for selected jobs without downloading them</source>
        <translation type="obsolete">Получить размер файла выбранных заданий без их загрузки</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="115"/>
        <source>Schedule</source>
        <translation type="obsolete">Запланировать</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="116"/>
        <source>Schedule select job(s)</source>
        <translation type="obsolete">Запланировать выбранное задание(ия)</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="118"/>
        <source>Job properties...</source>
        <translation type="obsolete">Свойства задания...</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="120"/>
        <source>Change selected job&apos;s properties</source>
        <translation type="obsolete">Изменить свойства выбранного задания</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="123"/>
        <source>Delete selected finished job(s)</source>
        <translation type="obsolete">Удалить выбранное законченное задание(ия)</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="155"/>
        <source>Adjust interface to make all text in columns fit</source>
        <translation type="obsolete">Подстроить интерфейс, чтобы текст во всех столбцах был полностью виден</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="156"/>
        <source>Active jobs list columns...</source>
        <translation type="obsolete">Столбцы списка активных заданий...</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="168"/>
        <source>Select finished jobs list columns</source>
        <translation type="obsolete">Выбрать столбцы списка завершенных заданий</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="159"/>
        <source>Ctrl+Shift+P</source>
        <translation type="obsolete">Ctrl+Shift+P</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="160"/>
        <source>Open plugin&apos;s preferences dialog</source>
        <translation type="obsolete">Отрыть диалог настроек плагина</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="167"/>
        <source>Finished jobs list columns...</source>
        <translation type="obsolete">Столбцы списка законченных заданий...</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="100"/>
        <source>Start</source>
        <translation type="obsolete">Запустить</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="157"/>
        <source>Select active jobs list columns</source>
        <translation type="obsolete">Выбрать столбцы списка активных заданий</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="145"/>
        <source>&amp;HTTP/FTP</source>
        <translation>&amp;HTTP/FTP</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="463"/>
        <source>Name: %1, size %2</source>
        <translation>Имя: %1, размер: %2</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="520"/>
        <source>Question</source>
        <translation>Вопрос</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="520"/>
        <source>Do you really want to delete selected jobs?</source>
        <translation>Ты действительно хочешь удалить выбранные задания?</translation>
    </message>
</context>
<context>
    <name>Job</name>
    <message>
        <location filename="job.cpp" line="370"/>
        <source>Could not create directory&lt;br /&gt;&lt;code&gt;%1&lt;/code&gt;&lt;br /&gt;&lt;br /&gt;Stopping work.</source>
        <translation>Не могу создать директорию&lt;br /&gt;&lt;code&gt;%1&lt;/code&gt;&lt;br /&gt;&lt;br /&gt;Останавливаю работу.</translation>
    </message>
    <message>
        <location filename="job.cpp" line="338"/>
        <source>Could not open file for write/append&lt;br /&gt;&lt;code&gt;%1&lt;/code&gt;&lt;br /&gt;&lt;br /&gt;Flushing cache to temp file&lt;br /&gt;&lt;code&gt;%2&lt;/code&gt;&lt;br /&gt;and stopping work.</source>
        <translation type="obsolete">Не могу открыть для записи файл &lt;br /&gt;&lt;code&gt;%1&lt;/code&gt;&lt;br /&gt;&lt;br /&gt;Скидываю кэш в файл&lt;br /&gt;&lt;code&gt;%2&lt;/code&gt;&lt;br /&gt;и останавливаюсь.</translation>
    </message>
    <message>
        <location filename="job.cpp" line="203"/>
        <source>Question.</source>
        <translation>Вопрос.</translation>
    </message>
    <message>
        <location filename="job.cpp" line="203"/>
        <source>File on remote server is newer than local. Should I redownload it from scratch or just leave it alone?</source>
        <translation>Файл на удаленном сервере новее локального. Должен ли я грузить его сначала (отменить в противном случае)?</translation>
    </message>
    <message>
        <location filename="job.cpp" line="235"/>
        <source>File could be neither removed, nor truncated. Check your rights or smth.</source>
        <translation>Файл не может быть ни удален, ни усечен. Проверь права.</translation>
    </message>
    <message>
        <location filename="job.cpp" line="467"/>
        <source>Connection refused</source>
        <translation>Соединение отклонено</translation>
    </message>
    <message>
        <location filename="job.cpp" line="468"/>
        <source>Remote host closed connection</source>
        <translation>Удаленный сервер закрыл соединение</translation>
    </message>
    <message>
        <location filename="job.cpp" line="469"/>
        <source>Host not found</source>
        <translation>Сервер не найден</translation>
    </message>
    <message>
        <location filename="job.cpp" line="470"/>
        <source>Socket access error</source>
        <translation>Ошибка доступа к сокету</translation>
    </message>
    <message>
        <location filename="job.cpp" line="471"/>
        <source>Socker resource error</source>
        <translation>Ошибка ресурса сокета</translation>
    </message>
    <message>
        <location filename="job.cpp" line="472"/>
        <source>Socket timed out</source>
        <translation>Сокет истек временем</translation>
    </message>
    <message>
        <location filename="job.cpp" line="473"/>
        <source>Datagram too large</source>
        <translation>Слишком большая датаграмма</translation>
    </message>
    <message>
        <location filename="job.cpp" line="474"/>
        <source>Network error</source>
        <translation>Сетевая ошибка</translation>
    </message>
    <message>
        <location filename="job.cpp" line="475"/>
        <source>Address already in use</source>
        <translation>Адрес уже используется</translation>
    </message>
    <message>
        <location filename="job.cpp" line="476"/>
        <source>Socket address not available</source>
        <translation>Адрес сокета недоступен</translation>
    </message>
    <message>
        <location filename="job.cpp" line="477"/>
        <source>Unsupported socket operation</source>
        <translation>Неподдерживаемая сокетом операция</translation>
    </message>
    <message>
        <location filename="job.cpp" line="478"/>
        <source>Unfinished socket operation</source>
        <translation>Незаконченная операция с сокетом</translation>
    </message>
    <message>
        <location filename="job.cpp" line="479"/>
        <source>Proxy autentication required</source>
        <translation>Требуется аутентификация на прокси-сервере</translation>
    </message>
    <message>
        <location filename="job.cpp" line="480"/>
        <source>Unknown socket error</source>
        <translation>Неизвестная ошибка сокета</translation>
    </message>
    <message>
        <location filename="job.cpp" line="268"/>
        <source>Warning</source>
        <translation>Предупреждение</translation>
    </message>
    <message>
        <location filename="job.cpp" line="268"/>
        <source>Could not open file %1 for write. Aborting.</source>
        <translation>Не могу открыть файл %1 на запись. Останавливаюсь.</translation>
    </message>
</context>
<context>
    <name>JobAdderDialog</name>
    <message>
        <location filename="jobadderdialog.ui" line="21"/>
        <source>URL:</source>
        <translation>URL:</translation>
    </message>
    <message>
        <location filename="jobadderdialog.ui" line="31"/>
        <source>Local path:</source>
        <translation>Локальный путь:</translation>
    </message>
    <message>
        <location filename="jobadderdialog.ui" line="43"/>
        <source>Browse...</source>
        <translation>Обзор...</translation>
    </message>
    <message>
        <location filename="jobadderdialog.cpp" line="28"/>
        <source>OK</source>
        <translation type="obsolete">OK</translation>
    </message>
    <message>
        <location filename="jobadderdialog.cpp" line="29"/>
        <source>Cancel</source>
        <translation type="obsolete">Отмена</translation>
    </message>
    <message>
        <location filename="jobadderdialog.cpp" line="34"/>
        <source>Autostart</source>
        <translation type="obsolete">Автозапуск</translation>
    </message>
    <message>
        <location filename="jobadderdialog.cpp" line="65"/>
        <source>Select directory</source>
        <translation>Выбрать директорию</translation>
    </message>
    <message>
        <location filename="jobadderdialog.ui" line="35"/>
        <source>Local path</source>
        <translation type="obsolete">Локальный путь</translation>
    </message>
    <message>
        <location filename="jobadderdialog.ui" line="64"/>
        <source>Automatically start the job</source>
        <translation>Автоматически запустить задание</translation>
    </message>
    <message>
        <location filename="jobadderdialog.ui" line="13"/>
        <source>Add a job</source>
        <translation>Добавление задания</translation>
    </message>
    <message>
        <location filename="jobadderdialog.ui" line="52"/>
        <source>File name:</source>
        <translation>Имя файла:</translation>
    </message>
    <message>
        <location filename="jobadderdialog.ui" line="74"/>
        <source>Range download</source>
        <translation>Закачка диапазона</translation>
    </message>
    <message>
        <location filename="jobadderdialog.ui" line="86"/>
        <source>Start position:</source>
        <translation>Начальный байт:</translation>
    </message>
    <message>
        <location filename="jobadderdialog.ui" line="93"/>
        <source>Stop position:</source>
        <translation>Конечный байт:</translation>
    </message>
    <message>
        <location filename="jobadderdialog.ui" line="113"/>
        <source>999999999999999; </source>
        <translation>999999999999999;(sp)</translation>
    </message>
    <message>
        <location filename="jobadderdialog.ui" line="116"/>
        <source>0</source>
        <translation>0</translation>
    </message>
</context>
<context>
    <name>JobManager</name>
    <message>
        <location filename="jobmanager.cpp" line="28"/>
        <source>State</source>
        <translation>Статус</translation>
    </message>
    <message>
        <location filename="jobmanager.cpp" line="28"/>
        <source>Local name</source>
        <translation>Локальное имя</translation>
    </message>
    <message>
        <location filename="jobmanager.cpp" line="28"/>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <location filename="jobmanager.cpp" line="28"/>
        <source>Progress</source>
        <translation>Прогресс</translation>
    </message>
    <message>
        <location filename="jobmanager.cpp" line="28"/>
        <source>Speed</source>
        <translation>Скорость</translation>
    </message>
    <message>
        <location filename="jobmanager.cpp" line="28"/>
        <source>ETA</source>
        <translation>Оставшееся время</translation>
    </message>
    <message>
        <location filename="jobmanager.cpp" line="28"/>
        <source>Download time</source>
        <translation>Время загрузки</translation>
    </message>
    <message>
        <location filename="jobmanager.cpp" line="383"/>
        <source>/s</source>
        <translation>/с</translation>
    </message>
</context>
<context>
    <name>MainViewDelegate</name>
    <message>
        <location filename="mainviewdelegate.cpp" line="47"/>
        <source>/s</source>
        <translation>/с</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="mainwindow.ui" line="13"/>
        <source>HTTP/FTP 0.3</source>
        <translation>HTTP/FTP 0.3</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="49"/>
        <source>State</source>
        <translation type="obsolete">Статус</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="66"/>
        <source>Local name</source>
        <translation>Локальное имя</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="71"/>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="64"/>
        <source>%</source>
        <translation type="obsolete">%</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="69"/>
        <source>Speed</source>
        <translation type="obsolete">Скорость</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="74"/>
        <source>Download time</source>
        <translation type="obsolete">Время загрузки</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="79"/>
        <source>ETA</source>
        <translation type="obsolete">Оставшееся время</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="84"/>
        <source>Downloaded</source>
        <translation type="obsolete">Скачано</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="89"/>
        <source>Total</source>
        <translation type="obsolete">Всего</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="76"/>
        <source>Size</source>
        <translation>Размер</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="81"/>
        <source>Average speed</source>
        <translation>Средняя скорость</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="140"/>
        <source>Time to complete</source>
        <translation type="obsolete">Время до завершения</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="108"/>
        <source>Jobs</source>
        <translation>Задания</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="112"/>
        <source>Active</source>
        <translation>Активные</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="124"/>
        <source>Finished</source>
        <translation>Законченные</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="136"/>
        <source>Tools</source>
        <translation>Сервис</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="149"/>
        <source>Active jobs</source>
        <translation>Активные задания</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="170"/>
        <source>Main toolbar</source>
        <translation>Главный тулбар</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="183"/>
        <source>Finished jobs</source>
        <translation>Законченные задания</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="203"/>
        <source>Add job...</source>
        <translation>Добавить задание...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="214"/>
        <source>Remove job</source>
        <translation>Удалить задание</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="225"/>
        <source>Start</source>
        <translation>Запустить</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="236"/>
        <source>Stop</source>
        <translation>Остановить</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="247"/>
        <source>Start all</source>
        <translation>Запустить все</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="258"/>
        <source>Stop all</source>
        <translation>Остановить все</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="269"/>
        <source>Get file size</source>
        <translation>Получить размер файла</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="280"/>
        <source>Schedule</source>
        <translation>Запланировать</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="288"/>
        <source>Job properties...</source>
        <translation>Свойства задания...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="296"/>
        <source>Remove finished</source>
        <translation>Удалить законченные</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="304"/>
        <source>Autoadjust interface</source>
        <translation>Подстроить интерфейс</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="309"/>
        <source>Active jobs list columns...</source>
        <translation>Столбцы списка активных заданий...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="314"/>
        <source>Finished jobs list columns...</source>
        <translation>Столбцы списка законченных заданий...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="322"/>
        <source>Preferences...</source>
        <translation>Настройки...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="330"/>
        <source>Copy URL to clipboard</source>
        <translation>Скопировать URL в буфер</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="64"/>
        <source>Progress</source>
        <translation type="obsolete">Прогресс</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="86"/>
        <source>TTC</source>
        <translation>Время</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="206"/>
        <source>Ins, Ctrl+S</source>
        <translation>Ins, Ctrl+S</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="217"/>
        <source>Del</source>
        <translation>Del</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="228"/>
        <source>R</source>
        <translation>R</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="239"/>
        <source>S</source>
        <translation>S</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="250"/>
        <source>Shift+R</source>
        <translation>Shift+R</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="261"/>
        <source>Shift+S, Ctrl+S</source>
        <translation>Shift+S, Ctrl+S</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="272"/>
        <source>G</source>
        <translation>G</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="299"/>
        <source>Shift+Del, Ctrl+S</source>
        <translation>Shift+Del, Ctrl+S</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="325"/>
        <source>Ctrl+P</source>
        <translation>Ctrl+P</translation>
    </message>
</context>
<context>
    <name>SettingsManager</name>
    <message>
        <location filename="settingsmanager.cpp" line="34"/>
        <source>Current directory</source>
        <translation type="obsolete">Текущая директория</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="95"/>
        <source>Default download directory</source>
        <translation type="obsolete">Директория для загрузки по умолчанию</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="181"/>
        <source>Local options</source>
        <translation type="obsolete">Локальные опции</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="99"/>
        <source>Max concurrent jobs per server</source>
        <translation type="obsolete">Максимальное количество одновременных скачиваний с сервера</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="144"/>
        <source>Network options</source>
        <translation type="obsolete">Сетевые опции</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="103"/>
        <source>Max total concurrent jobs</source>
        <translation type="obsolete">Максимальное количество одновременных скачиваний вообще</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="107"/>
        <source>Retry timeout</source>
        <translation type="obsolete">Интервал между повторами</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="109"/>
        <source> s</source>
        <translation type="obsolete"> с</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="113"/>
        <source>Connection timeout</source>
        <translation type="obsolete">Таймаут соединения</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="174"/>
        <source> ms</source>
        <translation type="obsolete"> мс</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="119"/>
        <source>Timeout for other operations</source>
        <translation type="obsolete">Таймаут других операций</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="125"/>
        <source>Stop timeout</source>
        <translation type="obsolete">Таймаут остановки</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="131"/>
        <source>Proxy enabled</source>
        <translation type="obsolete">Прокси-сервер включен</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="144"/>
        <source>Proxy</source>
        <translation type="obsolete">Прокси-сервер</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="134"/>
        <source>Proxy address</source>
        <translation type="obsolete">Адрес прокси-сервера</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="138"/>
        <source>Proxy port</source>
        <translation type="obsolete">Порт прокси-сервера</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="147"/>
        <source>Default login</source>
        <translation type="obsolete">Логин по умолчанию</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="140"/>
        <source>FTP options</source>
        <translation type="obsolete">Настройки FTP</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="151"/>
        <source>Default password</source>
        <translation type="obsolete">Пароль по умолчанию</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="155"/>
        <source>Cache size</source>
        <translation type="obsolete">Размер кэша</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="155"/>
        <source>IO</source>
        <translation type="obsolete">Ввод/вывод</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="156"/>
        <source> kb</source>
        <translation type="obsolete"> кб</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="161"/>
        <source>Autostart spawned jobs</source>
        <translation type="obsolete">Автозапуск рожденных заданий</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="167"/>
        <source>Mask as user agent</source>
        <translation type="obsolete">Маскироваться как</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="156"/>
        <source>HTTP options</source>
        <translation type="obsolete">Опции HTTP</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="164"/>
        <source>Get file size on job addition</source>
        <translation type="obsolete">Получать размер файла при добавлении задания</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="172"/>
        <source>Interface update interval</source>
        <translation type="obsolete">Интервал обновления интерфейса</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="181"/>
        <source>Interface</source>
        <translation type="obsolete">Интерфейс</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="178"/>
        <source>Show current speed</source>
        <translation type="obsolete">Показывать текущую скорость</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="170"/>
        <source>Show current time</source>
        <translation type="obsolete">Показывать время до завершения согласно текущей скорости</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="181"/>
        <source>Show estimated time based on current speed</source>
        <translation type="obsolete">Показывать время до завершения согласно текущей скорости</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="151"/>
        <source>FTP</source>
        <translation type="obsolete">FTP</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="167"/>
        <source>HTTP</source>
        <translation type="obsolete">HTTP</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="167"/>
        <source>HTTP/FTP options</source>
        <translation type="obsolete">Опции HTTP/FTP</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="141"/>
        <source>Proxy login</source>
        <translation type="obsolete">Логин прокси-сервера</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="144"/>
        <source>Proxy password</source>
        <translation type="obsolete">Пароль прокси-сервера</translation>
    </message>
</context>
</TS>
