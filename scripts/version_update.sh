#!/bin/bash

# bash scripts/version_update.sh path

# build cmake
mkdir -p ./build
cd ./build/
cmake ..
make pre
make -j4
cd ..
echo "Compilation ends successfully!"

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

# 新版本路径
new_path=$(pwd)'/'

# -------------------通过gshow获取低版本所有数据库------------------------
# 进入旧版本路径
cd $old_path
current_time=$(date "+%Y%m%d%H%M%S")
temp_show_log=$new_path'/show'$current_time.log
# echo "temp_show_log: $temp_show_log"
bin/gshow > $temp_show_log
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

old_db_home=""
if [ -f "./init.conf" ];then
    old_db_home=$(grep -m 1 'db_home' ./init.conf)
else
    # echo "init.conf not exist"
    if [ -f "./conf.ini" ];then
        old_db_home=$(grep -m 1 'db_home' ./conf.ini)
    else
        echo "conf.ini not exist"
        exit
    fi
fi

# echo "old_db_home:$old_db_home"
old_db_home_path=$old_path
if [[ -n $old_db_home ]];
then
    # 是否注释，默认为数据库为gstore当前目录
    is_enable=${old_db_home:0: 1}
    # echo $is_enable
    if [[ $is_enable != '#' ]];
    then
        pos=$(echo "${old_db_home}" | awk -F '=' '{print $1}' | wc -c)
        for i in ${old_db_home:${pos}:${#old_db_home}-${pos}};
        do
            if [[ $i != ' ' ]];
            then
                old_db_home_path=$old_db_home_path$i
            fi
        done
    fi
fi

if [[ $old_db_home_path = '.' ]];
then
    old_db_home_path=""
fi
# 删除show.log文件
rm -rf $temp_show_log
rm -rf $temp_database_log

# echo "old_db_home_path:$old_db_home_path"

# -------------------获取新版本数据库------------------------
# 切入新版本
cd $new_path
new_db_home=""
if [ -f "./conf/conf.ini" ];then
    new_db_home=$(grep -m 1 'db_home' ./conf/conf.ini)
fi

if [[ $new_db_home = '.' ]];
then
    new_db_home="db_home"
fi

# echo "new_db_home:$new_db_home"
new_db_home_path=$new_path
if [[ -n $new_db_home ]];
then
    # 是否注释，默认为数据库为gstore当前目录
    is_enable=${new_db_home:0: 1}
    # echo $is_enable
    if [[ $is_enable != '#' ]];
    then
        pos=$(echo "${new_db_home}" | awk -F '=' '{print $1}' | wc -c)
        for i in ${new_db_home:${pos}:${#new_db_home}-${pos}};
        do
            if [[ $i != ' ' ]];
            then
                new_db_home_path=$new_db_home_path$i
            fi
        done
    else
        new_db_home_path=$new_db_home_path"./dbhome/"
    fi
fi
# echo "new_db_home_path:$new_db_home_path"

for database_name in ${database_list[@]}
do
    # 新版本已经有对应库是否覆盖旧版本
    old_path=$old_db_home_path$database_name'.db'
    new_path=$new_db_home_path$database_name'.db'
    # echo $old_path $new_path
    if [ -d $new_path ];then
        read -r -p "the new version already has a $database_name.db, overwrite [Y/n] " input
        case $input in
            [yY][eE][sS]|[yY])
                rm -rf $new_path
                cp -rf $old_db_home_path$database_name'.db' $new_db_home_path
                echo "overwrite succeffully"
                ;;
            [nN][oO]|[nN])
                echo "no overwrite"
                ;;
            *)
                echo "Invalid input..."
                ;;
	    esac
    fi
done

# ------------新版本初始化所有数据库-------------------
for database_name in ${database_list[@]}
do
    if [[ $database_name != 'system' ]];
    then
        bin/ginit -db $database_name
    fi
done



