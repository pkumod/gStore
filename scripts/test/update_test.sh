#!/bin/bash
set -eo pipefail

# ===================== 【全局配置】 =====================
CONF_FILE="./conf/conf.ini"
USERNAME="root"
PASSWORD="123456"

DEFAULT_DATA="./data/lubm/lubm.nt"
UPDATE_TEMPLATE="./data/update_test.nt"

# 颜色
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
NC='\033[0m'

# ===================== 端口配置 =====================
PORT=$(grep -m1 'port=' "${CONF_FILE}" 2>/dev/null)
PORT_STR="${PORT#*=}"
BASE_URL="http://127.0.0.1:${PORT_STR}/api"

# ===================== 工具函数 =====================
call_api() {
    local data="$1"
    curl -s -X POST -H 'Content-Type: application/json' -d "$data" "$BASE_URL" 2>/dev/null || echo '{"statusCode":-1,"statusMsg":"curl request failed"}'
}

escape_sparql() {
    local input="$1"
    echo "$input" | sed 's/\\/\\\\/g' | sed 's/"/\\"/g' | tr -d '\r' | sed ':a;N;$!ba;s/\n/\\n/g' | sed 's/\t/\\t/g'
}

divider() {
    echo -e "${BLUE}=========================================================================${NC}"
}

# ===================== 帮助 =====================
show_help() {
cat <<EOF
用法：
1. 默认测试（自动创建并删除库）：
   $0
   默认测试 ./data/lubm/lubm.nt，最后自动 drop

2. 帮助信息：
   $0 -h | --help

3. 测试已存在库（不删除库）：
   $0 -b db_name -n insert_count
   -b 数据库名，-n 插入条数

4. 新建库测试（自动创建并删除库）：
   $0 -d data_path -n insert_count
   -d 数据路径，-n 插入条数
EOF
}

# ===================== 读取模板 =====================
if [ ! -f "${UPDATE_TEMPLATE}" ]; then
    echo -e "${RED}错误：模板文件不存在 ${UPDATE_TEMPLATE}${NC}"
    exit 1
fi

TRIPLE=$(cat "${UPDATE_TEMPLATE}" | xargs)
BASE_S="${TRIPLE%% *}"
BASE_P="${TRIPLE#* }"
BASE_P="${BASE_P% *}"
BASE_O="${TRIPLE##* }"
BASE_S="${BASE_S//</}"
BASE_S="${BASE_S//>/}"
BASE_P="${BASE_P//</}"
BASE_P="${BASE_P//>/}"
BASE_O="${BASE_O//</}"
BASE_O="${BASE_O//>.}"

# ===================== 执行批量 insert =====================
run_batch_insert() {
    local db="$1"
    local total="$2"
    divider
    echo -e "${YELLOW}🚀 开始批量插入 ${total} 条数据${NC}"
    divider

    for ((idx=0; idx<total; idx++)); do
        s="s$((idx % 10))"
        p="p$((idx % 10))"
        o="o${idx}"

        sparql="insert data { <${s}> <${p}> <${o}>. }"
        esc=$(escape_sparql "${sparql}")

        json="{\"operation\":\"query\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"sparql\":\"${esc}\"}"
        echo -e "${PURPLE}[${idx}] 请求：${NC}${sparql}"

        resp=$(call_api "${json}")
        echo -e "${GREEN}响应：${NC}${resp}${NC}"
    done

    divider
    echo -e "${GREEN}✅ 批量插入完成${NC}"
    divider
}

# ===================== 主流程 =====================
divider
echo -e "${PURPLE}          🚀 GStore Update 批量插入测试${NC}"
divider

# 默认模式
if [ $# -eq 0 ]; then
    DB="update_auto_$(date +%s)"
    echo -e "${YELLOW}📌 默认测试：${DEFAULT_DATA}${NC}"

    echo -e "\n${BLUE}[1/5] build${NC}"
    build="{\"operation\":\"build\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB}\",\"db_path\":\"${DEFAULT_DATA}\"}"
    echo -e "${PURPLE}请求：${NC}$build"
    call_api "${build}"

    echo -e "\n${BLUE}[2/5] load${NC}"
    load="{\"operation\":\"load\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB}\"}"
    echo -e "${PURPLE}请求：${NC}$load"
    call_api "${load}"

    run_batch_insert "${DB}" 10

    echo -e "\n${BLUE}[4/5] unload${NC}"
    unload="{\"operation\":\"unload\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB}\"}"
    call_api "${unload}"

    echo -e "\n${BLUE}[5/5] drop${NC}"
    drop="{\"operation\":\"drop\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB}\"}"
    call_api "${drop}"

    exit 0
fi

# 参数解析
db=""
data=""
num=""
while getopts ":hb:d:n:" opt; do
    case $opt in
        h) show_help; exit 0 ;;
        b) db="$OPTARG" ;;
        d) data="$OPTARG" ;;
        n) num="$OPTARG" ;;
        \?) echo "参数错误"; exit 1 ;;
    esac
done

# 已存在库 -b db -n num
if [ -n "${db}" ] && [ -n "${num}" ]; then
    echo -e "${YELLOW}📌 测试已存在库：${db} 插入条数：${num}${NC}"

    echo -e "\n${BLUE}[1/3] load${NC}"
    load="{\"operation\":\"load\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\"}"
    call_api "${load}"

    run_batch_insert "${db}" "${num}"

    echo -e "\n${BLUE}[3/3] unload${NC}"
    unload="{\"operation\":\"unload\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\"}"
    call_api "${unload}"

    exit 0
fi

# 新建库 -d data -n num
if [ -n "${data}" ] && [ -n "${num}" ]; then
    DB="update_build_$(date +%s)"
    echo -e "${YELLOW}📌 新建库测试：${data} 插入条数：${num}${NC}"

    echo -e "\n${BLUE}[1/5] build${NC}"
    build="{\"operation\":\"build\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB}\",\"db_path\":\"${data}\"}"
    call_api "${build}"

    echo -e "\n${BLUE}[2/5] load${NC}"
    load="{\"operation\":\"load\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB}\"}"
    call_api "${load}"

    run_batch_insert "${DB}" "${num}"

    echo -e "\n${BLUE}[4/5] unload${NC}"
    unload="{\"operation\":\"unload\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB}\"}"
    call_api "${unload}"

    echo -e "\n${BLUE}[5/5] drop${NC}"
    drop="{\"operation\":\"drop\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB}\"}"
    call_api "${drop}"

    exit 0
fi

show_help
exit 0