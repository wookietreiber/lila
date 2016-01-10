# Language Features

The following originates from best practices and can be considered a wishlist of what I would
like to see combined in a single programming language.

- strong, static typing
- type inference
- expressions over statements, everything returns a value
- immutable over mutable
- infix notation
- user-defined operator definitions
- automatic memory management
- first class functions
- no `null`
- typeclasses
- small, yet extensible language, allow to do a lot as library
- compiled to binary
- scales from interpreted scripts (`#!/usr/bin/env interpreter`) to large projects
- repl
- supports both low-level (machine-near) stuff as well as high-level abstractions
- building high-level abstractions serves code organization, compiler should be able to deconstruct
  into low-level, no/low overhead machine code
- implicit parameters
- intelligent string interpolation

# Syntax Sucks

Everyone likes to read code in a different way. Some prefer `fun` for function definitions, some
`def`. Some prefer tabs, some prefer two or four spaces.

A reasonably intelligent code formatter/editor should be able to display the code the way you like,
not the way the language designer thought was best or the code author wanted it to look like. There
may very well be a default that is used for documentation, although even there a reasonably well
designed web page can render in your personal preference.

Allowing multiple keywords is no problem to me. Keywords are rare and common enough to not be used
as identifiers anyway.

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

They can both be used as `add(thing, number)`, `thing.add(number)` and with infix notation
`thing add number`.

# Returning Multiple Values

Definition:

    def foo(): (Int,Int)

Compiler translation:

    def foo(ret1: Int, ret2: Int)

Using:

    val (a,b) = foo()
