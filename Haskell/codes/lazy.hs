{-
    Haskell has zero side effects (for example change the global variables), they are 
    constrained inside the IO monad.

    myRepeat will only expand 3 times, no more than it. See intro.md laziness.
-}

myRepeat :: a -> [a]
myRepeat x = x : myRepeat x

myTake :: Int -> [a] -> [a]
myTake n _      | n <= 0 =  []
myTake _ []              =  []
myTake n (x:xs)          =  x : myTake (n-1) xs

-- Short-circuiting operators like && and ||, this is a mimic
myComp :: Bool -> Bool -> Bool
myComp True x  = x
myComp False _ = False

-- We don't use if a lot in Haskell, we use guards and pattern match
if' :: Bool -> a -> a -> a
if' True  x _ = x
if' False _ y = y