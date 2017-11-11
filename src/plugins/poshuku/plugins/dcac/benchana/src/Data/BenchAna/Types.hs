module Data.BenchAna.Types where

import qualified Data.ByteString.Char8 as BS
import qualified Data.Map as M
import qualified Data.Vector as V

data Dims = Dims { width :: Int
                 , height :: Int
                 } deriving (Eq, Ord, Show)

data BenchConfig = BenchConfig { dims :: Dims
                               , instr :: BS.ByteString
                               } deriving (Eq, Ord, Show)

type BenchResults = M.Map BenchConfig (V.Vector Int)

data StatsPair = StatsPair { mean :: Double
                           , stddev :: Double
                           } deriving (Eq, Ord, Show)

data Stats = Stats { stats :: StatsPair
                   , statsNoOutliers :: StatsPair
                   } deriving (Eq, Ord, Show)

type BenchStats = M.Map BenchConfig Stats
