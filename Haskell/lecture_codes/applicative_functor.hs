{- Part I
We want (a -> b -> c) -> (f a -> f b -> f c)

class Functor f => Applicative f where
    pure  :: a -> f a
    (<*>) :: f (a -> b) -> f a -> f b

We get:
    fmap2 :: (a -> b -> c) -> f a -> f b -> f c
and:
    liftA2 :: Applicative f => (a -> b -> c) -> f a -> f b -> f c
    liftA2 h fa fb = (h `fmap` fa) <*> fb
where:
    (<*>) :: Applicative f => f (a -> b) -> f a -> f b

We can go on:
    liftA3 :: Applicative f => (a -> b -> c -> d) -> f a -> f b -> f c -> f d
    liftA3 h fa fb fc = ((h <$> fa) <*> fb) <*> fc
notice that the () are not needed

And we can get some pure function inside:
    liftX :: Applicative f => (a -> b -> c -> d) -> f a -> b -> f c -> f d
    liftX h fa b fc = h <$> fa <*> pure b <*> fc
where b is pure

Laws: (only one)
    f `fmap` x === pure f <*> x
-}

type Name = String

data Employee = Employee { name  :: Name
                         , phone :: String }
                deriving Show

m_name1, m_name2 :: Maybe Name
m_name1 = Nothing
m_name2 = Just "Brent"

m_phone1, m_phone2 :: Maybe String
m_phone1 = Nothing
m_phone2 = Just "555-1234"

ex01 = Employee <$> m_name1 <*> m_phone1
ex02 = Employee <$> m_name1 <*> m_phone2
ex03 = Employee <$> m_name2 <*> m_phone1
ex04 = Employee <$> m_name2 <*> m_phone2

{-
Let's explain ex04, first <$> is just fmap, so
    fmap Employee (Just "Brent") = Just (Employee "Brent")
then:
    Just (Employee "Brent") <*> Just "555-1234"
    = Just (Employee "Brent" "555-1234")
    = Just (Employee {name = "Brent", phone = "555-1234"})

Only if all things are Just, then it is Just, otherwise it is Nothing.
-}

{- Part II. More Example
We have to write Functor and Application such that:
    fmap g x = pure g <*> x
-}

-- The default one is all possible combination
names  = ["Joe", "Sara", "Mae"]
phones = ["555-5555", "123-456-7890", "555-4321"]

employees1 = Employee <$> names <*> phones -- will return 3 * 3 = 9 elements

(.+) = liftA2 (+)    -- 将加法提升到某个 Applicative 上下文中
(.*) = liftA2 (*)    -- 乘法同理

-- 非确定性算术
n = ([4,5] .* pure 2) .+ [6,1] -- （4 或 5）乘以 2，再加上 6 或 1

-- What about zip like implementation (like in python), element to element combination
newtype ZipList a = ZipList { getZipList :: [a] }
    deriving (Eq, Show, Functor)

instance Applicative ZipList where
    pure = ZipList . repeat
    ZipList fs <*> ZipList xs = ZipList (zipWith ($) fs xs)

employees2 = getZipList $ Employee <$> ZipList names <*> ZipList phones

-- Example: render / enviroment applicative, on the (->) e
data BigRecord = BR { getName         :: Name
                    , getSSN          :: String
                    , getSalary       :: Integer
                    , getPhone        :: String
                    , getLicensePlate :: String
                    , getNumSickDays  :: Int
                    }

r = BR "Brent" "XXX-XX-XXX4" 600000000 "555-1234" "JGX-55T3" 2

getEmp :: BigRecord -> Employee
getEmp = Employee <$> getName <*> getPhone

ex01' = getEmp r

-- Example: pair
pair :: Applicative f => f a -> f b -> f (a,b)
-- pair fa fb = (\x y -> (x,y)) <$> fa <*> fb -- then more concise
-- pair fa fb = (,) <$> fa <*> fb             -- then more concise
-- pair fa fb = liftA2 (,) fa fb              -- then more concise
pair = liftA2 (,)

testPair = pair [1, 2] [3, 4]