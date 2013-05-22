#!/usr/bin/env bash

replacing="0"
project_dir=$(cd ../../ && pwd)

while getopts ":p::i::r" opt; do
	case $opt in
		p)
			plugin_path=$OPTARG
			;;
		i)
			icon_path=$OPTARG
			;;
		r)
			replacing="1"
			;;
		\?)
			echo "Invalid option: "$OPTARG
			exit 1
			;;
		:)
			echo "Option $OPTARG requires an argument."
			exit 1
			;;
	esac
done

function get_full_plugin_path()
{
	full_plugin_path=$project_dir"/src/plugins/"$(echo $plugin_path | sed 's/:/\/plugins\//g')
	echo $full_plugin_path
}

function get_plugin_name()
{
	plugin_name=$(echo $plugin_path | awk -F : '{print $NF}')
	echo $plugin_name
}

function generate_qrc()
{
	qrc_path=$(get_full_plugin_path)"/"$(get_plugin_name)"resources.qrc"
	if [ ! -f $qrc_path ]; then
		echo -e "<RCC>\n  <qresource prefix=\"/"$(echo $plugin_path | sed 's/:/\//')"\" >\n    <file>resources/images/"$(get_plugin_name)".svg</file>\n  </qresource>\n</RCC>" >> $qrc_path
		echo -e $qrc_path" created."
	else
		sed -i "s/<\/qresource>/    <file>resources\/images\/"$(get_plugin_name)".svg<\/file>\n  <\/qresource>/"
		echo -e $qrc_path" edited."
	fi
}

function edit_cpp()
{
	cpp_path=$(get_full_plugin_path)"/"$(get_plugin_name)".cpp"
	icon_prefix=$(echo $plugin_path | sed 's#:#\\\/#')
	sed -i "s/return QIcon ()\;/static QIcon icon (\":\/"$icon_prefix"\/resources\/images\/"$(get_plugin_name)".svg\")\;\n\t\treturn icon\;/" $cpp_path
	echo -e $cpp_path" edited."
}

function edit_cmake()
{
	cmake_path=$(get_full_plugin_path)"/CMakeLists.txt"
	if [ ! -f $(get_full_plugin_path)"/"$(get_plugin_name)"resources.qrc" ]; then
		sed -i "s/ADD_LIBRARY/SET ("$(get_plugin_name | tr '[:lower:]' '[:upper:]')"_RESOURCES "$(get_plugin_name)"resources.qrc)\nQT4_ADD_RESOURCES ("$(get_plugin_name | tr '[:lower:]' '[:upper:]    ')"_RCCS \${"$(get_plugin_name | tr '[:lower:]' '[:upper:]')"_RESOURCES})\n\nADD_LIBRARY/;s/ADD_LIBRARY (leechcraft_"$(echo $plugin_path | sed 's/:/_/')" SHARED/ADD_LIBRARY (leechcraft_"$(echo     $plugin_path | sed 's/:/_/')" SHARED\n\t\${"$(get_plugin_name | tr '[:lower:]' '[:upper:]')"_RCCS}/" $cmake_path
		echo -e $cmake_path" edited."
	fi
}

function insert_icon()
{
	mkdir -p $(get_full_plugin_path)"/resources/images"
	cp $icon_path $(get_full_plugin_path)"/resources/images/"$(get_plugin_name)".svg"
	if [ $replacing == "1" ]; then
		echo "Icon replaced."
	else
		echo "Icon placed."
	fi
}

function git_add_files()
{
	cd $(get_full_plugin_path) && git add $(get_full_plugin_path)"/"$(get_plugin_name)".cpp" $(get_full_plugin_path)"/"$(get_plugin_name)"resources.qrc" $(get_full_plugin_path)"/CMakeLists.txt" $(get_full_plugin_path)"/resources/images/"$(get_plugin_name)".svg"
	echo "Now you can commit your changes."
}

if [ -d $(get_full_plugin_path) ]; then
	if [ ! -f $(get_full_plugin_path)"/resources/images/"$(get_plugin_name)".svg" ]; then
		if [ -f $(get_full_plugin_path)"/CMakeLists.txt" ]; then
			if [ -f $(get_full_plugin_path)"/"$(get_plugin_name)".cpp" ]; then
				if [[ -f $icon_path && $icon_path != '' ]]; then 
					edit_cpp
					edit_cmake
					generate_qrc
					insert_icon
					git_add_files
				else
					echo 'Error: no icon file given. Use -i <arg> to specify icon file path.'
					exit 1
				fi
			else
				echo "Error: plugin directory does not contain main c++ file."
				exit 1
			fi
		else
			echo "Error: plugin directory does not contain CMakeLists.txt."
			exit 1
		fi
	elif [ $replacing == "1" ]; then
		insert_icon
	else
		echo "Error: this plugin already has an icon. If you want to replace an existing icon, use -r."
		exit 1
	fi
else
	echo 'Error: no such plugin or directory. Use -p <arg> to provide plugin path. Syntax: plugin[:subplugin[:subplugin[:...]]]'
fi
