fib :: Integer -> Integer
fib 0 = 0
fib 1 = 1
fib x = fib (x - 1) + fib (x - 2)

fibs1 :: [Integer]
fibs1 = map fib [0..] -- slow version

fibs2 :: [Integer]
fibs2 = 0 : 1 : zipWith (+) fibs2 (drop 1 fibs2)

data Stream a = Cons a (Stream a)

streamToList :: Stream a -> [a]
streamToList (Cons x xs) = x : streamToList xs

instance Show a => Show (Stream a) where
    show s = "Stream (" ++ show (take 20 (streamToList s)) ++ ", ...)"

-- Generate a stream of infinitely many copies of the given element
streamRepeat :: a -> Stream a
streamRepeat x = Cons x (streamRepeat x)

-- Apply a function to every element of a Stream
streamMap :: (a -> b) -> Stream a -> Stream b
streamMap f (Cons x xs) = Cons (f x) (streamMap f xs)

-- Generate a Stream from a seed and an unfolding rule
streamFromSeed :: (a -> a) -> a -> Stream a
streamFromSeed f seed = Cons seed (streamFromSeed f (f seed))

-- Natural numbers: 0, 1, 2, 3, ...
nats :: Stream Integer
nats = streamFromSeed (+1) 0