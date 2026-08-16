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

height :: Tree a -> Integer
height Leaf = -1  -- 空树高度为 -1，这样单节点树高度为 0
height (Node h _ _ _) = h

singleton :: a -> Tree a
singleton x = Node 0 Leaf x Leaf

insertTree :: a -> Tree a -> Tree a
insertTree x Leaf = singleton x
insertTree x (Node h left val right)
    | height left <= height right = 
        let newLeft = insertTree x left
        in Node (1 + max (height newLeft) (height right)) newLeft val right
    | otherwise =
        let newRight = insertTree x right
        in Node (1 + max (height left) (height newRight)) left val newRight

foldTree :: [a] -> Tree a
foldTree = foldr insertTree Leaf

-- Exercise 3
xor :: [Bool] -> Bool
xor = even . length . filter (==True)

map' :: (a -> b) -> [a] -> [b]
map' f = foldr (\x acc -> f x : acc) []

myFoldl :: (a -> b -> a) -> a -> [b] -> a
myFoldl f base xs = foldr step id xs base
    where
        step x acc = \current -> acc (f current x)

-- Exercise 4

sundaramSieveBool :: Int -> [Bool]
sundaramSieveBool n = 
    let indices = [i + j + 2 * i * j | i <- [1..n], j <- [i..n], i + j + 2 * i * j <= n]
    in markIndices indices (replicate (n + 1) True)
  where
    markIndices :: [Int] -> [Bool] -> [Bool]
    markIndices [] arr = arr
    markIndices (idx:idxes) arr = 
        let newArr = take idx arr ++ [False] ++ drop (idx + 1) arr
        in markIndices idxes newArr

sundaramSieve :: Int -> [Int]
sundaramSieve n
    | n < 2     = []
    | otherwise = [2] ++ [2 * i + 1 | i <- [1..n], bools !! i]
    where
        bools = sundaramSieveBool n

{-
def sieve_of_Sundaram(n):
    integers_list = [True] * (n + 1)
    for i in range(1, n):
        j = i
        while i + j + 2 * i * j <= (n + 1):
            integers_list[i + j + 2 * i * j] = False
            j += 1
        
    if n > 2:
        print(2, end=" ")
        
    for i in range(1, n + 1):
        if integers_list[i]:
            print(f"{2 * i + 1}", end=" ")
-}