#pragma once
#include <Arduino.h>

#include "ui/builder.h"

namespace gh {

class Show {
   public:
    Show(gh::Builder& b, bool en) : b(b) {
        b.show(en);
    }
    ~Show() {
        b.show(true);
    }

   private:
    gh::Builder& b;
};

class Row {
   public:
    Row(gh::Builder& b, uint16_t width = 1) : b(b) {
        b.beginRow(width);
    }
    ~Row() {
        b.endRow();
    }

   private:
    gh::Builder& b;
};

class Col {
   public:
    Col(gh::Builder& b, uint16_t width = 1) : b(b) {
        b.beginCol(width);
    }
    ~Col() {
        b.endCol();
    }

   private:
    gh::Builder& b;
};

}  // namespace gh