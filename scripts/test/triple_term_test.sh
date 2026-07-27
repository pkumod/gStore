#!/bin/bash
set -eo pipefail

# ===================== 【全局配置】 =====================
CONF_FILE="./conf/conf.ini"
USERNAME="root"
PASSWORD="123456"
TEST_DATA_DIR="scripts/test/triple_term_data"

# 颜色
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
NC='\033[0m'

# ===================== 自动获取端口 =====================
PORT=$(grep -m 1 'port=' "${CONF_FILE}" 2>/dev/null | cut -d'=' -f2 | xargs)
if [ -z "${PORT}" ]; then
    PORT=9000
fi
BASE_URL="http://127.0.0.1:${PORT}/api"

# ===================== 工具函数 =====================
call_api() {
    local data="$1"
    curl -s --noproxy '*' --max-time 30 -X POST \
        -H 'Content-Type: application/json' \
        -d "$data" "${BASE_URL}" 2>/dev/null || echo '{"StatusCode":-1}'
}

divider() {
    echo -e "${BLUE}=========================================================================${NC}"
}

# ===================== 单用例校验 =====================
check_case() {
    local case_name="$1"
    local result="$2"
    local python_check="$3"
    echo "$result" | python3 -c "$python_check" 2>/dev/null && {
        echo -e "  ${GREEN}PASS${NC}: $case_name"
        return 0
    } || {
        echo -e "  ${RED}FAIL${NC}: $case_name"
        return 1
    }
}

# ===================== 清理旧数据 =====================
cleanup_dbs() {
    for db in tt_test1 tt_test2 tt_test3 tt_test4; do
        rm -rf dbhome/${db}.db 2>/dev/null || true
    done
}

# ===================== 测试 1：N-Triples 三元组词项构建 =====================
test_build_ntriples() {
    divider
    echo -e "${PURPLE}📌 测试 1：N-Triples 三元组词项构建${NC}"
    divider
    local result=$(call_api '{"operation":"build","username":"root","password":"123456","db_name":"tt_test1","db_path":"'"${TEST_DATA_DIR}"'/tt1.nt"}')
    echo -e "${YELLOW}响应：${NC}$result"
    check_case "N-Triples build (2 triples)" "$result" "
import sys,json
d=json.load(sys.stdin)
assert d['StatusCode']==0, 'build failed'
assert d['success_num']==2, f'expected 2, got {d.get(\"success_num\")}'
print('OK')
"
}

# ===================== 测试 2：三元组词项 JSON 序列化 =====================
test_json_format() {
    divider
    echo -e "${PURPLE}📌 测试 2：三元组词项 JSON 序列化${NC}"
    divider
    call_api '{"operation":"load","username":"root","password":"123456","db_name":"tt_test1"}' > /dev/null
    local result=$(call_api '{"operation":"query","username":"root","password":"123456","db_name":"tt_test1","sparql":"SELECT * WHERE { ?s ?p ?o }","format":"json"}')
    check_case "JSON triple term format" "$result" "
import sys,json
d=json.load(sys.stdin)
bindings=d.get('results',{}).get('bindings',[])
assert len(bindings)==2, f'expected 2, got {len(bindings)}'
b0=bindings[0]
assert b0['o']['type']=='triple', f'expected triple, got {b0[\"o\"].get(\"type\")}'
assert 'value' in b0['o'], 'missing value'
assert 'subject' in b0['o']['value'], 'missing subject'
assert 'predicate' in b0['o']['value'], 'missing predicate'
assert 'object' in b0['o']['value'], 'missing object'
print('OK')
"
}

# ===================== 测试 3：嵌套三元组词项 =====================
test_nested() {
    divider
    echo -e "${PURPLE}📌 测试 3：嵌套三元组词项${NC}"
    divider
    cat > "${TEST_DATA_DIR}/tt_nested.nt" << 'NESTED_EOF'
<http://example/s> <http://example/p> <<( <http://example/s1> <http://example/p1> <<( <http://example/s2> <http://example/p2> <http://example/o2> )>> )>> .
NESTED_EOF
    local result=$(call_api '{"operation":"build","username":"root","password":"123456","db_name":"tt_test2","db_path":"'"${TEST_DATA_DIR}"'/tt_nested.nt"}')
    call_api '{"operation":"load","username":"root","password":"123456","db_name":"tt_test2"}' > /dev/null
    result=$(call_api '{"operation":"query","username":"root","password":"123456","db_name":"tt_test2","sparql":"SELECT * WHERE { ?s ?p ?o }","format":"json"}')
    check_case "Nested triple term" "$result" "
import sys,json
d=json.load(sys.stdin)
bindings=d.get('results',{}).get('bindings',[])
assert len(bindings)==1, f'expected 1, got {len(bindings)}'
o=bindings[0]['o']
assert o['type']=='triple', 'outer not triple'
assert o['value']['object']['type']=='triple', 'inner not triple'
print('OK')
"
}

# ===================== 测试 4：rdf:reifies 三元组词项 =====================
test_reifies() {
    divider
    echo -e "${PURPLE}📌 测试 4：rdf:reifies 三元组词项${NC}"
    divider
    cat > "${TEST_DATA_DIR}/tt_reifies.nt" << 'REIFIES_EOF'
<http://example/a> <http://www.w3.org/1999/02/22-rdf-syntax-ns#reifies> <<( <http://example/s> <http://example/p> <http://example/o> )>> .
REIFIES_EOF
    local result=$(call_api '{"operation":"build","username":"root","password":"123456","db_name":"tt_test3","db_path":"'"${TEST_DATA_DIR}"'/tt_reifies.nt"}')
    call_api '{"operation":"load","username":"root","password":"123456","db_name":"tt_test3"}' > /dev/null
    result=$(call_api '{"operation":"query","username":"root","password":"123456","db_name":"tt_test3","sparql":"SELECT * WHERE { ?s ?p ?o }","format":"json"}')
    check_case "rdf:reifies" "$result" "
import sys,json
d=json.load(sys.stdin)
bindings=d.get('results',{}).get('bindings',[])
assert len(bindings)==1, f'expected 1, got {len(bindings)}'
assert bindings[0]['o']['type']=='triple'
print('OK')
"
}

# ===================== 测试 5：紧凑格式三元组词项 =====================
test_compact() {
    divider
    echo -e "${PURPLE}📌 测试 5：紧凑无空格三元组词项${NC}"
    divider
    cat > "${TEST_DATA_DIR}/tt_compact.nt" << 'COMPACT_EOF'
<http://example/s><http://example/p><<(<http://example/s1><http://example/p1><http://example/o1>)>>.
COMPACT_EOF
    local result=$(call_api '{"operation":"build","username":"root","password":"123456","db_name":"tt_test4","db_path":"'"${TEST_DATA_DIR}"'/tt_compact.nt"}')
    call_api '{"operation":"load","username":"root","password":"123456","db_name":"tt_test4"}' > /dev/null
    result=$(call_api '{"operation":"query","username":"root","password":"123456","db_name":"tt_test4","sparql":"SELECT * WHERE { ?s ?p ?o }","format":"json"}')
    check_case "Compact triple term" "$result" "
import sys,json
d=json.load(sys.stdin)
bindings=d.get('results',{}).get('bindings',[])
assert len(bindings)==1, f'expected 1, got {len(bindings)}'
assert bindings[0]['o']['type']=='triple'
print('OK')
"
}

# ===================== 主流程 =====================
main() {
    divider
    echo -e "${PURPLE}          🚀 gStore TripleTerm 综合功能测试${NC}"
    divider

    cleanup_dbs

    local pass=0
    local fail=0

    test_build_ntriples   && pass=$((pass+1)) || fail=$((fail+1))
    test_json_format      && pass=$((pass+1)) || fail=$((fail+1))
    test_nested           && pass=$((pass+1)) || fail=$((fail+1))
    test_reifies          && pass=$((pass+1)) || fail=$((fail+1))
    test_compact          && pass=$((pass+1)) || fail=$((fail+1))

    divider
    echo -e "${GREEN}✅ 通过：${pass} / 5${NC}"
    if [ $fail -gt 0 ]; then
        echo -e "${RED}❌ 失败：${fail} / 5${NC}"
    fi
    divider

    cleanup_dbs
    return $fail
}

main "$@"
