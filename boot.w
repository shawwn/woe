: . p _ ;                    \ a --

: x '' e ;                   \ (a) -- (a)
: y ('' ,') ~ , '' ,' e ;    \ (a) -- (a)

: ', () ,' ;                 \ a -- (a)

: _e ~ ', , e ;              \ a (b) -- a
: _e2 (_e) ,' _e ;           \ a b (c) -- a b
: _e3 (_e2) ,' _e ;          \ a b c (d) -- a b c
: _e4 (_e3) ,' _e ;          \ a b c d (e) -- a b c d

: _^ () ,' _e ;              \ a b -- b a
: _^2 () ,' ,' _e ;          \ a b c -- b c a
: _^3 () ,' ,' ,' _e ;       \ a b c d -- b c d a

: _, (() ,') _e ~ e ;        \ a b -- b a
: _,2 (() ,' ,') _e ~ e ;    \ a b c -- c a b
: _,3 (() ,' ,' ,') _e ~ e ; \ a b c d -- d a b c

: ''2 ('') _e '' _,2 ;       \ a b -- a b a b
: ''3 (''2) _e '' _,3 ;      \ a b c -- a b c a b c

: # ~ '' _,2 (e) _e ;        \ a (b) -- a
: #2 (''2) _e _e2 ;          \ a b (c) -- a b
: #3 (''3) _e _e3 ;          \ a b c (d) -- a b c

: t? ~ (e) (_) ? ;           \ ? (t) --
: f? ~ (_) (e) ? ;           \ ? (f) --
: or _^2 (_) (~ _) ? ;       \ ? t f -- t/f

: & f | ;                    \ ? ? -- ?
: | t ~ or ;                 \ ? ? -- ?
: x| ', '' (not) , ~ ? ;     \ ? ? -- ?
: ! f t | ;                  \ ? -- ?

: r (e) # (r) ,' t? ;        \ (p) --
: r1 '' _e2 ;                \ (p) (a) -- p (a)
: r? ~ r1 , (r) ,' t? ;      \ (p) (a) --
: r!? ((not) ,) _e r? ;      \ (p) (a) --
