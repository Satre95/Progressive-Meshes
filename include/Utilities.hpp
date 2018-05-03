#pragma once

class Utilities {
public:
    template<typename Tptr>
    static void Safe_Delete(Tptr *& a) {
        delete a;
        a = nullptr;
    }
};