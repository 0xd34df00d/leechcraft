<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS><TS version="1.1">
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
        <translation>/с</translation>
    </message>
</context>
<context>
    <name>FtpImp</name>
    <message>
        <location filename="ftpimp.cpp" line="91"/>
        <source>Wrong login.</source>
        <translation>Неправильный логин.</translation>
    </message>
    <message>
        <location filename="ftpimp.cpp" line="98"/>
        <source>Wrong password.</source>
        <translation>Неправильный пароль.</translation>
    </message>
    <message>
        <location filename="ftpimp.cpp" line="105"/>
        <source>Wrong path, file not found.</source>
        <translation>Неправильный путь, файл не найден.</translation>
    </message>
    <message>
        <location filename="ftpimp.cpp" line="112"/>
        <source>Wrong restart position.</source>
        <translation>Сервер не принял точку старта.</translation>
    </message>
    <message>
        <location filename="ftpimp.cpp" line="119"/>
        <source>PASV failed</source>
        <translation>Команда PASV не выполнена</translation>
    </message>
    <message>
        <location filename="ftpimp.cpp" line="126"/>
        <source>NLST failed.</source>
        <translation>Команда NLST не выполнена.</translation>
    </message>
    <message>
        <location filename="ftpimp.cpp" line="133"/>
        <source>RETR failed.</source>
        <translation>Команда RETR не выполнена.</translation>
    </message>
    <message>
        <location filename="ftpimp.cpp" line="140"/>
        <source>SIZE failed.</source>
        <translation>Команда SIZE не выполнена.</translation>
    </message>
    <message>
        <location filename="ftpimp.cpp" line="147"/>
        <source>CWD failed.</source>
        <translation>Команда CWD не выполнена.</translation>
    </message>
    <message>
        <location filename="ftpimp.cpp" line="154"/>
        <source>TYPE I failed.</source>
        <translation>Команда TYPE I не выполнена.</translation>
    </message>
</context>
<context>
    <name>HttpImp</name>
    <message>
        <location filename="httpimp.cpp" line="243"/>
        <source>400 Bad request. The request contains bad syntax or cannot be fulfilled.</source>
        <translation>400 Плохой запрос. Запрос не понят сервером из-за наличия синтаксической ошибки. Клиенту следует повторно обратиться к ресурсу с изменённым запросом.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="246"/>
        <source>401 Unauthorized. Authentication is possible but has failed or not yet been provided.</source>
        <translation>401 Неавторизован. Запрос требует идентификации пользователя. Клиент должен запросить имя и пароль у пользователя и передать их в записи WWW-Authenticate заголовка в следующем запросе. В случае ввода ошибочных данных сервер снова вернёт этот же статус.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="249"/>
        <source>403 Forbidden. The request was legal, but server is refusing to respond to it. Authenticating will make no difference.</source>
        <translation>403 Запрещено. Сервер понял запрос, но он отказывается его выполнять из-за каких-то ограничений в доступе. Идентификация через протокол HTTP здесь не поможет. Скорее всего на сервере нужно провести аутентификацию другим способом, сделать запрос с определёнными параметрами или удовлетворить каким-либо условиям.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="252"/>
        <source>404 Resource not found.</source>
        <translation>404 Ресурс не найден. Сервер понял запрос, но не нашёл соответствующего ресурса по указанному URI. Если серверу известно что по этому адресу был документ, то ему желательно использовать код 410 вместо этого. Этот код может использоваться вместо 403 если требуется тщательно скрыть от посторонних глаз определённые ресурсы.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="255"/>
        <source>405 Method not allowed. Request method not supported by the URL.</source>
        <translation>405 Метод не поддерживается. Указанный клиентом метод нельзя применить к ресурсу. Сервер так же должен передать в заголовке ответа поле Allow со списком доступных методов.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="258"/>
        <source>406 Not acceptable.</source>
        <translation>406 Не приемлимо. Запрошенный URI не может удовлетворить переданным в заголовке характеристикам. Если метод был не HEAD, то сервер должен вернуть список допустимых характеристик для данного ресурса.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="261"/>
        <source>407 Proxy authentication required.</source>
        <translation>407 Необходима авторизация на прокси-сервере. Ответ аналогичен коду 401 за исключением того, что аутентификация производится для прокси-сервера. Механизм аналогичен идентификации на обычном сервере.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="264"/>
        <source>408 Request timeout.</source>
        <translation>408 Время ожидания истекло. Время ожидания сервером передачи от клиента истекло. Клиент может повторить аналогичный предыдущему запрос в любое время.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="267"/>
        <source>409 Conflict.</source>
        <translation>409 Конфликт. Запрос не может выполнен из-за конфликтного обращения к ресурсу. Такое возможно, например, когда два клиента пытаются изменить ресурс с помощью метода PUT.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="270"/>
        <source>410 Gone. Resource is not available and will not be available again. Maybe it was intentionally removed.</source>
        <translation>410 Удален. Такой ответ сервер посылает когда ресурс раньше был по указанному URI, но был удалён и теперь не доступен. Серверу в этом случае не известно и местоположение альтернативного документа (например, копии). Если у сервера есть подозрение что документ в ближайшее время может быть восстановлен, то лучше клиенту передать код 404.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="273"/>
        <source>411 Length required.</source>
        <translation>411 Необходима длина. Для указанного ресурса клиент должен указать Content-Length в заголовке запроса. Без указания этого поля не стоит делать повторную попытку запроса к серверу по данному URI.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="276"/>
        <source>412 Precondition failed.</source>
        <translation>412 Предварительное условие не удовлетворено. Возвращается если ни одно из условных полей заголовка запроса не было выполнено.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="279"/>
        <source>413 Request entity too large.</source>
        <translation>413 Запрашиваемые данные слишком большие. Возвращается если сервер по каким-то причинам не может передать запрашиваемый объём информации. Если проблема временная, то сервер может в ответе указать в поле Retry-After через которое можно повторить аналогичный запрос.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="282"/>
        <source>414 Request URI too long.</source>
        <translation>414 Запрашиваемый URI слишком длинный. Сервер не может обработать запрос из-за слишком длинного указанного URI. Такую ошибку можно спровоцировать, например, когда клиент пытается передать длинные параметры через метод GET, а не POST.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="285"/>
        <source>415 Unsupported media type.</source>
        <translation>415 Неподдерживаемый тип данных. По каким-то причинам сервер отказывается работать с указанным типом данных при данном методе.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="288"/>
        <source>417 Expectation failed.</source>
        <translation>417 Ожидаемое ошибочно. По каким-то причинам сервер не может удовлетворить значению поля Except заголовка запроса.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="291"/>
        <source>422 Unprocessable entity (WebDAV). The request was well-formed but was unable to be followed due to semantic errors.</source>
        <translation>422 Необрабатываемый экземпляр (WebDAV). Сервер успешно принял запрос, может работать с указанным видом данных, в теле запроса XML-документ имеет верный синтаксис, но имеется какая-то логическая ошибка из-за которой не возможно произвести операцию над ресурсом.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="294"/>
        <source>423 Locked (WebDAV). The resource that is being accessed is locked.</source>
        <translation>423 Заблокировано (WebDAV). Целевой ресурс из запроса заблокирован от применения к нему указанного метода.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="297"/>
        <source>424 Failed dependency (WebDAV). The request failed due to failure of a previous request.</source>
        <translation>424 Неудовлетворенная зависимость (WebDAV). Реализация текущего запроса может зависеть от успешности выполнения другой операции. Если она провалена и из-за этого нельзя выполнить текущий запрос, то сервер вернёт код 424.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="300"/>
        <source>425 Unordered collection (WebDAV). You really never should see this message.</source>
        <translation>425 Неупорядоченная коллекция (WebDAV). Ты не должен видеть это сообщение, так как этот код ответа нигде не применяется.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="303"/>
        <source>426 Upgrade required. The client should switch to TLS/1.0.</source>
        <translation>426 Необходимо обновление. Сервер указывает клиенту на необходимость обновить протокол (TLS/1.0). Заголовок ответа должен содержать правильно сформированные поля Upgrade и Connection.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="306"/>
        <source>449 Retry with. A Microsoft extension: The request should be retried after doing the appropriate action.</source>
        <translation>449 Попробуй снова. Мелкомягкое расширение: запрос должен быть повторен после выполнения соответствующих действий.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="309"/>
        <source>500 Internal server error. Server failed to fulfil the request due to misconfiguration.</source>
        <translation>500 Внутренняя ошибка сервера. Серверу не удалось выполнить запрос из-за внутренней ошибки, не входящей в другие класса 5xx.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="312"/>
        <source>501 Not implemented.</source>
        <translation>501 Не реализовано. Сервер не поддерживает возможностей, необходимых для обработки запроса. Типичный ответ для случаев, когда сервер не понимает указанный в запросе метод.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="315"/>
        <source>502 Bad gateway.</source>
        <translation>502 Плохой шлюз. Сервер в роли шлюза или прокси получил сообщение о неудачном выполнении промежуточной операции.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="318"/>
        <source>503 Service unavailable.</source>
        <translation>503 Сервис недоступен. Сервер временно не имеет возможности обрабатывать запросы по техническим причинам (обслуживание, перегрузка и прочее). В поле Retry-After заголовка сервер может указать время, через которое клиенту рекомендуется повторить запрос. Хотя во время перегрузки очевидным является сразу разрывать соединение, эффективней может оказаться установка большого значения поля Retry-After для уменьшения частоты избыточных запросов.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="321"/>
        <source>504 Gateway timeout.</source>
        <translation>504 Шлюз не отвечает. Сервер в роли шлюза или прокси не дождался ответа от вышестоящего сервера для завершения текущего запроса.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="324"/>
        <source>505 HTTP version not supported.</source>
        <translation>505 Версия HTTP не поддерживается. Сервер не поддерживает или отказывается поддерживать указанную в запросе версию протокола HTTP.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="327"/>
        <source>506 Variant also negotiates.</source>
        <translation>506 Вариант тоже согласован. В результате ошибочной конфигурации выбранный вариант указывает сам на себя, из-за чего процесс связывания прерывается.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="330"/>
        <source>507 Insufficient storage (WebDAV).</source>
        <translation>507 Не хватает места (WebDAV). Не хватает места для выполнения текущего запроса. Проблема может быть временной.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="333"/>
        <source>509 Bandwidth limit exceeded.</source>
        <translation>509 Превышен предел полосы пропускания. Неофициальный код, тем не менее, довольно часто используемый. Используется для указания израсходованной полосы пропускания.</translation>
    </message>
    <message>
        <location filename="httpimp.cpp" line="336"/>
        <source>510 Not extented.</source>
        <translation>510 Не расширено. На сервере отсутствует расширение, которое планирует использовать клиент. Сервер может дополнительно передать информацию о доступных ему расширениях.</translation>
    </message>
</context>
<context>
    <name>HttpPlugin</name>
    <message>
        <location filename="httpplugin.cpp" line="47"/>
        <source>HTTP/FTP worker 0.2</source>
        <translation>Скачиватель по HTTP/FTP 0.2</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="82"/>
        <source>&amp;Tools</source>
        <translation>&amp;Инструменты</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="81"/>
        <source>&amp;Jobs</source>
        <translation>&amp;Задания</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="93"/>
        <source>Add...</source>
        <translation>Добавить задание...</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="95"/>
        <source>Add a new job</source>
        <translation>Добавить новое задание задание</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="97"/>
        <source>Start current</source>
        <translation type="obsolete">Запустить выделенное</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="101"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="103"/>
        <source>Start all</source>
        <translation>Запустить все</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="104"/>
        <source>Ctrl+Shift+S</source>
        <translation>Ctrl+Shift+S</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="101"/>
        <source>Stop current</source>
        <translation type="obsolete">Остановить выделенное</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="107"/>
        <source>Ctrl+I</source>
        <translation>Ctrl+I</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="109"/>
        <source>Stop all</source>
        <translation>Остановить все</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="110"/>
        <source>Ctrl+Shift+I</source>
        <translation>Ctrl+Shift+I</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="108"/>
        <source>Stop selected job(s)</source>
        <translation>Запланировать выбранные задания</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="154"/>
        <source>Autoadjust interface</source>
        <translation>Подстроить интерфейс</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="158"/>
        <source>Preferences...</source>
        <translation>Настройки...</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="119"/>
        <source>Ctrl+P</source>
        <translation>Ctrl+P</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="199"/>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="224"/>
        <source>Local name</source>
        <translation>Локальное имя</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="224"/>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="199"/>
        <source>Speed</source>
        <translation>Скорость</translation>
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
        <translation>Размер</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="433"/>
        <source>0</source>
        <translation>0</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="582"/>
        <source>: Preferences</source>
        <translation>: Настройки</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="587"/>
        <source>Job error</source>
        <translation>Ошибка задания</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="568"/>
        <source>Job with URL %1 signals about following error:&lt;br /&gt;&lt;code&gt;%2&lt;/code&gt;</source>
        <translation type="obsolete">Задание с URL %1 говорит о следующей ошибке:&lt;br /&gt;&lt;code&gt;%2&lt;/code&gt;</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="605"/>
        <source>/s</source>
        <translation>/с</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="249"/>
        <source>Simple HTTP and FTP plugin, providing basic functionality.</source>
        <translation>Простой HTTP/FTP плагин, обеспечивающий базовую функциональность.</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="112"/>
        <source>Get file size</source>
        <translation>Получить размер файла</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="132"/>
        <source>Copy URL to clipboard</source>
        <translation>Скопировать URL в буфер</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="199"/>
        <source>State</source>
        <translation>Статус</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="199"/>
        <source>Remaining time</source>
        <translation>Оставшееся время</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="199"/>
        <source>Downloaded</source>
        <translation>Скачано</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="199"/>
        <source>Total</source>
        <translation>Размер</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="199"/>
        <source>Download time</source>
        <translation>Время загрузки</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="224"/>
        <source>Average speed</source>
        <translation>Средняя скорость</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="224"/>
        <source>Time to complete</source>
        <translation>Время до завершения</translation>
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
        <location filename="httpplugin.cpp" line="587"/>
        <source>Job with URL&lt;br /&gt;%1&lt;br /&gt;signals about following error:&lt;br /&gt;&lt;br /&gt;&lt;em&gt;%2&lt;/em&gt;</source>
        <translation>Задание с URL&lt;br /&gt;%1&lt;br /&gt;сообщает об ошибке:&lt;br /&gt;&lt;em&gt;%2&lt;/em&gt;</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="121"/>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="98"/>
        <source>Delete selected job(s)</source>
        <translation>Удалить выбранное задание(ия)</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="102"/>
        <source>Start selected job(s)</source>
        <translation>Запустить выбранное задание(ия)</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="105"/>
        <source>Start all jobs</source>
        <translation>Запустить все задания</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="106"/>
        <source>Stop</source>
        <translation>Остановить</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="111"/>
        <source>Stop all jobs</source>
        <translation>Остановить все задания</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="113"/>
        <source>Get file size for selected jobs without downloading them</source>
        <translation>Получить размер файла выбранных заданий без их загрузки</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="115"/>
        <source>Schedule</source>
        <translation>Запланировать</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="116"/>
        <source>Schedule select job(s)</source>
        <translation>Запланировать выбранное задание(ия)</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="118"/>
        <source>Job properties...</source>
        <translation>Свойства задания...</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="120"/>
        <source>Change selected job&apos;s properties</source>
        <translation>Изменить свойства выбранного задания</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="123"/>
        <source>Delete selected finished job(s)</source>
        <translation>Удалить выбранное законченное задание(ия)</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="155"/>
        <source>Adjust interface to make all text in columns fit</source>
        <translation>Подстроить интерфейс, чтобы текст во всех столбцах был полностью виден</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="156"/>
        <source>Active jobs list columns...</source>
        <translation>Столбцы списка активных заданий...</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="168"/>
        <source>Select finished jobs list columns</source>
        <translation>Выбрать столбцы списка завершенных заданий</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="159"/>
        <source>Ctrl+Shift+P</source>
        <translation>Ctrl+Shift+P</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="160"/>
        <source>Open plugin&apos;s preferences dialog</source>
        <translation>Отрыть диалог настроек плагина</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="167"/>
        <source>Finished jobs list columns...</source>
        <translation>Столбцы списка законченных заданий...</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="100"/>
        <source>Start</source>
        <translation>Запустить</translation>
    </message>
    <message>
        <location filename="httpplugin.cpp" line="157"/>
        <source>Select active jobs list columns</source>
        <translation>Выбрать столбцы списка активных заданий</translation>
    </message>
</context>
<context>
    <name>Job</name>
    <message>
        <location filename="job.cpp" line="262"/>
        <source>Could not create directory&lt;br /&gt;&lt;code&gt;%1&lt;/code&gt;&lt;br /&gt;&lt;br /&gt;Stopping work.</source>
        <translation>Не могу создать директорию&lt;br /&gt;&lt;code&gt;%1&lt;/code&gt;&lt;br /&gt;&lt;br /&gt;Останавливаюсь.</translation>
    </message>
    <message>
        <location filename="job.cpp" line="330"/>
        <source>Could not open file for write/append&lt;br /&gt;&lt;code&gt;%1&lt;/code&gt;&lt;br /&gt;&lt;br /&gt;Flushing cache to temp file&lt;br /&gt;&lt;code&gt;%2&lt;/code&gt;&lt;br /&gt;and stopping work.</source>
        <translation>Не могу открыть для записи файл &lt;br /&gt;&lt;code&gt;%1&lt;/code&gt;&lt;br /&gt;&lt;br /&gt;Скидываю кэш в файл&lt;br /&gt;&lt;code&gt;%2&lt;/code&gt;&lt;br /&gt;и останавливаюсь.</translation>
    </message>
    <message>
        <location filename="job.cpp" line="336"/>
        <source>Could not open temporary file for write&lt;br /&gt;&lt;code&gt;%1&lt;/code&gt;</source>
        <translation>Не могу открыть для записи временный файл&lt;br /&gt;&lt;code&gt;%1&lt;/code&gt;</translation>
    </message>
    <message>
        <location filename="job.cpp" line="167"/>
        <source>Question.</source>
        <translation>Вопрос.</translation>
    </message>
    <message>
        <location filename="job.cpp" line="167"/>
        <source>File on remote server is newer than local. Should I redownload it from scratch or just leave it alone?</source>
        <translation>Файл на удаленном сервере новее локального. Должен ли я грузить его сначала (отменить в противном случае)?</translation>
    </message>
    <message>
        <location filename="job.cpp" line="198"/>
        <source>File could be neither removed, nor truncated. Check your rights or smth.</source>
        <translation>Файл не может быть ни удален, ни усечен. Проверь права.</translation>
    </message>
</context>
<context>
    <name>JobAdderDialog</name>
    <message>
        <location filename="jobadderdialog.cpp" line="10"/>
        <source>URL:</source>
        <translation type="obsolete">URL:</translation>
    </message>
    <message>
        <location filename="jobadderdialog.cpp" line="11"/>
        <source>Local path:</source>
        <translation type="obsolete">Локальный путь:</translation>
    </message>
    <message>
        <location filename="jobadderdialog.ui" line="45"/>
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
        <location filename="jobadderdialog.cpp" line="43"/>
        <source>Select directory</source>
        <translation>Выбрать директорию</translation>
    </message>
    <message>
        <location filename="jobadderdialog.ui" line="21"/>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <location filename="jobadderdialog.ui" line="35"/>
        <source>Local path</source>
        <translation>Локальный путь</translation>
    </message>
    <message>
        <location filename="jobadderdialog.ui" line="54"/>
        <source>Automatically start the job</source>
        <translation>Автоматически запустить задание</translation>
    </message>
    <message>
        <location filename="jobadderdialog.ui" line="13"/>
        <source>Add a job</source>
        <translation>Добавление задания</translation>
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
        <location filename="settingsmanager.cpp" line="94"/>
        <source>Default download directory</source>
        <translation>Директория для загрузки по умолчанию</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="180"/>
        <source>Local options</source>
        <translation>Локальные опции</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="98"/>
        <source>Max concurrent jobs per server</source>
        <translation>Максимальное количество одновременных скачиваний с сервера</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="143"/>
        <source>Network options</source>
        <translation>Сетевые опции</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="102"/>
        <source>Max total concurrent jobs</source>
        <translation>Максимальное количество одновременных скачиваний вообще</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="106"/>
        <source>Retry timeout</source>
        <translation>Интервал между повторами</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="108"/>
        <source> s</source>
        <translation> с</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="112"/>
        <source>Connection timeout</source>
        <translation>Таймаут соединения</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="173"/>
        <source> ms</source>
        <translation> мс</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="118"/>
        <source>Timeout for other operations</source>
        <translation>Таймаут других операций</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="124"/>
        <source>Stop timeout</source>
        <translation>Таймаут остановки</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="130"/>
        <source>Proxy enabled</source>
        <translation>Прокси-сервер включен</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="143"/>
        <source>Proxy</source>
        <translation>Прокси-сервер</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="133"/>
        <source>Proxy address</source>
        <translation>Адрес прокси-сервера</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="137"/>
        <source>Proxy port</source>
        <translation>Порт прокси-сервера</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="146"/>
        <source>Default login</source>
        <translation>Логин по умолчанию</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="140"/>
        <source>FTP options</source>
        <translation type="obsolete">Настройки FTP</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="150"/>
        <source>Default password</source>
        <translation>Пароль по умолчанию</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="154"/>
        <source>Cache size</source>
        <translation>Размер кэша</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="154"/>
        <source>IO</source>
        <translation>Ввод/вывод</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="155"/>
        <source> kb</source>
        <translation> кб</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="160"/>
        <source>Autostart spawned jobs</source>
        <translation>Автозапуск рожденных заданий</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="166"/>
        <source>Mask as user agent</source>
        <translation>Маскироваться как</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="156"/>
        <source>HTTP options</source>
        <translation type="obsolete">Опции HTTP</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="163"/>
        <source>Get file size on job addition</source>
        <translation>Получать размер файла при добавлении задания</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="171"/>
        <source>Interface update interval</source>
        <translation>Интервал обновления интерфейса</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="180"/>
        <source>Interface</source>
        <translation>Интерфейс</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="177"/>
        <source>Show current speed</source>
        <translation>Показывать текущую скорость</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="170"/>
        <source>Show current time</source>
        <translation type="obsolete">Показывать время до завершения согласно текущей скорости</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="180"/>
        <source>Show estimated time based on current speed</source>
        <translation>Показывать время до завершения согласно текущей скорости</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="150"/>
        <source>FTP</source>
        <translation>FTP</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="166"/>
        <source>HTTP</source>
        <translation>HTTP</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="166"/>
        <source>HTTP/FTP options</source>
        <translation>Опции HTTP/FTP</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="140"/>
        <source>Proxy login</source>
        <translation>Логин прокси-сервера</translation>
    </message>
    <message>
        <location filename="settingsmanager.cpp" line="143"/>
        <source>Proxy password</source>
        <translation>Пароль прокси-сервера</translation>
    </message>
</context>
</TS>
