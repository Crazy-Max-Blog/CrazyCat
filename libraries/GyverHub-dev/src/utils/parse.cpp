#include "parse.h"

String gh::listIdx(const String& li, int16_t idx, char div) {
    // int cnt = 0, p = 0, i = 0;
    // while (1) {
    //     if (li[i] == div || !li[i]) {
    //         if (cnt == idx) return li.substring(p, i);
    //         if (!li[i]) return String();
    //         cnt++;
    //         p = i + 1;
    //     }
    //     i++;
    // }
    // return String();
    int16_t p = -1, st = 0;
    while (idx-- >= 0) {
        p = li.indexOf(div, p + 1);
        if (!idx) st = p;
        if (idx < 0 && p < 0) p = li.length();
    }
    if (p < 0 || st < 0) return String();
    else return li.substring(st ? st + 1 : st, p);
}
