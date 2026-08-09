-- 1. Enumeration types
    -- data: defining algebraic data type Thing
data Thing = Shoe 
           | Ship 
           | SealingWax 
           | Cabbage 
           | King
    deriving Show -- Just for `show King` can work -> will return a string "King"

thing :: Thing
thing = Shoe

    -- must start with lower case, like aThings. Things is not right. O' just as of, as of is reserved
listO'Things :: [Thing] -- this listO' is just conventions
listO'Things = [Shoe, SealingWax, King, Cabbage, King]

isShip :: Thing -> Bool
isShip Ship = True
isShip _    = False

-- 2. beyond enumeration, the more general is algebraic data types
data FailableDouble = Failure
                    | OK Double
    deriving Show

safeDiv :: Double -> Double -> FailableDouble
safeDiv _ 0 = Failure
safeDiv x y = OK (x / y)

failureToZero :: FailableDouble -> Double
failureToZero Failure = 0
failureToZero (OK d)  = d

data Person = Person String Int Thing -- This two Person are not the same, they are in different namespace
    deriving Show
brent = Person "Brent" 31 SealingWax
getAge :: Person -> Int
getAge (Person _ a _) = a

{-
More general algebraic data types:
data AlgDataType = Constr1 Type11 Type12
                 | Constr2 Type21
                 | Constr3 Type31 Type32 Type33
                 | Constr4
-}

-- 3. Pattern Matching, Case expressions and Recursive data types
baz :: Person -> String
baz p@(Person n _ _) = "The name field of (" ++ show p ++ ") is " ++ n -- p is just Person

failureToZero' :: FailableDouble -> Double -- functions are just simpler case expression
failureToZero' x = case x of
                     Failure -> 0
                     OK d    -> d

data IntList = Empty | Cons Int IntList
{-
    -- Built-in          Your custom
    []                   Empty
    (:)                  Cons
    1 : 2 : []           Cons 1 (Cons 2 Empty)

    So Cons Int IntList means:
    "A value built with Cons contains two things:
        an Int (the first element)
        another IntList (the remaining elements)"
-}
intListProd :: IntList -> Int
intListProd Empty      = 1
intListProd (Cons x l) = x * intListProd l