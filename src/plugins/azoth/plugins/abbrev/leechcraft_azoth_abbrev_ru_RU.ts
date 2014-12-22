<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="ru_RU">
<context>
    <name>LeechCraft::Azoth::Abbrev::AbbrevsManager</name>
    <message>
        <location filename="abbrevsmanager.cpp" line="52"/>
        <source>Abbreviation with this pattern already exists.</source>
        <translation>Сокращение с этим шаблоном уже существует.</translation>
    </message>
    <message>
        <location filename="abbrevsmanager.cpp" line="55"/>
        <source>Abbeviation pattern is empty.</source>
        <translation>Шаблон сокращения пуст.</translation>
    </message>
    <message>
        <location filename="abbrevsmanager.cpp" line="58"/>
        <source>Abbeviation expansion is empty.</source>
        <translation>Расширение сокращения пусто.</translation>
    </message>
    <message>
        <location filename="abbrevsmanager.cpp" line="123"/>
        <source>Too much expansions during abbreviations application. Check your rules.</source>
        <translation>Слишком много расширений при применении сокращений. Проверьте ваши правила.</translation>
    </message>
</context>
<context>
    <name>LeechCraft::Azoth::Abbrev::Plugin</name>
    <message>
        <location filename="abbrev.cpp" line="69"/>
        <source>Adds a new abbreviation to the list of abbreviations.</source>
        <translation>Добавляет новое сокращение.</translation>
    </message>
    <message>
        <location filename="abbrev.cpp" line="83"/>
        <source>Lists all abbreviations that were previously added.</source>
        <translation>Отображает все ранее добавленные сокращения.</translation>
    </message>
    <message>
        <location filename="abbrev.cpp" line="99"/>
        <source>Removes a previously added abbreviation.</source>
        <translation>Удаляет ранее добавленное сокращение.</translation>
    </message>
    <message>
        <location filename="abbrev.cpp" line="70"/>
        <source>Usage: @/abbrev@ _pattern_ _text_

Adds a new _pattern_ that expands to the given _text_, which can span multiple lines.

@/listabbrevs@ lists all available abbreviations and @/unabbrev@ allows removing them.</source>
        <translation>Использование: @/abbrev@ _шаблон_ _текст_

Добавляет новый _шаблон_, который разворачивается в данный _текст_. _текст_ может быть многострочным..

@/listabbrevs@ перечисляет все доступные сокращения, а @/unabbrev@ позволяет их удалять.</translation>
    </message>
    <message>
        <location filename="abbrev.cpp" line="65"/>
        <source>Pattern %1 has been added successfully.</source>
        <translation>Шаблон %1 был успешно добавлен.</translation>
    </message>
    <message>
        <location filename="abbrev.cpp" line="95"/>
        <source>Pattern %1 has been removed successfully.</source>
        <translation>Шаблон %1 был успешно удалён.</translation>
    </message>
    <message>
        <location filename="abbrev.cpp" line="100"/>
        <source>Usage: @/unabbrev@ &lt;_pattern_|_index_&gt;

Removes a previously added abbrevation either by its _pattern_ or by its _index_ in the list returned by @/listabbrevs@.</source>
        <translation>Использование: @/unabbrev@ &lt;_шаблон_|_индекс_&gt;

Удаляет ранее добавленное сокращение либо по соответствующему _шаблон_у, либо по _индекс_у в выдаче команды @/listabbrevs@.</translation>
    </message>
    <message>
        <location filename="abbrev.cpp" line="129"/>
        <source>Provides support for automatically expanding abbreviations for Azoth.</source>
        <translation>Обеспечивает поддержку автоматически разворачиваемых сокращений для Azoth.</translation>
    </message>
    <message numerus="yes">
        <location filename="abbrev.cpp" line="167"/>
        <source>%n abbreviation(s):</source>
        <translation>
            <numerusform>%n сокращение:</numerusform>
            <numerusform>%n сокращения:</numerusform>
            <numerusform>%n сокращений:</numerusform>
        </translation>
    </message>
    <message>
        <location filename="abbrev.cpp" line="195"/>
        <source>Unable to find abbreviation %1.</source>
        <translation>Невозможно найти сокращение %1.</translation>
    </message>
</context>
<context>
    <name>LeechCraft::Azoth::Abbrev::ShortcutsManager</name>
    <message>
        <location filename="shortcutsmanager.cpp" line="76"/>
        <source>Expand abbreviations in current message edit text.</source>
        <translation>Развернуть сокращения в текущем тексте редактора сообщения.</translation>
    </message>
</context>
</TS>
