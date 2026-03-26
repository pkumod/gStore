#!/bin/bash
set -eo pipefail

# ===================== 【全局配置】 =====================
CONF_FILE="./conf/conf.ini"
USERNAME="root"
PASSWORD="123456"
DEFAULT_DATA="./data/lubm/lubm.nt"
DEFAULT_SPARQL="select ?s ?p ?o where { ?s ?p ?o . } limit 10"

# 颜色
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
NC='\033[0m'

# ===================== conf配置端口 =====================
PORT=$(grep -m1 'port=' "${CONF_FILE}" 2>/dev/null)
PORT_STR="${PORT#*=}"
BASE_URL="http://127.0.0.1:${PORT_STR}/api"

# ===================== 工具函数  =====================
call_api() {
    local data="$1"
    curl -s -X POST -H 'Content-Type: application/json' -d "$data" "$BASE_URL" 2>/dev/null || echo '{"statusCode":-1,"statusMsg":"curl request failed"}'
}

divider() {
    echo -e "${BLUE}=========================================================================${NC}"
}

# 【核心：完全照搬你能跑的转义函数】
escape_sparql() {
    local input="$1"
    echo "$input" | sed 's/\\/\\\\/g' | sed 's/"/\\"/g' | tr -d '\r' | sed ':a;N;$!ba;s/\n/\\n/g' | sed 's/\t/\\t/g'
}

# ===================== 帮助 =====================
show_help() {
cat <<EOF
用法：
1. 默认测试（自动创建并删除库）：
   $0

2. 帮助信息：
   $0 -h | --help

3. 测试已存在的库（不删除库）：
   $0 -b db_name -q query_file_or_dir

4. 新建库并测试（自动创建并删除库）：
   $0 -d data_path -q query_file_or_dir
EOF
}

# ===================== 查询执行（文件/目录遍历） =====================
run_query() {
    local db="$1"
    local qpath="$2"

    if [ -d "${qpath}" ]; then
        echo -e "\n${YELLOW}📂 遍历目录：${qpath}${NC}"
        for file in "${qpath}"/*; do
            [ -f "${file}" ] || continue
            echo -e "\n${BLUE}🔍 查询文件：$(basename ${file})${NC}"
            SPARQL_RAW=$(cat "${file}")
            SPARQL_ESCAPED=$(escape_sparql "${SPARQL_RAW}")
            QUERY_DATA="{\"operation\":\"query\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"sparql\":\"${SPARQL_ESCAPED}\"}"
            echo -e "${PURPLE}请求：${NC} $QUERY_DATA"
            QUERY_RESP=$(call_api "${QUERY_DATA}")
            echo -e "${GREEN}响应：${NC} ${QUERY_RESP}"
        done
    elif [ -f "${qpath}" ]; then
        echo -e "\n${BLUE}🔍 查询文件：$(basename ${qpath})${NC}"
        SPARQL_RAW=$(cat "${qpath}")
        SPARQL_ESCAPED=$(escape_sparql "${SPARQL_RAW}")
        QUERY_DATA="{\"operation\":\"query\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"sparql\":\"${SPARQL_ESCAPED}\"}"
        echo -e "${PURPLE}请求：${NC} $QUERY_DATA"
        QUERY_RESP=$(call_api "${QUERY_DATA}")
        echo -e "${GREEN}响应：${NC} ${QUERY_RESP}"
    else
        echo -e "${RED}错误：查询路径不存在 ${qpath}${NC}"
        exit 1
    fi
}

# ===================== 主逻辑 =====================
divider
echo -e "${PURPLE}          🚀 GStore 数据库测试脚本${NC}"
divider

# 1. 默认模式
if [ $# -eq 0 ]; then
    DB="lubm_auto_$(date +%s)"
    echo -e "\n${YELLOW}📌 默认测试：${DEFAULT_DATA}${NC}"

    echo -e "\n${BLUE}[1/5] build${NC}"
    BUILD_DATA="{\"operation\":\"build\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB}\",\"db_path\":\"${DEFAULT_DATA}\"}"
    echo -e "${PURPLE}请求：${NC} $BUILD_DATA"
    BUILD_RESP=$(call_api "$BUILD_DATA")
    echo -e "${GREEN}响应：${NC} $BUILD_RESP"

    echo -e "\n${BLUE}[2/5] load${NC}"
    LOAD_DATA="{\"operation\":\"load\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB}\"}"
    echo -e "${PURPLE}请求：${NC} $LOAD_DATA"
    LOAD_RESP=$(call_api "$LOAD_DATA")
    echo -e "${GREEN}响应：${NC} $LOAD_RESP"

    echo -e "\n${BLUE}[3/5] query${NC}"
    SPARQL_ESCAPED=$(escape_sparql "${DEFAULT_SPARQL}")
    QUERY_DATA="{\"operation\":\"query\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB}\",\"sparql\":\"${SPARQL_ESCAPED}\"}"
    echo -e "${PURPLE}请求：${NC} $QUERY_DATA"
    QUERY_RESP=$(call_api "${QUERY_DATA}")
    echo -e "${GREEN}响应：${NC} ${QUERY_RESP}"

    echo -e "\n${BLUE}[4/5] unload${NC}"
    UNLOAD_DATA="{\"operation\":\"unload\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB}\"}"
    echo -e "${PURPLE}请求：${NC} $UNLOAD_DATA"
    UNLOAD_RESP=$(call_api "$UNLOAD_DATA")

    echo -e "\n${BLUE}[5/5] drop${NC}"
    DROP_DATA="{\"operation\":\"drop\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB}\"}"
    echo -e "${PURPLE}请求：${NC} $DROP_DATA"
    DROP_RESP=$(call_api "$DROP_DATA")
    echo -e "${GREEN}响应：${NC} $DROP_RESP"

    exit 0
fi

# 2. 参数解析
db=""
data=""
q=""
while getopts ":hb:d:q:" opt; do
    case $opt in
        h) show_help; exit 0 ;;
        b) db="$OPTARG" ;;
        d) data="$OPTARG" ;;
        q) q="$OPTARG" ;;
        \?) echo "参数错误"; exit 1 ;;
    esac
done

# 3. 已存在库：-b db -q query → 不 drop
if [ -n "${db}" ] && [ -n "${q}" ]; then
    echo -e "\n${YELLOW}📌 测试已存在库：${db}${NC}"

    echo -e "\n${BLUE}[1/3] load${NC}"
    LOAD_DATA="{\"operation\":\"load\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\"}"
    echo -e "${PURPLE}请求：${NC} $LOAD_DATA"
    LOAD_RESP=$(call_api "$LOAD_DATA")
    echo -e "${GREEN}响应：${NC} $LOAD_RESP"

    echo -e "\n${BLUE}[2/3] query${NC}"
    run_query "${db}" "${q}"

    echo -e "\n${BLUE}[3/3] unload${NC}"
    UNLOAD_DATA="{\"operation\":\"unload\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\"}"
    echo -e "${PURPLE}请求：${NC} $UNLOAD_DATA"
    UNLOAD_RESP=$(call_api "$UNLOAD_DATA")

    exit 0
fi

# 4. 新建库：-d data -q query → 最后 drop
if [ -n "${data}" ] && [ -n "${q}" ]; then
    DB="auto_$(date +%s)"
    echo -e "\n${YELLOW}📌 新建库测试：${data}${NC}"

    echo -e "\n${BLUE}[1/5] build${NC}"
    BUILD_DATA="{\"operation\":\"build\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB}\",\"db_path\":\"${data}\"}"
    echo -e "${PURPLE}请求：${NC} $BUILD_DATA"
    BUILD_RESP=$(call_api "$BUILD_DATA")
    echo -e "${GREEN}响应：${NC} $BUILD_RESP"

    echo -e "\n${BLUE}[2/5] load${NC}"
    LOAD_DATA="{\"operation\":\"load\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB}\"}"
    echo -e "${PURPLE}请求：${NC} $LOAD_DATA"
    LOAD_RESP=$(call_api "$LOAD_DATA")
    echo -e "${GREEN}响应：${NC} $LOAD_RESP"

    echo -e "\n${BLUE}[3/5] query${NC}"
    run_query "${DB}" "${q}"

    echo -e "\n${BLUE}[4/5] unload${NC}"
    UNLOAD_DATA="{\"operation\":\"unload\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB}\"}"
    echo -e "${PURPLE}请求：${NC} $UNLOAD_DATA"
    UNLOAD_RESP=$(call_api "$UNLOAD_DATA")

    echo -e "\n${BLUE}[5/5] drop${NC}"
    DROP_DATA="{\"operation\":\"drop\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB}\"}"
    echo -e "${PURPLE}请求：${NC} $DROP_DATA"
    DROP_RESP=$(call_api "$DROP_DATA")
    echo -e "${GREEN}响应：${NC} $DROP_RESP"

    exit 0
fi

show_help
exit 0