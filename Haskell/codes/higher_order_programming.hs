{-
    Higher Order Programming and Type Inference
-}

-- 1. Lambda functions or anonymous functions
greaterThan100 :: [Integer] -> [Integer]
greaterThan100 xs = filter (\x -> x > 100) xs
    -- Another way: operator section, ?y is \x -> x ? y
greaterThan100_2 :: [Integer] -> [Integer]
greaterThan100_2 xs = filter (>100) xs

-- 2. Function composition
myTest :: [Integer] -> Bool
myTest xs = even (length (greaterThan100 xs))

-- We can rewrite this as: (.) represents the composition
-- :t (.) returns (.) :: (b -> c) -> (a -> b) -> a -> c
-- function arrow -> is right-associative, a -> b -> c means a -> (b -> c)
-- function apply is left-associative, see intro.md curring
myTest' :: [Integer] -> Bool
myTest' = even . length . greaterThan100

-- uncurry (+) (2,3)

{-
    fold z f [1,2,3] = f 1 (f 2 (f 3 z))

    And the official fold function is foldr
-}
fold :: b -> (a -> b -> b) -> [a] -> b
fold z f []     = z
fold z f (x:xs) = f x (fold z f xs)

sum''     = fold 0 (+)
product'' = fold 1 (*)
length''  = fold 0 (\_ s -> 1 + s)

strFold = fold "" (\x acc -> show x ++ acc) [1,2,3] -- differnet type of a and b