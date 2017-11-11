module Data.BenchAna.Types where

import qualified Data.ByteString.Char8 as BS
import qualified Data.Map as M

data Dims = Dims { width :: Int
                 , height :: Int
                 } deriving (Eq, Ord, Show)

data BenchConfig = BenchConfig { dims :: Dims
                               , instr :: BS.ByteString
                               } deriving (Eq, Ord, Show)

type BenchResults = M.Map BenchConfig [Int]
