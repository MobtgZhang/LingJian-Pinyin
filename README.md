灵键拼音（LingJian Pinyin）
===========================

灵键拼音是一个基于 Fcitx5 的中文拼音输入法项目，目标是提供 **轻量、响应快、可高度定制** 的输入体验。

## 功能特性

- **整句拼音输入**：支持连续拼音输入，自动切分音节，整句候选
- **拼音切分引擎**：自动将连续拼音串（如 `nihao`）切分为音节（`ni'hao`）
- **语言模型评分**：基于词频的 Unigram/Bigram 语言模型对候选词句评分
- **Beam Search 解码**：整句级 Beam Search 解码，输出最优候选句
- **状态栏协调**：状态栏和候选栏自动联动，中/英文模式切换
- **皮肤滑动选择器**：点击皮肤按钮弹出滑动窗口，可视化预览并选择 8 种内置皮肤
- **自定义皮肤**：支持从 ZIP 文件加载皮肤，兼容搜狗皮肤格式（skin.ini + 图片资源）
- **词典支持**：基于 [rime-ice 雾凇拼音](https://github.com/iDvel/rime-ice) 词库，约 88 万+ 词条，覆盖常用汉字和词组
- **语音输入**：基于 [Vosk](https://github.com/alphacep/vosk-api) 离线语音识别，支持普通话和英语，点击麦克风按钮或按 F2 开始说话

## 输入法架构

```
拼音输入 (用户键入)
   ↓
拼音切分 (PinyinSegmenter)
   ↓
拼音→汉字候选 (Dictionary)
   ↓
语言模型评分 (LanguageModel)
   ↓
Beam Search (SentenceDecoder)
   ↓
输出候选句
```

## 目录结构

- `src/core/`：核心输入逻辑
  - `context.cpp/h`：输入上下文，管理按键、候选、翻页
  - `decoder.cpp/h`：解码器入口，串联整个流水线
  - `pinyin_segmenter.cpp/h`：拼音切分引擎
  - `language_model.cpp/h`：语言模型评分
  - `sentence_decoder.cpp/h`：Beam Search 整句解码
  - `dictionary.cpp/h`：词典加载与查询
- `src/fcitx/`：Fcitx5 输入法 addon（引擎、候选列表、按键处理）
- `src/ui/`：独立 UI 进程
  - `candidate_view.cpp/h`：候选栏
  - `input_widget.cpp/h`：输入框
  - `status_bar.cpp/h`：状态栏
  - `skin_selector.cpp/h`：皮肤滑动选择窗口
  - `voice_input_dialog.cpp/h`：语音输入弹窗
  - `theme_manager.cpp/h`：主题管理
  - `skin_loader.cpp/h`：皮肤加载器
- `src/voice/`：语音识别模块
  - `vosk_speech_engine.cpp/h`：Vosk 离线语音识别封装
- `src/dict/`：系统词典、用户词典实现
- `src/utils/`：工具函数
- `data/`：拼音词典数据、皮肤资源文件
- `scripts/`：词库更新脚本

### 更新词库

词库源自 [rime-ice 雾凇拼音](https://github.com/iDvel/rime-ice)，可通过脚本更新：

```bash
python3 scripts/update_dict_from_rime.py
```

脚本会下载 8105 字表、base、ext、others 词库，转换为灵键格式并合并到 `data/pinyin_dict.txt`。

## 使用说明

### 安装构建依赖

```bash
# Ubuntu / Debian
sudo apt install build-essential cmake g++ qt6-base-dev qt6-multimedia-dev libgl1-mesa-dev \
    fcitx5 libfcitx5core-dev libfcitx5utils-dev fcitx5-modules-dev
```

语音输入需要 Qt6 Multimedia（麦克风采集）和可选的 libvosk（离线语音识别）。

### 构建 Fcitx5 输入法 addon

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_FCITX5=ON
cmake --build . -- -j$(nproc)
sudo cmake --install .
```

安装完成后重新加载 fcitx5：

```bash
fcitx5-remote -r
```

然后在 `fcitx5-configtool` 中添加「灵键拼音」即可使用。

### 构建独立 UI Demo

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_UI=ON -DBUILD_FCITX5=OFF
cmake --build . -- -j$(nproc)
cd .. && ./build/src/ui/lingjian_ui
```

或直接使用构建脚本：

```bash
bash run.sh
```

### 构建 .deb 安装包

```bash
./build_deb.sh
sudo dpkg -i lingjian-pinyin_0.1.0_amd64.deb
```

### 输入操作

| 按键         | 功能                      |
|-------------|--------------------------|
| a-z         | 输入拼音                  |
| 空格         | 选择当前高亮候选词         |
| 1-9         | 选择对应编号的候选词       |
| Backspace   | 删除最后一个拼音字母       |
| Escape      | 取消当前拼音输入           |
| Enter       | 将拼音原文上屏             |
| - / =       | 上一页 / 下一页           |
| ↑ / ↓       | 上一页 / 下一页           |
| PageUp/Down | 上一页 / 下一页           |
| ← / →       | 当前页内左右移动选择候选   |
| F2          | 打开语音输入 / 开始说话    |

### 语音输入

点击状态栏的「🎤」按钮或按 F2 打开语音输入弹窗。弹窗包含：

- **语言选择**：普通话 / 英语
- **输入设备**：选择麦克风设备
- **麦克风按钮**：点击开始说话，再次点击结束并识别

语音识别基于 [Vosk](https://github.com/alphacep/vosk-api) 离线引擎。使用前需：

1. **安装 libvosk**（可选，未安装时点击会提示）：
   - **Ubuntu 24.04**：官方仓库暂无 libvosk 包，请用预编译库安装：
     ```bash
     cd /tmp
     wget https://github.com/alphacep/vosk-api/releases/download/v0.3.45/vosk-linux-x86_64-0.3.45.zip
     unzip vosk-linux-x86_64-0.3.45.zip
     # 解压后可能是当前目录下的 libvosk.so，或子目录（如 vosk-linux-x86_64-0.3.45/libvosk.so）
     sudo cp libvosk.so /usr/local/lib/ 2>/dev/null || sudo cp vosk-linux-x86_64-0.3.45/libvosk.so /usr/local/lib/
     sudo ldconfig
     ```
     若不想动系统目录，可解压到任意目录（如 `$HOME/.local/lib/vosk`），然后运行程序前执行：  
     `export VOSK_LIB_PATH=$HOME/.local/lib/vosk`
   - 若 libvosk 已在自定义目录，可设置环境变量 **`VOSK_LIB_PATH`**：  
     - 指向目录：`export VOSK_LIB_PATH=/path/to/dir`；  
     - 或指向完整路径：`export VOSK_LIB_PATH=/path/to/libvosk.so`。
2. **下载语音模型**并放置于 `data/vosk-models/` 目录：
   - 普通话：`vosk-model-small-cn-0.22`（从 [Vosk 模型](https://alphacephei.com/vosk/models) 下载）
   - 英语：`vosk-model-small-en-us-0.15`

### 皮肤系统

皮肤文件是一个 ZIP 压缩包（扩展名 .zip / .ssf / .ljs），内部结构：

```
skin.zip
├── skin.ini          # 皮肤配置文件
├── background.png    # 候选框背景图（可选）
└── statusbar_bg.png  # 状态栏背景图（可选）
```

skin.ini 格式（参考搜狗输入法皮肤配置）：

```ini
[General]
name=我的皮肤
author=作者名
version=1.0

[Display]
candidate_font_size=14
preedit_font_size=13

[CandidateView]
background_color=#FFFFFF
border_color=#D2D2D2
text_color=#333333
highlight_color=#4A90D9
preedit_color=#FF6600
background_image=background.png

[StatusBar]
background_color=#FFFFFF
border_color=#D2D2D2
text_color=#323232
logo_color=#FF7800
```

点击状态栏的「🎨」按钮会弹出皮肤滑动选择窗口，内置 8 种皮肤（亮色、暗色、海蓝、草绿、星紫、玫瑰、落霞、冰川），
支持鼠标滚轮或拖拽滑动浏览。点击「+」按钮可加载自定义皮肤 ZIP 文件。

## X11 / Wayland 兼容性

灵键拼音通过 Fcitx5 框架天然支持 X11 和 Wayland 两种显示服务器，输入法引擎本身无需做任何平台特定适配：

| 显示服务器 | 协议 | 前端模块 |
|-----------|------|---------|
| X11 | XIM / Fcitx5 GTK/Qt IM Module | `fcitx5-frontend-gtk3`、`fcitx5-frontend-qt5` |
| Wayland | `zwp_input_method_v2` | Fcitx5 内置 Wayland 前端 |

建议安装以下前端模块以获得最佳体验：

```bash
# GTK 应用支持
sudo apt install fcitx5-frontend-gtk3 fcitx5-frontend-gtk4

# Qt 应用支持
sudo apt install fcitx5-frontend-qt5 fcitx5-frontend-qt6

# 配置工具
sudo apt install fcitx5-config-qt
```

在 Wayland 环境下，请确保以下环境变量已设置（通常由桌面环境自动配置）：

```bash
export GTK_IM_MODULE=fcitx
export QT_IM_MODULE=fcitx
export XMODIFIERS=@im=fcitx
export INPUT_METHOD=fcitx
```

## 依赖

- CMake >= 3.16
- C++17
- Fcitx5 >= 5.0（`libfcitx5core-dev`、`libfcitx5utils-dev`）
- Qt6 Widgets（可选，仅独立 UI Demo 需要）
- unzip（用于解压皮肤 ZIP 文件）
