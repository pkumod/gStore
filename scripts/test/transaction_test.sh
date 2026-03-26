#!/bin/bash
set -eo pipefail

# ===================== 【全局配置】 =====================
CONF_FILE="./conf/conf.ini"
USERNAME="root"
PASSWORD="123456"
# 自动创建空库（无需数据文件）
DB_NAME="txn_test_$(date +%s)"

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
    echo -e "${PURPLE}【请求】${NC}"
    echo "$data"
    local resp
    resp=$(curl -s -X POST -H 'Content-Type: application/json' -d "$data" "$BASE_URL" 2>/dev/null || echo '{"statusCode":-1,"statusMsg":"curl request failed"}')
    echo -e "${GREEN}【响应】${NC}"
    echo "$resp"
    echo -e "${BLUE}----------------------------------------${NC}"
}

escape_sparql() {
    local input="$1"
    echo "$input" | sed 's/\\/\\\\/g' | sed 's/"/\\"/g' | tr -d '\r' | sed ':a;N;$!ba;s/\n/\\n/g' | sed 's/\t/\\t/g'
}

divider() {
    echo -e "${BLUE}=========================================================================${NC}"
}

# ===================== 事务1：txn1（Commit） =====================
run_txn1() {
    local db="$1"
    divider
    echo -e "${YELLOW}🚀 执行事务 #1（Commit）${NC}"
    divider

    # 1. Begin 事务（使用默认隔离级别1:SR）
    echo -e "${BLUE}[1/8] Begin 事务${NC}"
    begin_resp=$(call_api "{\"operation\":\"begin\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"isolevel\":\"1\"}")
    # 提取返回的TID
    tid=$(echo "$begin_resp" | grep -o '"TID":"[^"]*"' | cut -d'"' -f4)
    echo -e "${YELLOW}获取事务ID: ${tid}${NC}"

    # 2. 执行 update2: delete where {?x <mingzhi> ?y}
    echo -e "${BLUE}[2/8] 执行 delete where {?x <mingzhi> ?y}${NC}"
    sparql="delete where {?x <mingzhi> ?y}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 3. 执行 query1: select ?x where { ?x <mingzhi> ?y}
    echo -e "${BLUE}[3/8] 执行 select ?x where { ?x <mingzhi> ?y}${NC}"
    sparql="select ?x where { ?x <mingzhi> ?y}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 4. 执行 update1: insert data { <me> <mingzhi> <zhangzhe>}
    echo -e "${BLUE}[4/8] 执行 insert data { <me> <mingzhi> <zhangzhe>}${NC}"
    sparql="insert data { <me> <mingzhi> <zhangzhe>}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 5. 执行 update3: insert data { <you> <mingzhi> <SBBBBB>}
    echo -e "${BLUE}[5/8] 执行 insert data { <you> <mingzhi> <SBBBBB>}${NC}"
    sparql="insert data { <you> <mingzhi> <SBBBBB>}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 6. 再次执行 query1
    echo -e "${BLUE}[6/8] 再次执行 select ?x where { ?x <mingzhi> ?y}${NC}"
    sparql="select ?x where { ?x <mingzhi> ?y}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 7. 执行 update4: delete data { <you> <mingzhi> <SBBBBB>}
    echo -e "${BLUE}[7/8] 执行 delete data { <you> <mingzhi> <SBBBBB>}${NC}"
    sparql="delete data { <you> <mingzhi> <SBBBBB>}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 8. 再次执行 query1
    echo -e "${BLUE}[8/8] 再次执行 select ?x where { ?x <mingzhi> ?y}${NC}"
    sparql="select ?x where { ?x <mingzhi> ?y}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # Commit 事务
    echo -e "${BLUE}✅ Commit 事务 #1${NC}"
    call_api "{\"operation\":\"commit\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\"}"
}

# ===================== 事务2：txn2（Rollback） =====================
run_txn2() {
    local db="$1"
    divider
    echo -e "${YELLOW}🚀 执行事务 #2（Rollback）${NC}"
    divider

    # 1. Begin 事务
    echo -e "${BLUE}[1/7] Begin 事务${NC}"
    begin_resp=$(call_api "{\"operation\":\"begin\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"isolevel\":\"1\"}")
    tid=$(echo "$begin_resp" | grep -o '"TID":"[^"]*"' | cut -d'"' -f4)
    echo -e "${YELLOW}获取事务ID: ${tid}${NC}"

    # 2. 执行 query1
    echo -e "${BLUE}[2/7] 执行 select ?x where { ?x <mingzhi> ?y}${NC}"
    sparql="select ?x where { ?x <mingzhi> ?y}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 3. 执行 update1
    echo -e "${BLUE}[3/7] 执行 insert data { <me> <mingzhi> <zhangzhe>}${NC}"
    sparql="insert data { <me> <mingzhi> <zhangzhe>}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 4. 再次执行 query1
    echo -e "${BLUE}[4/7] 再次执行 select ?x where { ?x <mingzhi> ?y}${NC}"
    sparql="select ?x where { ?x <mingzhi> ?y}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 5. 执行 update2
    echo -e "${BLUE}[5/7] 执行 delete where {?x <mingzhi> ?y}${NC}"
    sparql="delete where {?x <mingzhi> ?y}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 6. 再次执行 query1
    echo -e "${BLUE}[6/7] 再次执行 select ?x where { ?x <mingzhi> ?y}${NC}"
    sparql="select ?x where { ?x <mingzhi> ?y}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # Rollback 事务
    echo -e "${BLUE}🔄 Rollback 事务 #2${NC}"
    call_api "{\"operation\":\"rollback\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\"}"
}

# ===================== 事务3：txn3（异常回滚） =====================
run_txn3() {
    local db="$1"
    divider
    echo -e "${YELLOW}🚀 执行事务 #3（异常回滚测试）${NC}"
    divider

    # 1. Begin 事务
    echo -e "${BLUE}[1/3] Begin 事务${NC}"
    begin_resp=$(call_api "{\"operation\":\"begin\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"isolevel\":\"1\"}")
    tid=$(echo "$begin_resp" | grep -o '"TID":"[^"]*"' | cut -d'"' -f4)
    echo -e "${YELLOW}获取事务ID: ${tid}${NC}"

    # 2. 执行 delete where {?x ?y ?z}
    echo -e "${BLUE}[2/3] 执行 delete where {?x ?y ?z}${NC}"
    sparql="delete where {?x ?y ?z}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 3. Commit 事务
    echo -e "${BLUE}✅ Commit 事务 #3${NC}"
    call_api "{\"operation\":\"commit\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\"}"
}

# ===================== 事务4：txn4（gStore节点事务） =====================
run_txn4() {
    local db="$1"
    divider
    echo -e "${YELLOW}🚀 执行事务 #4（gStore节点事务）${NC}"
    divider

    # 1. Begin 事务
    echo -e "${BLUE}[1/6] Begin 事务${NC}"
    begin_resp=$(call_api "{\"operation\":\"begin\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"isolevel\":\"1\"}")
    tid=$(echo "$begin_resp" | grep -o '"TID":"[^"]*"' | cut -d'"' -f4)
    echo -e "${YELLOW}获取事务ID: ${tid}${NC}"

    # 2. 执行 query
    echo -e "${BLUE}[2/6] 执行 select ?x ?y where{<gStore> ?x ?y}${NC}"
    sparql="select ?x ?y where{<gStore> ?x ?y}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 3. 执行 insert
    echo -e "${BLUE}[3/6] 执行 insert data {<gStore> <test> <commit>.<gStore> <test> <abort>.<gStore> <test> <begin>.<gStore> <test> <all>.}${NC}"
    sparql="insert data {<gStore> <test> <commit>.<gStore> <test> <abort>.<gStore> <test> <begin>.<gStore> <test> <all>.}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 4. 再次执行 query
    echo -e "${BLUE}[4/6] 再次执行 select ?x ?y where{<gStore> ?x ?y}${NC}"
    sparql="select ?x ?y where{<gStore> ?x ?y}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 5. 执行 remove
    echo -e "${BLUE}[5/6] 执行 delete where {<gStore> ?x ?y}${NC}"
    sparql="delete where {<gStore> ?x ?y}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 6. 再次执行 query
    echo -e "${BLUE}[6/6] 再次执行 select ?x ?y where{<gStore> ?x ?y}${NC}"
    sparql="select ?x ?y where{<gStore> ?x ?y}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # Commit 事务
    echo -e "${BLUE}✅ Commit 事务 #4${NC}"
    call_api "{\"operation\":\"commit\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\"}"
}

# ===================== 事务5：test（增量插入事务） =====================
run_test() {
    local db="$1"
    divider
    echo -e "${YELLOW}🚀 执行事务 #5（增量插入事务）${NC}"
    divider

    # 1. Begin 事务
    echo -e "${BLUE}[1/8] Begin 事务${NC}"
    begin_resp=$(call_api "{\"operation\":\"begin\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"isolevel\":\"1\"}")
    tid=$(echo "$begin_resp" | grep -o '"TID":"[^"]*"' | cut -d'"' -f4)
    echo -e "${YELLOW}获取事务ID: ${tid}${NC}"

    # 2. 执行 inc1
    echo -e "${BLUE}[2/8] 执行 insert data {<V1> <R1> \"0\".}${NC}"
    sparql="insert data {<V1> <R1> \"0\".}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 3. 执行 query
    echo -e "${BLUE}[3/8] 执行 select ?x ?y where{?x <R1> ?y}${NC}"
    sparql="select ?x ?y where{?x <R1> ?y}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 4. 执行 inc2
    echo -e "${BLUE}[4/8] 执行 insert data {<V1> <R1> \"10\".}${NC}"
    sparql="insert data {<V1> <R1> \"10\".}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 5. 再次执行 query
    echo -e "${BLUE}[5/8] 再次执行 select ?x ?y where{?x <R1> ?y}${NC}"
    sparql="select ?x ?y where{?x <R1> ?y}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 6. 执行 inc3
    echo -e "${BLUE}[6/8] 执行 insert data {<V2> <R1> \"20\".}${NC}"
    sparql="insert data {<V2> <R1> \"20\".}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 7. 再次执行 query
    echo -e "${BLUE}[7/8] 再次执行 select ?x ?y where{?x <R1> ?y}${NC}"
    sparql="select ?x ?y where{?x <R1> ?y}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 8. Commit 事务
    echo -e "${BLUE}✅ Commit 事务 #5${NC}"
    call_api "{\"operation\":\"commit\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\"}"
}

# ===================== 事务6：test2（更新删除事务） =====================
run_test2() {
    local db="$1"
    divider
    echo -e "${YELLOW}🚀 执行事务 #6（更新删除事务）${NC}"
    divider

    # 1. Begin 事务
    echo -e "${BLUE}[1/8] Begin 事务${NC}"
    begin_resp=$(call_api "{\"operation\":\"begin\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"isolevel\":\"1\"}")
    tid=$(echo "$begin_resp" | grep -o '"TID":"[^"]*"' | cut -d'"' -f4)
    echo -e "${YELLOW}获取事务ID: ${tid}${NC}"

    # 2. 执行 query
    echo -e "${BLUE}[2/8] 执行 select ?v {<V1> <R1> ?v.}${NC}"
    sparql="select ?v {<V1> <R1> ?v.}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 3. 执行 delete1
    echo -e "${BLUE}[3/8] 执行 delete data { <V1> <R1> \"0\"^^<http://www.w3.org/2001/XMLSchema#integer> }${NC}"
    sparql="delete data { <V1> <R1> \"0\"^^<http://www.w3.org/2001/XMLSchema#integer> }"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 4. 执行 insert1
    echo -e "${BLUE}[4/8] 执行 insert data { <V1> <R1> \"1\"^^<http://www.w3.org/2001/XMLSchema#integer> }${NC}"
    sparql="insert data { <V1> <R1> \"1\"^^<http://www.w3.org/2001/XMLSchema#integer> }"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 5. 执行 query
    echo -e "${BLUE}[5/8] 执行 select ?v {<V1> <R1> ?v.}${NC}"
    sparql="select ?v {<V1> <R1> ?v.}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 6. 执行 delete2
    echo -e "${BLUE}[6/8] 执行 delete data { <V1> <R1> \"1\"^^<http://www.w3.org/2001/XMLSchema#integer> }${NC}"
    sparql="delete data { <V1> <R1> \"1\"^^<http://www.w3.org/2001/XMLSchema#integer> }"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 7. 执行 insert2
    echo -e "${BLUE}[7/8] 执行 insert data { <V1> <R1> \"2\"^^<http://www.w3.org/2001/XMLSchema#integer> }${NC}"
    sparql="insert data { <V1> <R1> \"2\"^^<http://www.w3.org/2001/XMLSchema#integer> }"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # 8. 执行 query
    echo -e "${BLUE}[8/8] 执行 select ?v {<V1> <R1> ?v.}${NC}"
    sparql="select ?v {<V1> <R1> ?v.}"
    esc=$(escape_sparql "$sparql")
    call_api "{\"operation\":\"tquery\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\",\"sparql\":\"${esc}\"}"

    # Commit 事务
    echo -e "${BLUE}✅ Commit 事务 #6${NC}"
    call_api "{\"operation\":\"commit\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${db}\",\"tid\":\"${tid}\"}"
}

# ===================== 主流程 =====================
divider
echo -e "${PURPLE}          🚀 GStore 事务完整测试脚本${NC}"
divider
echo -e "${YELLOW}📌 自动创建空库：${DB_NAME}${NC}"

# 1. Build 空库（无需数据文件，直接创建空库）
echo -e "\n${BLUE}[1/8] Build 空库${NC}"
call_api "{\"operation\":\"build\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB_NAME}\",\"db_path\":\"\"}"

# 2. Load 库
echo -e "\n${BLUE}[2/8] Load 库${NC}"
call_api "{\"operation\":\"load\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB_NAME}\"}"

# 3. 执行6组事务
run_txn1 "${DB_NAME}"
run_txn2 "${DB_NAME}"
run_txn3 "${DB_NAME}"
run_txn4 "${DB_NAME}"
run_test "${DB_NAME}"
run_test2 "${DB_NAME}"

# 4. Unload 库
echo -e "\n${BLUE}[7/8] Unload 库${NC}"
call_api "{\"operation\":\"unload\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB_NAME}\"}"

# 5. Drop 库
echo -e "\n${BLUE}[8/8] Drop 库${NC}"
call_api "{\"operation\":\"drop\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB_NAME}\"}"

divider
echo -e "${GREEN}🎉 所有事务测试执行完毕${NC}"
divider
exit 0