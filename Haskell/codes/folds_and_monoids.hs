data Tree a = Empty
            | Node (Tree a) a (Tree a)
  deriving (Show, Eq)

-- myTree = Node (Node (Node Empty 7 Empty) 2 Empty ) 5  (Node Empty 3 Empty)

leaf :: a -> Tree a
leaf x = Node Empty x Empty

treeSize :: Tree a -> Integer
treeSize Empty        = 0
treeSize (Node l _ r) = 1 + treeSize l + treeSize r

treeSum :: Tree Integer -> Integer
treeSum Empty        = 0
treeSum (Node l x r) = x + treeSum l + treeSum r

treeDepth :: Tree a -> Integer
treeDepth Empty        = 0
treeDepth (Node l _ r) = 1 + max (treeDepth l) (treeDepth r)

flattenTree :: Tree a -> [a]
flattenTree Empty        = []
flattenTree (Node l x r) = (flattenTree l) ++ [x] ++ (flattenTree r)

-- Generalize these functions
treeFold :: b -> (b -> a -> b -> b) -> Tree a -> b
treeFold e _ Empty        = e
treeFold e f (Node l x r) = f (treeFold e f l) x (treeFold e f r)

treeSize' :: Tree a -> Integer
treeSize' = treeFold 0 (\l _ r -> 1 + l + r)

treeSum' :: Tree Integer -> Integer
treeSum' = treeFold 0 (\l x r -> l + x + r)

treeDepth' :: Tree a -> Integer
treeDepth' = treeFold 0 (\l _ r -> 1 + max l r)

flattenTree' :: Tree a -> [a]
flattenTree' = treeFold [] (\l x r -> l ++ [x] ++ r)

treeMax :: (Ord a, Bounded a) => Tree a -> a
treeMax = treeFold minBound (\l x r -> l `max` x `max` r)


-- Monoids 幺半群
-- newtype Sum a = Sum a deriving (Semigroup, Monoid) via (Sum a) -- but this requires some extensions

newtype Sum a = Sum a
    deriving (Eq, Ord, Num, Show)

instance Num a => Semigroup (Sum a) where
    Sum x <> Sum y = Sum (x + y)

instance Num a => Monoid (Sum a) where
    mempty = Sum 0

newtype Product a = Product a
    deriving Show

instance Num a => Semigroup (Product a) where
    Product x <> Product y = Product (x * y)

instance Num a => Monoid (Product a) where
    mempty = Product 1

getProduct :: Product a -> a
getProduct (Product a) = a