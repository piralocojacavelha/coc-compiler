
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define coc_String const char *
#define coc_Exists(__VA_ARGS__) 0

#define coc_thisPath "linear.coc"

template <typename A, typename B>
struct coc_Exists {
    A x;
    B y;
};

template <typename A>
struct coc_Maybe {
    A x;
    bool ok;
};

template <>
struct coc_Maybe<void> {
    template <typename A>
    operator coc_Maybe<A>() {
        return {};
    }
};

static inline
auto coc_openFile = [](const char *path) {
    using E = coc_Exists<int, coc_Exists<int, FILE *>>;
    FILE *f = fopen(path, "rb");
    if (!f) {
        return coc_Maybe<E>{};
    }
    return coc_Maybe<E>{{0, {0, f}}, true};
};

static inline
auto coc_matchOnExists = [](auto A) {
    return [=](auto B) {
        return [=](auto exists) {
            return [=](auto R) {
                return [=](auto f) {
                    return f(exists.x)(exists.y);
                };
            };
        };
    };
};

static inline
auto coc_matchOnMaybe = [](auto A) {
    return [=](auto maybe) {
        return [=](auto R) {
            return [=](auto f) {
                return [=](auto x) {
                    if (maybe.ok) {
                        return f(maybe.x);
                    }
                    return (decltype(f(maybe.x)))x;
                };
            };
        };
    };
};

#define coc_nothing(...) coc_Maybe<void>{}

static inline
auto coc_readFile = [](auto S) {
    return [=](auto h) {
        return [=](FILE *f) {
            char buf[32*1024];
            fread(buf, 1, sizeof(buf), f);
            return coc_Maybe<const char *>{strdup(buf), true};
        };
    };
};

static inline
auto coc_printString(const char *s) {
    printf("%s\n", s);
    return 0;
}

#define coc_pure 0

#include "out.cpp"

int main(void) {
}
