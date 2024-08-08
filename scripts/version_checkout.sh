#!/bin/bash

new_version=$1
echo "------------------start version checkout $new_version"

if [[ -z $1 ]]; then
    echo "please input checkout version!!!"
    exit
fi

cmake_version=$(cmake --version)
if [[ $1 > "1.3" ]] && [[ $cmake_version < "cmake version 3.17" ]]; then
    echo "The cmake version is earlier than version 3.17, please install a later version, 3.23.2 is recommended"
    exit
fi

cur_version=""
if [ -f "./conf.ini" ]; then
    cur_version=$(awk -F '=' '/version/ {print$2}' "./conf.ini")
else
    cur_version=$(awk -F '=' '/version/ {print$2}' "conf/conf.ini")
fi

if [[ -z $cur_version ]]; then
    echo "version not compatible"
    exit
fi

# 低于1.0版本不需要处理
if [[ $cur_version < "1.0" ]]; then
    echo "current version lower 1.0, not compatible"
    exit
fi

echo "current version is $cur_version"

# 相同版本重新编译即可
if [[ $cur_version = $new_version ]]; then
    if [[ $cur_version < "1.3" ]]; then
        make clean
        make -j4
    else
        cd build/
        make clean
        make -j4
    fi
    exit
fi

# 不同版本，进行备份
gstore_path=$(pwd)'/'
current_time=$(date "+%Y%m%d%H%M%S")
backup_path="version$new_version"'_'$current_time'/'
mkdir -p $backup_path
conf_ini="conf.ini"
ini_conf="ini.conf"
ipAllow="ipAllow.config"
ipDeny="ipDeny.config"
pfn="fun"
if [[ $cur_version < "1.3" ]]; then
    cp -f $conf_ini $backup_path
    cp -f $ini_conf $backup_path
    cp -f $ipAllow $backup_path
    cp -f $ipDeny $backup_path
else
    pfn="pfn"
    cp -f 'conf/'$conf_ini $backup_path
    cp -f 'conf/'$ipAllow $backup_path
    cp -f 'conf/'$ipDeny $backup_path
    cp -r $pfn $backup_path
fi

# gshow查看数据库
temp_show_log=$new_path'/show'$current_time.log
# echo "temp_show_log: $temp_show_log"
bin/gshow > $temp_show_log
temp_database_log=$new_path'/tmp'$current_time.log
tac "$temp_show_log" | grep '\-\-\-\-\-\-\-\-\-\-\-\-' -m 1 -A 10000 > $backup_path$temp_database_log

# 切换版本
git checkout -b $new_version origin/$new_version
git checkout $new_version
if [[ $new_version < "1.3" ]]; then
    make clean
    make pre
    make -j4
else
    mkdir -p ./build
    cd ./build/
    cmake ..
    make pre
    make -j4
    make init
    cd ..
    echo "Compilation ends successfully!"
fi

# 进行拷贝数据库初始化
database_list=()
while read line
do
    is_database="false"
    database_name=''
    for i in $line;
    do
        if [ $is_database = "false" ]; then
            if [ $i = '|' ]; then
                is_database="true"
                continue
            else
                break
            fi
        fi

        if [ $database_name ] && [ $i != " " ]; then
            break
        fi
        if [ $is_database = "true" ] && [ $i != " " ]; then
            database_name=$database_name$i 
        fi
    done
    if [ $database_name ]; then
        database_list[${#database_list[*]}]=$database_name
    fi
done < $backup_path$temp_database_log

old_db_home=""
if [ -f $backup_path$ini_conf ]; then
    old_db_home=$(grep -m 1 'db_home' $backup_path$ini_conf)
else
    # echo "init.conf not exist"
    if [ -f $backup_path$conf_ini ]; then
        old_db_home=$(grep -m 1 'db_home' $backup_path$conf_ini)
    else
        echo "conf.ini not exist"
        exit
    fi
fi

old_db_home_path=""
if [[ -n $old_db_home ]]; then
    # 是否注释，默认为数据库为gstore当前目录
    is_enable=${old_db_home:0: 1}
    # echo $is_enable
    if [[ $is_enable != '#' ]]; then
        pos=$(echo "${old_db_home}" | awk -F '=' '{print $1}' | wc -c)
        for i in ${old_db_home:${pos}:${#old_db_home}-${pos}};
        do
            if [[ $i != ' ' ]]; then
                old_db_home_path=$old_db_home_path$i
            fi
        done
    fi
fi

if [[ -z $old_db_home_path ]]; then
    if [[ $cur_version < "1.3" ]]; then
        old_db_home_path='.'
    else
        old_db_home_path='./dbhome/'
    fi
fi

if [[ $old_db_home_path = '.' ]]; then
    old_db_home_path=""
fi

if [[ $cur_version < "1.3" ]]; then
    old_db_home_path='.'
else
    old_db_home_path='./dbhome/'
fi

new_db_home_path=''
if [[ $new_version < "1.3" ]]; then
    new_db_home_path=$old_db_home_path
elif [[ $cur_version < "1.3" ]]; then
    new_db_home_path='./dbhome/'
fi

for database_name in ${database_list[@]}
do
    # 新版本已经有对应库是否覆盖旧版本
    old_path=$old_db_home_path$database_name'.db'
    new_path=$new_db_home_path$database_name'.db'
    # echo $old_path $new_path
    if [ -d $new_path ]; then
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
    if [[ $database_name != 'system' ]]; then
        bin/ginit -db $database_name
    fi
done