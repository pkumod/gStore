#!/bin/bash

# bash scripts/version_update.sh path

old_path=$1
echo "------------------start version update package------------------"

if [ -z "$old_path" ];
then
    echo "please input old version gstore path!!!"
    exit
fi

# 判断路径是否存在
if [ -d $old_path ];then
    echo "[old version path:]" ${old_path}
else
    echo "path not exist"
    exit 1
fi

old_path=$1'/'
old_version=""
if [[ -f $old_path'conf.ini' ]]; then
    old_version=$(awk -F '=' '/version/ {print$2}' $old_path'conf.ini')
else
    old_version=$(awk -F '=' '/version/ {print$2}' $old_path'conf/conf.ini')
fi

# 新版本路径
new_path=$(pwd)'/'
new_version=""
if [ -f "./conf.ini" ]; then
    new_version=$(awk -F '=' '/version/ {print$2}' "./conf.ini")
else
    new_version=$(awk -F '=' '/version/ {print$2}' "conf/conf.ini")
fi

if [[ $new_version < $old_version ]]; then
    echo "the current version shouldn't lower old version !!!"
    exit
fi

# -------------------通过gshow获取低版本所有数据库------------------------
# 进入旧版本路径
cd $old_path
current_time=$(date "+%Y%m%d%H%M%S")
temp_show_log=$new_path'/show'$current_time.log
# echo "temp_show_log: $temp_show_log"
bin/gshow > $temp_show_log
if [ $? -ne 0 ]; then
    echo "bin/gshow execution failure !!!"
    exit
fi
temp_database_log=$new_path'/tmp'$current_time.log
tac "$temp_show_log" | grep '\-\-\-\-\-\-\-\-\-\-\-\-' -m 1 -A 10000 > $temp_database_log

database_list=()
while read line
do
    is_database="false"
    database_name=''
    for i in $line;
    do
        if [ $is_database = "false" ];
        then
            if [ $i = '|' ];
            then
                is_database="true"
                continue
            else
                break
            fi
        fi

        if [ $database_name ] && [ $i != " " ];
        then
            break
        fi
        if [ $is_database = "true" ] && [ $i != " " ];
        then
            database_name=$database_name$i 
        fi
    done
    if [ $database_name ];
    then
        database_list[${#database_list[*]}]=$database_name
    fi
done < $temp_database_log

conf_path=""
if [[ $old_version > "1.2" ]]; then
    conf_path="conf/"
fi

old_db_home=""
if [ -f $conf_path'init.conf' ];then
    old_db_home=$(grep -m 1 'db_home' $conf_path'init.conf')
else
    # echo "init.conf not exist"
    if [ -f $conf_path"conf.ini" ];then
        old_db_home=$(grep -m 1 'db_home' $conf_path'conf.ini')
    else
        echo "conf.ini not exist"
        exit
    fi
fi

# echo "old_db_home:$old_db_home"
old_db_home_path=""
if [[ -n $old_db_home ]];
then
    # 是否注释，默认为数据库为gstore当前目录
    is_enable=${old_db_home:0: 1}
    # echo $is_enable
    if [[ $is_enable != '#' ]];
    then
        pos=$(echo "${old_db_home}" | awk -F '=' '{print $1}' | wc -c)
        temp_d=${old_db_home:${pos}:${#old_db_home}-${pos}}
        old_db_home_path=$(echo ${temp_d//\"/})
    fi
fi

if [[ -z $old_db_home_path ]]; then
    if [[ $cur_version < "1.3" ]]; then
        old_db_home_path='.'
    else
        old_db_home_path='./dbhome/'
    fi
fi

if [[ $old_db_home_path = '.' ]];
then
    old_db_home_path=""
fi
old_db_home_path=$old_path$old_db_home_path
# 删除show.log文件
rm -rf $temp_show_log
rm -rf $temp_database_log

# echo "old_db_home_path:$old_db_home_path"

# -------------------获取新版本数据库------------------------
# 切入新版本
cd $new_path
# 静态包不进行编译
if [[ -f 'makefile' ]]; then
    make clean
    make pre
    make -j4
    echo "Compilation ends successfully!"
elif [[ -f 'CMakeLists.txt' ]]; then
    mkdir -p ./build
    cd ./build/
    cmake ..
    make pre
    make -j4
    make init
    cd ..
    echo "Compilation ends successfully!"
fi
new_db_home=""
if [[ $new_version < "1.3" ]]; then
    if [ -f "./conf.ini" ];then
        new_db_home=$(grep -m 1 'db_home' ./conf.ini)
    else
        new_db_home=$(grep -m 1 'db_home' ./init.conf)
    fi
elif [ -f "./conf/conf.ini" ];then
    new_db_home=$(grep -m 1 'db_home' ./conf/conf.ini)
fi

# echo "new_db_home:$new_db_home"
new_db_home_path=""
if [[ -n $new_db_home ]];
then
    # 是否注释，默认为数据库为gstore当前目录
    is_enable=${new_db_home:0: 1}
    # echo $is_enable
    if [[ $is_enable != '#' ]];
    then
        pos=$(echo "${new_db_home}" | awk -F '=' '{print $1}' | wc -c)
        temp_d=${new_db_home:${pos}:${#new_db_home}-${pos}}
        new_db_home_path=$(echo ${temp_d//\"/})
    fi
fi

if [[ -z $new_db_home_path ]]; then
    if [[ $new_version < "1.3" ]]; then
        new_db_home_path=""
    else
        new_db_home_path="./dbhome/"
    fi
fi

new_db_home_path=$new_path$new_db_home_path
# echo "new_db_home_path:$new_db_home_path"

for database_name in ${database_list[@]}
do
    # 新版本已经有对应库是否覆盖旧版本
    temp_old_path=$old_db_home_path'/'$database_name'.db'
    temp_new_path=$new_db_home_path'/'$database_name'.db'
    echo $temp_old_path $temp_new_path
    if [ -d $temp_new_path ]; then
        read -r -p "the new version already has a $database_name.db, overwrite [Y/n] " input
        case $input in
            [yY][eE][sS]|[yY])
                rm -rf $temp_new_path
                cp -rf $temp_old_path $new_db_home_path
                echo "overwrite succeffully"
                ;;
            [nN][oO]|[nN])
                echo "no overwrite"
                ;;
            *)
                echo "Invalid input..."
                ;;
        esac
    else
         cp -rf $temp_old_path $new_db_home_path
    fi
done

# ------------新版本数据迁移-------------------
if [[ $new_version < "1.3" ]]; then
    if [[ $old_db_home_path ]] && [[ $new_db_home_path ]] && [[ $old_db_home_path != $new_db_home_path ]]; then
        temp_new_path='# db_home=\".\"'
        replace_path=$(grep "db_home" conf.ini)
        if [[ -z $replace_path ]]; then
            replace_path=$(grep "db_home" ini.conf)
        fi
        replace_path=$(echo ${replace_path//\"/\\\"})
        replace_path=$(echo ${replace_path//\//\\\/})
        path='s/'$temp_new_path'/'$replace_path'/g'
        sed -i $path conf.ini
    fi
fi

if [[ $old_version < "1.3" ]] && [[ $new_version > "1.2" ]] && [[ $old_path'fun/' ]]; then
    mv ./pfn/cpp ./pfn/cpp.bak
    cp -rf $old_path'fun/' './pfn/'
    mv ./pfn/fun ./pfn/cpp/
elif [[ $old_version < "1.3" ]] && [[ $new_version < "1.3" ]]; then
    cp -rf $old_path'fun/' ./
elif [[ $old_version > "1.2" ]] && [[ $new_version > "1.2" ]]; then
    mv ./pfn/cpp ./pfn/cpp.bak
    cp -rf $old_path'pfn/cpp/' ./pfn/
fi

for database_name in ${database_list[@]}
do
    if [[ $database_name != 'system' ]];
    then
        bin/ginit -db $database_name
    fi
done



