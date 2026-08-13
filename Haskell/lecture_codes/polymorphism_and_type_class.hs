-- This can not work, because && can only work for bool, and a should be any type
-- f :: a -> a -> a
-- f x y = x && y

{-
    Num, Eq, Ord, and Show are type classes, and we say that (==), (<), and (+) are “type-class polymorphic”. 

    Type classes correspond to sets of types which have certain operations defined for them

    class Eq a where
        (==) :: a -> a -> Bool
        (/=) :: a -> a -> Bool
    
    (==) :: Eq a => a -> a -> Bool
    The Eq a that comes before the => is a type class constraint.
-}

data Foo = F Int | G Char

instance Eq Foo where
  (F i1) == (F i2) = i1 == i2
  (G c1) == (G c2) = c1 == c2
  _ == _ = False

--   foo1 /= foo2 = not (foo1 == foo2)

class Listable a where
    toList :: a -> [Int]

instance Listable Int where
    -- toList :: Int -> [Int]
    toList x = [x]

instance Listable Bool where
    toList True  = [1]
    toList False = [0]

data Tree a = Empty | Node a (Tree a) (Tree a)

instance Listable (Tree Int) where
    toList Empty        = []
    toList (Node x l r) = toList l ++ [x] ++ toList r