#!/bin/bash
set -eo pipefail

# ===================== 配置 =====================
CONF_FILE="./conf/conf.ini"
USERNAME="root"
PASSWORD="123456"

# 颜色
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
NC='\033[0m'

# ===================== 自动获取端口 =====================
PORT=$(grep -m1 'port=' "${CONF_FILE}" 2>/dev/null | cut -d'=' -f2 | xargs)
if [ -z "${PORT}" ]; then
    PORT=9000
fi
API_URL="http://127.0.0.1:${PORT}/api"

divider() {
    echo -e "${BLUE}=========================================================================${NC}"
}

# ===================== 检查服务是否启动 =====================
check_service() {
    echo -e "${YELLOW}🔍 检查 gStore 服务状态...${NC}"
    response=$(curl -s -X POST \
        -H 'Content-Type: application/json' \
        -d '{"operation":"check"}' \
        "${API_URL}" 2>/dev/null || echo "failed")

    if echo "${response}" | grep -q '"StatusCode":0'; then
        echo -e "${GREEN}✅ 服务已启动${NC}"
        return 0  # 已启动
    else
        echo -e "${RED}❌ 服务未启动${NC}"
        return 1  # 未启动
    fi
}

# ===================== 启动/停止服务 =====================
start_service() {
    echo -e "${YELLOW}🚀 启动 gStore 服务...${NC}"
    ./bin/gserver -b
    sleep 3  # 等待启动完成
}

stop_service() {
    echo -e "${YELLOW}🛑 停止 gStore 服务...${NC}"
    ./bin/gserver -t
    sleep 2
}

# ===================== 执行单个测试 =====================
run_test() {
    local test_script="$1"
    divider
    echo -e "${PURPLE}🚀 开始执行：${test_script}${NC}"
    divider
    bash "${test_script}"
    echo -e "${GREEN}✅ ${test_script} 执行完成${NC}"
    sleep 1
}

# ===================== 主流程 =====================
divider
echo -e "${PURPLE}          🚀 自动执行全部 6 个测试脚本${NC}"
divider

# 1. 检查服务
need_stop=0
if ! check_service; then
    need_stop=1
    start_service
fi

divider
echo -e "${YELLOW}📌 开始执行所有测试...${NC}"
divider

# 2. 依次执行 6 个脚本
run_test "scripts/test/gtest.sh"
run_test "scripts/test/basic_test.sh"
run_test "scripts/test/parser_test.sh"
run_test "scripts/test/triple_term_test.sh"
run_test "scripts/test/update_test.sh"
run_test "scripts/test/transaction_test.sh"

divider
echo -e "${GREEN}🎉 所有 6 个测试脚本全部执行完毕！${NC}"
divider

# 3. 如果是脚本启动的服务，自动停止
if [ "${need_stop}" -eq 1 ]; then
    stop_service
    echo -e "${GREEN}✅ 服务已自动停止${NC}"
else
    echo -e "${YELLOW}⚠️  服务原本已启动，保持运行${NC}"
fi

divider
echo -e "${GREEN}🏁 全部流程结束${NC}"
divider
exit 0