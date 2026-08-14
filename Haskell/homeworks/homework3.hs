import Data.List

-- Task 1 : Not efficient at all!

testDataOne = "ABCD"
testDataTwo = [1, 2, 3, 4, 5]

skipn :: Int -> [a] -> [a]
skipn _ [] = []
skipn n x  
    | n <= 0 = [] 
    | otherwise = (taken x) ++ (skipn n (drop n x))
    where 
        taken x = case (x !? (n-1)) of
            Nothing -> []
            Just num -> [num]

skips :: [a] -> [[a]]
skips [] = []
skips x  = map (flip skipn x) [1..(length x)]
-- Or: skips x  = map (\n -> skipn n x) [1..(length x)], more readable

-- Task 1 : DeepSeek Expert Answer
skipn' :: Int -> [a] -> [a]
skipn' n xs
  | n <= 0 = []
  | otherwise = case drop (n-1) xs of
      [] -> []
      (y:ys) -> y : skipn' n ys

skips' :: [a] -> [[a]]
skips' xs = [skipn' n xs | n <- [1..length xs]]

-- Task 2, first is my code
localMaxima :: [Integer] -> [Integer]
localMaxima xs | length xs < 3 = []
localMaxima (x:y:z:xs) 
    | ((x < y) && (z < y)) = y : localMaxima (y:z:xs)
    | otherwise            = localMaxima (y:z:xs)

localMaxima'' :: [Integer] -> [Integer]
localMaxima'' (x:y:z:xs)
    | x < y && y > z = y : localMaxima'' (y:z:xs)
    | otherwise      = localMaxima'' (y:z:xs)
localMaxima'' _ = []

localMaxima' :: [Integer] -> [Integer]
localMaxima' xs = [y | (x, y, z) <- zip3 xs (drop 1 xs) (drop 2 xs), x < y, y > z]

-- Task3, my code
countDigits :: [Integer] -> [Integer]
countDigits xs = map (\x -> fromIntegral (length (filter (==x) xs))) [0..9]

generateOneLine :: Integer -> [Integer] -> String
generateOneLine _ [] = []
generateOneLine lineNumber xs = map (\x -> if (x + lineNumber) < getMaxDigits xs then ' ' else '*') (countDigits xs)
    where getMaxDigits xs = maximum (countDigits xs)

generateLines :: [Integer] -> [String]
generateLines [] = []
generateLines xs = map (\x -> generateOneLine x xs) [0..(getMaxDigits xs - 1)]
    where getMaxDigits xs = maximum (countDigits xs)

histogram :: [Integer] -> String
histogram xs = unlines (generateLines xs ++ ["==========\n0123456789"])

-- Task3, the DeepSeek Expert, !!!!!!!!!!! This way counts and maxCount will only computed once
histogram' :: [Integer] -> String
histogram' xs = unlines (rows ++ ["==========", "0123456789"])
  where
    counts   = [ length (filter (== d) xs) | d <- [0..9] ]
    maxCount = maximum counts
    rows     = [ [ if c >= level then '*' else ' ' | c <- counts ]
               | level <- reverse [1 .. maxCount] ]