import Data.List

-- Not efficient at all!

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

-- DeepSeek Expert Answer
skipn' :: Int -> [a] -> [a]
skipn' n xs
  | n <= 0 = []
  | otherwise = case drop (n-1) xs of
      [] -> []
      (y:ys) -> y : skipn' n ys

skips' :: [a] -> [[a]]
skips' xs = [skipn' n xs | n <- [1..length xs]]