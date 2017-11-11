{-# LANGUAGE RecordWildCards #-}

module Data.BenchAna.Pretty(pretty) where

import qualified Data.ByteString.Char8 as BS
import qualified Data.Map as M
import qualified Text.Tabular as T
import qualified Text.Tabular.AsciiArt as T
import Data.List
import Data.Monoid

import Data.BenchAna.Types

pretty :: Show a => M.Map BenchConfig a -> String
pretty results = T.render prettyDim BS.unpack show $ T.Table
        (T.Group T.DoubleLine $ T.Header <$> dimss)
        (T.Group T.DoubleLine $ T.Header <$> instrs)
        [ [ results M.! BenchConfig { .. } | instr <- instrs ] | dims <- dimss ]
    where instrs = sort $ nub $ instr <$> M.keys results
          dimss = sort $ nub $ dims <$> M.keys results

prettyDim :: Dims -> String
prettyDim Dims { .. } = show width <> "x" <> show height
