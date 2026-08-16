{-
Using :k or :kind to check the type of types (we call it type constructors):
Maybe :: * -> * which is a function on types, [] :: * -> * too.

(->) :: * -> * -> *

Maybe Int :: *, Int :: *, Bool :: *, ...

class Functor f where               -- This is in prelude, Functor :: (* -> *) -> Constraint
    fmap :: (a -> b) -> f a -> f b

instance Functor Maybe where
    fmap _ Nothing  = Nothing
    fmap h (Just a) = Just (h a)

fmap = <$>
-}

mapToJust = fmap (+1) (Just 5)

-- 假设我们有一个可能失败的计算
safeDivide :: Double -> Double -> Maybe Double
safeDivide _ 0 = Nothing
safeDivide x y = Just (x / y)

-- 普通方式
result1 = case safeDivide 10 2 of
    Nothing -> Nothing
    Just x  -> case safeDivide x 5 of
        Nothing -> Nothing
        Just y  -> Just (y * 100)

-- 使用 Functor（虽然还是有点繁琐）
result2 = case safeDivide 10 2 of
    Nothing -> Nothing
    Just x  -> (*100) <$> safeDivide x 5

-- Define a functor
-- 定义一个二叉树
data Tree a = Leaf a | Node (Tree a) (Tree a) deriving (Show)

-- 实现 Functor
instance Functor Tree where
    fmap f (Leaf x) = Leaf (f x)
    fmap f (Node left right) = Node (fmap f left) (fmap f right)

-- 使用
tree = Node (Leaf 1) (Node (Leaf 2) (Leaf 3))
testTree = fmap (*2) tree