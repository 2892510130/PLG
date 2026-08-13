{- 
    Without main we have to use ghci to run this.
    You can use :l intro.hs or :load intro.hs to load the hs file
-}

-- 1. Declarations and variables
x :: Int
x = 3 -- This can not change, = is not assigned to, but defination
    -- x = 4 -> multiple declarations error

-- 2. Types
    -- Thera are the same types like other languages, like Int, Double, Float, Char, Bool
    -- Int depends on the architecture of your machine
    -- We also have Arbitrary-precision integers
    -- We also have pairs
    -- String is just [Char]
arbiInt :: Integer
arbiInt = 123456789101112131415
p :: (Int, Char)
p = (3, 'c')
nums :: [Integer]
nums = [1, 2, 3, 4]
numsEqual = 1 : 2 : 3 : 4 : [] -- They are the same
numsNew = 3 : (1 : []) -- same as 3 : 1 : [], same as [3, 1]

-- 3. Arithmetic, Boolean logic
ex01 = mod 19 3
ex02 = x `div` x -- for int div
ex03 = 0.39 / 0.13 -- for float div
ex04 = (16 /= 3) -- not equal

-- 4. Basic functions
sumtorial :: Integer -> Integer
sumtorial 0 = 0
sumtorial n = n + sumtorial(n-1)

    -- This is called guards -> |, this can be as many as you want
hailstone :: Integer -> Integer
hailstone n
  | n `mod` 2 == 0 = n `div` 2
  | otherwise      = 3 * n + 1

sumPair :: (Int, Int) -> Int
sumPair (x, y) = x + y

hailstoneSeq :: Integer -> [Integer]
hailstoneSeq 1 = [1]
hailstoneSeq n = n : hailstoneSeq (hailstone n)

intListLength :: [Integer] -> Integer
intListLength []     = 0
intListLength (x:xs) = 1 + intListLength xs -- first element is x, remainder is xs

hailstoneLen :: Integer -> Integer
hailstoneLen n = intListLength (hailstoneSeq n) - 1