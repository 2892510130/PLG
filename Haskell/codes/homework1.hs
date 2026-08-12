cardNumber :: [Integer] = [1, 3, 8, 6]

toDigitRev :: Integer -> [Integer] -- I wrote two toDigitRev, first one use |, second not
toDigitRev x 
    | x <= 0 = []
    | otherwise = (x `mod` 10) : toDigitRev (x `div` 10)

toDigit :: Integer -> [Integer]
toDigit x | x <= 0 = []
-- toDigit x = toDigit (x `div` 10) ++ [(x `mod` 10)] -- ++ is O(n^2), not efficient
toDigit x = (reverse . toDigitRev) x

doubleEveryOtherRev :: [Integer] -> [Integer]
doubleEveryOtherRev []       = []
doubleEveryOtherRev [x]      = [x]
doubleEveryOtherRev (x:y:xs) = x : 2 * y : doubleEveryOtherRev xs -- [x, 2 * y] ++ doubleEveryOtherRev xs can work (I write like this)

doubleEveryOther :: [Integer] -> [Integer]
doubleEveryOther x = reverse (doubleEveryOtherRev (reverse x))

{-
Cleaner way to write this:
doubleEveryOther :: [Integer] -> [Integer]
doubleEveryOther = reverse . go . reverse
  where
    go []       = []
    go [x]      = [x]
    go (x:y:xs) = x : (2 * y) : go xs
-}

sumDigit :: [Integer] -> Integer
sumDigit []     = 0
sumDigit (x:xs) = sum (toDigit x) + sumDigit xs
-- sumDigits = sum . concatMap toDigitsRev -- cleaner way

validate :: Integer -> Bool
validate x 
    | x <= 0    = False
    | otherwise = (sumDigit (doubleEveryOther (toDigit x))) `mod` 10 == 0

{-
Hanoi tower problem
1. move n − 1 discs from a to c using b as temporary storage
2. move the top disc from a to b
3. move n − 1 discs from c to b using a as temporary storage.
-}

type Peg = String
type Move = (Peg, Peg)
hanoi :: Integer -> Peg -> Peg -> Peg -> [Move]
hanoi 0 _ _ _ = []
hanoi n source target aux = 
    hanoi (n-1) source aux target 
    ++ [(source, target)] 
    ++ hanoi (n-1) aux target source

{-
More discs?
For example, with three pegs it takes 215 − 1 = 32767 moves
to transfer 15 discs. With four pegs it can be done in 129 moves. 
-}