#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"

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
  "${UI_BIN}"
else
  echo "未找到可执行文件：${UI_BIN}"
  echo "请检查 CMake 配置或编译输出。"
  exit 1
fi

