-- Use iterate :: (a -> a) -> a -> [a] and takeWhile :: (a -> Bool) -> [a] -> [a]
fun1 :: [Integer] -> Integer
fun1 [] = 1
fun1 (x:xs)
    | even x = (x - 2) * fun1 xs
    | otherwise = fun1 xs

fun2 :: Integer -> Integer
fun2 1 = 0
fun2 n 
    | even n = n + fun2 (n `div` 2)
    | otherwise = fun2 (3 * n + 1)

fun1' :: [Integer] -> Integer
fun1' = product . map (subtract 2) . filter even
-- my code
-- fun1' [] = 1
-- fun1' xs = product (map (\x -> if even x then (x-2) else 1) xs)

fun2' :: Integer -> Integer
fun2' = sum . filter even . takeWhile (/= 1) . iterate next
  where next n | even n    = n `div` 2
               | otherwise = 3 * n + 1

-- Exercise 2, using foldr
-- A binary tree is balanced if the height of its left and right
-- subtrees differ by no more than 1, and its left and right subtrees are
-- also balanced.
data Tree a = Leaf | Node Integer (Tree a) a (Tree a)
    deriving (Show, Eq)

-- foldTree :: [a] -> Tree a
-- foldTree [] = Leaf
-- foldTree

-- Exercise 3
xor :: [Bool] -> Bool
xor = even . length . filter (==True)

map' :: (a -> b) -> [a] -> [b]
map' f = foldr (\x acc -> f x : acc) []
--                        f x        z

-- myFoldl :: (a -> b -> a) -> a -> [b] -> a
-- myFoldl f base xs = foldr