{-# LANGUAGE RecordWildCards, OverloadedStrings #-}
{-# LANGUAGE FlexibleContexts #-}

module LC.Build.CollectDeps where

import qualified Data.ByteString.Char8 as BS
import Control.Monad.Extra
import Control.Monad.Writer.Strict
import Data.List
import System.Directory.PathWalk
import System.FilePath

data PluginEntry = PluginEntry
  { pluginRoot :: String
  , pluginCpp :: String
  } deriving (Eq, Ord, Show)

collectPlugins :: (MonadIO m, MonadWriter [String] m) => FilePath -> m [PluginEntry]
collectPlugins base = pathWalkAccumulate base cb
  where
    cb dir _ files
      | "CMakeLists.txt" `notElem` files = pure []
      | otherwise = do
          maybePluginCpp <- derivePluginCpp dir files
          case maybePluginCpp of
            Just pluginCpp -> pure [PluginEntry { pluginRoot = dir, .. }]
            Nothing -> tell ["Unable to derive plugin file for " <> dir] >> pure []

derivePluginCpp :: MonadIO m => FilePath -> [FilePath] -> m (Maybe FilePath)
derivePluginCpp base files = liftIO $ findM isPluginFile cpps
  where
    cpps = filter (".cpp" `isSuffixOf`) files
    isPluginFile file = ("LC_EXPORT_PLUGIN" `BS.isInfixOf`) <$> BS.readFile (base </> file)
