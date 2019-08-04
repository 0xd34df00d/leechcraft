module Main where

import Control.Monad.Writer.Strict
import System.Environment

import LC.Build.CollectDeps

main :: IO ()
main = do
  [path] <- getArgs
  (paths, failures) <- runWriterT $ collectPlugins path
  mapM_ print paths
  mapM_ putStrLn failures
