{-# LANGUAGE ViewPatterns #-}
{-# LANGUAGE RecordWildCards #-}
{-# LANGUAGE DeriveGeneric, DeriveAnyClass #-}
{-# LANGUAGE OverloadedStrings #-}

import qualified Data.HashMap.Strict as M
import qualified Data.ByteString.Lazy.Char8 as BS
import qualified Data.Text as T
import qualified Data.Text.IO as TI
import System.Environment
import Data.Aeson
import Data.Monoid
import Control.Arrow
import GHC.Generics

data ClientDescr = ClientDescr {
                       node :: T.Text,
                       id :: T.Text,
                       name :: T.Text
                   } deriving (Eq, Show, Generic, FromJSON)

data ClientIds = ClientIds {
                     partialMatches :: [ClientDescr],
                     fullMatches :: [ClientDescr]
                 } deriving (Eq, Show, Generic, FromJSON)

checkNonInjective :: [ClientDescr] -> [T.Text]
checkNonInjective descrs | null msgs = []
                         | otherwise = "The following nodes map to the same info:" : msgs
    where toPair ClientDescr { .. } = ((id, name), [node])
          msgs = map (T.unwords . snd) $ filter ((> 1) . length . snd) $ M.toList $ M.fromListWith (++) $ toPair <$> descrs

process :: BS.ByteString -> T.Text
process file | Just ClientIds { .. } <- dec' = T.unlines $ checkNonInjective $ partialMatches <> fullMatches
             | otherwise = "Invalid JSON"
    where dec' = decode file

main' :: [String] -> IO ()
main' [filename] = do
    file <- BS.readFile filename
    TI.putStrLn $ process file
main' _ = putStrLn "Usage: CheckClientIds <clientids.json>"

main :: IO ()
main = getArgs >>= main'
