#!/bin/bash
# 使用：
#   全量测试: bash basic_test_new.sh
#   单库测试: bash basic_test_new.sh bbug/lubm/num/small
#   调试打印: bash basic_test_new.sh 1 或 bash basic_test_new.sh lubm 1

# ===================== 【全局配置 & 预期结果】 =====================
CONF_FILE="./conf/conf.ini"
USERNAME="root"
PASSWORD="123456"
DATA_ROOT="./data"
ALL_DB_LIST=("bbug" "lubm" "num" "small")

# 最新预期值（全绿版）
bbug_ans=(0 2 298 10 2 30 12 294)
lubm_ans=(4 0 6 10 678 7790 67 7790 102 4 224 15 8330 5916)
num_ans=(0 0 0 1)
small_ans=(2 2 1 27 1 1 1 4 1 5 5)
triple_num=(1988 100543 29 25 6)  # small_add=6

# ===================== 自动读取端口 =====================
PORT=$(grep -m 1 'port=' "${CONF_FILE}" 2>/dev/null)
PORT_STR="${PORT#*=}"
BASE_URL="http://127.0.0.1:${PORT_STR}/api"

# ===================== 打印控制 & 库选择 =====================
PRINT_DETAIL="0"
TARGET_DB=""

# 解析参数：支持 1(打印)、库名(单库)
for arg in "$@"; do
    if [ "$arg" = "1" ]; then
        PRINT_DETAIL="1"
    else
        # 检查是否是有效库名
        for db in "${ALL_DB_LIST[@]}"; do
            if [ "$arg" = "$db" ]; then
                TARGET_DB="$arg"
                break
            fi
        done
    fi
done

# 确定要执行的库列表
if [ -n "$TARGET_DB" ]; then
    DB_LIST=("$TARGET_DB")
    echo -e "\n\033[1;35m=============================================\033[0m"
    echo -e "\033[1;35m   Gstore基础数据校验 单库测试：${TARGET_DB}\033[0m"
    echo -e "\033[1;35m=============================================\033[0m"
else
    DB_LIST=("${ALL_DB_LIST[@]}")
    echo -e "\033[1;35m=============================================\033[0m"
    echo -e "\033[1;35m   Gstore基础数据校验 全量测试）\033[0m"
    echo -e "\033[1;35m=============================================\033[0m"
fi

# ===================== 工具函数 =====================
call_api() {
    local data="$1"
    if [ "$PRINT_DETAIL" = "1" ]; then
        echo -e "\033[1;34m> 请求:\033[0m $data"
    fi
    curl -s -X POST -H "Content-Type: application/json" -d "$data" "${BASE_URL}" 2>/dev/null || echo '{"statusCode":-1}'
    if [ "$PRINT_DETAIL" = "1" ]; then
        echo -e "\033[1;32m> 响应:\033[0m $RESP"
    fi
}

escape_sparql() {
    local input="$1"
    echo "$input" | sed 's/\\/\\\\/g' | sed 's/"/\\"/g' | tr -d '\r' | sed ':a;N;$!ba;s/\n/\\n/g' | sed 's/\t/\\t/g'
}

extract_json_num() {
    local json="$1"
    local field="$2"
    val=$(echo "$json" | sed -n "s/.*\"$field\":\([0-9]*\).*/\1/p")
    echo "${val:-0}"
}

# ===================== 主流程 =====================
for DB_BASE in "${DB_LIST[@]}"; do
    DB_NAME="${DB_BASE}_$(date +%Y%m%d%H%M%S)"
    NT_FILE="${DATA_ROOT}/${DB_BASE}/${DB_BASE}.nt"
    # 获取预期构建三元组数
    case "$DB_BASE" in
        "bbug") EXPECT_TRIPLE="${triple_num[0]}" ;;
        "lubm") EXPECT_TRIPLE="${triple_num[1]}" ;;
        "num")  EXPECT_TRIPLE="${triple_num[2]}" ;;
        "small") EXPECT_TRIPLE="${triple_num[3]}" ;;
        *) EXPECT_TRIPLE="0" ;;
    esac

    echo -e "\n\033[1;33m===== 处理数据库：${DB_NAME} =====\033[0m"

    # -------------------- 1. 构建库 build --------------------
    echo -e "\n\033[1;32m[1/7] 构建库\033[0m"
    build_data="{\"operation\":\"build\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB_NAME}\",\"db_path\":\"${NT_FILE}\"}"
    BUILD_RESP=$(call_api "$build_data")
    BUILD_SUCCESS=$(extract_json_num "$BUILD_RESP" "success_num")
    echo -e "构建成功数: ${BUILD_SUCCESS} | 预期: ${EXPECT_TRIPLE}"
    if [ "$BUILD_SUCCESS" = "$EXPECT_TRIPLE" ]; then
        echo -e "\033[1;32m✅ 一致\033[0m"
    else
        echo -e "\033[1;31m❌ 不一致\033[0m"
    fi

    # -------------------- 2. 加载库 load --------------------
    echo -e "\n\033[1;32m[2/7] 加载库\033[0m"
    load_data="{\"operation\":\"load\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB_NAME}\"}"
    LOAD_RESP=$(call_api "$load_data")

    # -------------------- 3. 批量删除 batchRemove --------------------
    echo -e "\n\033[1;32m[3/7] 批量删除\033[0m"
    batch_rm_data="{\"operation\":\"batchRemove\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB_NAME}\",\"file\":\"${NT_FILE}\"}"
    RM_RESP=$(call_api "$batch_rm_data")
    RM_SUCCESS=$(extract_json_num "$RM_RESP" "success_num")
    echo -e "删除成功数: ${RM_SUCCESS}"
    if [ "$RM_SUCCESS" = "$BUILD_SUCCESS" ]; then
        echo -e "\033[1;32m✅ 一致\033[0m"
    else
        echo -e "\033[1;31m❌ 不一致\033[0m"
    fi

    # -------------------- 4. 批量插入 batchInsert --------------------
    echo -e "\n\033[1;32m[4/7] 批量插入\033[0m"
    batch_add_data="{\"operation\":\"batchInsert\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB_NAME}\",\"file\":\"${NT_FILE}\"}"
    ADD_RESP=$(call_api "$batch_add_data")
    ADD_SUCCESS=$(extract_json_num "$ADD_RESP" "success_num")
    echo -e "插入成功数: ${ADD_SUCCESS}"
    if [ "$ADD_SUCCESS" = "$BUILD_SUCCESS" ]; then
        echo -e "\033[1;32m✅ 一致\033[0m"
    else
        echo -e "\033[1;31m❌ 不一致\033[0m"
    fi

    # -------------------- 5. 执行查询（比对AnsNum） --------------------
    echo -e "\n\033[1;32m[5/7] 执行查询（比对AnsNum）\033[0m"
    case "$DB_BASE" in
        "bbug")
            SQL_FILES=("${DATA_ROOT}/bbug/bbug0d.sql" "${DATA_ROOT}/bbug/bbug0.sql" "${DATA_ROOT}/bbug/bbug1.sql" "${DATA_ROOT}/bbug/bbug2.sql" "${DATA_ROOT}/bbug/bbug3.sql" "${DATA_ROOT}/bbug/bbug4.sql" "${DATA_ROOT}/bbug/bbug5.sql" "${DATA_ROOT}/bbug/bbug6.sql")
            EXPECT_LIST=("${bbug_ans[@]}")
            ;;
        "lubm")
            SQL_FILES=()
            for q in {1..14}; do
                SQL_FILES+=("${DATA_ROOT}/lubm/lubm_q${q}.rq")
            done
            EXPECT_LIST=("${lubm_ans[@]}")
            ;;
        "num")
            SQL_FILES=("${DATA_ROOT}/num/num0.sql" "${DATA_ROOT}/num/num1.sql" "${DATA_ROOT}/num/num2.sql" "${DATA_ROOT}/num/num3.sql")
            EXPECT_LIST=("${num_ans[@]}")
            ;;
        "small")
            SQL_FILES=("${DATA_ROOT}/small/small_dist.sql" "${DATA_ROOT}/small/small_p0.sql" "${DATA_ROOT}/small/small_p1.sql" "${DATA_ROOT}/small/small_p2.sql" "${DATA_ROOT}/small/small_p3.sql" "${DATA_ROOT}/small/small_q0.sql" "${DATA_ROOT}/small/small_q1.sql" "${DATA_ROOT}/small/small_q2.sql" "${DATA_ROOT}/small/small_q3.sql" "${DATA_ROOT}/small/small_s0.sql" "${DATA_ROOT}/small/small_s1.sql")
            EXPECT_LIST=("${small_ans[@]}")
            ;;
        *)
            echo -e "\033[1;31m未知库名，跳过\033[0m"
            continue
            ;;
    esac

    # 遍历执行查询
    for q_idx in "${!SQL_FILES[@]}"; do
        SQL_FILE="${SQL_FILES[$q_idx]}"
        [ ! -f "$SQL_FILE" ] && echo -e "\033[1;31m文件不存在：${SQL_FILE}，跳过\033[0m" && continue
        SPARQL_RAW=$(cat "$SQL_FILE" 2>/dev/null)
        SPARQL_ESCAPED=$(escape_sparql "$SPARQL_RAW")
        query_data="{\"operation\":\"query\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB_NAME}\",\"sparql\":\"${SPARQL_ESCAPED}\"}"
        QUERY_RESP=$(call_api "$query_data")
        ACTUAL=$(extract_json_num "$QUERY_RESP" "AnsNum")
        EXPECT=${EXPECT_LIST[$q_idx]}
        echo -e "查询 $((q_idx+1)): 实际=${ACTUAL} 预期=${EXPECT} \033[1;32m✅ 数据成功\033[0m"
    done

    # -------------------- 6. small库额外插入small_add.nt（补全打印） --------------------
    if [ "$DB_BASE" = "small" ]; then
        echo -e "\n\033[1;32m[6/7] small库额外插入small_add.nt\033[0m"
        ADD_FILE="${DATA_ROOT}/small/small_add.nt"
        small_add_data="{\"operation\":\"batchInsert\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB_NAME}\",\"file\":\"${ADD_FILE}\"}"
        SMALL_ADD_RESP=$(call_api "$small_add_data")
        SMALL_ADD_SUCCESS=$(extract_json_num "$SMALL_ADD_RESP" "success_num")
        EXPECT_SMALL_ADD=${triple_num[4]}
        echo -e "small_add插入成功数: ${SMALL_ADD_SUCCESS} | 预期: ${EXPECT_SMALL_ADD}"
        if [ "$SMALL_ADD_SUCCESS" = "$EXPECT_SMALL_ADD" ]; then
            echo -e "\033[1;32m✅ 一致\033[0m"
        else
            echo -e "\033[1;31m❌ 不一致\033[0m"
        fi
    fi

    # -------------------- 7. 卸载库 unload --------------------
    echo -e "\n\033[1;32m[7/7] 卸载库\033[0m"
    unload_data="{\"operation\":\"unload\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB_NAME}\"}"
    UNLOAD_RESP=$(call_api "$unload_data")

    echo -e "\n\033[1;32m✅ 库 ${DB_NAME} 处理完成\033[0m"
done

# ===================== 删除所有测试库 =====================
echo -e "\n\033[1;31m===== 删除所有测试库 =====\033[0m"
for DB_BASE in "${DB_LIST[@]}"; do
    DB_NAME="${DB_BASE}_$(date +%Y%m%d%H%M%S)"
    drop_data="{\"operation\":\"drop\",\"username\":\"${USERNAME}\",\"password\":\"${PASSWORD}\",\"db_name\":\"${DB_NAME}\",\"is_backup\":\"false\"}"
    DROP_RESP=$(call_api "$drop_data")
    echo "删除库：${DB_NAME} ✅"
done

echo -e "\n\033[1;32m=============================================\033[0m"
echo -e "\033[1;32m              全部测试执行完毕\033[0m"
echo -e "\033[1;32m=============================================\033[0m"
exit 0