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
import Data.Foldable
import GHC.Generics

newtype NodeType = NodeType { getNode :: Either T.Text [T.Text] } deriving (Eq, Show, Generic)

instance FromJSON NodeType where
    parseJSON (String s) = pure $ NodeType $ Left s
    parseJSON (Array a) | Just vals' <- vals = pure $ NodeType $ Right vals'
                        | otherwise = fail "Strings expected"
        where vals = mapM toVal $ toList a
              toVal (String s) = Just s
              toVal _ = Nothing
    parseJSON _ = fail "Unexpected node type"

data ClientDescr = ClientDescr {
                       node :: NodeType,
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
    where toPair ClientDescr { .. } = ((id, name), [either pure Prelude.id $ getNode node])
          msgs = map (T.unwords . concat . snd) $ filter ((> 1) . length . snd) $ M.toList $ M.fromListWith (++) $ toPair <$> descrs

process :: BS.ByteString -> T.Text
process file | Right ClientIds { .. } <- dec' = T.unlines $ checkNonInjective $ partialMatches <> fullMatches
             | Left str <- dec' = "Invalid JSON: " <> T.pack str
    where dec' = eitherDecode file

main' :: [String] -> IO ()
main' [filename] = do
    file <- BS.readFile filename
    TI.putStrLn $ process file
main' _ = putStrLn "Usage: CheckClientIds <clientids.json>"

main :: IO ()
main = getArgs >>= main'
