#pragma once
#include <Arduino.h>

namespace gh {

String listIdx(const String& li, int16_t idx, char div = ';');

namespace core {

template <uint8_t cap>
struct Splitter {
    Splitter(char* url, char div = '/') {
        while (size < cap) {
            char* divp = strchr(url, div);
            str[size] = url;
            size++;
            if (divp && size < cap) {
                url = divp + 1;
                *divp = 0;
            } else break;
        }
    }

    char* str[cap] = {0};
    uint8_t size = 0;
};

/*struct Splitter {
  Splitter(char** buf, uint8_t max, char* str, char div = '/') {
    while (size < max) {
      char* divp = strchr(str, div);
      buf[size] = str;
      size++;
      if (divp && size < max) {
        str = divp + 1;
        *divp = 0;
      } else break;
    }
  }

  uint8_t size = 0;
};*/

class Parser {
   public:
    Parser(char* list, char div = ';') : _list(list), _div(div) {}

    bool next() {
        if (!_div) return 0;
        if (_next) _list = _next;
        char* n = strchr(_list, _div);
        if (n) {
            _next = n + 1;
            *n = '\0';
        } else {
            _div = 0;
        }
        return *_list;
    }

    char* get() {
        return _list;
    }

   private:
    char* _list;
    char _div;
    char* _next = nullptr;
};

}  // namespace core

}  // namespace gh