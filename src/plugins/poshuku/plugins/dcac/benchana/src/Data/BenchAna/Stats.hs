{-# LANGUAGE RecordWildCards #-}

module Data.BenchAna.Stats(stats) where

import qualified Data.Vector.Unboxed as V
import qualified Statistics.Sample as S
import qualified Statistics.Function as S

import Data.BenchAna.Types

stats :: BenchResults -> BenchStats
stats = fmap singleStats

singleStats :: V.Vector Int -> Stats
singleStats vec = Stats { .. }
    where statsWithOutliers = singleStats' vec'
          statsNoOutliers = singleStats' $ S.sort vec'
          vec' = V.map fromIntegral vec

singleStats' :: V.Vector Double -> StatsPair
singleStats' vec = StatsPair { .. }
    where mean = S.mean vec
          stddev = S.stdDev vec
