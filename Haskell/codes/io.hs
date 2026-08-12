main :: IO() = putStrLn "Please enter a number: " >> (readLn >>= (\n -> putStrLn (show (n+1))))

-- (>>) :: Monad m => m a -> m b -> m b, this will discard first element (but show will still work)
-- (>>=) :: Monad m => m a -> (a -> m b) -> m b, this will not

-- Record syntax
data D = C Int Double Char
data E = F { field1 :: Int, field2 :: Double, field3 :: Char } -- field1 :: E -> Int
-- e = F 5 7 'f', field1 e = 5, field1 (e {field1 = 10}) = 10, and this will not change e