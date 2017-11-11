{-# LANGUAGE RecordWildCards #-}

module Data.BenchAna.Stats where

import qualified Data.Vector.Unboxed as V
import qualified Statistics.Sample as S
import qualified Statistics.Function as S

import Data.BenchAna.Types

produceStats :: V.Vector Int -> Stats
produceStats vec = Stats { .. }
    where stats = produceStats' vec'
          statsNoOutliers = produceStats' $ S.sort vec'
          vec' = V.map fromIntegral vec

produceStats' :: V.Vector Double -> StatsPair
produceStats' vec = StatsPair { .. }
    where mean = S.mean vec
          stddev = S.stdDev vec
