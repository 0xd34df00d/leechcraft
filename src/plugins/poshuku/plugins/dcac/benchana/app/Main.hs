module Main where

import qualified Data.ByteString.Char8 as BS
import qualified Data.Map as M
import Data.Monoid
import System.Environment

import Data.BenchAna.Parser
import Data.BenchAna.Stats
import Data.BenchAna.Pretty

main :: IO ()
main = do
    files <- getArgs
    contents <- mapM BS.readFile files
    let parsed = M.unionsWith (<>) $ map parse contents
    if M.null parsed
        then putStrLn "No data points found"
        else putStrLn $ pretty $ stats parsed
