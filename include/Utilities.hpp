#pragma once

class Utilities {
public:
    template<typename Tptr>
    static void Safe_Delete(Tptr *& a) {
        delete a;
        a = nullptr;
    }
    
    template<typename T>
    static void ReplaceObject(T *& obj, T & replacement) {
        //Call destructor
        obj->~T();
        
        // Create a new object in the same space
        obj = new (obj) T(replacement);
    }

    template <typename T>
    static void hash_combine(std::size_t & seed, const T& val) {
        seed ^= std::hash<T>()(val) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    }
};
