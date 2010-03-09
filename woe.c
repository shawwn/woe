#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define LSP(x) (x == ' ' || x == '\t')
#define ZP(x)  (x == NULL)
#define R      return

#define D1(e) e->d
#define D2(e) D1(e)->n
#define D3(e) D2(e)->n
#define D4(e) D3(e)->n

#define C1(e) e->c
#define C2(e) C1(e)->n

#define P1(o,e) do { o->n = D2(e); D1(e) = o; } while (0);
#define P2(o,e) do { o->n = D3(e); D1(e) = o; } while (0);

#define A1(e, w)                                \
  if (ZP(D1(e))) {                              \
    re(e, "stack underflow", w); R;             \
  }
#define A2(e, w)                                \
  if (ZP(D1(e)) || ZP(D2(e))) {                 \
    re(e, "stack underflow", w); R;             \
  }
#define A3(e, w)                                \
  if (ZP(D1(e)) || ZP(D2(e)) || ZP(D3(e))) {    \
    re(e, "stack underflow", w); R;             \
  }

#define T1(e, a, s, w)                          \
  if (D1(e)->t != a) {                          \
    re(e, s, w); R;                             \
  }
#define T2(e, a, s, w)                          \
  if (D1(e)->t != a || D2(e)->t != a) {         \
    re(e, s, w); R;                             \
  }

#define TP(e, a, w)                             \
  do {                                          \
    N _n; _n = nb(e->dh, 0); A1(e, w);          \
    if (D1(e)->t == a) _n->v.i = 1;             \
    _n->n = D2(e); D1(e) = _n;                  \
  } while (0);

#define F1(e, n, w) N n; A1(e, w); T1(e, N_F, "expected two floats", w);
#define F2(e, n, w) N n; A2(e, w); T2(e, N_F, "expected a float", w);
#define I1(e, n, w) N n; A1(e, w); T1(e, N_I, "expected two integers", w);
#define I2(e, n, w) N n; A2(e, w); T2(e, N_I, "expected an integer", w);

#define BYTESP(n) (n->t == N_S || n->t == N_Y)

typedef void   V;
typedef char   C;
typedef char*  S;
typedef size_t L;
typedef long   I;
typedef double F;

typedef const char* CS;

typedef struct t {
  enum {
    T_EL, T_EF,                 /* terminators */
    T_S, T_I, T_F, T_LP, T_Y,   /* term start  */
    T_CL, T_SCL, T_RP           /* term end    */
  } t;
  union { I i; F f; S s; } v;
} T;

typedef struct p { FILE *s; L l, p; C b[1024]; }* P;

typedef struct h { S d; L u, s; }* H;

typedef struct e {
  struct n *d, *c;
  struct w *w; H dh, ch;
}* E;

typedef struct n {
  enum {
    N_I, N_F, N_B,   /* atomic types    */
    N_S, N_Y,        /* composite types */
    N_Q              /* nested types    */
  } t;
  union {
    I i; F f;
    struct { L l; S b; } d;
    struct n *q;
  } v;
  struct n *o, *n;
}* N;

typedef struct w {
  enum { W_F, W_Q } t; S s;
  union { V (*f)(E); N q; } c;
  struct w *n;
}* W;

typedef const struct n* CN;

V* ma(H h, L n) { V *p; p = h->d + h->u; h->u += n; R (p); }

V pns(CN);

V pn(CN n)
{
  if (ZP(n)) R;
  switch (n->t) {
  case N_S:
    printf("\"%s\"", n->v.d.b); break;
  case N_Y:
    printf("%s", n->v.d.b); break;
  case N_I:
    printf("%ld", n->v.i); break;
  case N_F:
    printf("%.2f", n->v.f); break;
  case N_B:
    if (n->v.i) printf("t");
    else printf("f");
    break;
  case N_Q:
    printf("("); pns(n->v.q); printf(")"); break;
  }
}

V pns(CN n)
{
  C sp; sp = 0;
  while (!ZP(n)) {
    if (sp) printf(" ");
    pn(n); n = n->n; sp = 1;
  }
}

V ip(P p, FILE *f) { p->s = f; p->l = 0; p->p = 0; }

C rc(P p)
{
  C c;
  if (p->p == p->l) {
    if (ZP(fgets(p->b, 1024, p->s))) R ('\0');
    p->l = strlen(p->b); p->p = 0;
  }
  c = p->b[p->p++];
  R (c);
}

V uc(P p) { p->p--; }

T rs(H h, P p)
{
  L l; C c, b[1024]; T t;
  l = 0; t.t = T_S;
  while ((c = rc(p)) != '"') {
    if (c == '\\') {
      b[l++] = c; c = rc(p);
    }
    b[l++] = c;
  }
  b[l++] = '\0';
  t.v.s  = (C*)ma(h, sizeof(C)*l);
  strncpy(t.v.s, b, l);
  R (t);
}

T ri(P p)
{
  L l; C c, b[1024]; T t;
  l = 0; t.t = T_Y;
  while ((c = rc(p)) != '\0') {
    if (!strchr("0123456789+-", c)) {
      if (strchr(".eE", c)) t.t = T_F;
      else {
        uc(p);
        if (l > 1 || (l == 1 && b[0] != '-')) break;
        R (t);
      }
    }
    b[l++] = c;
  }
  b[l] = '\0';
  if (t.t == T_F)
    t.v.f = strtod(b, NULL);
  else {
    t.t = T_I; t.v.i = strtol(b, NULL, 10);
  }
  R (t);
}

T ry(H h, P p)
{
  L l; C c, b[1024]; T t;
  l = 0; t.t = T_Y;
  while ((c = rc(p)) != '\0') {
    if (LSP(c) || c == '\n' || strchr("():;", c)) {
      uc(p); break;
    }
    b[l++] = c;
  }
  b[l++] = '\0';
  t.v.s = (C*)ma(h, sizeof(C)*l);
  strncpy(t.v.s, b, l);
  R (t);
}

T rt(H h, P p)
{
  C c; T t;
restart:
  do {
    c = rc(p);
  } while (LSP(c));
  if (c == '\\') {
    do {
      c = rc(p);
    } while (c != '\n');
    goto restart;
  }
  switch (c) {
  case '\n': t.t = T_EL;  R (t);
  case '\0': t.t = T_EF;  R (t);
  case '(':  t.t = T_LP;  R (t);
  case ')':  t.t = T_RP;  R (t);
  case ':':  t.t = T_CL;  R (t);
  case ';':  t.t = T_SCL; R (t);
  case '"':  R (rs(h, p));
  case '-':  uc(p); if ((t = ri(p)).t != T_Y) R (t);
  default:
    uc(p);
    if (isdigit(c)) t = ri(p);
    else t = ry(h, p);
    R (t);
  }
}

V pe(CS s) { printf("PARSE ERROR: %s\n", s); }
V re(E e, CS s, CS w) { printf("ERROR %s: %s\n", w, s); C1(e) = NULL; }

N nn(H h)
{
  N n; n = (N)ma(h, sizeof(struct n));
  memset(n, 0, sizeof(struct n));
  R (n);
}

N ni(H h, I i) { N n; n = nn(h); n->t = N_I; n->v.i = i; R (n); }
N nf(H h, F f) { N n; n = nn(h); n->t = N_F; n->v.f = f; R (n); }
N nb(H h, I i) { N n; n = nn(h); n->t = N_B; n->v.i = i; R (n); }

N ns(H h, S s)
{
  N n; n = nn(h); n->t = N_S;
  n->v.d.b = s; n->v.d.l = strlen(s) + 1;
  R (n);
}

N ny(H h, S s)
{
  N n; n = nn(h); n->t = N_Y;
  n->v.d.b = s;
  n->v.d.l = strlen(s) + 1;
  R (n);
}

N nq(H h) { N n; n = nn(h); n->t = N_Q; R (n); }
W nw(H h, S s) { W w; w = (W)ma(h, sizeof(struct w)); w->s = s; R (w); }

N cb(H h, N n, CN o)
{
  S b; L l;
  l = o->v.d.l; b = ma(h, l);
  memcpy(b, o->v.d.b, l);
  n->v.d.b = b; n->v.d.l = l;
  R (n);
}

N cn(H h, CN o)
{
  if (ZP(o)) R (NULL);
  {
    N n, q;
    switch (o->t) {
    case N_S: case N_Y: {
      n = nn(h); n->t = o->t;
      cb(h, n, o); R (n);
    }
    case N_Q: {
      N t; n = nq(h);
      n->v.q = q = cn(h, o->v.q);
      t = o->v.q;
      while (!ZP(t)) {
        q->n = cn(h, t->n); q = q->n; t = t->n;
      }
      R (n);
    }
    default: n = nn(h); n->t = o->t; n->v = o->v;
    }
    R (n);
  }
}

N app(N n, N *h, N *l)
{
  if (ZP(*h)) *h = n;
  if (ZP(*l)) *l = n;
  else (*l)->n = n;
  R (n);
}

N rq(H, P);

N ra(H h, P p, T t)
{
  switch (t.t) {
  case T_I:  R (ni(h, t.v.i));
  case T_F:  R (nf(h, t.v.f));
  case T_S:  R (ns(h, t.v.s));
  case T_Y:  R (ny(h, t.v.s));
  case T_LP: R (rq(h, p));
  default:   R (NULL);
  }
}

N rq(H h, P p)
{
  T t; N l, n;
  n = nq(h); l = NULL;
  while ((t = rt(h, p)).t <= T_Y) {
    switch (t.t) {
    case T_EL: continue;
    case T_EF: pe("unexpected end-of-file"); R (NULL);
    default: l = app(ra(h, p, t), &n->v.q, &l);
    }
  }
  R (n);
}

W rw(H h, P p)
{
  T t; W w;
  if ((t = rt(h, p)).t == T_Y) {
    w = nw(h, t.v.s);
    w->t = W_Q; w->c.q = rq(h, p);
    R (w);
  }
  pe("expected word name after ':'");
  R (NULL);
}

H nh(L s)
{
  H h; h = (H)malloc(sizeof(struct h));
  h->u = 0; h->s = s; h->d = (C*)malloc(s);
  if (ZP(h) || ZP(h->d)) {
    perror("ERROR:"); exit (1);
  }
  R(h);
}

L ln(N n)
{
  if (!ZP(n) && BYTESP(n))
    R (sizeof(struct n) + n->v.d.l);
  R (sizeof(struct n));
}

V cp(N *p, H h)
{
  if (ZP(*p)) R;
  if (ZP((*p)->o)) {
    N t; t = nn(h);
    memcpy(t, *p, sizeof(struct n));
    (*p)->o = t; *p = t;
    if (BYTESP((*p))) cb(h, t, t);
  } else *p = (*p)->o;
}

V gc(E e)
{
  S r; N t; H h;
  h = nh(e->dh->s); r = h->d;
  cp(&e->d, h); cp(&e->c, h);
  while (r < (h->d + h->u)) {
    t = (N)r;
    if (t->t == N_Q) cp(&t->v.q, h);
    cp(&t->n, h); r += ln(t);
  }
  free(e->dh->d); free(e->dh);
  e->dh = h;
}

W wl(W w, CS s)
{
  while (!ZP(w)) {
    if (strcasecmp(w->s, s) == 0) R (w);
    w = w->n;
  }
  R (NULL);
}

V ev(E);

V eq(E e, N n)
{
  struct e i;
  i.c = n->v.q; i.d = D1(e); i.w = e->w;
  i.dh = e->dh; i.ch = e->ch;
  ev(&i); D1(e) = i.d;
}

V wc(const W w, E e)
{
  if (w->t == W_F) w->c.f(e);
  else eq(e, w->c.q);
}

V ev(E e)
{
  while (!ZP(C1(e))) {
    if (C1(e)->t == N_Y) {
      W w;
      if ((w = wl(e->w, C1(e)->v.d.b))) {
        C1(e) = C2(e);
        wc(w, e);
      } else re(e, "undefined word", C1(e)->v.d.b);
    } else {
      N n; n = cn(e->dh, C1(e));
      n->n = D1(e); D1(e) = n; C1(e) = C2(e);
    }
  }
}

V w_swap(E e) /* b a -- a b */
{ N n; A2(e, "~"); n = D2(e); D2(e) = n->n; n->n = D1(e); D1(e) = n; }

V w_dup(E e) /* a -- a a */
{ N n; A1(e, "''"); n = cn(e->dh, D1(e)); n->n = D1(e); D1(e) = n; }

V w_pop(E e) /* b a -- b */ { A1(e, "_"); D1(e) = D2(e); }

V w_cat(E e) /* (b) (a) -- (b a) */
{
  N l; A2(e, ","); T2(e, N_Q, "cannot concatenate non-quotations", ",");
  l = D2(e)->v.q;
  if (!ZP(l)) {
    while (!ZP(l->n)) l = l->n;
    l->n = D1(e)->v.q;
  }
  w_pop(e);
}

V w_cons(E e) /* b (a) -- (b a) */
{
  N n; A2(e, ",'"); T1(e, N_Q, "cannot cons onto a non-quotation", ",'");
  n = D2(e); D2(e) = D3(e);
  n->n = D1(e)->v.q; D1(e)->v.q = n;
}

V w_e(E e) /* (a) -- */
{
  N n; A1(e, "e"); T1(e, N_Q, "cannot evaluate a non-quotation", "e");
  n = D1(e); D1(e) = D2(e); eq(e, n);
}

V w_t(E e) /* -- ? */
{ N n; n = nb(e->dh, 1); n->n = D1(e); D1(e) = n; }

V w_f(E e) /* -- ? */
{ N n; n = nb(e->dh, 0); n->n = D1(e); D1(e) = n; }

V w_ip(E e) /* (a -- ?) */ { TP(e, N_I, "IP"); }
V w_fp(E e) /* (a -- ?) */ { TP(e, N_F, "FP"); }
V w_bp(E e) /* (a -- ?) */ { TP(e, N_B, "BP"); }
V w_sp(E e) /* (a -- ?) */ { TP(e, N_S, "SP"); }
V w_qp(E e) /* (a -- ?) */ { TP(e, N_Q, "QP"); }

V w_b(E e) /* ? (t) (f) -- */
{
  N n; A3(e, "?"); T2(e, N_Q, "cannot branch to a non-quotation", "?");
  if (D3(e)->v.i) n = D2(e);
  else n = D1(e);
  D1(e) = D4(e); eq(e, n);
}

V w_iadd(E e) /* i i -- i */
{ I2(e, n, "I+"); n = ni(e->dh, D1(e)->v.i + D2(e)->v.i); P2(n, e); }

V w_isub(E e) /* i i -- i */
{ I2(e, n, "I-"); n = ni(e->dh, D1(e)->v.i - D2(e)->v.i); P2(n, e); }

V w_idiv(E e) /* i i -- i */
{ I2(e, n, "I/"); n = ni(e->dh, D1(e)->v.i / D2(e)->v.i); P2(n, e); }

V w_imul(E e) /* i i -- i */
{ I2(e, n, "I*"); n = ni(e->dh, D1(e)->v.i * D2(e)->v.i); P2(n, e); }

V w_imod(E e) /* i -- i */
{ I2(e, n, "I%"); n = ni(e->dh, D1(e)->v.i % D2(e)->v.i); P2(n, e); }

V w_itof(E e) /* i -- f */
{ I1(e, n, "I."); n = nf(e->dh, (F)D1(e)->v.i); P1(n, e); }

V w_ilt(E e) /* i i -- ? */
{ I2(e, n, "I<"); n = nb(e->dh, D1(e)->v.i < D2(e)->v.i); P2(n, e); }

V w_igt(E e) /* i i -- ? */
{ I2(e, n, "I>"); n = nb(e->dh, D1(e)->v.i > D2(e)->v.i); P2(n, e); }

V w_ile(E e) /* i i -- ? */
{ I2(e, n, "I<="); n = nb(e->dh, D1(e)->v.i <= D2(e)->v.i); P2(n, e); }

V w_ige(E e) /* i i -- ? */
{ I2(e, n, "I>="); n = nb(e->dh, D1(e)->v.i >= D2(e)->v.i); P2(n, e); }

V w_ieq(E e) /* i i -- ? */
{ I2(e, n, "I="); n = nb(e->dh, D1(e)->v.i == D2(e)->v.i); P2(n, e); }

V w_fadd(E e) /* f -- f */
{ F2(e, n, "F+"); n = nf(e->dh, D1(e)->v.f + D2(e)->v.f); P2(n, e); }

V w_fsub(E e) /* f f -- f */
{ F2(e, n, "F-"); n = nf(e->dh, D1(e)->v.f - D2(e)->v.f); P2(n, e); }

V w_fdiv(E e) /* f f -- f */
{ F2(e, n, "F/"); n = nf(e->dh, D1(e)->v.f / D2(e)->v.f); P2(n, e); }

V w_fmul(E e) /* f f -- f */
{ F2(e, n, "F*"); n = nf(e->dh, D1(e)->v.f * D2(e)->v.f); P2(n, e); }

V w_fmod(E e) /* f f -- f */
{ F2(e, n, "F%"); n = nf(e->dh, fmod(D1(e)->v.f, D2(e)->v.f)); P2(n, e); }

V w_ftoi(E e) /* f -- i */
{ F1(e, n, ".I"); n = ni(e->dh, (I)D1(e)->v.f); P1(n, e); }

V w_fflr(E e) /* f -- f */
{ F1(e, n, "F_"); n = nf(e->dh, floor(D1(e)->v.f)); P1(n, e); }

V w_fcil(E e) /* f -- f */
{ F1(e, n, "F^"); n = nf(e->dh, ceil(D1(e)->v.f)); P1(n, e); }

V w_frnd(E e) /* f -- f */
{ F1(e, n, "F~"); n = nf(e->dh, round(D1(e)->v.f)); P1(n, e); }

V w_flt(E e) /* f f -- ? */
{ F2(e, n, "F<"); n = nb(e->dh, D1(e)->v.f < D2(e)->v.f); P2(n, e); }

V w_fgt(E e) /* f f -- ? */
{ F2(e, n, "F>"); n = nb(e->dh, D1(e)->v.f > D2(e)->v.f); P2(n, e); }

V w_fle(E e) /* f f -- ? */
{ F2(e, n, "F<="); n = nb(e->dh, D1(e)->v.f <= D2(e)->v.f); P2(n, e); }

V w_fge(E e) /* f f -- ? */
{ F2(e, n, "F>="); n = nb(e->dh, D1(e)->v.f >= D2(e)->v.f); P2(n, e); }

V w_feq(E e) /* f f -- ? */
{ F2(e, n, "F="); n = nb(e->dh, D1(e)->v.f == D2(e)->v.f); P2(n, e); }

V w_p(E e) /* a -- a */ { pn(D1(e)); printf("\n"); }

struct w id[] = {
  { W_F, "~",    { w_swap } },
  { W_F, "''",   { w_dup  } },
  { W_F, "_",    { w_pop  } },
  { W_F, ",",    { w_cat  } },
  { W_F, ",'",   { w_cons } },
  { W_F, "E",    { w_e    } },
  { W_F, "T",    { w_t    } },
  { W_F, "F",    { w_f    } },
  { W_F, "I?",   { w_ip   } },
  { W_F, "F?",   { w_fp   } },
  { W_F, "B?",   { w_bp   } },
  { W_F, "S?",   { w_sp   } },
  { W_F, "Q?",   { w_qp   } },
  { W_F, "?",    { w_b    } },
  { W_F, "p",    { w_p    } },
  { W_F, "i+",   { w_iadd } },
  { W_F, "i-",   { w_isub } },
  { W_F, "i/",   { w_idiv } },
  { W_F, "i*",   { w_imul } },
  { W_F, "i%",   { w_imod } },
  { W_F, "i.",   { w_itof } },
  { W_F, "i<",   { w_ilt  } },
  { W_F, "i>",   { w_igt  } },
  { W_F, "i<=",  { w_ige  } },
  { W_F, "i>=",  { w_ige  } },
  { W_F, "i=",   { w_ieq  } },
  { W_F, "f+",   { w_fadd } },
  { W_F, "f-",   { w_fsub } },
  { W_F, "f/",   { w_fdiv } },
  { W_F, "f*",   { w_fmul } },
  { W_F, "f%",   { w_fmod } },
  { W_F, ".i",   { w_ftoi } },
  { W_F, "f_",   { w_fflr } },
  { W_F, "f^",   { w_fcil } },
  { W_F, "f~",   { w_frnd } },
  { W_F, "f<",   { w_flt  } },
  { W_F, "f>",   { w_fgt  } },
  { W_F, "f<=",  { w_fle  } },
  { W_F, "f>=",  { w_fge  } },
  { W_F, "f=",   { w_feq  } },
};

W nd(H h)
{
  int i, l; W p, w, d;
  l = sizeof(id) / sizeof(id[0]); d = id; p = NULL;
  for (i = 0; i < l; i++) {
    w = nw(h, d[i].s);
    w->t = d[i].t; w->c.f = d[i].c.f;
    w->n = p; p = w;
  }
  R (w);
}

V ef(E e, FILE *f, C in)
{
  struct p _p; P p; T t; N l;
  p = &_p; l = NULL; ip(p, f);
read:
  gc(e);
  if (in) printf("> ");
  while (1)
  {
    switch ((t = rt(e->dh, p)).t) {
    case T_EF: fclose(f); R;
    case T_EL: ev(e); goto read;
    case T_CL: {
      W w;
      if (!ZP((w = rw(e->ch, p)))) {
        w->n = e->w; e->w = w;
      }
      break;
    }
    case T_SCL: case T_RP: break;
    default: l = app(ra(e->dh, p, t), &e->c, &l);
    }
  }
}

V ie(E e, H h, H c)
{
  e->d = NULL; e->c = NULL;
  e->dh = h; e->ch = c; e->w = nd(e->ch);
}

int
main(int argc, char *argv[])
{
  struct e _e; E e; H h, c;
  e = &_e; h = nh(1024*512); c = nh(1024*512);
  ie(e, h, c);
  if (argc == 2) {
    FILE *f;
    if (!ZP((f = fopen(argv[1], "r")))) ef(e, f, 0);
    else perror(argv[1]);
  }
  ef(e, stdin, 1);
  R (0);
}
