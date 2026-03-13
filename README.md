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
- **词典支持**：内置约 2500+ 条拼音词典数据，覆盖常用汉字和词组

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
- `src/fcitx/`：Fcitx5 插件入口与引擎适配层
- `src/ui/`：独立 UI 进程
  - `candidate_view.cpp/h`：候选栏
  - `input_widget.cpp/h`：输入框
  - `status_bar.cpp/h`：状态栏
  - `skin_selector.cpp/h`：皮肤滑动选择窗口
  - `theme_manager.cpp/h`：主题管理
  - `skin_loader.cpp/h`：皮肤加载器
- `src/dict/`：系统词典、用户词典实现
- `src/utils/`：工具函数
- `data/`：拼音词典数据、皮肤资源文件

## 使用说明

### 构建与运行

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -- -j$(nproc)

# 从项目根目录运行（词典需要相对路径）
cd .. && ./build/src/ui/lingjian_ui
```

或直接使用构建脚本：

```bash
bash run.sh
```

### 输入操作

| 按键         | 功能                      |
|-------------|--------------------------|
| a-z         | 输入拼音                  |
| 空格         | 选择第一个候选词           |
| 1-9         | 选择对应编号的候选词       |
| Backspace   | 删除最后一个拼音字母       |
| Escape      | 取消当前拼音输入           |
| Enter       | 将拼音原文上屏             |
| - / =       | 上一页 / 下一页           |
| PageUp/Down | 上一页 / 下一页           |

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

## 依赖

- CMake >= 3.16
- Qt6 Widgets
- C++17
- unzip（用于解压皮肤 ZIP 文件）
