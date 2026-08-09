### Important
Type and data constructor names must always start with a capital letter; variables (including names of functions) must always start with a lowercase letter.

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

### Literate Haskell
- File end with .lhs (rather than .hs), the code must come after > and a space, other lines are all comments and documents.
- This encourage programmer to think before code, and produce better document with lhs2TeX tools.