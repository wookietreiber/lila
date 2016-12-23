Overview
========

This document contains what I would like to achieve with this programming language.

**Note:** Later, once features are implemented, this document will be moved piece by piece to both a more technical specification and a more user-friendly documentation.

Goals
=====

In general, the language should encourage:

- to write **readable** code
- to write **expressive** code
- to write **concise** code
- to be **explicit** when necessary, i.e. it helps clarity, readability and understanding of the code
- to **avoid accidental code complexity** and **avoid code smells**

**Note:** For the first stages the focus should be on features that make code more readable without adding much magic, simply speaking: let the language and the compiler help you write *better* code wherever they can.

**Note:** (mainly to myself) These goals should all be high-level and contain no technical details, i.e. what is supposed to be achieved without immediately relying on technical details.

Language Features
=================

The following is a list of features I know about that will support the above-mentioned goals.

-   **no semicolons**

    Semicolons are *visual noise* that distracts from what is actually important, especially when reading code.

-   **allow the programmer to not name things**

        numbers.map(number => number * 2)

    is unnecessarily verbose when compared to

        numbers.map(_ * 2)

-   **avoid useless use of for loops**

        for (i in 0 until numbers.size)
          numbers(i) = numbers(i) * 2

    can be avoided with functional programming

        numbers.map(_ * 2)

    can even be re-written by the compiler to a low-level loop, so there are no performance implications

-   **strong, static typing**

    is an easy way for you to get help by the compiler to avoid bugs

-   ... coupled with **type inference**

    helps keep writing concise code and not mention the obvious, e.g.

        val foo: String = "foo"

    `foo` is obviously a `String` and in this case there should be no need to specify that explicitly

        val foo = "foo"

-   **no (explicit) `null`**

    As far as I know, there is no technical reason for why `null` exists. Do not repeat the billion dollar mistake. 

    Also, there are better ways of handling empty results, e.g. an `Option` type. Approaches like this should always be used for FFIs to languages where `null` may be used.

-   **do not separate declaration from definition**

    It removes the need for `null` as an initializer. To me, it seems like very old imperative style, where you needed to write e.g.

        int a;
        
        if (expr) {
          a = 1;
        } else {
          a = 2;
        }

    instead of being able to write

        val a = if (expr) 1 else 2

-   **named arguments**

    can make call site more readable

        def foo(flag1: Boolean, flag2: Boolean) = ???
        foo(flag1 = true, flag2 = false)

-   **default arguments**

    Allows you to write less when using sane defaults.

        def log(x: Double, base: Double = E): Double = ???
        log(42)

    Also, it removes the need for similar function signatures:

        def log(x: Double, base: Double): Double = ???
        def ln(x: Double) = log(x, base = E)

    DRY.

-   **user-defined operator definitions**

    help grow a language with library functions, e.g. `BigDecimal`

        class BigDecimal {
          def +(other: BigDecimal) = ???
        }

-   ... coupled with **infix notation**

    can improve readability

        this + that

    instead of

        this.+(that)

-   **pattern matching**

    avoid overly nested if constructs

    organically decompose objects

-   **implicit parameters**

    keep code concise

    helps with dependencies through the stack, e.g. if you are using concurrency you have a thread pool, this can be handled as an implicit argument

        implicit val pool = ThreadPool(nThreads)
        def parallel(foo: Foo)(implicit pool: ThreadPool) = ???
        parallel(foo)

Design Choices
==============

-   zero or one based indexing?

    is indexing even required (in higher-level abstractions)? differentiate library / application?

-   standard library collections: no inefficient operations, e.g. indexing linear sequences

- expressions over statements, everything returns a value
- immutable over mutable
- automatic memory management
- first class functions
- typeclasses
- small, yet extensible language, allow to do a lot as library
- compiled to binary
- scales from interpreted scripts (shebang) to large projects
- repl
- supports both low-level (machine-near) stuff as well as high-level abstractions
- building high-level abstractions serves code organization, compiler should be able to deconstruct into low-level, no/low overhead machine code
- intelligent string interpolation

Infrastructure
==============

## Program: lila

- interactive interpreter (repl)
- interpreter for scripts with shebang `#!/usr/bin/env lila`

## Program: lilac

- compiler

Syntax Sucks
============

Everyone likes to read code in a different way. Some prefer `fun` for function definitions, some `def`. Some prefer tabs, some prefer two or four spaces.

A reasonably intelligent code formatter/editor should be able to display the code the way you like, not the way the language designer thought was best or the code author wanted it to look like. This also keeps how the code looks consistent. There may very well be a default that is used for documentation, although a reasonably well designed web page can render the code in your personal preference.

Allowing multiple keywords is no problem to me. Keywords are rare and common enough to not be used as identifiers anyway.

## Semicolons

Semicolons are optional. The compiler can figure out when an expression ends.

## Spaces or Tabs

Spaces versus tabs is one of the most stupid discussions ever.

## Definitions

`val foo = ...` is the same as `let foo = ...`.

`var foo = ...` is the same as `let mut foo = ...`.

`def foo = ...` is the same as `fn foo = ...` is the same as `fun foo = ...`.

## Calling Methods

`Foo.run()` is the same as `Foo::run()`.

## Creating Things / Constructor

`new Foo()` is the same as `Foo::new()`.

## Camel vs Underscores

`Foo.doStuff()` is the same as `Foo::do_stuff()`.

## Namespacing

`import stuff`, `include stuff` and `use stuff` are all the same.

Also, `import stuff.util` is the same as `import stuff::util`.

## Comments

All of these are comments:

    # foo comment

    // foo comment

    /* foo comment */

## Using Methods / Functions

Both add the `add` method to `Thing`:

    def add(thing: Thing, number: Int)

    class Thing {
      def add(number: Int)
    }

They can both be used as `add(thing, number)`, `thing.add(number)` and, with infix notation, `thing add number`.

## Returning Multiple Values

Definition:

    def foo(): (Int,Int)

Compiler translation:

    def foo(ret1: Int, ret2: Int)

Using:

    val (a,b) = foo()