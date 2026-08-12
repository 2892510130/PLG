### Important
- https://www.engineering.upenn.edu/~cis1940/spring13/lectures.html
- Type and data constructor names must always start with a capital letter; variables (including names of functions) must always start with a lowercase letter.
- Constructors are functions in Haskell.
- :l or :load to load a .hs file in ghci, :t someThing to get the type
- () can decide the order of the function: f (g x) first apply g then f, we can use $ to reduce (), and . to composition which is different
- function arrow -> is right-associative, a -> b -> c means a -> (b -> c), see the Curry section below
- a `f` b == f a b

### Functional
- Functions are **first-class**, aka, functions are values.
- The meaning of haskell program is centered around **evaluating expressions** rather than **executing instructions**.

### Pure
Which means:
- No mutation
- Expressions never have “side effects” (like updating global variables or printing to the screen)
- Calling the same function with the same arguments results in the same output every time

Which will benifit us:
- Equational reasoning and refactoring: In Haskell one can always “replace equals by equals”
- Parallelism: Evaluating expressions in parallel is easy when they are guaranteed not to affect one another
- Fewer headaches

### Lazy
- It is easy to define a new control structure just by defining a function
- It is possible to define and work with infinite data structures
- It enables a more compositional programming style
- However, is that reasoning about time and space usage becomes much more complicated!

### Static Type
- Helps clarify thinking and express program structure, the first step in writing a Haskell program is usually to write down all the types.
- Serves as a form of documentation
- Turns run-time errors into compile-time errors

### Abstraction and Wholemeal programming
- Abstraction can reduce the repetation of the code
- Wholemeal programming: develop a solution space, rather than an individual solution.
- Projective programming: complemented wholemeal programming, first solve a more general problem, then extract the interesting bits and pieces by transforming the general program into more specialised ones.
- This style of coding in which we define a function without reference to its arguments—in some sense saying what a function is rather than what it does—is known as **point-free** style.

### Literate Haskell
- File end with .lhs (rather than .hs), the code must come after > and a space, other lines are all comments and documents.
- This encourage programmer to think before code, and produce better document with lhs2TeX tools.

### Currying
- Functions in Haskell only takes one arguments!!!
  - This idea of representing multi-argument functions as one-argument functions returning functions is known as currying, named after Haskell Curry.
- right arrow is right associated, f :: Int -> Int -> Int is f :: Int -> (Int -> Int), which means f takes one Int arguments and return a function which is Int -> Int. Let f = 2 * x + y
- function apply is left associated, f 3 4 is (f 3) 4, f 3 will return a function as we know Int -> Int, g = 2 * 3 + y, then g 4 will be Int which is 10.

### Laziness
- Expressions are only evaluated when pattern-matched
- only as far as necessary for the match to proceed, and no farther!
- But laziness bring purity, and maybe not good, see this
    - ```Haskell
        foldl (+) 0 [1,2,3]
        = foldl (+) (0+1) [2,3]
        = foldl (+) ((0+1)+2) [3]
        = foldl (+) (((0+1)+2)+3) []
        = (((0+1)+2)+3)
        = ((1+2)+3)
        = (3+3)
        = 6
        ```
    - Laziness will first get (((0+1)+2)+3), it is slow, and it will put them in stack and may create stack overflow
    - So foldl' give us different implement which will do this:
    - ```Haskell
        foldl' (+) 0 [1,2,3]
        = foldl' (+) (0+1) [2,3]
        = foldl' (+) 1 [2,3]
        = foldl' (+) (1+2) [3]
        = foldl' (+) 3 [3]
        = foldl' (+) (3+3) []
        = foldl' (+) 6 []
        = 6
        ```

### Monoids
- What is monoids:
  - A list of elements
  - One binary operator
  - One identity element
  - Associative law
- What is semigroup: (without the identity element)
  - A list of elements
  - One binary operator
  - Associative law