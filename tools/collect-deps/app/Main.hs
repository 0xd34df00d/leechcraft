module Main where

import System.Environment

import LC.Build.CollectDeps

main :: IO ()
main = do
  [path] <- getArgs
  collectPlugins path >>= mapM_ print
