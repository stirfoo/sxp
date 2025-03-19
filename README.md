# sxp
A bytecode interpreted lisp-1 with clojure-like syntax.

The only dependency is the Boehm garbage collector. I've only built and ran
sxp on Linux.

Have a look at reader.cpp to see the various forms sxp will evaluate. The directory sxpsrc
has a few files with lots of sxp expression. Most of which actually work. You'll see the heavy
clojure influence.

The VM is based on ideas from the JVM, and ideas found in the language Wren, which I believe
is based on ideas from Lua.

The try/catch/finally/throw emitted bytecode is very loosly based on the JVM.

Lazy sequences don't work at all. That was the last feature I tried to bolt onto sxp. Doing
so exposed some show-stopping flaws in the design.

sxp is, in fact, a bug-ridden toy. But it's a fun little toy.
