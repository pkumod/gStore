#!/bin/bash

# API基础URL
PORT=$(grep -m 1 'port=' ./conf/conf.ini)
BASE_URL=http://127.0.0.1:"${PORT:5:${#PORT}-5}"/api
# 备份路径
BACKUP_PATH=""
# 数据库
DB_NAME=lubm

# 心跳检测
check() {
    echo "心跳检测"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"check"}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"check"}' "$BASE_URL"
}

# 测试连接
test_connection() {
    echo "测试连接"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"testConnect","username":"root","password":"123456"}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"testConnect","username":"root","password":"123456"}' "$BASE_URL"
}

# 获取版本信息
get_core_version() {
    echo "获取版本信息"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"getCoreVersion","username":"root","password":"123456"}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"getCoreVersion","username":"root","password":"123456"}' "$BASE_URL"
}

# 数据库构建
build() {
    echo "数据库构建"
    db_path="data/$DB_NAME/$DB_NAME.nt"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"build","username":"root","password":"123456","db_name":"$DB_NAME","db_path":"$db_path"}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"build","username":"root","password":"123456","db_name":"'"$DB_NAME"'","db_path":"'"$db_path"'"}' "$BASE_URL"
}

# 数据库列表
show() {
    echo "数据库列表"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"show","username":"root","password":"123456"}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"show","username":"root","password":"123456"}' "$BASE_URL"
}

# 数据库统计
monitor() {
    echo "数据库统计"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"monitor","username":"root","password":"123456","db_name":"$DB_NAME"}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"monitor","username":"root","password":"123456","db_name":"'"$DB_NAME"'"}' "$BASE_URL"
}

# 加载数据库
load() {
    echo "加载数据库"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"load","username":"root","password":"123456","db_name":"$DB_NAME"}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"load","username":"root","password":"123456","db_name":"'"$DB_NAME"'"}' "$BASE_URL"
}

# 数据库查询
query() {
    echo "数据库查询"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"query","username":"root","password":"123456","db_name":"$DB_NAME","sparql":"select ?x ?p where {?x ?p \<FullProfessor0\>".}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"query","username":"root","password":"123456","db_name":"'"$DB_NAME"'","sparql":"select ?x ?p where {?x ?p <FullProfessor0>.}"}' "$BASE_URL"
}

# 数据库导出
export() {
    echo "数据库导出"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"export","username":"root","password":"123456","db_name":"$DB_NAME","db_path":"./export"}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"export","username":"root","password":"123456","db_name":"'"$DB_NAME"'","db_path":"./export"}' "$BASE_URL"
}

# 数据库备份
backup() {
    echo "数据库备份"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"backup","username":"root","password":"123456","db_name":"$DB_NAME"}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"backup","username":"root","password":"123456","db_name":"'"$DB_NAME"'"}' "$BASE_URL" > "rt.txt" 2>&1
    BACKUP_PATH=$(awk -F '"' '/backupfilepath/ {print$14}' "rt.txt")
    rm "rt.txt"
    echo "backupfilepath：$BACKUP_PATH"
}

# 卸载数据库
unload() {
    echo "卸载数据库"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"unload","username":"root","password":"123456","db_name":"$DB_NAME"}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"unload","username":"root","password":"123456","db_name":"'"$DB_NAME"'"}' "$BASE_URL"
}

# 还原数据库
restore() {
    echo "还原数据库"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"restore","username":"root","password":"123456","db_name":"$DB_NAME","backup_path":"$BACKUP_PATH"}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"restore","username":"root","password":"123456","db_name":"'"$DB_NAME"'","backup_path":"'"$BACKUP_PATH"'"}' "$BASE_URL"
    echo ""
    echo "重新加载数据库"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"load","username":"root","password":"123456","db_name":"'"$DB_NAME"'"}' "$BASE_URL"
}

# 开启事务
begin() {
    echo "开启事务"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"begin","username":"root","password":"123456","db_name":"$DB_NAME","isolevel":"1"}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"begin","username":"root","password":"123456","db_name":"'"$DB_NAME"'","isolevel":"1"}' "$BASE_URL"
}

# 执行事务
execute() {
    echo "执行事务"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"tquery","username":"root","password":"123456","db_name":"$DB_NAME","tid":"1","sparql":"insert data {\<s\> \<p\> \<o\>.}"}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"tquery","username":"root","password":"123456","db_name":"'"$DB_NAME"'","tid":"1","sparql":"insert data {<s> <p> <o>.}"}' "$BASE_URL"
}

# 提交事务
commit() {
    echo "提交事务"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"commit","username":"root","password":"123456","db_name":"$DB_NAME","tid":"1"}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"commit","username":"root","password":"123456","db_name":"'"$DB_NAME"'","tid":"1"}' "$BASE_URL"
}

# 回滚事务
rollback() {
    echo "回滚事务"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"begin","username":"root","password":"123456","db_name":"'"$DB_NAME"'","isolevel":"1"}' "$BASE_URL"
    echo ""
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"tquery","username":"root","password":"123456","db_name":"'"$DB_NAME"'","tid":"2","sparql":"delete data {<s> <p> <o>.}"}' "$BASE_URL"
    echo ""
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"rollback","username":"root","password":"123456","db_name":"$DB_NAME","tid":"2"}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"rollback","username":"root","password":"123456","db_name":"'"$DB_NAME"'","tid":"2"}' "$BASE_URL"
}

# 数据落盘
checkpoint() {
    echo "数据落盘"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"checkpoint","username":"root","password":"123456","db_name":"$DB_NAME"}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"checkpoint","username":"root","password":"123456","db_name":"'"$DB_NAME"'"}' "$BASE_URL"
}

# 新增用户
add_user() {
    echo "新增用户"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"usermanage","username":"root","password":"123456","type":"1","op_username":"test","op_password":"123456"}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"usermanage","username":"root","password":"123456","type":"1","op_username":"test","op_password":"123456"}' "$BASE_URL"
}

# 查看所有用户
show_users() {
    echo "查看所有用户"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"showuser","username":"root","password":"123456"}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"showuser","username":"root","password":"123456"}' "$BASE_URL"
}

# 设置用户权限
set_user_permission() {
    echo "设置用户权限"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"userprivilegemanage","username":"root","password":"123456","type":"1","op_username":"test","privileges":"1,2,3","db_name":"$DB_NAME"}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"userprivilegemanage","username":"root","password":"123456","type":"1","op_username":"test","privileges":"1,2,3","db_name":"'"$DB_NAME"'"}' "$BASE_URL"
}

# 清除用户权限
clear_user_permission() {
    echo "清除用户权限"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"userprivilegemanage","username":"root","password":"123456","type":"3","op_username":"test"}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"userprivilegemanage","username":"root","password":"123456","type":"3","op_username":"test"}' "$BASE_URL"
}

# 修改密码
change_password() {
    echo "修改密码"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"userpassword","username":"test","password":"123456", "op_password":"111111"}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"userpassword","username":"test","password":"123456", "op_password":"111111"}' "$BASE_URL"
}

# 删除用户
delete_user() {
    echo "删除用户"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"usermanage","username":"root","password":"123456","type":"2","op_username":"test"}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"usermanage","username":"root","password":"123456","type":"2","op_username":"test"}' "$BASE_URL"
}

# 重命名数据库
rename() {
    echo "重命名数据库"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"rename","username":"root","password":"123456","db_name":"$DB_NAME","new_name":"new$DB_NAME"}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"unload","username":"root","password":"123456","db_name":"'"$DB_NAME"'"}' "$BASE_URL"
    echo ""
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"rename","username":"root","password":"123456","db_name":"'"$DB_NAME"'","new_name":"new'$DB_NAME'"}' "$BASE_URL"
}

# 删除数据库
drop() {
    echo "删除数据库"
    echo "curl -X POST -H 'Content-Type: application/json' -d '{"operation":"drop","username":"root","password":"123456","db_name":"newlubm"}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d '{"operation":"drop","username":"root","password":"123456","db_name":"newlubm", "is_backup":"false"}' "$BASE_URL"
    rm -rf backups/"$DB_NAME"*
    rm -rf export/"$DB_NAME"*
}

# 操作数组
operations=("check" "test_connection" "get_core_version" "build" "show" "monitor" "load" "query" "export" "backup" "unload" "restore" "begin" "execute" "commit" "rollback" "checkpoint" "add_user" "set_user_permission" "show_users" "clear_user_permission" "change_password" "delete_user" "rename" "drop")

# 执行所有操作
for operation in "${operations[@]}"; do
    eval "$operation"
    echo ""
    echo ""
    sleep 1s
done
