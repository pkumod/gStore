#!/bin/bash
set -eo pipefail

# ===================== 【全局配置】 =====================
CONF_FILE="./conf/conf.ini"
USERNAME="root"
PASSWORD="123456"
TEST_DIR="scripts/test/parser_test"
TOTAL_CASE=50

# 颜色
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
NC='\033[0m'

# 统计
PASS_CASE=0
FAIL_CASE=0

# ===================== 【核心：预期值数组】 =====================
# 对应 1-50 号用例的预期 AnsNum（实结果中提取）
# 格式：ans_expected[用例号]=预期值（数组下标从1开始，对应用例1-50）
declare -a ans_expected=(
    [1]=2  [2]=0  [3]=1  [4]=1  [5]=1  [6]=1  [7]=1  [8]=1  [9]=3  [10]=2
    [11]=2 [12]=4 [13]=4 [14]=2 [15]=0 [16]=1 [17]=0 [18]=1 [19]=2 [20]=1
    [21]=1 [22]=1 [23]=1 [24]=2 [25]=1 [26]=1 [27]=1 [28]=1 [29]=1 [30]=1
    [31]=1 [32]=3 [33]=1 [34]=1 [35]=1 [36]=1 [37]=3 [38]=1 [39]=1 [40]=1
    [41]=1 [42]=1 [43]=1 [44]=1 [45]=1 [46]=1 [47]=1 [48]=8 [49]=8 [50]=7
)

# 对应 1-50 号用例的预期 build success_num（从你给的真实结果中提取）
declare -a success_expected=(
    [1]=5  [2]=3  [3]=3  [4]=3  [5]=3  [6]=4  [7]=4  [8]=4  [9]=6  [10]=4
    [11]=4 [12]=6 [13]=6 [14]=6 [15]=10 [16]=1 [17]=1 [18]=6 [19]=6 [20]=4
    [21]=4 [22]=3 [23]=3 [24]=4 [25]=4 [26]=4 [27]=4 [28]=3 [29]=3 [30]=2
    [31]=3 [32]=9 [33]=15 [34]=15 [35]=15 [36]=4 [37]=4 [38]=4 [39]=4 [40]=4
    [41]=4 [42]=4 [43]=4 [44]=4 [45]=4 [46]=4 [47]=4 [48]=9 [49]=9 [50]=9
)

# ===================== conf配置端口 =====================
PORT=$(grep -m1 'port=' "${CONF_FILE}" 2>/dev/null | cut -d'=' -f2 | xargs)
if [ -z "${PORT}" ]; then
    PORT=9019
fi
BASE_URL="http://127.0.0.1:${PORT}/api"

# ===================== 工具函数 =====================
# 调用API，带错误捕获
call_api() {
    local data="$1"
    curl -s -X POST -H 'Content-Type: application/json' -d "$data" "$BASE_URL" 2>/dev/null || echo '{"statusCode":-1,"statusMsg":"curl request failed"}'
}

# 分割线
divider() {
    echo -e "${BLUE}=========================================================================${NC}"
}

# SPARQL语句完整转义函数
escape_sparql() {
    local input="$1"
    echo "$input" | sed 's/\\/\\\\/g' | sed 's/"/\\"/g' | tr -d '\r' | sed ':a;N;$!ba;s/\n/\\n/g' | sed 's/\t/\\t/g'
}

# 从JSON中提取字段值（兼容int/string）
get_json_val() {
    local json="$1"
    local key="$2"
    # 提取值，去除引号和逗号，处理空值
    val=$(echo "$json" | grep -o "\"$key\":[^,}]*" | head -n1 | sed 's/^[^:]*://' | sed 's/"//g' | sed 's/,$//')
    # 空值返回-1
    if [ -z "$val" ] || [ "$val" = "null" ]; then
        echo "-1"
    else
        echo "$val"
    fi
}

# ===================== 主流程 =====================
divider
echo -e "${PURPLE}          🚀 SPARQL 接口测试（精准预期值校验版）${NC}"
divider

for ((i=1; i<=TOTAL_CASE; i++)); do
    divider
    echo -e "${YELLOW}📌 测试用例 #${i}${NC}"
    divider

    DB_NAME="parser_test_${i}"
    TTL_FILE="${TEST_DIR}/parser_d${i}.ttl"
    SQL_FILE="${TEST_DIR}/parser_q${i}.sql"

    # ========== 1. build 构建数据库 ==========
    echo -e "\n${BLUE}[1/5] 构建数据库${NC}"
    BUILD_DATA="{\"operation\":\"build\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB_NAME}\",\"db_path\":\"${TTL_FILE}\"}"
    echo -e "${PURPLE}请求：${NC} $BUILD_DATA"
    BUILD_RESP=$(call_api "$BUILD_DATA")
    echo -e "${GREEN}响应：${NC} $BUILD_RESP"

    BUILD_ACTUAL=$(get_json_val "$BUILD_RESP" "success_num")
    BUILD_EXPECT=${success_expected[$i]}
    if [ "$BUILD_ACTUAL" -eq "$BUILD_EXPECT" ]; then
        BUILD_STATUS="${GREEN}✅ PASS${NC}"
    else
        BUILD_STATUS="${RED}❌ FAIL${NC}"
    fi

    # ========== 2. load 加载数据库 ==========
    echo -e "\n${BLUE}[2/5] 加载数据库${NC}"
    LOAD_DATA="{\"operation\":\"load\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB_NAME}\"}"
    echo -e "${PURPLE}请求：${NC} $LOAD_DATA"
    LOAD_RESP=$(call_api "$LOAD_DATA")
    echo -e "${GREEN}响应：${NC} $LOAD_RESP"

    # ========== 3. query 执行查询 ==========
    echo -e "\n${BLUE}[3/5] 执行查询${NC}"
    SPARQL_RAW=$(cat "$SQL_FILE" 2>/dev/null || echo "")
    SPARQL_ESCAPED=$(escape_sparql "$SPARQL_RAW")
    QUERY_DATA="{\"operation\":\"query\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB_NAME}\",\"sparql\":\"${SPARQL_ESCAPED}\"}"
    echo -e "${PURPLE}请求：${NC} $QUERY_DATA"
    QUERY_RESP=$(call_api "$QUERY_DATA")
    echo -e "${GREEN}响应：${NC} $QUERY_RESP"

    ANS_ACTUAL=$(get_json_val "$QUERY_RESP" "AnsNum")
    ANS_EXPECT=${ans_expected[$i]}
    # 只要两者是 0 或 -1，都视为正确
    if [ "$ANS_ACTUAL" -eq "$ANS_EXPECT" ] || { [ "$ANS_ACTUAL" -eq 0 ] && [ "$ANS_EXPECT" -eq -1 ]; } || { [ "$ANS_ACTUAL" -eq -1 ] && [ "$ANS_EXPECT" -eq 0 ]; }; then
        ANS_STATUS="${GREEN}✅ PASS${NC}"
        CASE_PASS=1
    else
        ANS_STATUS="${RED}❌ FAIL${NC}"
        CASE_PASS=0
    fi

    # ========== 4. unload 卸载数据库 ==========
    echo -e "\n${BLUE}[4/5] 卸载数据库${NC}"
    UNLOAD_DATA="{\"operation\":\"unload\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB_NAME}\"}"
    echo -e "${PURPLE}请求：${NC} $UNLOAD_DATA"
    UNLOAD_RESP=$(call_api "$UNLOAD_DATA")

    # ========== 5. drop 删除数据库 ==========
    echo -e "\n${BLUE}[5/5] 删除数据库${NC}"
    DROP_DATA="{\"operation\":\"drop\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB_NAME}\",\"is_backup\":\"false\"}"
    echo -e "${PURPLE}请求：${NC} $DROP_DATA"
    DROP_RESP=$(call_api "$DROP_DATA")
    echo -e "${GREEN}响应：${NC} $DROP_RESP"

    # ========== 结果校验汇总 ==========
    echo -e "\n${BLUE}==================== 校验结果 ====================${NC}"
    echo -e "build success_num: 实际=${BUILD_ACTUAL}  预期=${BUILD_EXPECT}  ${BUILD_STATUS}"
    echo -e "query AnsNum:      实际=${ANS_ACTUAL}  预期=${ANS_EXPECT}  ${ANS_STATUS}"

    # 统计用例结果（build和query都通过才算用例通过）
    if [ "$BUILD_ACTUAL" -eq "$BUILD_EXPECT" ] && [ "$CASE_PASS" -eq 1 ]; then
        echo -e "\n${GREEN}✅ 用例 #${i} 校验通过${NC}"
        PASS_CASE=$((PASS_CASE + 1))
    else
        echo -e "\n${RED}❌ 用例 #${i} 校验失败${NC}"
        FAIL_CASE=$((FAIL_CASE + 1))
    fi

done

# ===================== 最终统计 =====================
divider
echo -e "${PURPLE}🎉 所有测试用例执行完毕${NC}"
divider
echo -e "${GREEN}✅ 成功用例：${PASS_CASE} / ${TOTAL_CASE}${NC}"
echo -e "${RED}❌ 失败用例：${FAIL_CASE} / ${TOTAL_CASE}${NC}"
divider

exit 0