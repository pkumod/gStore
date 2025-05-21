#!/bin/bash
param=$1
# API基础URL
PORT=$(grep -m 1 'port=' ./conf/conf.ini)
BASE_URL=http://127.0.0.1:"${PORT:5:${#PORT}-5}"/api
# 备份路径
BACKUP_PATH=""
# 数据库
DB_NAME=lubm
if [ -n "$param" ]; then
    DB_NAME=$param
fi
RDF_FILE="data/$DB_NAME/$DB_NAME.nt"

# 参数集合
SPARQL1="select ?s ?p ?o where {?s ?p ?o.} limit 10"
SPARQL2="insert data {<s> <p> <o>.}"
SPARQL3="delete data {<s> <p> <o>.}"
API_DATA=(
    '{"operation":"check"}' 
    '{"operation":"login","username":"root","password":"123456"}'
    '{"operation":"build","username":"root","password":"123456","db_name":"'"$DB_NAME"'","db_path":"'"$RDF_FILE"'"}'
    '{"operation":"show","username":"root","password":"123456"}'
    '{"operation":"monitor","username":"root","password":"123456","db_name":"'"$DB_NAME"'"}'
    '{"operation":"load","username":"root","password":"123456","db_name":"'"$DB_NAME"'"}'
    '{"operation":"query","username":"root","password":"123456","db_name":"'"$DB_NAME"'","sparql":"'"$SPARQL1"'"}'
    '{"operation":"export","username":"root","password":"123456","db_name":"'"$DB_NAME"'"}' 
    '{"operation":"backup","username":"root","password":"123456","db_name":"'"$DB_NAME"'"}' 
    '{"operation":"batchRemove","username":"root","password":"123456","db_name":"'"$DB_NAME"'","file":"'"$RDF_FILE"'"}' 
    '{"operation":"batchInsert","username":"root","password":"123456","db_name":"'"$DB_NAME"'","file":"'"$RDF_FILE"'"}' 
    '{"operation":"unload","username":"root","password":"123456","db_name":"'"$DB_NAME"'"}' 
    '{"operation":"restore","username":"root","password":"123456","db_name":"'"$DB_NAME"'","backup_path":""}' 
    '{"operation":"begin","username":"root","password":"123456","db_name":"'"$DB_NAME"'","isolevel":"1"}' 
    '{"operation":"tquery","username":"root","password":"123456","db_name":"'"$DB_NAME"'","tid":"1","sparql":"'"$SPARQL2"'"}' 
    '{"operation":"commit","username":"root","password":"123456","db_name":"'"$DB_NAME"'","tid":"1"}' 
    '{"operation":"rollback","username":"root","password":"123456","db_name":"'"$DB_NAME"'","tid":"2"}' 
    '{"operation":"checkpoint","username":"root","password":"123456","db_name":"'"$DB_NAME"'"}' 
    '{"operation":"usermanage","username":"root","password":"123456","type":"1","op_username":"test","op_password":"123456"}' 
    '{"operation":"showuser","username":"root","password":"123456"}' 
    '{"operation":"userprivilegemanage","username":"root","password":"123456","type":"1","op_username":"test","privileges":"1,2,3","db_name":"'"$DB_NAME"'"}' 
    '{"operation":"userprivilegemanage","username":"root","password":"123456","type":"3","op_username":"test"}' 
    '{"operation":"userpassword","username":"test","password":"123456","op_password":"111111"}' 
    '{"operation":"usermanage","username":"root","password":"123456","type":"2","op_username":"test"}' 
    '{"operation":"rename","username":"root","password":"123456","db_name":"'"$DB_NAME"'","new_name":"new'"$DB_NAME"'"}' 
    '{"operation":"drop","username":"root","password":"123456","db_name":"new'"$DB_NAME"'","is_backup":"false"}' 
    )

# 心跳检测
check() {
    echo "心跳检测"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[0]}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[0]} "$BASE_URL"
}

# 登录连接
login() {
    echo "登录"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[1]}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[1]} "$BASE_URL"
}

# 数据库构建
build() {
    echo "数据库构建"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[2]}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[2]} "$BASE_URL"
}

# 数据库列表
show() {
    echo "数据库列表"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[3]}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[3]} "$BASE_URL"
}

# 数据库统计
monitor() {
    echo "数据库统计"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[4]}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[4]} "$BASE_URL"
}

# 加载数据库
load() {
    echo "加载数据库"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[5]}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[5]} "$BASE_URL"
}

# 数据库查询
query() {
    echo "数据库查询"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[6]}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d "${API_DATA[6]}" "$BASE_URL"
}

# 数据库导出
export() {
    echo "数据库导出"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[7]}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[7]} "$BASE_URL"
}

# 数据库备份
backup() {
    echo "数据库备份"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[8]}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[8]} "$BASE_URL" > "rt.txt" 2>&1
    BACKUP_PATH=$(awk -F '"' '/backupfilepath/ {print$10}' "rt.txt")
    rm "rt.txt"
    echo "backupfilepath：$BACKUP_PATH"
    API_DATA[12]='{"operation":"restore","username":"root","password":"123456","db_name":"'"$DB_NAME"'","backup_path":"'"$BACKUP_PATH"'"}'
}

# 批量删除
batchRemove() {
    echo "批量删除"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[9]}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[9]} "$BASE_URL"
}

# 批量新增
batchInsert() {
    echo "批量新增"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[10]}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[10]} "$BASE_URL"
}

# 卸载数据库
unload() {
    echo "卸载数据库"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[11]}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[11]} "$BASE_URL"
}

# 还原数据库
restore() {
    echo "还原数据库"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[12]}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[12]} "$BASE_URL"
    echo ""
    echo "重新加载数据库"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[5]} "$BASE_URL"
}

# 开启事务
begin() {
    echo "开启事务"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[13]}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[13]} "$BASE_URL"
}

# 执行事务
execute() {
    echo "执行事务"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[14]}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d "${API_DATA[14]}" "$BASE_URL"
}

# 提交事务
commit() {
    echo "提交事务"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[15]}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[15]} "$BASE_URL"
}

# 回滚事务
rollback() {
    echo "回滚事务"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[13]} "$BASE_URL"
    echo ""
    API_DATA[14]='{"operation":"tquery","username":"root","password":"123456","db_name":"'"$DB_NAME"'","tid":"2","sparql":"'"$SPARQL3"'"}'
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[14]}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d "${API_DATA[14]}" "$BASE_URL"
    echo ""
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[16]}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[16]} "$BASE_URL"
}

# 数据落盘
checkpoint() {
    echo "数据落盘"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[17]}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[17]} "$BASE_URL"
}

# 新增用户
add_user() {
    echo "新增用户"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[18]} ' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[18]} "$BASE_URL"
}

# 查看所有用户
show_users() {
    echo "查看所有用户"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[19]}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[19]} "$BASE_URL"
}

# 设置用户权限
set_user_permission() {
    echo "设置用户权限"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[20]} ' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[20]} "$BASE_URL"
}

# 清除用户权限
clear_user_permission() {
    echo "清除用户权限"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[21]} ' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[21]} "$BASE_URL"
}

# 修改密码
change_password() {
    echo "修改密码"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[22]}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[22]} "$BASE_URL"
}

# 删除用户
delete_user() {
    echo "删除用户"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[23]}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[23]} "$BASE_URL"
}

# 重命名数据库
rename() {
    echo "重命名数据库"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[24]}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[11]} "$BASE_URL"
    echo ""
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[24]} "$BASE_URL"
}

# 删除数据库
drop() {
    echo "删除数据库"
    echo "curl -X POST -H 'Content-Type: application/json' -d '${API_DATA[25]}' $BASE_URL"
    curl -X POST -H 'Content-Type: application/json' -d ${API_DATA[25]} "$BASE_URL"
    rm -rf backups/"$DB_NAME"*
    rm -rf export/"$DB_NAME"*
}

# 操作数组
operations=("check" "login" "build" "show" "monitor" "load" "query" "export" "backup" "batchRemove" "batchInsert" "unload" "restore" "begin" "execute" "commit" "rollback" "checkpoint" "add_user" "set_user_permission" "show_users" "clear_user_permission" "change_password" "delete_user" "rename" "drop")
# operations=("query")

# 执行所有操作
echo "测试数据集为: $DB_NAME"
for operation in "${operations[@]}"; do
    eval "$operation"
    echo ""
    echo ""
    sleep 1s
done
