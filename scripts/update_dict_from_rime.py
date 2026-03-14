#!/usr/bin/env python3
"""
从 rime-ice 和 ssnhd/rime 下载词库，转换为灵键拼音格式。

灵键格式: pinyin<TAB>word<TAB>frequency
Rime 格式: word pinyin1 [pinyin2 ...] frequency (空格分隔，多字词拼音用空格分开)

Emoji 支持: 参考 rime-emoji 的 emoji_word.txt、emoji_category.txt，
输入中文词时可在候选框中选择对应表情。

参考:
- https://github.com/iDvel/rime-ice
- https://github.com/ssnhd/rime
- https://github.com/rime/rime-emoji
"""

import os
import re
import sys
import urllib.request
from pathlib import Path
from collections import defaultdict

# 词库来源
RIME_ICE_BASE = "https://raw.githubusercontent.com/iDvel/rime-ice/main"
RIME_EMOJI_BASE = "https://raw.githubusercontent.com/rime/rime-emoji/master/opencc"
EMOJI_URLS = [
    f"{RIME_EMOJI_BASE}/emoji_word.txt",
    f"{RIME_EMOJI_BASE}/emoji_category.txt",
]
DICT_URLS = [
    f"{RIME_ICE_BASE}/cn_dicts/8105.dict.yaml",   # 通用规范汉字表 8105 字
    f"{RIME_ICE_BASE}/cn_dicts/base.dict.yaml",    # 基础词库（两字词+调频）
    f"{RIME_ICE_BASE}/cn_dicts/ext.dict.yaml",     # 扩展词库
    f"{RIME_ICE_BASE}/cn_dicts/others.dict.yaml",  # 杂项
]

# 可选：tencent 词库较大，部署较慢，按需启用
# f"{RIME_ICE_BASE}/cn_dicts/tencent.dict.yaml",


def parse_rime_dict(content: str) -> list[tuple[str, str, float]]:
    """
    解析 Rime dict.yaml 格式，返回 [(pinyin_flat, word, freq), ...]

    支持两种格式:
    1) Tab 分隔: word \\t pinyin1 pinyin2 ... \\t freq  (如 base/ext: "你好\\tni hao\\t100")
    2) 空格分隔: word pinyin freq  (如 8105 单字: "啊 a 5366043")
    """
    results = []
    in_data = False
    header_end = re.compile(r"^\.\.\.\s*$")

    for line in content.splitlines():
        line = line.rstrip()
        # 跳过空行和注释
        if not line or line.startswith("#"):
            continue
        # 检测 YAML 数据块结束
        if header_end.match(line):
            in_data = True
            continue
        # 跳过 YAML 头部 (name:, version:, sort: 等)
        if not in_data:
            if ":" in line and not re.search(r"\s[a-z]+\s+[a-z]", line):
                continue
            in_data = True

        word = ""
        pinyin_flat = ""
        freq = 0.0

        if "\t" in line:
            # Tab 分隔: word \t pinyin [\t freq]  (others 词库可能无 freq)
            parts = line.split("\t")
            if len(parts) < 2:
                continue
            word = parts[0]
            pinyin_str = parts[1]  # 可能含空格 "ni hao"
            freq = 1.0
            if len(parts) >= 3:
                try:
                    freq = float(parts[2])
                except ValueError:
                    pass
            pinyin_flat = pinyin_str.replace(" ", "").replace("'", "").lower()
        else:
            # 空格分隔: word pinyin1 [pinyin2 ...] freq (8105 单字)
            parts = line.split()
            if len(parts) < 2:
                continue
            word = parts[0]
            try:
                freq = float(parts[-1])
            except ValueError:
                continue
            pinyin_parts = parts[1:-1]
            pinyin_flat = "".join(p.replace("'", "") for p in pinyin_parts).lower()

        # 过滤非中文词条
        if not any("\u4e00" <= c <= "\u9fff" for c in word):
            continue
        if not pinyin_flat or not pinyin_flat.isalpha():
            continue

        results.append((pinyin_flat, word, freq))

    return results


def merge_entries(entries: list[tuple[str, str, float]]) -> dict[tuple[str, str], float]:
    """
    合并重复 (pinyin, word)，保留词频最高的
    """
    merged: dict[tuple[str, str], float] = {}
    for py, word, freq in entries:
        key = (py, word)
        merged[key] = max(merged.get(key, 0), freq)
    return merged


def download_url(url: str) -> str:
    """下载 URL 内容"""
    print(f"  下载: {url}")
    req = urllib.request.Request(url, headers={"User-Agent": "LingJian-Pinyin/1.0"})
    with urllib.request.urlopen(req, timeout=60) as resp:
        return resp.read().decode("utf-8")


def _is_emoji_char(c: str) -> bool:
    """判断字符是否为 emoji（含表情符号、符号等）"""
    if not c:
        return False
    code = ord(c[0])
    # Emoji 主要范围: 1F300-1F9FF, 2600-26FF, 2700-27BF, FE00-FE0F (变体)
    return (
        0x1F300 <= code <= 0x1F9FF
        or 0x2600 <= code <= 0x26FF
        or 0x2700 <= code <= 0x27BF
        or 0xFE00 <= code <= 0xFE0F
        or 0x1F000 <= code <= 0x1F02F
        or 0x1F0A0 <= code <= 0x1F0FF
        or code in (0x200D, 0x2640, 0x2642)  # ZWJ, 性别
    )


def _token_has_emoji(tok: str) -> bool:
    """判断 token 是否包含 emoji 字符"""
    return any(_is_emoji_char(c) for c in tok)


def parse_emoji_file(content: str) -> list[tuple[str, list[str]]]:
    """
    解析 emoji_word.txt / emoji_category.txt 格式。
    格式: 关键词 [关键词] emoji1 emoji2 ...  (Tab 或空格分隔)
    返回: [(keyword, [emoji1, emoji2, ...]), ...]
    """
    results = []
    for line in content.splitlines():
        line = line.rstrip()
        if not line or line.startswith("#"):
            continue
        # 统一按空白分割
        tokens = line.replace("\t", " ").split()
        if len(tokens) < 2:
            continue
        # 找到第一个 emoji 的位置
        emoji_start = 0
        for i, tok in enumerate(tokens):
            if _token_has_emoji(tok):
                emoji_start = i
                break
        if emoji_start == 0:
            continue
        keyword = tokens[0]  # 使用第一个词作为触发词
        emojis = [t for t in tokens[emoji_start:] if _token_has_emoji(t)]
        if not emojis:
            continue
        # 关键词需含中文
        if not any("\u4e00" <= c <= "\u9fff" for c in keyword):
            continue
        results.append((keyword, emojis))
    return results


def main():
    script_dir = Path(__file__).resolve().parent
    project_root = script_dir.parent
    output_path = project_root / "data" / "pinyin_dict.txt"
    cache_dir = project_root / "data" / "rime_cache"
    cache_dir.mkdir(parents=True, exist_ok=True)

    print("=== 灵键拼音词库更新工具 ===\n")
    print("来源: rime-ice (雾凇拼音) https://github.com/iDvel/rime-ice\n")

    all_entries: dict[tuple[str, str], float] = {}

    for url in DICT_URLS:
        try:
            cache_file = cache_dir / Path(url).name
            if cache_file.exists():
                print(f"  使用缓存: {cache_file.name}")
                content = cache_file.read_text(encoding="utf-8")
            else:
                content = download_url(url)
                cache_file.write_text(content, encoding="utf-8")

            entries = parse_rime_dict(content)
            print(f"  解析到 {len(entries)} 条")

            for (py, word, freq) in entries:
                key = (py, word)
                all_entries[key] = max(all_entries.get(key, 0), freq)

        except Exception as e:
            print(f"  错误: {e}")
            continue

    print(f"\n合并后共 {len(all_entries)} 条")

    # 构建 词语->拼音 映射（取词频最高的拼音），用于 emoji 匹配
    word_to_pinyin: dict[str, tuple[str, float]] = {}
    for (py, word), freq in all_entries.items():
        if word not in word_to_pinyin or freq > word_to_pinyin[word][1]:
            word_to_pinyin[word] = (py, freq)

    # 下载并合并 emoji 词条（参考 rime-emoji）
    opencc_dir = project_root / "data" / "opencc"
    opencc_dir.mkdir(parents=True, exist_ok=True)
    emoji_freq = 50  # emoji 词频较低，排在正常词后面

    for emoji_url in EMOJI_URLS:
        try:
            cache_file = cache_dir / Path(emoji_url).name
            if cache_file.exists():
                print(f"  使用缓存: {cache_file.name}")
                content = cache_file.read_text(encoding="utf-8")
            else:
                content = download_url(emoji_url)
                cache_file.write_text(content, encoding="utf-8")

            # 同时保存到 data/opencc 供参考
            dest = opencc_dir / Path(emoji_url).name
            dest.write_text(content, encoding="utf-8")

            entries = parse_emoji_file(content)
            added = 0
            for keyword, emojis in entries:
                if keyword not in word_to_pinyin:
                    continue
                py, _ = word_to_pinyin[keyword]
                for emoji in emojis:
                    key = (py, emoji)
                    if key not in all_entries:
                        all_entries[key] = emoji_freq
                        added += 1
            print(f"  解析到 {len(entries)} 条关键词，新增 {added} 条 emoji 候选")
        except Exception as e:
            print(f"  Emoji 文件 {emoji_url}: {e}")

    # 按 pinyin 分组，组内按 freq 降序
    by_pinyin: dict[str, list[tuple[str, float]]] = defaultdict(list)
    for (py, word), freq in all_entries.items():
        by_pinyin[py].append((word, freq))

    for py in by_pinyin:
        by_pinyin[py].sort(key=lambda x: -x[1])

    # 写入灵键格式: pinyin<TAB>word<TAB>freq
    with open(output_path, "w", encoding="utf-8") as f:
        f.write("# LingJian Pinyin Dictionary\n")
        f.write("# 灵键拼音词典 - 源自 rime-ice 雾凇拼音 + rime-emoji 表情\n")
        f.write("# Format: pinyin<TAB>word<TAB>frequency\n")
        f.write("# 格式：拼音<TAB>词语<TAB>词频\n")
        f.write("# 更新: 运行 scripts/update_dict_from_rime.py\n\n")

        for py in sorted(by_pinyin.keys()):
            for word, freq in by_pinyin[py]:
                f.write(f"{py}\t{word}\t{int(freq)}\n")

    print(f"\n已写入: {output_path}")
    print(f"总词条数: {sum(len(v) for v in by_pinyin.values())}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
