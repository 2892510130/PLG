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
-}