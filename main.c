
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>
// #define exit(_) assert(0)

#define SOURCES 0

typedef struct {
    enum {
        SORT,
        VAR,
        LAM,
        PI,
        APP,
        LET,
    } tag;

    union {
        int sort;

        struct {
            const char *var;
            int varptr;
        };

        struct {
            const char *param;
            int type;
            int body;
            int ffi;
            int ret_type;
        } binder;

        struct { 
            int func;
            int arg;
        } app;

        struct {
            const char *name;
            int type;
            int value;
            int body;
        } let;
    };
} Term;

#define NUM_TERMS 1024*1024
Term *terms;
int *types;
int *sources;
static int terms_count = 1;

typedef struct {
    const char *names[NUM_TERMS];
    int terms[NUM_TERMS];
    int count;
} Ctx;

const char *input;
static int pos = 0;

int is_large_term(int id) {
    return terms[id].tag == LAM || terms[id].tag == PI;
}

int is_medium_term(int id) {
    return is_large_term(id) || terms[id].tag == APP;
}

int occurs(const char *name, int id);

int get_source(int id) {
    if (sources[id]) {
        return get_source(sources[id]);
    }
    return id;
}

void print_term(int id, int depth)
{
    if (SOURCES) id = get_source(id);
    Term term = terms[id];

    switch (term.tag) {

    case SORT:
        printf("Type");
        if (term.sort > 0) {
            printf(" %d", term.sort);
        }
        break;

    case VAR:
        printf("%s", term.var);
        break;

    case LAM:
        printf("\\%s: ", term.binder.param);
        if (is_large_term(term.binder.type)) {
            printf("\n");
            depth++;
            for (int i=0; i < depth; ++i) printf("    ");
        }
        print_term(term.binder.type, depth);
        printf(".\n");
        if (is_large_term(term.binder.type)) {
            depth--;
            for (int i=0; i < depth; ++i) printf("    ");
        }
        for (int i=0; i < depth; ++i) printf("    ");
        print_term(term.binder.body, depth);
        break;

    case PI:
        if (occurs(term.binder.param, term.binder.body)) {
            printf("(%s: ", term.binder.param);
            print_term(term.binder.type, depth);
            printf(") ->");
            if (is_large_term(term.binder.body)) {
                printf("\n");
                for (int i=0; i < depth; ++i) printf("    ");
            }
            print_term(term.binder.body, depth);
        } else {
            int paren_func = is_large_term(term.binder.type);

            if (paren_func) {
                printf("(");
            }

            print_term(term.binder.type, depth);

            if (paren_func) {
                printf(")");
            }
            printf(" -> ");
            if (is_large_term(term.binder.body)) {
                printf("\n");
                for (int i=0; i < depth; ++i) printf("    ");
            }
            print_term(term.binder.body, depth);
        }
        break;

    case APP:;
        term = terms[id];

        int paren_func = is_large_term(term.app.func);

        if (paren_func) {
            printf("(");
        }

        print_term(term.app.func, depth);

        if (paren_func) {
            printf(")");
        }

        printf(" ");

        int paren_arg = is_medium_term(term.app.arg);
        if (paren_arg) {
            printf("(");
        }

        if (is_large_term(term.app.arg)) {
            printf("\n");
            depth++;
            for (int i=0; i < depth; ++i) printf("    ");
        }

        print_term(term.app.arg, depth);

        if (is_large_term(term.app.arg)) {
            depth--;
            printf("\n");
            for (int i=0; i < depth; ++i) printf("    ");
        }

        if (paren_arg) {
            printf(")");
        }

        break;

    case LET:
        print_term(term.let.body, 0);
        // assert(0);
        break;
    }
}

int alloc_term(int id, Term term)
{
    int count = terms_count++;
    terms[count] = term;
    if (id) {
        types[count] = types[id];
        sources[count] = sources[id];
    }

    return count;
}

void parse_spaces(void)
{
    while (isspace(input[pos])) {
        ++pos;
    }
}

int parse_token(const char *token)
{
    int len = strlen(token);
    parse_spaces();
  
    if (!strncmp(input + pos, token, len)) {
        pos += len;
        parse_spaces();
        return 1;
    }

    return 0;
}

const char *parse_name(void)
{
    parse_spaces();
    int begin = pos;

    while (input[pos] && !isspace(input[pos]) &&
        input[pos] != '{' &&
        input[pos] != '}' &&
        input[pos] != '(' &&
        input[pos] != ')' &&
        input[pos] != ':' &&
        input[pos] != ';' &&
        input[pos] != '\\' &&
        input[pos] != ',')
    {
        ++pos;
    }

    while (pos > 2 && !strncmp(input + pos - 2, ".0", 2) || !strncmp(input + pos - 2, ".1", 2)) pos-=2;
    while (pos > 0 && input[pos-1] == '.') --pos;

    if (begin == pos) {
        return 0;
    }

    char *name = strndup(input + begin, pos - begin);

    if (!strcmp(name, "=")) {
        return 0;
    }

    if (!strcmp(name, "->")) {
        return 0;
    }
    
    return name;
}


int parse_term(void);


int parse_var(void)
{
    parse_spaces();

    const char *name = parse_name();

    if (!name) {
        return 0;
    }
 
    if (!strcmp(name, "Type")) {
        parse_spaces();
        int begin = pos;
        while (isdigit(input[pos])) ++pos;
        Term term = {
            .tag = SORT,
            .sort = atoi(strndup(input+begin, pos-begin)),
        };
        return alloc_term(0, term);
    }

    Term term = {
        .tag = VAR,
        .var = name,
    };

    return alloc_term(0, term);
}

int parse_keyword(const char *kw)
{
    parse_spaces();

    int begin = pos;
    const char *name = parse_name();

    if (!name) {
        return 0;
    }

    if (!strcmp(name, kw)) return 1;
    pos = begin;
    return 0;
}

int parse_include(void)
{
    parse_spaces();

    if (input[pos] != '"') {
        return 0;
    }

    ++pos;

    int old = pos;

    while (input[pos] && input[pos] != '"') {
        ++pos;
    }

    if (input[pos] != '"') {
        return 0;
    }

    ++pos;

    char path_buf[512];
    strncpy(path_buf, input + old, pos - old - 1);
    path_buf[pos - old - 1]=0;

    if (strlen(path_buf) < 4 || strncmp(path_buf + pos - old - 5, ".coc", 4)) {
        strcat(path_buf + pos - old - 1, ".coc");
    }

    FILE *file = fopen(path_buf, "rb");
    if (!file) {
        printf("file not found: %s\n", path_buf);
        exit(1);
    }

    char *dir = dirname(path_buf);
    int old_dir = open(".", O_RDONLY | O_DIRECTORY);

    chdir(dir);

    char content[16*1024];

    content[fread(content, 1, sizeof(content)-1, file)] = 0;
    fclose(file);

    const char *old_input = input;
    int old_pos = pos;

    input = content;
    pos = 0;

    int term_id = parse_term();

    fchdir(old_dir);

    input = old_input;
    pos = old_pos;

    if (!term_id) return 0;
    if (!parse_token(";")) return 0;

    int body = parse_term();
    if (!body) return 0;

    int b = term_id;
    while (terms[b].tag == LAM || terms[b].tag == LET) {
        if (terms[b].tag == LAM) {
            b = terms[b].binder.body;
        } else {
            b = terms[b].let.body;
        }
    }

    terms[b] = terms[body];
    return term_id;
} 

int parse_lam(void);
int parse_atom(void)
{
    int begin = pos;

    if (parse_token(",")) return parse_term();

    int id = parse_include();

    if (id) {
        return id;
    }

    pos = begin;
    id = parse_var();

    if (id) {
        return id;
    }

    pos = begin;
    if (!parse_token("(")) {
        return 0;
    }

    id = parse_term();

    if (!id) {
        return 0;
    }

    if (!parse_token(")")) {
        return 0;
    }

    return id;
}

int parse_apps(void);

int parse_binding(void)
{
    const char *name = parse_name();

    if (!name) {
        return 0;
    }

    int type=0;
    if (parse_token(":")) {
        type = parse_term();

        if (!type) {
            return 0;
        }

        if (!parse_token(";")) {
            return 0;
        }

        if (!parse_keyword(name)) {
            return 0;
        }
    }

    if (!parse_token("=")) {
        return 0;
    }

    int value = parse_term();

    if (!value) {
        return 0;
    }

    if (!parse_token(";")) {
        return 0;
    }

    int body = parse_term();

    if (!body) {
        body = alloc_term(0, (Term){
            .tag = SORT,
        });
    }

    return alloc_term(0, (Term){
        .tag = LET,
        .let = {
            .name = name,
            .type = type,
            .value = value,
            .body = body,
        },
    });
}

int parse_decl(void)
{
    const char *name = parse_name();

    if (!name) {
        return 0;
    }

    if (!parse_token(":")) {
        return 0;
    }

    int type = parse_term();

    if (!type) {
        return 0;
    }

    if (!parse_token(";")) {
        return 0;
    }

    int body = parse_term();

    if (!body) {
        body = alloc_term(0, (Term){
            .tag = SORT,
        });
    }

    return alloc_term(0, (Term){
        .tag = LAM,
        .binder = {
            .param = name,
            .type = type,
            .body = body,
            .ffi = 1,
        },
    });
}

int parse_lam(void)
{
    if (!parse_token("\\")) {
        return 0;
    }

    const char *param = parse_name();
    if (!param) {
        return 0;
    }

    if (!parse_token(":")) return 0;

    int type = parse_term();
    if (!type) return 0;
                
    if (!parse_token(".")) return 0;

    int body = parse_term();
    if (!body) {
        return 0;
    }

    Term term = {
        .tag = LAM,
        .binder = {
            .param = param,
            .type = type,
            .body = body,
        },
    };

    return alloc_term(0, term);
}

int parse_pi(void)
{
    if (!parse_token("(")) {
        return 0;
    }

    const char *param = parse_name();
    if (!param) {
        return 0;
    }

    if (!parse_token(":")) return 0;

    int type = parse_term();
    if (!type) return 0;
                
    if (!parse_token(")")) return 0;
    if (!parse_token("->")) return 0;

    int body = parse_term();
    if (!body) {
        return 0;
    }

    Term term = {
        .tag = PI,
        .binder = {
            .param = param,
            .type = type,
            .body = body,
        },
    };

    return alloc_term(0, term);
}

int parse_anon(void)
{
    int type = parse_apps();
    if (!type) return 0;
                
    if (!parse_token("->")) return type;

    int body = parse_term();
    if (!body) {
        return 0;
    }

    Term term = {
        .tag = PI,
        .binder = {
            .param = "_",
            .type = type,
            .body = body,
        },
    };

    return alloc_term(0, term);
}

int parse_apps(void)
{
    int func = parse_atom();

    if (!func) {
        return 0;
    }

    while (1) {
        parse_spaces();
        int arg = parse_atom();

        if (!arg) {
            break;
        }

        Term term = {
            .tag = APP,
            .app = {
                .func = func,
                .arg = arg,
            },
        };

        func = alloc_term(0, term);
    }

    return func;
}

int parse_term(void)
{
    int begin = pos;
    int id = parse_lam();
    if (id) return id;
    pos = begin;
    id = parse_binding();
    if (id) return id;
    pos = begin;
    id = parse_decl();
    if (id) return id;
    pos = begin;

    id = parse_pi();
    if (id) return id;
    pos = begin;
    return parse_anon();
}

int occurs(const char *name, int id)
{
    Term term = terms[id];

    switch (term.tag) {
    case SORT:
        return 0;

    case VAR:
        return !strcmp(name, term.var);

    case LAM:
    case PI:
        if (occurs(name, term.binder.type)) {
            return 1;
        }

        if (!strcmp(name, term.binder.param)) {
            return 0;
        }

        return occurs(name, term.binder.body);

    case APP:
        return occurs(name, term.app.func) || occurs(name, term.app.arg);

    case LET:
        if (term.let.type && occurs(name, term.let.type)) {
            return 1;
        }

        if (occurs(name, term.let.value)) {
            return 1;
        }

        if (!strcmp(name, term.let.name)) {
            return 0;
        }

        return occurs(name, term.let.body);
    }
}

char names[1024*1024];
int names_len=0;

Term make_unique_var(void)
{
    static int unique = 0;

    char buffer[64];
    int n = sprintf(buffer, "_%d", unique);

    unique++;

    strncpy(names + names_len, buffer, n);
    Term term = {
        .tag = VAR,
        .var = names + names_len,
    };
    names[names_len + n] = 0;
    names_len += n + 1;

    return term;
}

int capture_avoiding = 0;

int subst(const char *name, int value, int id)
{
    Term old = terms[id];
    if (sources[id]) {
        // int new_id = alloc_term(id, terms[id]);
        // types[new_id] = subst(name, value, types[id]);
        // id = new_id;
    }
    Term term = terms[id];

    switch (term.tag) {
    case SORT:
        return id;

    case VAR:
        if (!strcmp(name, term.var)) {
            // return value;
            // return value;
            int new_id = alloc_term(value, terms[value]);
            if (!types[value]) types[new_id] = types[id];
            return new_id;
        }
        
        return id;

    case LAM:
    case PI:
        term.binder.type = subst(name, value, term.binder.type);

        if (!strcmp(name, term.binder.param)) {
            return alloc_term(id, term);
        }

        if (capture_avoiding && occurs(term.binder.param, value)) {
            Term unique = make_unique_var();

            term.binder.body = subst(term.binder.param, alloc_term(0, unique), term.binder.body);
            term.binder.param = unique.var;
        }

        term.binder.body = subst(name, value, term.binder.body);

        if (old.binder.type == term.binder.type && old.binder.body == term.binder.body) return id;
        return alloc_term(id, term);

    case APP:
        term.app.func = subst(name, value, term.app.func);
        term.app.arg = subst(name, value, term.app.arg);
        if (old.app.func == term.app.func && old.app.arg == term.app.arg) return id;

        return alloc_term(id, term);

    case LET:
        if (term.let.type) term.let.type = subst(name, value, term.let.type);
        term.let.value = subst(name, value, term.let.value);

        if (!strcmp(name, term.let.name)) {
            return alloc_term(id, term);
        }

        if (capture_avoiding && occurs(term.let.name, value)) {
            Term unique = make_unique_var();

            term.let.body = subst(term.let.name, alloc_term(0, unique), term.let.body);
            term.let.name = unique.var;
        }

        term.let.body = subst(name, value, term.let.body);

        return alloc_term(id, term);
    }
}

void ctx_push(Ctx *ctx, const char *name, int id)
{
    ctx->names[ctx->count] = name;
    ctx->terms[ctx->count] = id;

    ctx->count++;
}

void ctx_pop(Ctx *ctx)
{
    ctx->count--;
}

int ctx_find(Ctx *ctx, const char *name)
{
    for (int i = ctx->count - 1; i >= 0; --i) {
        if (!strcmp(ctx->names[i], name)) {
            if (ctx->terms[i]) {
                return ctx->terms[i];
            }

            return 0;
        }
    }

    return 0;
}

static Ctx ctx;
static Ctx env;

int betas = 0;
int normalize(int id)
{
    Term term = terms[id];

    switch (term.tag) {
    case SORT:
        return id;

    case VAR:;
        if (term.varptr) return normalize(term.varptr);
        int found = ctx_find(&env, term.var);

        if (found) {
            return found;
        }

        return id;

    case LAM:
    case PI:
        term.binder.type = normalize(term.binder.type);

        ctx_push(&env, term.binder.param, 0);
        term.binder.body = normalize(term.binder.body);
        ctx_pop(&env);
        
        return alloc_term(id, term);

    case APP:;
        int old = term.app.func;
        term.app.func = normalize(term.app.func);
        term.app.arg = normalize(term.app.arg);

        Term func = terms[term.app.func];
        if (func.tag == LAM) {
            betas++;
            int newid = normalize(subst(func.binder.param, term.app.arg, func.binder.body));
            if (SOURCES) {
                while (sources[id]) id = sources[id];
                while (sources[old]) old = sources[old];
                if (terms[old].tag != LAM) {
                    sources[newid] = alloc_term(id, term);
                }
            }
            return newid;
        }

        return alloc_term(id, term);
            
    case LET:
        // term.let.type = normalize(term.let.type);
        term.let.value = normalize(term.let.value);

        // ctx_push(&ctx, term.let.name, term.let.type);
        ctx_push(&env, term.let.name, term.let.value);
        if (SOURCES) {
            sources[term.let.value] = alloc_term(0, (Term){
                .tag = VAR,
                .var = term.let.name,
            });
        }


        term.let.body = subst(term.let.name, term.let.value, normalize(term.let.body));
        // ctx_pop(&ctx);
        ctx_pop(&env);

        return term.let.body;
    }
}

int equal(int lhs_id, int rhs_id)
{
    Term lhs = terms[lhs_id];
    Term rhs = terms[rhs_id];

    if (lhs.tag != rhs.tag) {
        return 0;
    }

    switch (lhs.tag) {
    case SORT:
        return 1;
        return lhs.sort == rhs.sort;

    case VAR:
        return !strcmp(lhs.var, rhs.var);

    case LAM:
    case PI:
        if (!equal(lhs.binder.type, rhs.binder.type)) {
            return 0;
        }

        if (strcmp(lhs.binder.param, rhs.binder.param)) {
            Term param = {
                .tag = VAR,
                .var = lhs.binder.param,
            };

            rhs.binder.body = subst(rhs.binder.param, alloc_term(0, param), rhs.binder.body);
        }

        return equal(lhs.binder.body, rhs.binder.body);

    case APP:
        return equal(lhs.app.func, rhs.app.func) && equal(lhs.app.arg, rhs.app.arg);

    case LET:
        assert(0 && "Unreachable");
    }
}

int infer(int id, int expected)
{
    int old_id = id;

    Term term = terms[id];

    switch (term.tag) {
    case SORT:
        term.sort++;
        types[id] = alloc_term(0, term);
        break;

    case VAR:;
        int found = ctx_find(&ctx, term.var);

        if (found) {
            // term.varptr = ctx_find(&env, term.var);
            id = normalize(alloc_term(id, term));
            types[id] = normalize(found);
            break;
        }

        if (term.var[0] == '?') {
            printf("hole found\n");
            printf("expected: ");
            print_term(expected, 0);
            exit(1);
        }

        printf("Unbound identifier: %s\n", term.var);
        exit(1);

    case LAM:
    case PI:;
        term.binder.type = infer(term.binder.type, 0);
        int domain_kind = types[term.binder.type];

        ctx_push(&env, term.binder.param, 0);
        ctx_push(&ctx, term.binder.param, term.binder.type);

        int body = 0;
        if (term.tag == LAM && expected) {
            if (terms[expected].tag != PI) {
                expected = normalize(expected);
                if (terms[expected].tag != PI) {
                    print_term(expected, 0);
                    exit(42);
                }
            }
            Term var = {
                .tag = VAR,
                .var = term.binder.param,
            };
            if (strcmp(terms[expected].binder.param, term.binder.param)) {
                body = subst(terms[expected].binder.param, alloc_term(0, var), terms[expected].binder.body);
            } else {
                body = terms[expected].binder.body;
            }
        }
        term.binder.body = infer(term.binder.body, body);

        ctx_pop(&env);
        ctx_pop(&ctx);

        if (term.tag != LAM) {
            int codomain_kind = types[term.binder.body];

            Term domain_term = terms[domain_kind];
            Term codomain_term = terms[codomain_kind];

            assert(domain_term.tag == SORT);
            assert(codomain_term.tag == SORT);

            Term result;
            if (codomain_term.sort == 0) {
                result = codomain_term;
            } else {
                result = (Term){
                    .tag = SORT,
                    .sort = domain_term.sort>codomain_term.sort ? domain_term.sort : codomain_term.sort,
                };
            }

            id = alloc_term(id, term);
            types[id] = alloc_term(0, result);
            break;
        }

        term.tag = PI;
        int old = term.binder.body;
        term.binder.body = types[old];
        int type = alloc_term(0, term);
        term.tag = LAM;
        term.binder.body = old;
        term.binder.ret_type = types[old];
        id = alloc_term(id, term);
        types[id] = type;
        break;

    case APP:
        term.app.func = infer(term.app.func, 0);

        Term func = terms[normalize(types[term.app.func])];

        if (func.tag != PI) {
            print_term(term.app.func, 0);
            exit(69);
        }

        term.app.arg = normalize(infer(term.app.arg, func.binder.type));

        id = normalize(alloc_term(id, term));//evaluate(alloc_term(id, term));
        betas++;
        types[id] = normalize(subst(func.binder.param, term.app.arg, func.binder.body));
        assert(types[id]);
        break;

    case LET:;
        if (term.let.type) {
            term.let.type = infer(term.let.type, 0);
            int type_kind = types[term.let.type];
            assert(terms[type_kind].tag == SORT);
        }

        // ctx_push(&ctx, term.let.name, term.let.type);
        term.let.value = normalize(infer(term.let.value, term.let.type));
        int value_type = types[term.let.value];

        term.let.type = value_type;

        // ctx_pop(&ctx);
        ctx_push(&ctx, term.let.name, value_type);
        // int oc = occurs(term.let.name, term.let.value);
        // if (!oc)
            ctx_push(&env, term.let.name, term.let.value);

        sources[term.let.value] = alloc_term(0, (Term){
            .tag = VAR,
            .var = term.let.name,
        });

        betas++;
        term.let.body = infer(term.let.body, expected);
        
        ctx_pop(&ctx);
        // if (!oc)
            ctx_pop(&env);

        id = alloc_term(id, term);//term.let.body;
        types[id] = types[term.let.body];
        break;
    }

    // if (!types[id]) {
    //     print_term(id);
    // }

    types[id] = normalize(types[id]);
    if (expected) {
        expected = normalize(expected);
        if (!equal(types[id], expected)) {
            printf("\n");
            print_term(expected, 0);
            printf("\n");
            print_term(types[id], 0);
            printf("\n");
            print_term(id, 0);
            exit(1);
        }
    }

    return id;
}

void print_name(const char *s) {
    while (*s) {
        if (*s == '.') printf("_");
        else printf("%c", *s);
        ++s;
    }
}

int is_type(int cp) {
    while (terms[cp].tag == PI) {
        cp = terms[cp].binder.body;
    }
    if (terms[cp].tag == SORT) {
        return 1;
    }
    return 0;
}

void compile_type(int id)
{
    Term term = terms[id];

    switch (term.tag) {
    case SORT:
        break;

    case PI:
        printf("auto");
        break;

    case VAR:
        printf("coc_");
        print_name(term.var);
        break;

    case LAM:;
        return;
      
    case APP:;
        int args[32];
        int count = 0;

        while (terms[id].tag == APP) {
            // if (is_type(types[terms[id].app.arg]))
                args[count++] = terms[id].app.arg;

            id = terms[id].app.func;
        }

        compile_type(id);

        printf("<");
        for (int i = count-1; i >= 0; --i) {
            if (i < count-1) printf(", ");
            compile_type(args[i]);
        }
        printf(">");
        break;

    case LET:
        assert(0);
    }
}

int eta_expand(int id) {
    while (terms[types[id]].tag == PI && terms[id].tag != LAM) {
        // if (is_type(terms[types[id]].binder.type)) {
            // return id;
        // }
        Term v = make_unique_var();
        int v2 = alloc_term(0, v);
        types[v2] = terms[types[id]].binder.type;
        int ap = alloc_term(0, (Term){
            .tag = APP,
            .app = {alloc_term(0, terms[id]), v2},
        });
        types[ap] = terms[types[id]].binder.body;
        id = alloc_term(id, (Term){
            .tag = LAM,
            .binder = {
                .param = v.var,
                .type = terms[types[id]].binder.type,
                .body = eta_expand(ap),
            },
        });
    }
    return id;
}

void compile(int id, int depth)
{
    Term term = terms[id];
    switch (term.tag) {
    case SORT:
    case PI:
ty:
        printf("0");
        break;

    case VAR:
        if (is_type(types[id])) {
            printf("0");
            return;
        }
        printf("coc_");
        print_name(term.var);
        break;

    case LAM:;
        if ((is_type(types[id])) || term.binder.ffi) {
            compile(term.binder.body, depth);
            return;
        }

        printf("[=](");
        int val = id;
        int i = 0;
        // while (terms[val].tag == LAM) {
            // if (i > 0) printf(", ");
            // compile_type(terms[val].binder.type);
            printf("auto coc_%s", terms[val].binder.param);
            ++i;
            val = terms[val].binder.body;
        // }
        // if (terms[term.binder.body].tag != LAM) {
        //     printf(" -> ");
        //     compile_type(term.binder.ret_type);
        // }

        printf(") { return ");

        compile(val, depth);
        printf("; }");
        return;
      
    case APP:;
        if (is_type(types[id])) {
            printf("0");
            return;
        }
        int type_args[32];
        int tcount = 0;
        int args[32];
        int count = 0;

        while (terms[id].tag == APP) {
            // if (!is_type(types[terms[id].app.arg])) {
                args[count++] = terms[id].app.arg;
            // } else {
                // type_args[tcount++] = terms[id].app.arg;
            // }

            id = terms[id].app.func;
        }

        if (!count) return;

        printf("(");
        compile(id, depth);

        // if (tcount && !count) {
        //     printf("<");
        //     for (int i = tcount-1; i >= 0; --i) {
        //         if (i < tcount-1) printf(", ");
        //         compile_type(type_args[i]);
        //     }
        //     printf(">");
        // }

        printf("(");
        for (int i = count-1; i >= 0; --i) {
            if (i < count-1) printf(")(");
            compile(args[i], depth);
        }
        printf("))");
        break;

    case LET:
        if (is_type(types[term.let.value])) {
            compile(term.let.body, depth);
            return;
        }
        if (terms[term.let.value].tag == LAM) {
            const char *tyargs[32];
            int tcount=0;
            {
            int val = term.let.value;
            while (terms[val].tag == LAM) {
                // if (is_type(terms[val].binder.type)) {
                    // tyargs[tcount++] = terms[val].binder.param;
                // }
                val = terms[val].binder.body;
            }
            }

            // if (tcount) {
            //     printf("template <");
            //     for (int i=0; i < tcount; ++i) {
            //         if (i>0) printf(", ");
            //         printf("typename coc_%s", tyargs[i]);
            //     }
            //     printf(">\n");
            // }

            int val = term.let.value;
            int last = val;
            while (terms[last].tag == LAM) last = terms[last].binder.body;
            printf("static ");
            // compile_type(types[last]);
            printf("auto coc_%s(", term.let.name);
            int i = 0;
            // while (terms[val].tag == LAM) {
                // if (is_type(terms[val].binder.type)) {
                    // val = terms[val].binder.body;
                    // return;
                    // continue;
                // }
                // if (i > 0) printf(", ");
                // compile_type(terms[val].binder.type);
                printf("auto coc_%s", terms[val].binder.param);
                ++i;
                val = terms[val].binder.body;
            // }
            printf(") {\nreturn ");
            compile(val, depth + 1);
            printf(";\n}\n\n");
        } else
        {
            // if (is_type(term.let.type)) {
                // goto next;
            // }
            int val = term.let.value;
            while (terms[val].tag == LAM) {
                // if (is_type(terms[val].binder.type)) goto next;
                val = terms[val].binder.body;
            }
            printf("auto coc_%s = []() { return ", term.let.name);
            compile(term.let.value, depth);
            printf("; }();\n\n");
        }
next:
        if (terms[term.let.body].tag != SORT)
            compile(term.let.body, depth);
    }
}

int main(int argc, char **argv) {
    assert(argc >= 2);
    const char *file_name = argv[1];
    char *buf = malloc(16*4096);
    memset(buf, 0, 16*4096);

    terms = malloc(sizeof(*terms) * NUM_TERMS);
    types = malloc(sizeof(*types) * NUM_TERMS);
    memset(types, 0, sizeof(*types) * NUM_TERMS);
    sources = malloc(sizeof(*sources) * NUM_TERMS);
    memset(sources, 0, sizeof(*sources) * NUM_TERMS);

    FILE *f = fopen(argv[1], "rb");
    int n = fread(buf, 1, 16*4096, f);
    fclose(f);
    buf[n] = 0;
    input = buf;

    char *dr = dirname(argv[1]);
    chdir(dr);

    int v = parse_term();
    if (!v) exit(1);
    parse_spaces();
    if (input[pos]) {
        printf("%s\n", input + pos);
        exit(1);
    }
    capture_avoiding = 1;
    v = (infer(v, 0));
    compile((v), 0);
}
