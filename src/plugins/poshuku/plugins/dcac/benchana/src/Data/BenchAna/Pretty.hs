{-# LANGUAGE RecordWildCards, QuasiQuotes #-}

module Data.BenchAna.Pretty(pretty) where

import qualified Data.ByteString.Char8 as BS
import qualified Data.Map as M
import qualified Text.Tabular as T
import qualified Text.Tabular.AsciiArt as T
import Data.List
import Data.Monoid
import Data.String.Interpolate

import Data.BenchAna.Types

pretty :: CellItem a => M.Map BenchConfig a -> String
pretty results = T.render prettyDim BS.unpack asCell $ T.Table
        (T.Group T.DoubleLine $ T.Header <$> dimss)
        (T.Group T.DoubleLine $ T.Header <$> instrs)
        [ [ results M.! BenchConfig { .. } | instr <- instrs ] | dims <- dimss ]
    where instrs = sort $ nub $ instr <$> M.keys results
          dimss = sort $ nub $ dims <$> M.keys results

prettyDim :: Dims -> String
prettyDim Dims { .. } = show width <> "x" <> show height

class CellItem a where
    asCell :: a -> String

instance CellItem Stats where
    asCell Stats { .. } = [i|#{asCell statsNoOutliers} (w/ outliers: #{asCell statsWithOutliers})|]

instance CellItem StatsPair where
    asCell StatsPair { .. } = [i|μ = #{mean}; σ = #{rnd 3 stddev}|]
        where rnd p n = fromInteger (round $ n * 10 ** p) / 10.0 ** p
