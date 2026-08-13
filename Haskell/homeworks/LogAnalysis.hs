{-# OPTIONS_GHC -Wall #-}

-- This is homework2

module LogAnalysis where

import Text.Read (readMaybe)

import Log

{-
words :: String -> [String], split the string with ' '
-}
parseMessage :: String -> LogMessage
parseMessage str = case words str of
    [] -> Unknown str
    ("I":ts:msg) -> make Info ts msg
    ("W":ts:msg) -> make Warning ts msg
    ("E":level:ts:msg) -> case readMaybe level of
        Just n -> make (Error n) ts msg
        Nothing -> Unknown str
    _ -> Unknown str
    where
        make typ ts msg = case readMaybe ts of
            Just n -> LogMessage typ n (unwords msg)
            Nothing -> Unknown str
{-
I write this: above code abstract one layer
parseMessage str = case words str of
    [] -> Unknown str
    ("I":ts:msg) -> case readMaybe ts of
        Just n -> LogMessage Info n (unwords msg)
        Nothing -> Unknown str
    ("W":ts:msg) -> case readMaybe ts of
        Just n -> LogMessage Warning n (unwords msg)
        Nothing -> Unknown str
    ("E":level:ts:msg) -> case readMaybe level of
        Just n -> case readMaybe ts of
            Just x -> LogMessage (Error n) x (unwords msg)
            Nothing -> Unknown str
        Nothing -> Unknown str
    _ -> Unknown str
-}

parse :: String -> [LogMessage]
parse str = map parseMessage (lines str)

{- insert
which inserts a new LogMessage into an existing MessageTree, pro-
ducing a new MessageTree. insert may assume that it is given a
sorted MessageTree, and must produce a new sorted MessageTree
containing the new LogMessage in addition to the contents of the
original MessageTree.
-}
insert :: LogMessage -> MessageTree -> MessageTree
insert (Unknown _) msgTree = msgTree
insert logMsg Leaf = Node Leaf logMsg Leaf
insert logMsg (Node l n r)
    | (getTs logMsg) <= (getTs n) = Node (insert logMsg l) n r
    | otherwise                   = Node l n (insert logMsg r)
    where
        getTs (LogMessage _ ts _) = ts
        getTs (Unknown _) = error "Already handled"

build :: [LogMessage] -> MessageTree
build [] = Leaf
build (x:xs) = insert x (build xs)

inOrder :: MessageTree -> [LogMessage]
inOrder Leaf = []
inOrder (Node l m r) = (inOrder l) ++ [m] ++ (inOrder r)

whatWentWrong :: [LogMessage] -> [String]
whatWentWrong [] = []
whatWentWrong (x:xs)
    | checkError x = (getStr x) ++ (whatWentWrong xs)
    | otherwise    = whatWentWrong xs
    where
        checkError (LogMessage (Error level) _ _) = level >= 50
        checkError _ = False
        getStr (LogMessage _ _ str) = [str]
        getStr (Unknown str) = [str]

a :: IO [String] = testWhatWentWrong parse whatWentWrong "sample.log"
b :: IO [String] = testWhatWentWrong parse whatWentWrong "error.log"