<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="ru_RU">
<context>
    <name>LeechCraft::Azoth::OTRoid::Authenticator</name>
    <message>
        <location filename="authenticator.cpp" line="61"/>
        <source>%1 (%2) wants to authenticate with you via a question. The question is:</source>
        <translation>%1 (%2) хочет аутентифицироваться при помощи вопроса:</translation>
    </message>
    <message>
        <location filename="authenticator.cpp" line="67"/>
        <source>%1 (%2) wants to authenticate with you via a shared secret.</source>
        <translation>%1 (%2) хочет аутентифицироваться при помощи общего секрета.</translation>
    </message>
    <message>
        <location filename="authenticator.cpp" line="74"/>
        <location filename="authenticator.cpp" line="101"/>
        <location filename="authenticator.cpp" line="110"/>
        <location filename="authenticator.cpp" line="119"/>
        <source>OTR authentication</source>
        <translation>Аутентификация OTR</translation>
    </message>
    <message>
        <location filename="authenticator.cpp" line="102"/>
        <source>Failed to authenticate %1 (%2).</source>
        <translation>Не удалось аутентифицировать %1 (%2).</translation>
    </message>
    <message>
        <location filename="authenticator.cpp" line="111"/>
        <source>Failed to authenticate %1 (%2): cheating detected.</source>
        <translation>Не удалось аутентифицировать %1 (%2): обнаружен обман.</translation>
    </message>
    <message>
        <location filename="authenticator.cpp" line="120"/>
        <source>Congratulations! Contact %1 (%2) authenticated successfully!</source>
        <translation>Поздравляем! Контакт %1 (%2) успешно аутентифицирован!</translation>
    </message>
</context>
<context>
    <name>LeechCraft::Azoth::OTRoid::FPManager</name>
    <message>
        <location filename="fpmanager.cpp" line="195"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="fpmanager.cpp" line="195"/>
        <source>Entry ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="fpmanager.cpp" line="195"/>
        <source>Keys count</source>
        <translation>Количество ключей</translation>
    </message>
    <message>
        <location filename="fpmanager.cpp" line="252"/>
        <source>Confirm fingerprints deletion</source>
        <translation>Подтверждение удаления отпечатков</translation>
    </message>
    <message numerus="yes">
        <location filename="fpmanager.cpp" line="253"/>
        <source>Are you sure you want to delete %n fingerprint(s)?</source>
        <translation>
            <numerusform>Вы уверены, что хотите удалить %n отпечаток?</numerusform>
            <numerusform>Вы уверены, что хотите удалить %n отпечатка?</numerusform>
            <numerusform>Вы уверены, что хотите удалить %n отпечатков?</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>LeechCraft::Azoth::OTRoid::InitiateAuthDialog</name>
    <message>
        <location filename="initiateauthdialog.cpp" line="47"/>
        <source>Choose authentication method for %1 (%2):</source>
        <translation>Выберите метод аутентификации для %1 (%2):</translation>
    </message>
</context>
<context>
    <name>LeechCraft::Azoth::OTRoid::OtrHandler</name>
    <message>
        <location filename="otrhandler.cpp" line="140"/>
        <source>The following message received from %1 was not encrypted:</source>
        <translation>Следующее сообщение, полученное от %1, было незашифровано:</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="144"/>
        <source>Your message was not sent. Either end your private conversation, or restart it.</source>
        <translation>Ваше сообщение не было отправлено. Либо завершите вашу защищённую беседу, либо начните его снова.</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="148"/>
        <source>Unreadable encrypted message was received.</source>
        <translation>Получено нечитаемое зашифрованное сообщение.</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="151"/>
        <source>Received an encrypted message but it cannot be read because no private connection is established yet.</source>
        <translation>Получено зашифрованное сообщение, но оно не может быть прочитано, так как защищённая беседа ещё не установлена.</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="156"/>
        <source>Received message is unreadable.</source>
        <translation>Полученное сообщение нечитаемо.</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="159"/>
        <source>Received message contains malformed data.</source>
        <translation>Полученное сообщение содержит некорректные данные.</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="162"/>
        <source>OTR encryption error, the message has not been sent.</source>
        <translation>Ошибка шифрования OTR, сообщение не было отправлено.</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="165"/>
        <source>Trying to send unencrypted message while our policy requires OTR encryption.</source>
        <translation>Попытка отправить незашифрованное сообщение, хотя наша политика требует шифрования OTR.</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="168"/>
        <source>Private conversation could not be set up. Error %1, source %2.</source>
        <translation>Защищённая беседа не может быть установлена. Ошибка %1, источник %2.</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="173"/>
        <source>Received our own OTR message.</source>
        <translation>Получено наше сообственное OTR-сообщение.</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="176"/>
        <source>The previous message has been resent.</source>
        <translation>Предыдущее сообщение было повторно отправлено.</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="179"/>
        <source>Received (and discarded) message for other client instance.</source>
        <translation>Получено (и проигнорировано) сообщение для другого клиента.</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="182"/>
        <source>Received general OTR error.</source>
        <translation>Общая ошибка OTR.</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="193"/>
        <source>Original OTR message: %1.</source>
        <translation>Исходное сообщение OTR: %1.</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="219"/>
        <source>You have received a new fingerprint from %1: %2</source>
        <translation>Вы получили новый отпечаток от %1: %2</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="229"/>
        <source>Private conversation started</source>
        <translation>Защищённая беседа начата</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="238"/>
        <source>Private conversation lost</source>
        <translation>Защищённая беседа потеряна</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="457"/>
        <source>%1 has ended the private conversation with you, you should do the same.</source>
        <translation>%1 завершил(а) защищённую беседу с вами, вы должны сделать то же.</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="644"/>
        <source>Private keys for account %1 need to be generated. This takes quite some time (from a few seconds to a couple of minutes), and while you can use LeechCraft in the meantime, all the messages will be sent unencrypted until keys are generated. You will be notified when this process finishes. Do you want to generate keys now?&lt;br /&gt;&lt;br /&gt;You can also move mouse randomily to help generating entropy.</source>
        <translation>Необходимо создать приватные ключи для учётной записи %1. Это может занять некоторое время (от нескольких секунд до нескольких минут), и, хотя вы можете использовать LeechCraft в это время, все исходящие сообщения не будут зашифрованы, пока ключи не будут созданы. Вы получите уведомление, когда этот процесс завершится. Вы хотите создать ключи сейчас?&lt;br/&gt;&lt;br/&gt;Вы также можете случайно двигать мышкой, чтобы помочь создать побольше энтропии.</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="655"/>
        <source>Keys for account %1 are now being generated...</source>
        <translation>Создаются ключи для учётной записи %1...</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="690"/>
        <source>Keys are generated. Thanks for your patience.</source>
        <translation>Ключи созданы. Спасибо за терпение.</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="813"/>
        <source>Enable OTR</source>
        <translation>Включить OTR</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="846"/>
        <source>OTR</source>
        <translation>OTR</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="851"/>
        <source>Authenticate the contact</source>
        <translation>Аутентифицировать контакт</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="895"/>
        <source>Private conversation closed</source>
        <translation>Защищённая беседа завершена</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="902"/>
        <source>Attempting to start a private conversation</source>
        <translation>Попытка начать защищённую беседу</translation>
    </message>
    <message>
        <location filename="otrhandler.cpp" line="929"/>
        <source>You need to start a private conversation before authentication can take place. Do you want to start it?</source>
        <translation>Вам необходимо начать защищённую беседу перед аутентификацией. Вы хотите её начать?</translation>
    </message>
</context>
<context>
    <name>LeechCraft::Azoth::OTRoid::Plugin</name>
    <message>
        <location filename="otroid.cpp" line="91"/>
        <source>Azoth OTRoid adds support for Off-the-Record deniable encryption system.</source>
        <translation>Azoth OTRoid добавляет поддержку системы шифрования Off-the-Record.</translation>
    </message>
</context>
<context>
    <name>LeechCraft::Azoth::OTRoid::PrivKeyManager</name>
    <message>
        <location filename="privkeymanager.cpp" line="75"/>
        <source>Private keys generation</source>
        <translation>Создание личных ключей</translation>
    </message>
    <message>
        <location filename="privkeymanager.cpp" line="76"/>
        <source>Account %1 already has a private key, do you want to generate a new one?</source>
        <translation>Учётная запись %1 уже имеет личный ключ, вы хотите создать новый?</translation>
    </message>
    <message>
        <location filename="privkeymanager.cpp" line="87"/>
        <source>Account</source>
        <translation>Учётная запись</translation>
    </message>
    <message>
        <location filename="privkeymanager.cpp" line="87"/>
        <source>Private key</source>
        <translation>Личный ключ</translation>
    </message>
    <message>
        <location filename="privkeymanager.cpp" line="146"/>
        <source>Confirm private keys deletion</source>
        <translation>Подтверждение удаления личных ключей</translation>
    </message>
    <message numerus="yes">
        <location filename="privkeymanager.cpp" line="147"/>
        <source>Are you sure you want to delete %n private key(s)?</source>
        <translation>
            <numerusform>Вы уверены, что хотите удалить %n личный ключ?</numerusform>
            <numerusform>Вы уверены, что хотите удалить %n личных ключа?</numerusform>
            <numerusform>Вы уверены, что хотите удалить %n личных ключей?</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="otrhandler.cpp" line="247"/>
        <source>Private conversation refreshed</source>
        <translation>Защищённая беседа обновлена</translation>
    </message>
</context>
<context>
    <name>azothotroidsettings</name>
    <message>
        <location filename="dummy.cpp" line="2"/>
        <source>Azoth OTRoid</source>
        <translation>Azoth OTRoid</translation>
    </message>
    <message>
        <location filename="dummy.cpp" line="3"/>
        <source>Known fingerprints</source>
        <translation>Известные отпечатки</translation>
    </message>
    <message>
        <location filename="dummy.cpp" line="4"/>
        <location filename="dummy.cpp" line="7"/>
        <source>Refresh</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <location filename="dummy.cpp" line="5"/>
        <source>Private keys</source>
        <translation>Личные ключи</translation>
    </message>
    <message>
        <location filename="dummy.cpp" line="6"/>
        <source>Generate key</source>
        <translation>Создать ключ</translation>
    </message>
</context>
</TS>
