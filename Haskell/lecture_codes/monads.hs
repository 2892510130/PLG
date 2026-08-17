{-
在过去几周里，我们已经看到了 Applicative 类如何让我们能够以惯用的方式处理在某种“特殊上下文”中进行的计算——例如，
用 Maybe 考虑可能发生的失败，用 [] 表示多个可能的输出，用 ((->) e) 查询某种环境，或者像作业中那样用“组合子”方法来
构造解析器。

class Monad m where
    return :: a -> m a

    (>>=) :: m a -> (a -> m b) -> m b

    (>>)  :: m a -> m b -> m b
    m1 >> m2 = m1 >>= \_ -> m2

Monad is also Applicative, that pure = return.

The core is the bind operation:
    (>>=) :: m a -> (a -> m b) -> m b
It will take a `mobit` m a and then calculate the result a, and then based on this a we do next calculation.
And the default (>>) will ignore the first result.
-}

check :: Int -> Maybe Int
check n | n < 10    = Just n
        | otherwise = Nothing

halve :: Int -> Maybe Int
halve n | even n    = Just $ n `div` 2
        | otherwise = Nothing

ex01 = return 6 >>= check >>= halve   -- Just 3
ex02 = return 12 >>= check >>= halve  -- Nothing
ex03 = return 12 >>= halve >>= check  -- Just 6

addOneOrTwo :: Int -> [Int]
addOneOrTwo x = [x+1, x+2]

ex04 = [10,20,30] >>= addOneOrTwo
-- 结果：[11,12,21,22,31,32]

{-
sequence :: Monad m => [m a] -> m [a]
sequence [] = return []
sequence (ma:mas) =
  ma >>= \a ->
  sequence mas >>= \as ->
  return (a:as)
-}

replicateM :: Monad m => Int -> m a -> m [a]
replicateM n m = sequence (replicate n m)