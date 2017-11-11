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

data Stats = Stats { mean :: Double
                   , stddev :: Double
                   , meanNoOutliers :: Double
                   , stddevNoOutliers :: Double
                   } deriving (Eq, Ord, Show)

type BenchStats = M.Map BenchConfig Stats
