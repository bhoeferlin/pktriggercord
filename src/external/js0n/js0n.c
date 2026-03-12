// by jeremie miller - 2014
// public domain or MIT license, contributions/improvements welcome via github at https://github.com/quartzjer/js0n

#include <string.h> // one strncmp() is used to do key comparison, and a strlen(key) if no len passed in

// gcc started warning for the init syntax used here, is not helpful so don't generate the spam, supressing the warning is really inconsistently supported across versions
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Winitializer-overrides"
#pragma GCC diagnostic ignored "-Woverride-init"
#endif

// only at depth 1, track start pointers to match key/value
#define PUSH(i)                     \
    if (depth == 1) {               \
        if (!index) {               \
            val = cur + (i);        \
        } else {                    \
            if (klen && index == 1) \
                start = cur + (i);  \
            else                    \
                index--;            \
        }                           \
    }

// determine if key matches or value is complete
#define CAP(i)                                                                                 \
    if (depth == 1) {                                                                          \
        if (val && !index) {                                                                   \
            *vlen = (size_t)((cur + (i) + 1) - val);                                           \
            return val;                                                                        \
        }                                                                                      \
        if (klen && start) {                                                                   \
            index = (klen == (size_t)(cur - start) && strncmp(key, start, klen) == 0) ? 0 : 2; \
            start = 0;                                                                         \
        }                                                                                      \
    }

#if defined(_MSC_VER)

/* MSVC branch: no GNU range designators, no computed goto */

typedef enum js0n_state {
    JS0N_BAD = 0,
    JS0N_LOOP,
    JS0N_UP,
    JS0N_DOWN,
    JS0N_QUP,
    JS0N_QDOWN,
    JS0N_ESC,
    JS0N_UNESC,
    JS0N_BARE,
    JS0N_UNBARE,
    JS0N_UTF8_2,
    JS0N_UTF8_3,
    JS0N_UTF8_4,
    JS0N_UTF_CONTINUE
} js0n_state;

static void js0n_init_tables(
    js0n_state gostruct[256],
    js0n_state gobare[256],
    js0n_state gostring[256],
    js0n_state goutf8_continue[256],
    js0n_state goesc[256])
{
    int i;

    /* gostruct */
    for (i = 0; i < 256; ++i)
        gostruct[i] = JS0N_BAD;
    gostruct['\t'] = JS0N_LOOP;
    gostruct[' '] = JS0N_LOOP;
    gostruct['\r'] = JS0N_LOOP;
    gostruct['\n'] = JS0N_LOOP;
    gostruct['"'] = JS0N_QUP;
    gostruct[':'] = JS0N_LOOP;
    gostruct[','] = JS0N_LOOP;
    gostruct['['] = JS0N_UP;
    gostruct[']'] = JS0N_DOWN;
    gostruct['{'] = JS0N_UP;
    gostruct['}'] = JS0N_DOWN;
    gostruct['-'] = JS0N_BARE;
    for (i = '0'; i <= '9'; ++i)
        gostruct[i] = JS0N_BARE;
    for (i = 'A'; i <= 'Z'; ++i)
        gostruct[i] = JS0N_BARE;
    for (i = 'a'; i <= 'z'; ++i)
        gostruct[i] = JS0N_BARE;

    /* gobare */
    for (i = 0; i < 256; ++i)
        gobare[i] = JS0N_BAD;
    for (i = 32; i <= 126; ++i)
        gobare[i] = JS0N_LOOP;
    gobare['\t'] = JS0N_UNBARE;
    gobare[' '] = JS0N_UNBARE;
    gobare['\r'] = JS0N_UNBARE;
    gobare['\n'] = JS0N_UNBARE;
    gobare[','] = JS0N_UNBARE;
    gobare[']'] = JS0N_UNBARE;
    gobare['}'] = JS0N_UNBARE;
    gobare[':'] = JS0N_UNBARE;
    for (i = 127; i <= 255; ++i)
        gobare[i] = JS0N_BAD;

    /* gostring */
    for (i = 0; i < 256; ++i)
        gostring[i] = JS0N_BAD;
    for (i = 32; i <= 126; ++i)
        gostring[i] = JS0N_LOOP;
    for (i = 128; i <= 191; ++i)
        gostring[i] = JS0N_BAD;
    for (i = 192; i <= 223; ++i)
        gostring[i] = JS0N_UTF8_2;
    for (i = 224; i <= 239; ++i)
        gostring[i] = JS0N_UTF8_3;
    for (i = 240; i <= 247; ++i)
        gostring[i] = JS0N_UTF8_4;
    for (i = 248; i <= 255; ++i)
        gostring[i] = JS0N_BAD;
    for (i = 0; i <= 31; ++i)
        gostring[i] = JS0N_BAD;
    gostring[127] = JS0N_BAD;
    gostring['\\'] = JS0N_ESC;
    gostring['"'] = JS0N_QDOWN;

    /* goutf8_continue */
    for (i = 0; i <= 127; ++i)
        goutf8_continue[i] = JS0N_BAD;
    for (i = 128; i <= 191; ++i)
        goutf8_continue[i] = JS0N_UTF_CONTINUE;
    for (i = 192; i <= 255; ++i)
        goutf8_continue[i] = JS0N_BAD;

    /* goesc */
    for (i = 0; i < 256; ++i)
        goesc[i] = JS0N_BAD;
    goesc['"'] = JS0N_UNESC;
    goesc['\\'] = JS0N_UNESC;
    goesc['/'] = JS0N_UNESC;
    goesc['b'] = JS0N_UNESC;
    goesc['f'] = JS0N_UNESC;
    goesc['n'] = JS0N_UNESC;
    goesc['r'] = JS0N_UNESC;
    goesc['t'] = JS0N_UNESC;
    goesc['u'] = JS0N_UNESC;
}

const char* js0n(const char* key, size_t klen,
    const char* json, size_t jlen, size_t* vlen)
{
    const char* val = 0;
    const char *cur, *end, *start;
    size_t index = 1;
    int depth = 0;
    int utf8_remain = 0;

    js0n_state gostruct[256];
    js0n_state gobare[256];
    js0n_state gostring[256];
    js0n_state goutf8_continue[256];
    js0n_state goesc[256];
    js0n_state* go;
    js0n_state state;

    if (!json || jlen <= 0 || !vlen)
        return 0;
    *vlen = 0;

    js0n_init_tables(gostruct, gobare, gostring, goutf8_continue, goesc);
    go = gostruct;

    // no key is array mode, klen provides requested index
    if (!key) {
        index = klen;
        klen = 0;
    } else {
        if (klen <= 0)
            klen = strlen(key); // convenience
    }

    for (start = cur = json, end = cur + jlen; cur < end; cur++) {
        state = go[(unsigned char)*cur];

        switch (state) {
        case JS0N_LOOP:
            break;

        case JS0N_BAD:
            *vlen = (size_t)(cur - json); // where error'd
            return 0;

        case JS0N_UP:
            PUSH(0);
            ++depth;
            break;

        case JS0N_DOWN:
            --depth;
            CAP(0);
            break;

        case JS0N_QUP:
            PUSH(1);
            go = gostring;
            break;

        case JS0N_QDOWN:
            CAP(-1);
            go = gostruct;
            break;

        case JS0N_ESC:
            go = goesc;
            break;

        case JS0N_UNESC:
            go = gostring;
            break;

        case JS0N_BARE:
            PUSH(0);
            go = gobare;
            break;

        case JS0N_UNBARE:
            CAP(-1);
            go = gostruct;
            /* reprocess current byte in the new state table */
            state = go[(unsigned char)*cur];
            switch (state) {
            case JS0N_LOOP:
                break;
            case JS0N_BAD:
                *vlen = (size_t)(cur - json);
                return 0;
            case JS0N_UP:
                PUSH(0);
                ++depth;
                break;
            case JS0N_DOWN:
                --depth;
                CAP(0);
                break;
            case JS0N_QUP:
                PUSH(1);
                go = gostring;
                break;
            case JS0N_BARE:
                PUSH(0);
                go = gobare;
                break;
            default:
                *vlen = (size_t)(cur - json);
                return 0;
            }
            break;

        case JS0N_UTF8_2:
            go = goutf8_continue;
            utf8_remain = 1;
            break;

        case JS0N_UTF8_3:
            go = goutf8_continue;
            utf8_remain = 2;
            break;

        case JS0N_UTF8_4:
            go = goutf8_continue;
            utf8_remain = 3;
            break;

        case JS0N_UTF_CONTINUE:
            if (!(--utf8_remain))
                go = gostring;
            break;

        default:
            *vlen = (size_t)(cur - json);
            return 0;
        }
    }

    if (depth)
        *vlen = jlen; // incomplete
    return 0;
}

#else

/* Original GNU fast path */

const char* js0n(const char* key, size_t klen,
    const char* json, size_t jlen, size_t* vlen)
{
    const char* val = 0;
    const char *cur, *end, *start;
    size_t index = 1;
    int depth = 0;
    int utf8_remain = 0;
    static void* gostruct[] = {
        [0 ... 255] = &&l_bad,
        ['\t'] = &&l_loop,
        [' '] = &&l_loop,
        ['\r'] = &&l_loop,
        ['\n'] = &&l_loop,
        ['"'] = &&l_qup,
        [':'] = &&l_loop,
        [','] = &&l_loop,
        ['['] = &&l_up,
        [']'] = &&l_down, // tracking [] and {} individually would allow fuller validation but is really messy
        ['{'] = &&l_up,
        ['}'] = &&l_down,
        ['-'] = &&l_bare,
        [48 ... 57] = &&l_bare, // 0-9
        [65 ... 90] = &&l_bare, // A-Z
        [97 ... 122] = &&l_bare // a-z
    };
    static void* gobare[] = {
        [0 ... 31] = &&l_bad,
        [32 ... 126] = &&l_loop, // could be more pedantic/validation-checking
        ['\t'] = &&l_unbare,
        [' '] = &&l_unbare,
        ['\r'] = &&l_unbare,
        ['\n'] = &&l_unbare,
        [','] = &&l_unbare,
        [']'] = &&l_unbare,
        ['}'] = &&l_unbare,
        [':'] = &&l_unbare,
        [127 ... 255] = &&l_bad
    };
    static void* gostring[] = {
        [0 ... 31] = &&l_bad, [127] = &&l_bad, [32 ... 126] = &&l_loop, ['\\'] = &&l_esc, ['"'] = &&l_qdown, [128 ... 191] = &&l_bad, [192 ... 223] = &&l_utf8_2, [224 ... 239] = &&l_utf8_3, [240 ... 247] = &&l_utf8_4, [248 ... 255] = &&l_bad
    };
    static void* goutf8_continue[] = {
        [0 ... 127] = &&l_bad,
        [128 ... 191] = &&l_utf_continue,
        [192 ... 255] = &&l_bad
    };
    static void* goesc[] = {
        [0 ... 255] = &&l_bad,
        ['"'] = &&l_unesc,
        ['\\'] = &&l_unesc,
        ['/'] = &&l_unesc,
        ['b'] = &&l_unesc,
        ['f'] = &&l_unesc,
        ['n'] = &&l_unesc,
        ['r'] = &&l_unesc,
        ['t'] = &&l_unesc,
        ['u'] = &&l_unesc
    };
    void** go = gostruct;

    if (!json || jlen <= 0 || !vlen)
        return 0;
    *vlen = 0;

    // no key is array mode, klen provides requested index
    if (!key) {
        index = klen;
        klen = 0;
    } else {
        if (klen <= 0)
            klen = strlen(key); // convenience
    }

    for (start = cur = json, end = cur + jlen; cur < end; cur++) {
        goto* go[(unsigned char)*cur];
    l_loop:;
    }

    if (depth)
        *vlen = jlen; // incomplete
    return 0;

l_bad:
    *vlen = cur - json; // where error'd
    return 0;

l_up:
    PUSH(0);
    ++depth;
    goto l_loop;

l_down:
    --depth;
    CAP(0);
    goto l_loop;

l_qup:
    PUSH(1);
    go = gostring;
    goto l_loop;

l_qdown:
    CAP(-1);
    go = gostruct;
    goto l_loop;

l_esc:
    go = goesc;
    goto l_loop;

l_unesc:
    go = gostring;
    goto l_loop;

l_bare:
    PUSH(0);
    go = gobare;
    goto l_loop;

l_unbare:
    CAP(-1);
    go = gostruct;
    goto* go[(unsigned char)*cur];

l_utf8_2:
    go = goutf8_continue;
    utf8_remain = 1;
    goto l_loop;

l_utf8_3:
    go = goutf8_continue;
    utf8_remain = 2;
    goto l_loop;

l_utf8_4:
    go = goutf8_continue;
    utf8_remain = 3;
    goto l_loop;

l_utf_continue:
    if (!--utf8_remain)
        go = gostring;
    goto l_loop;
}

#endif

#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#pragma GCC diagnostic pop
#endif