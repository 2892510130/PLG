data IntList = Empty | Cons Int IntList
    deriving Show

absAll :: IntList -> IntList
absAll Empty = Empty
absAll (Cons x xs) = Cons (abs x) (absAll xs)

addOne x = x + 1
square x = x * x

-- 2. Polymorphic data types
data List t = E | C t (List t)
    deriving Show

lst1 :: List Int
lst1 = C 3 (C (-5) (C (-2) E))

filterList _ E = E
filterList p (C x xs)
  | p x       = C x (filterList p xs)
  | otherwise = filterList p xs

mapList :: (a -> b) -> List a -> List b
mapList _ E        = E
mapList f (C x xs) = C (f x) (mapList f xs)

-- 3. Partial Functions: there are certain inputs for which head will crash
    -- Total functions: functions which are well-defined on all possible inputs
    -- head is a partial function, tail too, so we use this to make it a total function
doStuff1 :: [Int] -> Int
doStuff1 []  = 0
doStuff1 [_] = 0
doStuff1 xs  = head xs + (head (tail xs)) 

doStuff2 :: [Int] -> Int
doStuff2 []        = 0
doStuff2 [_]       = 0
doStuff2 (x1:x2:_) = x1 + x2

    -- Or we can change the output to make it total
safeHead :: [a] -> Maybe a
safeHead []    = Nothing
safeHead (x:_) = Just x

    -- Or we can make sure we will never get to the condition that will crash
    -- We must make it clear we have this guarantee in functions and the compiler will enforce it
data NonEmptyList a = NEL a [a]

nelToList :: NonEmptyList a -> [a]
nelToList (NEL x xs) = x:xs

listToNel :: [a] -> Maybe (NonEmptyList a)
listToNel []     = Nothing
listToNel (x:xs) = Just $ NEL x xs -- $ is for reduce the (), first do NEL x xs then Just

headNEL :: NonEmptyList a -> a
headNEL (NEL a _) = a

tailNEL :: NonEmptyList a -> [a]
tailNEL (NEL _ as) = as