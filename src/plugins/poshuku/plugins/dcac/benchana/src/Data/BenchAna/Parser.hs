{-# LANGUAGE OverloadedStrings, RecordWildCards #-}

module Data.BenchAna.Parser(parse) where

import qualified Data.ByteString.Char8 as BS
import qualified Data.Attoparsec.ByteString.Char8 as AT
import qualified Data.Attoparsec.Combinator as AT
import qualified Data.Map as M
import Control.Arrow
import Data.Functor
import Data.Monoid
import Data.Maybe

import Data.BenchAna.Types

parse :: BS.ByteString -> BenchResults
parse = M.fromListWith (<>) . map (second pure) . mapMaybe parseLine . BS.lines

parseLine :: BS.ByteString -> Maybe (BenchConfig, Int)
parseLine line | Right r <- AT.parseOnly lineParser line = Just r
               | otherwise = Nothing

lineParser :: AT.Parser (BenchConfig, Int)
lineParser = do
    "QDEBUG"
    skipIncluding "bench"
    instr <- fmap BS.pack $ AT.manyTill AT.anyChar $ AT.char '('
    skipIncluding "QSize("
    width <- AT.decimal
    ", "
    height <- AT.decimal
    time <- skipIncluding' AT.decimal
    pure (BenchConfig { dims = Dims { .. }, .. }, time)

skipIncluding :: BS.ByteString -> AT.Parser ()
skipIncluding = void . AT.manyTill AT.anyChar . AT.string

skipIncluding' :: AT.Parser a -> AT.Parser a
skipIncluding' p = AT.manyTill AT.anyChar (AT.lookAhead p) >> p
