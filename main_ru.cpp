#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

// -------- UTF-8 helpers --------

std::vector<uint32_t> utf8ToCodepoints(const std::string &s) {
    std::vector<uint32_t> codepoints;
    size_t i = 0;
    while (i < s.size()) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        uint32_t cp = 0;
        size_t extra = 0;

        if ((c & 0x80) == 0) {            // 1-byte
            cp = c;
            extra = 0;
        } else if ((c & 0xE0) == 0xC0) {  // 2-byte
            cp = c & 0x1F;
            extra = 1;
        } else if ((c & 0xF0) == 0xE0) {  // 3-byte
            cp = c & 0x0F;
            extra = 2;
        } else if ((c & 0xF8) == 0xF0) {  // 4-byte
            cp = c & 0x07;
            extra = 3;
        } else {
            // Invalid byte, skip
            ++i;
            continue;
        }

        if (i + extra >= s.size()) break;

        for (size_t j = 0; j < extra; ++j) {
            unsigned char c2 = static_cast<unsigned char>(s[i + 1 + j]);
            if ((c2 & 0xC0) != 0x80) { // not a continuation byte
                cp = 0xFFFD; // replacement
                break;
            }
            cp = (cp << 6) | (c2 & 0x3F);
        }

        codepoints.push_back(cp);
        i += 1 + extra;
    }
    return codepoints;
}

std::string codepointToUtf8(uint32_t cp) {
    std::string out;
    if (cp <= 0x7F) {
        out.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        out.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | ((cp >> 18) & 0x07)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
    return out;
}

// -------- Language  → Russian transliteration --------

std::string translitLanguageToRussian(uint32_t cp) {
    //  syllables range
    const uint32_t SBase = 0xAC00;
    const uint32_t LBase = 0x1100;
    const uint32_t VBase = 0x1161;
    const uint32_t TBase = 0x11A7;
    const int LCount = 19;
    const int VCount = 21;
    const int TCount = 28;
    const int NCount = VCount * TCount;
    const int SCount = LCount * NCount;

    if (cp < SBase || cp >= SBase + SCount) {
        return "";
    }

    // Initial consonants (L) → Russian
    static const char* LTable[LCount] = {
        "г",   //  g
        "кк",  //  kk
        "н",   //  n
        "д",   //  d
        "тт",  //  tt
        "р",   //  r
        "м",   //  m
        "б",   //  b
        "пп",  //  pp
        "с",   //  s
        "сс",  //  ss
        "",    // (silent at beginning)
        "ч",   //  j
        "чч",  //  jj
        "чх",  //  ch
        "к",   //  k
        "т",   //  t
        "п",   //  p
        "х"    //  h
    };

    // Vowels (V) → Russian
    static const char* VTable[VCount] = {
        "а",   //  a
        "э",   //  ae
        "я",   //  ya
        "е",   //  yae
        "о",   //  eo
        "э",   //  e
        "ё",   //  yeo
        "е",   //  ye
        "о",   //  o
        "ва",  //  wa
        "вэ",  //  wae
        "ве",  //  oe
        "ё",   //  yo
        "у",   //  u
        "во",  //  wo
        "ве",  //  we
        "ви",  //  wi
        "ю",   //  yu
        "ы",   //  eu
        "и",   //  yi
        "и"    //  i
    };

    // Final consonants (T / ) → Russian (simplified)
    static const char* TTable[TCount] = {
        "",    // (none)
        "к",   // 
        "к",   // 
        "кс",  // 
        "н",   // 
        "ндж", // 
        "нх",  // 
        "т",   // 
        "ль",  // 
        "льк", // 
        "льм", // 
        "льб", // 
        "льс", // 
        "льт", // 
        "льп", // 
        "льх", // 
        "м",   // 
        "п",   // 
        "пс",  // 
        "т",   // 
        "т",   // 
        "н",   //  (ng, approximated as н)
        "т",   // 
        "т",   // 
        "к",   // 
        "т",   // 
        "п",   // 
        "т"    // 
    };

    int SIndex = cp - SBase;
    int LIndex = SIndex / NCount;
    int VIndex = (SIndex % NCount) / TCount;
    int TIndex = SIndex % TCount;

    std::string result;
    result += LTable[LIndex];
    result += VTable[VIndex];
    result += TTable[TIndex];

    return result;
}

// -------- Dispatcher --------

std::string romanizeCodepoint(uint32_t cp) {
    // Try Language → Russian transliteration
    std::string ko = translitLanguageToRussian(cp);
    if (!ko.empty()) return ko;

    // Fallback: return original character (for non-)
    return codepointToUtf8(cp);
}

int main() {
    // Make sure your console is set to UTF-8.
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string input;
    std::cout << "Enter UTF-8 text:\n";
    while (std::getline(std::cin, input)) {
        auto cps = utf8ToCodepoints(input);
        std::string out;
        for (uint32_t cp : cps) {
            out += romanizeCodepoint(cp);
        }
        std::cout << out << "\n";
    }
    return 0;
}
