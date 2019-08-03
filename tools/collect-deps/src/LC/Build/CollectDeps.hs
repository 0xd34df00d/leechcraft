{-# LANGUAGE RecordWildCards, OverloadedStrings #-}

module LC.Build.CollectDeps where

import qualified Data.ByteString.Char8 as BS
import Control.Monad.Extra
import Data.List
import System.Directory.PathWalk
import System.FilePath

data PluginEntry = PluginEntry
  { pluginRoot :: String
  , pluginCpp :: String
  } deriving (Eq, Ord, Show)

collectPlugins :: FilePath -> IO [PluginEntry]
collectPlugins base = pathWalkAccumulate base cb
  where
    cb dir _ files
      | "CMakeLists.txt" `notElem` files = pure []
      | otherwise = do
          maybePluginCpp <- derivePluginCpp dir files
          case maybePluginCpp of
            Just pluginCpp -> pure [PluginEntry { pluginRoot = dir, .. }]
            Nothing -> pure []

derivePluginCpp :: FilePath -> [FilePath] -> IO (Maybe FilePath)
derivePluginCpp base files = findM isPluginFile cpps
  where
    cpps = filter (".cpp" `isSuffixOf`) files
    isPluginFile file = ("LC_EXPORT_PLUGIN" `BS.isInfixOf`) <$> BS.readFile (base </> file)
