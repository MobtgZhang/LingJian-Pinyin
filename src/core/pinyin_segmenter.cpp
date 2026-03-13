#include "pinyin_segmenter.h"

#include <algorithm>
#include <unordered_set>

namespace core {

PinyinSegmenter::PinyinSegmenter() = default;

const std::vector<std::string> &PinyinSegmenter::validSyllables() {
    static const std::vector<std::string> table = {
        "a","o","e","ai","ei","ao","ou","an","en","ang","eng","er",
        "ba","bo","bi","bu","bai","bei","bao","ban","ben","bang","beng","bian","biao","bie","bin","bing",
        "pa","po","pi","pu","pai","pei","pao","pou","pan","pen","pang","peng","pian","piao","pie","pin","ping",
        "ma","mo","me","mi","mu","mai","mei","mao","mou","man","men","mang","meng","mian","miao","mie","min","ming","miu",
        "fa","fo","fu","fei","fou","fan","fen","fang","feng",
        "da","de","di","du","dai","dei","dao","dou","dan","den","dang","deng","dong","dian","diao","die","diu","ding","duan","dui","dun",
        "ta","te","ti","tu","tai","tei","tao","tou","tan","tang","teng","tong","tian","tiao","tie","ting","tuan","tui","tun",
        "na","ne","ni","nu","nv","nai","nei","nao","nou","nan","nen","nang","neng","nong","nian","niang","niao","nie","nin","ning","niu","nuan","nve",
        "la","le","li","lu","lv","lai","lei","lao","lou","lan","lang","leng","long","lian","liang","liao","lie","lin","ling","liu","luan","lve","lun",
        "ga","ge","gu","gai","gei","gao","gou","gan","gen","gang","geng","gong","gua","guai","guan","guang","gui","gun","guo",
        "ka","ke","ku","kai","kei","kao","kou","kan","ken","kang","keng","kong","kua","kuai","kuan","kuang","kui","kun","kuo",
        "ha","he","hu","hai","hei","hao","hou","han","hen","hang","heng","hong","hua","huai","huan","huang","hui","hun","huo",
        "ji","ju","jia","jian","jiang","jiao","jie","jin","jing","jiong","jiu","juan","jue","jun",
        "qi","qu","qia","qian","qiang","qiao","qie","qin","qing","qiong","qiu","quan","que","qun",
        "xi","xu","xia","xian","xiang","xiao","xie","xin","xing","xiong","xiu","xuan","xue","xun",
        "zhi","zha","zhe","zhu","zhai","zhei","zhao","zhou","zhan","zhen","zhang","zheng","zhong","zhua","zhuai","zhuan","zhuang","zhui","zhun","zhuo",
        "chi","cha","che","chu","chai","chao","chou","chan","chen","chang","cheng","chong","chua","chuai","chuan","chuang","chui","chun","chuo",
        "shi","sha","she","shu","shai","shei","shao","shou","shan","shen","shang","sheng","shua","shuai","shuan","shuang","shui","shun","shuo",
        "ri","re","ru","rao","rou","ran","ren","rang","reng","rong","rua","ruan","rui","run","ruo",
        "zi","za","ze","zu","zai","zei","zao","zou","zan","zen","zang","zeng","zong","zuan","zui","zun","zuo",
        "ci","ca","ce","cu","cai","cao","cou","can","cen","cang","ceng","cong","cuan","cui","cun","cuo",
        "si","sa","se","su","sai","sao","sou","san","sen","sang","seng","song","suan","sui","sun","suo",
        "ya","yo","ye","yi","yu","yai","yao","you","yan","yin","yang","ying","yong","yuan","yue","yun",
        "wa","wo","wu","wai","wei","wan","wen","wang","weng",
    };
    return table;
}

bool PinyinSegmenter::isValidSyllable(const std::string &s) {
    static std::unordered_set<std::string> set;
    if (set.empty()) {
        for (const auto &sy : validSyllables()) {
            set.insert(sy);
        }
    }
    return set.count(s) > 0;
}

void PinyinSegmenter::dfs(const std::string &input, std::size_t pos,
                           std::vector<std::string> &current,
                           std::vector<SegmentResult> &results) const {
    if (pos >= input.size()) {
        results.push_back({current, ""});
        return;
    }

    bool found = false;
    // 从长到短尝试匹配（最长匹配优先）
    for (std::size_t len = std::min<std::size_t>(6, input.size() - pos); len >= 1; --len) {
        std::string sub = input.substr(pos, len);
        if (isValidSyllable(sub)) {
            found = true;
            current.push_back(sub);
            dfs(input, pos + len, current, results);
            current.pop_back();
            if (results.size() >= 20) return;
        }
    }

    if (!found && current.empty()) {
        results.push_back({{}, input});
    } else if (!found) {
        results.push_back({current, input.substr(pos)});
    }
}

std::vector<PinyinSegmenter::SegmentResult> PinyinSegmenter::segment(const std::string &input) const {
    if (input.empty()) return {};

    std::vector<SegmentResult> results;
    std::vector<std::string> current;
    dfs(input, 0, current, results);

    std::sort(results.begin(), results.end(),
              [](const SegmentResult &a, const SegmentResult &b) {
                  if (a.remainder.size() != b.remainder.size())
                      return a.remainder.size() < b.remainder.size();
                  return a.syllables.size() < b.syllables.size();
              });

    return results;
}

PinyinSegmenter::SegmentResult PinyinSegmenter::bestSegment(const std::string &input) const {
    auto results = segment(input);
    if (results.empty()) {
        return {{}, input};
    }
    return results.front();
}

} // namespace core
