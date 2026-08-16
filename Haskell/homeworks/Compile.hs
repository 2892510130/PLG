{-# LANGUAGE TypeSynonymInstances #-}
{-# LANGUAGE FlexibleInstances #-}

import StackVM

class Expr a where
    compile :: a -> Program
    eval :: a -> Either String StackVal
    eval = stackVM . compile

instance Expr Integer where
    compile n = [PushI n]

instance Expr Bool where
    compile b = [PushB b]

data AddExpr a b = AddExpr a b
instance (Expr a, Expr b) => Expr (AddExpr a b) where
    compile (AddExpr x y) = compile x ++ compile y ++ [Add]

data MulExpr a b = MulExpr a b
instance (Expr a, Expr b) => Expr (MulExpr a b) where
    compile (MulExpr x y) = compile x ++ compile y ++ [Mul]

data AndExpr a b = AndExpr a b
instance (Expr a, Expr b) => Expr (AndExpr a b) where
    compile (AndExpr x y) = compile x ++ compile y ++ [And]

data OrExpr a b = OrExpr a b
instance (Expr a, Expr b) => Expr (OrExpr a b) where
    compile (OrExpr x y) = compile x ++ compile y ++ [Or]

add :: (Expr a, Expr b) => a -> b -> AddExpr a b
add = AddExpr

mul :: (Expr a, Expr b) => a -> b -> MulExpr a b
mul = MulExpr

andOp :: (Expr a, Expr b) => a -> b -> AndExpr a b
andOp = AndExpr

orOp :: (Expr a, Expr b) => a -> b -> OrExpr a b
orOp = OrExpr

main :: IO ()
main = do
    -- 明确指定类型为 Integer
    let expr1 = add (3 :: Integer) (5 :: Integer)
    print $ eval expr1  -- Right (IVal 8)
    
    let expr2 = mul (add (3 :: Integer) (5 :: Integer)) (2 :: Integer)
    print $ eval expr2  -- Right (IVal 16)
    
    let expr3 = andOp (True :: Bool) (False :: Bool)
    print $ eval expr3  -- Right (BVal False)
    
    -- 测试类型错误
    let expr4 = add (True :: Bool) (5 :: Integer)
    print $ eval expr4  -- Left "Encountered 'Add' opcode with ill-typed stack."