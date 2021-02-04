# CafeBabe Programming Language

The CafeBabe is an attempt to design modern language for implementation of backend applications.

## Motivation
There are a plenty of languages suitable for backend development, but none of them combines all properties 
I need:
 - Readable code suitable for long term development and maintenance
 - Support for FP (Functional Programming) and OOP (Object-Oriented Programming)
 - Simple regular syntax, expressive and consistent
 - Suitable for low level and for high level code without need to use other language
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
The approach to memory and resource management is inspired by Rust.

### Haskell
The Haskell helped to realize how important convenient type construction.

### Elm
Elegant and clean syntax for manipulation of immutable classes borrowed almost unchanged from Elm.

## Features

The CafeBabe is a statically typed, hybrid FP/OOP language compiled into native code.
Below listed some key features:
 - Compact, regular C-style syntax. Whole grammar contains less than 200 rules.
 - Functions are first class citizens
 - Immutable data structures by default
 - Type inference
 - Algebraic data types
 - Pattern matching and smart type cast 
 - No _null_
 - Unlimited interface (aka 'api') inheritance
 - No class inheritance (each class considered 'final' in Java terms)
 - Default method in interfaces
 - Extensions (similar to extension methods)
 - Ownership-based memory and resource management (aka Non-Lexical Lifetimes borrow checker in Rust)
 - Unsafe mode with access to manual memory management and address arithmetic
 - Class destructors 
 - Built-in primitive types completely interchangeable with wrapped types ('classes')
 - Templates (aka type classes) with variadic templates with fold expressions
 - Compile-time annotations with built-in annotation processing
 - Sequence comprehension
 - String interpolation
 - Named arguments
 - Duck typing
 - Data structure isomorphism
 - Structural type test

## Code Organization
The application consists of one or more source files aka compilation units. 
Source files are organized in the source tree according to package structure.

Each source file (compilation unit) has fixed structure:
 - Import declarations
 - Constant declarations
 - Type declarations
 - Extensions 
 - Functions
 - Unsafe code
 - Tests

Strict source code structure and organization of source files avoid unnecessary discussions and
reduce area for inventions of useless style guides.

## Code Examples
This section serves as a quick introduction to syntax, but this is not a complete guide nor tutorial.

### Import declarations

Import all from specified package (similar to star import in Java):
```
use lang.util._;
```
Import specified types and make some of them visible under different name in current compilation unit:
```
use lang.util.collection { List, Set, Map, LinkedList as LList};
```

### Constant
Local constant with inferred type:
```
const val PREFIX = "LOCAL_";
```
Constant visible outside of the compilation unit:
```
pub const List<String> ENV_NAMES = ["main", "staging", "test"];
```
Note that example above uses List constructor which accepts arrays.

### Types
Type construction consists of quite different ways to create types. All of them have 
similar syntax.

#### Type Aliases
Simple type alias (also may serve as partial specialization):
```
type Result = Option<Int>;
```
Partial specialization:
```
type Result<T> = Either<Failure, T>;
```
#### API (aka interface, aka protocol) declaration 
Simple publicly visible API with one method:
```
pub type Cloneable = api {
    <T:Cloneable> T clone();
} 
```
Note that declaration of `T` contains type constraints.

Assembling complex API from other API's with extra methods:
```
type toString = api {
   String toString();
}

type Equals = api {
    Bool equals(Equals other);
}

type HashCode = api {
    i64 hashCode();
}

type Object = ToString, Equals, HashCode {
    <T:Object> T clone();
}
```
#### Class declaration
Simple class with all boilerplate generated:
```
type Element<T> = class (T value) {}
```
This class gets: 
 - private all-args constructor
 - private destructor which destroys 'value' depending on the type
 - static factory method named after class name with leading character converted to lower case
 - Accessor (getter) for value

Note that class declaration shown above is a template which is instantiated for each different 
type for which this class is created. 

Example of more complex class which implements few api's:
```
type toString = api {
   String toString();
}

pub type Cloneable = api {
    <T:Cloneable> T clone();
} 

type Element<T> = class (T value) : Cloneable, ToString {
    // Two methods need to be implemented, one from Cloneable and one from ToString
    impl toString() = "Element ${value}";
    impl clone() = element(value);         
}
```

#### Enumerations
This type construction has many names - enums, case classes, sealed classes. 
Let's use __enumeration__ for clarity.

Simple enumeration:
```
type Bool = True | False;
```
Implementation of simple Option monad:
```
pub type Option<T> = Just<T> | Nothing<T> {
    const Nothing NOTHING = nothing();

    // Common API for both case classes
    Option<R> map(T -> R mapper);
    Option<R> flatMap(T -> Option<R> mapper);
    
    static Option<T> option(T value) = just(value);
    static Option<_> empty() = NOTHING;

    class Just<T>(T value) {
        impl map(mapper) = just(mapper(value));
        impl flatMap(mapper) = mapper(value);
    }

    class Nothing<_> {
        impl map(mapper) = this;
        impl flatMap(mapper) = this;
    }
}
```

#### Other type declarations
##### Function type
```
type Function<T, R> = T -> R;
```
##### Annotation type
```
type Derive = type annotation(T type) {
    for (var method : @type.methods) {
        @this.insertMethod(method.signatureFor(@this.type), "...");
    }
}
```
### Extensions
The extensions allow to add extension methods to class. It might be single
method or implementation of interface(s):

```
pub extend Bool : toString {
    impl toString() = match (this) {
        Nothing -> "Nothing";
        Just -> "${this.value()}";
    };
}
```

### Functions
#### Plain Functions
```
//all types explicit
String makeString(int a, int b) = F"${a} ${b}"; 

// return type inferred
fn makeString(int a, int b) = F"${a} ${b}"; 

// all types explicit, block form
String makeString(int a, int b) { 
    return F"${a} ${b}";
}

//return type inferred, block form
fn makeString(int a, int b) { 
    return F"${a} ${b}";
}
```
#### Generic Functions
```
<R> R add(R a, R b) = a + b; //fully type annotated
fn add(a, b) = a + b; //all types inferred
<R:Number> R add(R a, R b) = a + b; //type bounds, all types explicit 
```

### Unsafe code
Unsafe code is placed in dedicated `unsafe` block:

```
unsafe {
    fn unsafeOps() {
    }
}
```
The code inside unsafe block manages memory manually and enables address arithmetic for references.


## Variadic templates
Variadic templates is a feature which enables creation of generic types with variable number of 
type arguments. Such a templates are types which use fold expressions to describe code which should be instantiated
at compile time for given number of type parameters.

Variadic templates are using fold expansions of following 4 types:
- ( E op ... ) unary right fold -> (E1 op (... op (EN-1 op EN)))
- ( ... op E ) unary left fold -> (((E1 op E2) op ...) op EN)
- ( E op ... op I ) binary right fold -> (E1 op (... op (EN−1 op (EN op I))))
- ( I op ... op E ) binary left fold -> ((((I op E1) op E2) op ...) op EN)

Variadic templates for types example:
```
type FN<R, T, ...> = T ->... R;

// Tuple with variable number of components
type Tuple<T, ...> = class (T value, ...) {
    R map(FN<R, T, ...> mapper) = mapper(value, ...);
}
```
Variadic templates for functions example:
```
Mapper<T, ...> allOf(Result<T> value, ...) {
    // Binary right fold expansion with nested unary right fold expansion
    // value1.flatMap(vv1 ->  
    //  value2.flatMap(vv2 ->
    //   ...
    //    valueN.flatMap(vvN ->
    //     ok(tuple(vv1, vv2, ..., vvN))..);

    return () -> { return value.flatMap(vv ->... ok(tuple(vv, ...))); };
}
```

## Ranges, Sequences and iterables
Simple ranges:
```
const val q = 1..100;
```
Simple ranges as loop definition:
```
fn loopDemo() {
    for(int i : 1..100) {
        Console.out(F"Sequence ${i}");
    }
}
```
Sequence comprehensions:
```
const val seq1 = [x:1..100 -> x];   // => 1, 2, 3, ..., 100
const val seq2 = [x:1..10 | y:1..10 -> (x, y)]; // => (1, 1), (2, 2), (3, 3) ... (10, 10)
const val seq3 = [x:1..10, y:1..10 -> x != y | (x, y)]; // => (1, 2), (1, 3) ... (2, 1), (2, 3) ... (9, 10), (10, 1) ... (10, 9)
const val seq4 = [x:1.., y:1..10 -> (x, y)]; // infinite Sequence! => (1, 1), (1, 2) ... (1, 10), ... (2, 1), (2, 2) ... (2, 10) ...
```
Overall syntax for comprehension consists of three main parts:
 - ranges for parameters
 - oprional filtering condition
 - output expression

 Two variants of comprehension are supported - regular and parallel. In regular comprehension all variables are changing independently, so
the unfiltered output contains cartesian product of combinations of all variables. In parallel comprehension all variables are changing
simultaneously, so unfiltered output contains "zipped" set of values. In the example above `seq3` and `seq4` are regular comprehensions 
while `seq2` is a parallel comprehension. 

### Using Tuples and Immutable Classes
#### Immutable Classes
One of the most frequent tasks while working with immutable data - create modified copy of the class.
This case has special support in syntax:
```
type Person = class (String first, String last, Int age);

fn main() {
    var john = Person { first = "John", last = "Hobson", age = 81 };

    john = { john | last = "Adams", age = 22 };
}
```
#### Tuples

Tuples can be used to declare types:
```
type MyTuple = (String, Int, Bool);
type Unit = ();
```

Simple tuples can be created as follows:
```
val u = ();             // empty tuple, unit
val one = (1);          // tuple with one element
val two = (2, 4);       // tuple with two elements
val three = (one, two); // tuple with two tuples inside: ((1), (2, 4))
```
Tuples are immutable but modified copies of tuples can be created using 
syntax similar to classes:

```
val four = (three | (5), _);
```
Another approach is to use destructuring:
```
val (head, tail) = three;
val four = ((5), tail);
```

## Duck Typing


```
```

## Data Structure Isomorphism and Structural Type Test

### Data Structure Isomorphism
This feature enables handling of classes and tuples with identical internal structure as the same type. 
For example, if there is a function which accepts the class as a parameter, one can pass tuple insteat of class
given tuple contains values of same type and same order as the required class:
```
type Person = class (String first, String last, Int age);

fn printPerson(Person person) {
    println(F"${person.first} ${person.last}, age ${person.age}");
}

fn main() {
    var john = Person { first = "John", last = "Hobson", age = 81 };

    printPerson(john);
    
    var otherPerson = ("Peter", "Smith", 54);
    
    printPerson((Person) otherPerson); //allowed, structure is identical
}

```
Same is possible with classes with identical structure. In either case, when "foreign" object is passed, it need to be explicitly casted
to required type. This way any such structurally compatible conversions remain visible in code. 
Note that this is might be not a zero cost abstraction - compiler needs to insert thunk which provides required API.
There is no extra code if class is passed instead of tuple as all tuples have same API.

The function call parameters can be considered a tuple as well (both even have same notation!). This does 
mean, that it is allowed to pass tuple or class instead of function parameters given tuple or class contains 
values of same type and same order as the function parameters:

```
fn printPerson(String first, String last, Int age) {
    println(F"${first} ${last}, age ${age}");
}

fn main() {
    var john = Person { first = "John", last = "Hobson", age = 81 };

    printPerson(john);
    
    var otherPerson = ("Peter", "Smith", 54);
    
    printPerson(otherPerson); //allowed, structure is identical
}
```
Such a invocations generate no extra code comparing to passing parameters manually from specified structure or tuple.

### Structural Type Test
It's not just possible to use structurally identical classes and tuples interchangeably. It also possible to check variable if it 
holds class or tuple with specified structure. This is possible in the same cases when it's possible to check type, i.e. `isA` check
and pattern matching:
```
fn main() {
    val a = someFunction();

    if (a isA <A>) {
        println("a has structure <A>");
    }

    if (a isA <A, B>) {
        println("a has structure <A, B>");
    }

    val id = match(a) {
        A -> 1;         // a has type A
        B -> 2;         // a has type B
        <A> -> 3;       // a has structure <A>
        <A, B> -> 4;    // a has structure <A,B>
    }
}
```

