# CafeBabe Programming Language

The CafeBabe is an attempt to design modern language for implementation of backend applications.

## Motivation
There are a plenty of languages suitable for backend development, but none of them combines all properties 
I need:
 - Readable code suitable for long term development and maintenance
 - Support for FP (Functional Programming) and OOP (Object-Oriented Programming)
 - Simple regular syntax, expressive and consistent
 - Suitable for both, low level and high level code without need to use other language
 - Extensible with built-in tools
 - Fast yet safe memory management
 - Resource consumption dictated by application, not platform/VM application is running on
 - Convenient for developer (compilation speed, etc.)

## Inspirations
 
### Java
Over the years Java proved that it is very good for enterprise development. CafeBabe 
inherited very little from Java syntax, but enterprise focus is definitely inherited from Java.

### C++
Variadic templates and fold expansion are inspired by C++ (C++11 and C++17).

### Rust
The approach to memory and resource management is ispired by Rust. I believe that Rust approach to resource
management is an elegant solution for one of the most complex problems in programming.

### Haskell
The Haskell is a main source of inspiration for FP features. Main one - understanding that crating types must
be as simple and convenient as possible.

### Elm
Elegant and clean syntax for manipulation of immutable classes borrowed almost unchanged from Elm.

## Features

The CafeBabe is a statically typed, hybrid FP/OOP language compiled into native code.
Below listed some key features:
 - 
 - 

### Code Organization
The application consists of one or more source files. Source files are organized in the source tree 
according to package structure.

Each source file (compilation unit) has fixed structure:

 - Imports
 - Constant declarations
 - Type declarations
 - Extensions 
 - Functions
 - Unsafe code

Any of the above parts might be missing, but structure of the compilation unit follows this pattern.
Main reason to introduction of such a structure - make getting into new code base as simple as possible.

### 























































## Syntax Examples

### Plain Functions
```java
//all types explicit
String makeString(int a, int b) = "" + a + " " + b; 

// return type inferred
fn makeString(int a, int b) = "" + a + " " + b; 

// all types explicit, block form
String makeString(int a, int b) { 
    return "" + a + " " + b;
}

//return type inferred, block form
fn makeString(int a, int b) { 
    return "" + a + " " + b;
}
```

### Generic Functions
```java
<R> R add(R a, R b) = a + b; //fully type annotated
fn add(a, b) = a + b; //all types inferred
<R:Number> R add(R a, R b) = a + b; //type bounds, all types explicit 
```

### Lambdas
```java
int (int a, int b) = a + b  // explicit argument types and return type
(a, b) = a + b              // all types inferred
int (a, b) = a + b          // return type explicit, rest inferred
a, b = a + b                // without parentheses, accepted in limited context like where it passed as a parameter 
```

### Tuples
```java
() // empty tuple, unit
(a) // tuple with one element
(a, b) // tuple with two elements
```

### Type declarations
```java
type Mapper<T,R> = T -> R;  // function type
type Mapper = a -> b;       // function type, type parameters inferred
type FlatMapper<T,R> = T -> Result<R>;  // another function type
type FlatMapper = a -> Result<b>;       // another function type, parameters inferred

type Result<T> = Either<T, Failure>;    //Result is an alias to Either
type Result<T> = Success<T> | Failure;  //Result is one of two types - requires pattern matching to distinguish

--------------------------------------------------
//Version 1
type Mapper<T,R> = T -> R;  // function type
type FlatMapper<T,R> = T -> Result<R>;  // another function type

//Result is one of two types, both types implement same API
type Result<T> = Success<T> | Failure { 
    <R> Result<R> map(Mapper<T,R> mapper);
    <R> Result<R> flatMap(FlatMapper<T,R> mapper);
}

type Success<T> = class (T value) { //automatically generated constructor
    // explicit types
    <R> Success<R> map(Mapper<T,R> mapper) = Success(mapper(value));
    
    // alternative form
    // implementation of already defined method 
    impl map(mapper) = Success(mapper(value)); 
    
    // explicit types
    <R> Result<R> flatMap(FlatMapper<T,R> mapper) = mapper(value);

    // alternative form
    // implementation of already defined method 
    impl flatMap(mapper) = mapper(value);
}

type Failure = class {
    impl map(_) = this;
    impl flatMap(_) = this;
}
--------------------------------------------------
//Version 2
type Mapper<T,R> = T -> R;  // function type
type FlatMapper<T,R> = T -> Result<R>;  // another function type

//Result is one of two types, both types implement same API
//Referenced types implementation must be provided inside
type Result<T> = Success<T> | Failure { 
    <R> Result<R> map(Mapper<T,R> mapper);
    <R> Result<R> flatMap(FlatMapper<T,R> mapper);

    class Success<T> (T value) {
        impl map(mapper) = Success(mapper(value));
        impl flatMap(mapper) = mapper(value);
    }

    class Failure {
        impl map(_) = this;
        impl flatMap(_) = this;
    }
};
//Enum
type Color = Red | Green | Blue;
```

### Sequences
```java
fn loopDemo() {
    for(int i : [1..100, 2]) { // from 1 to 100 by 2
        Console.out("Sequence ${i}");
    }
}
```

### Variadic Templates
4 types of fold expansion:

 - ( E op ... ) unary right fold -> (E1 op (... op (EN-1 op EN)))
 - ( ... op E ) unary left fold -> (((E1 op E2) op ...) op EN)
 - ( E op ... op I ) binary right fold -> (E1 op (... op (EN−1 op (EN op I))))
 - ( I op ... op E ) binary left fold -> ((((I op E1) op E2) op ...) op EN)
 
```java
// Function with variable number of parameters (fixed at compile time)
type FN<R, T, ...> = T ->... R;

// Tuple with variable number of components
type Tuple<T, ...> = class (T value, ...) {
    R map(FN<R, T, ...> mapper) = mapper(value, ...);
    R map(FN<R, T, ...> mapper) = (value, ...);
}

Mapper<T, ...> allOf(Result<T> value, ...) {
    // Binary right fold expansion with nestes unary right fold expansion
    // value1.flatMap(vv1 ->  
    //  value2.flatMap(vv2 ->
    //   ...
    //    valueN.flatMap(vvN ->
    //     ok(tuple(vv1, vv2, ..., vvN))..);

    return () -> { return value.flatMap(vv ->... ok(tuple(vv, ...))); };
}
```

### Imports
```java
use lang.Math;
use org.github.GitHubApi;

```
### Interfaces and interface inheritance

```java
type toString = api {
   String toString();
}

type Equals = api {
    bool equals(Equals other);
}

type HashCode = api {
    i64 hashCode();
}

type Object = ToString, Equals, HashCode {
    <T> T clone();
}

type Result<T> = Success<T> | Failure : Object, Serializable {
}
```

### Enumerations/Sealed classes

```java
type FN<T,R> = T->R;

type Option<T> = Just<T> | Nothing<T> {
    // Common API

    // Full type annotations
    <R> Option<R> map(FN<T, R> mapper);
    <R> Option<R> flatMap(FN<T, Option<R>> mapper);
    
    // Shorter form
    Option<R> map(FN<T, R> mapper);
    Option<R> flatMap(FN<T, Option<R>> mapper);

    // Full type annotation
    static <T> Option<T> option(T value) = Just(value);
    // Shorter form
    static Option option(T value) = Just(value);
    static Option empty() = NONE;

    //Polymorphic constant
    const Option<_> NONE = Nothing();
    
    class Just<T> (T value) {
        impl map(mapper) = Just(mapper(value));
        impl flatMap(mapper) = mapper(value);
    }
    
    class Nothing<_> {
        impl map(mapper) = this;
        impl flatMap(mapper) = this;
    }
}

```
alternative version:
```java
use lang.Math;
use org.github.GitHubApi;

type FN<T,R> = T->R;

type OptionApi<T> = {
    <R> OptionApi<R> map(FN<T, R> mapper);
    <R> OptionApi<R> flatMap(FN<T, OptionApi<R>> mapper);
}

type Option<T> = Just<T> | Nothing<T> : OptionApi<T> {
    class Just<T> (T value) {
        impl map(mapper) = Just(mapper(value));
        impl flatMap(mapper) = mapper(value);
    }
    
    class Nothing<T> {
        impl map(mapper) = this;
        impl flatMap(mapper) = this;
    }
}

```






















```java



use lang.Math;
use org.github.GitHubApi;

type FN<T,R> = T->R;

// same, shorter form
type FN = T->R;

// two parameters
type FN = T1 -> T2 -> R;

type Option<T> = Just<T> | Nothing<T> : {
    Option<R> map(T -> R mapper);
    Option<R> flatMap(T -> Option<R> mapper);

    class Just {
        impl map(mapper) = Just(mapper(value));
        impl flatMap(mapper) = mapper(value);
    }

    class Nothing {
        impl map(mapper) = this;
        impl flatMap(mapper) = this;
    }
}

type Option<T> = Just<T> | Nothing<T> : {
    // Full type annotations
    <R> Option<R> map(FN<T, R> mapper);
    <R> Option<R> flatMap(FN<T, Option<R>> mapper);
    
    // Shorter form
    Option<R> map(FN<T, R> mapper);
    Option<R> flatMap(FN<T, Option<R>> mapper);

    // Full type annotation
    static <T> Option<T> option(T value) = Just(value);
    // Shorter form
    static Option option(T value) = Just(value);
    static Option empty() = NONE;

    //Polymorphic constant
    const Option<_> NONE = Nothing();
    
    class Just<T> (T value) {
        impl map(mapper) = Just(mapper(value));
        impl flatMap(mapper) = mapper(value);
    }
    
    class Nothing<_> {
        impl map(mapper) = this;
        impl flatMap(mapper) = this;
    }
}
```


# Inspirations

Pipe operator:
```java
let isValid =
  person
    |> parseData
    |> getAge
    |> validateAge;

equal to

let isValid = validateAge(getAge(parseData(person)));

```

```java
var isValid = 
    parseData(person)
    .flatMap(::getAge)
    .flatMap(::validateAge)
    .asBool();
```

Record 'copy with change' syntax
```java
> john = { first = "John", last = "Hobson", age = 81 }
{ age = 81, first = "John", last = "Hobson" }

> { john | last = "Adams" }
{ age = 81, first = "John", last = "Adams" }

> { john | age = 22 }
{ age = 22, first = "John", last = "Hobson" }

>  
```
