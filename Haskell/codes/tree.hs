data IntTree = Leaf | Node IntTree Int IntTree
    deriving (Show, Eq)

mytree = Node (Node (Node Leaf 1 Leaf) 5 (Node Leaf 10 Leaf)) 18 (Node Leaf 25 Leaf)

generateTree :: Int -> IntTree
generateTree n | n <= 0 = Leaf
-- generateTree 1 = Node Leaf 1 Leaf
generateTree n = Node (generateTree (n `div` 2)) n (generateTree (n - n `div` 2 - 1))

preOrder :: IntTree -> [Int]
preOrder Leaf = []
preOrder (Node l n r) = [n] ++ inOrder l ++ inOrder r

inOrder :: IntTree -> [Int]
inOrder Leaf = []
inOrder (Node l n r) = preOrder l ++ [n] ++ preOrder r

postOrder :: IntTree -> [Int]
postOrder Leaf = []
postOrder (Node l n r) = postOrder r ++ [n] ++ postOrder l

insertTree :: Int -> IntTree -> IntTree
insertTree n Leaf = Node Leaf n Leaf
insertTree n (Node l x r)
    | n <= x = Node (insertTree n l) x r
    | otherwise = Node l x (insertTree n r)