Overview
========

This document contains what I want to achieve with this language. The goals are high, but I will
start with small pieces.

Pieces of this document will later be migrated to both a more technical specification and a more
user-friendly documentation.

Goals
-----

For the first stages focus on features that make code more readable without adding much magic,
simply speaking: let the language and the compiler help you write better code wherever they can.

In general the language should encourage:

- readable code
- expressive code
- concise code
- be productive / get things done quicker
- to be explicit where necessary
- use expressions over statements

Features
--------

The following originates from best practices and can be considered a wishlist of what I would
like to see combined in a single programming language.

-   no semicolons

    semicolons are visual noise, that distracts from what is actually important

-   strong, static typing

    is an easy way for you to get help by the compiler to avoid bugs

-   ... with type inference

    helps writing concise code, not mention the obvious, i.e.

        val foo = "foo"

    is obviously a `String` and in this case there is no need to redundantly specify that explicitly

-   no (explicit) `null`

    is there a technical reason for why `null` exists?

    there are better ways of handling empty results, e.g. an `Option` type

-   do not separate declaration from definition

    having both means duplicating code and not being DRY

-   default arguments

        def log(x: Double, base: Double = E): Double
        log(42)

    type less when using defaults

    avoid similar signatures, be DRY

        def foo(a: Thing, b: Bippy) = ???
        def foo(a: Thing) = foo(a, DefaultBippy)

-   user-defined operator definitions

    help grow a language with library functions, e.g. `BigDecimal`

        class BigDecimal {
          def +(other: BigDecimal) = ???
        }

-   ... with infix notation

    can improve readability

        this + that

    vs

        this.+(that)

-   implicit parameters

    keep code concise

    helps with dependencies through the stack, e.g. if you are using concurrency you have a thread
    pool, this can be handled as an implicit argument

        implicit pool = ThreadPool(nThreads)
        def parallel(foo: Foo)(implicit pool: ThreadPool) = ???
        parallel(foo)

-   named arguments

    can make call site more readable

        def foo(flag1: Boolean, flag2: Boolean) = ???
        foo(flag1 = true, flag2 = false)

-   pattern matching

    avoid overly nested if constructs

    organically decompose objects

- expressions over statements, everything returns a value
- immutable over mutable
- automatic memory management
- first class functions
- typeclasses
- small, yet extensible language, allow to do a lot as library
- compiled to binary
- scales from interpreted scripts (`#!/usr/bin/env lila`) to large projects
- repl
- supports both low-level (machine-near) stuff as well as high-level abstractions
- building high-level abstractions serves code organization, compiler should be able to deconstruct
  into low-level, no/low overhead machine code
- intelligent string interpolation

Infrastructure
--------------

### Program: lila

- interactive interpreter (repl)
- interpreter for scripts with shebang `#!/usr/bin/env lila`

### Program: lilac

- compiler
- linker

### Linking

- `-c` option to compile only, not link
- also link without a `-c` option
- native linker vs llvm linker
- `-l` option, `LDFLAGS`
- `-L` option, `LD_LIBRARY_PATH`

Syntax Sucks
------------

Everyone likes to read code in a different way. Some prefer `fun` for function definitions, some
`def`. Some prefer tabs, some prefer two or four spaces.

A reasonably intelligent code formatter/editor should be able to display the code the way you like,
not the way the language designer thought was best or the code author wanted it to look like. There
may very well be a default that is used for documentation, although even there a reasonably well
designed web page can render in your personal preference.

Allowing multiple keywords is no problem to me. Keywords are rare and common enough to not be used
as identifiers anyway.

### Semicolons

Semicolons are optional. The compiler can figure out when an expression ends.

### Spaces or Tabs

Spaces versus tabs is one of the most stupid discussions ever.

### Definitions

`val foo = ...` is the same as `let foo = ...`.

`var foo = ...` is the same as `let mut foo = ...`.

`def foo = ...` is the same as `fn foo = ...` is the same as `fun foo = ...`.

### Calling Methods

`Foo.run()` is the same as `Foo::run()`.

### Creating Things / Constructor

`new Foo()` is the same as `Foo::new()`.

### Camel vs Underscores

`Foo.doStuff()` is the same as `Foo::do_stuff()`.

### Namespacing

`import stuff`, `include stuff` and `use stuff` are all the same.

Also, `import stuff.util` is the same as `import stuff::util`.

### Comments

All of these are comments:

    # foo comment

    // foo comment

    /* foo comment */

### Using Methods / Functions

Both add the `add` method to `Thing`:

    def add(thing: Thing, number: Int)

    class Thing {
      def add(number: Int)
    }

They can both be used as `add(thing, number)`, `thing.add(number)` and with infix notation
`thing add number`.

## Returning Multiple Values

Definition:

    def foo(): (Int,Int)

Compiler translation:

    def foo(ret1: Int, ret2: Int)

Using:

    val (a,b) = foo()
