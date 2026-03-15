#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"
MODELS_DIR="${ROOT_DIR}/data/vosk-models"

# Vosk 模型：中文与英文（与 https://github.com/alphacep/vosk-api 一致）
CN_MODEL_DIR="vosk-model-small-cn-0.22"
EN_MODEL_DIR="vosk-model-small-en-us-0.15"
CN_URL="https://alphacephei.com/vosk/models/vosk-model-small-cn-0.22.zip"
EN_URL="https://alphacephei.com/vosk/models/vosk-model-small-en-us-0.15.zip"

# 用户本地 libvosk（方法二：不装到系统）
VOSK_LIB_DIR="${HOME}/.local/lib/vosk"
VOSK_LIB_URL="https://github.com/alphacep/vosk-api/releases/download/v0.3.45/vosk-linux-x86_64-0.3.45.zip"

ensure_libvosk_local() {
    [[ "$(uname -s)" != "Linux" ]] && return 0
    if [[ -f "${VOSK_LIB_DIR}/libvosk.so" ]]; then
        echo "===> 用户本地 libvosk 已存在: ${VOSK_LIB_DIR}/libvosk.so"
        return 0
    fi
    echo "===> 未找到 libvosk，正在安装到 ${VOSK_LIB_DIR} ..."
    mkdir -p "${VOSK_LIB_DIR}"
    cd "${VOSK_LIB_DIR}"
    if command -v curl &>/dev/null; then
        curl -fSL -o vosk-linux-x86_64.zip "$VOSK_LIB_URL"
    elif command -v wget &>/dev/null; then
        wget -q --show-progress -O vosk-linux-x86_64.zip "$VOSK_LIB_URL"
    else
        echo "错误: 需要 curl 或 wget 以下载 libvosk"
        exit 1
    fi
    unzip -o -q vosk-linux-x86_64.zip
    rm -f vosk-linux-x86_64.zip
    if [[ ! -f libvosk.so ]]; then
        so_path=""
        while IFS= read -r -d '' p; do
            so_path="$p"
            break
        done < <(find . -name 'libvosk.so' -type f -print0 2>/dev/null)
        if [[ -n "$so_path" && "$so_path" != "./libvosk.so" ]]; then
            mv "$so_path" .
            find . -mindepth 1 -maxdepth 1 -type d -exec rm -rf {} +
        fi
    fi
    if [[ ! -f libvosk.so ]]; then
        echo "错误: 解压后未找到 libvosk.so"
        exit 1
    fi
    cd "${ROOT_DIR}"
    echo "===> libvosk 就绪: ${VOSK_LIB_DIR}/libvosk.so"
}

download_model_if_missing() {
    local name="$1"
    local dir="$2"
    local url="$3"
    local target="${MODELS_DIR}/${dir}"

    if [[ -d "$target" ]]; then
        echo "===> 模型已存在，跳过下载: $name ($dir)"
        return 0
    fi

    echo "===> 未找到模型 $name，开始下载: $url"
    mkdir -p "$MODELS_DIR"
    local zipfile="${MODELS_DIR}/${dir}.zip"

    if command -v curl &>/dev/null; then
        curl -fSL -o "$zipfile" "$url"
    elif command -v wget &>/dev/null; then
        wget -q --show-progress -O "$zipfile" "$url"
    else
        echo "错误: 需要 curl 或 wget 以下载模型"
        exit 1
    fi

    echo "===> 解压: $zipfile"
    unzip -o -q "$zipfile" -d "$MODELS_DIR"
    rm -f "$zipfile"
    echo "===> 模型就绪: $target"
}

echo "===> 检查用户本地 libvosk（未安装时自动下载到 ~/.local/lib/vosk）..."
ensure_libvosk_local
echo ""

echo "===> 检查 Vosk 语音模型..."
download_model_if_missing "中文" "$CN_MODEL_DIR" "$CN_URL"
download_model_if_missing "英文" "$EN_MODEL_DIR" "$EN_URL"
echo ""

echo "===> 构建 LingJianPinyin..."
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -- -j"$(nproc)"

echo
echo "===> 构建完成"
echo "可执行文件（UI Demo）：${BUILD_DIR}/src/ui/lingjian_ui"
echo

UI_BIN="${BUILD_DIR}/src/ui/lingjian_ui"
if [[ -x "${UI_BIN}" ]]; then
  echo "===> 启动灵键拼音候选框 UI，用于手动体验外观与渲染效果。"
  echo "（注意：目前尚未真正注册到 Fcitx5 中，只是一个独立 Demo 窗口。）"
  if [[ -f "${VOSK_LIB_DIR}/libvosk.so" ]]; then
    export VOSK_LIB_PATH="${VOSK_LIB_DIR}"
  fi
  "${UI_BIN}"
else
  echo "未找到可执行文件：${UI_BIN}"
  echo "请检查 CMake 配置或编译输出。"
  exit 1
fi
