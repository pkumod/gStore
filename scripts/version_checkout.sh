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

if [[ $new_version < $cur_version ]]; then
    echo "the new version shouldn't lower current version !!!"
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
conf_path=""
conf_ini="conf.ini"
ini_conf="init.conf"
ipAllow="ipAllow.config"
ipDeny="ipDeny.config"
slog_properties="slog.properties"
slog_stdout_properties="slog.stdout.properties"
pfn="fun"
if [[ $cur_version > "1.2" ]]; then
    conf_path="conf/"
    pfn="pfn"
fi
cp -f $conf_path$conf_ini $backup_path
cp -f $ini_conf $backup_path
cp -f $conf_path$ipAllow $backup_path
cp -f $conf_path$ipDeny $backup_path
cp -f $conf_path$slog_properties $backup_path
cp -f $conf_path$slog_stdout_properties $backup_path
cp -r $pfn $backup_path

# gshow查看数据库
temp_show_log=$new_path'/show'$current_time.log
# echo "temp_show_log: $temp_show_log"
bin/gshow > $temp_show_log
if [ $? -ne 0 ]; then
    echo "bin/gshow execution failure !!!"
    exit
fi
temp_database_log=$new_path'/tmp'$current_time.log
tac "$temp_show_log" | grep '\-\-\-\-\-\-\-\-\-\-\-\-' -m 1 -A 10000 > $backup_path$temp_database_log

# 切换版本,将远程仓库的文件回退忽略掉
git_version=$(git version)
if [[ $git_version < "git version 2.23" ]]; then
    git checkout -- $conf_path'backup.json'
    git checkout -- .gitignore
    git checkout -- api/http/cpp/example/GET-example
    git checkout -- api/http/cpp/example/POST-example
    git checkout -- api/http/cpp/example/Transaction-example
    git checkout -- $conf_path'conf.ini'
    git checkout -- init.conf
    git checkout -- $conf_path'slog.properties'
    git checkout -- $conf_path'slog.stdout.properties'
else
    git restore $conf_path'backup.json'
    git restore .gitignore
    git restore api/http/cpp/example/GET-example
    git restore api/http/cpp/example/POST-example
    git restore api/http/cpp/example/Transaction-example
    git restore $conf_path'conf.ini'
    git restore init.conf
    git restore $conf_path'slog.properties'
    git restore $conf_path'slog.stdout.properties'
fi

git checkout -b $new_version origin/$new_version
if [ $? -ne 0 ]; then
    echo "checkout origin $new_version failure !!!"
    exit
fi
git checkout $new_version
if [ $? -ne 0 ]; then
    echo "checkout $new_version failure !!!"
    exit
fi
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


new_db_home_path=''
if [[ $new_version < "1.3" ]]; then
    new_db_home_path=$old_db_home_path
elif [[ $cur_version < "1.3" ]]; then
    new_db_home_path='./dbhome/'
fi

for database_name in ${database_list[@]}
do
    # 新版本已经有对应库是否覆盖旧版本
    temp_old_path=$old_db_home_path'/'$database_name'.db'
    temp_new_path=$new_db_home_path'/'$database_name'.db'
    # echo $old_path $new_path
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

if [[ $cur_version < "1.3" ]] && [[ $new_version > "1.2" ]]; then
    mv ./pfn/cpp ./pfn/cpp.bak
    cp -rf $backup_path'fun/' ./pfn/
    mv ./pfn/fun ./pfn/cpp/
fi

for database_name in ${database_list[@]}
do
    if [[ $database_name != 'system' ]]; then
        bin/ginit -db $database_name
    fi
done

echo "version update successfully, backup file in the $backup_path"
