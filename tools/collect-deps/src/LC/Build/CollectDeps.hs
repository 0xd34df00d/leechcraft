module LC.Build.CollectDeps where

data PluginEntry = PluginEntry
  { cmakeLists :: String
  , pluginCpp :: String
  } deriving (Eq, Ord, Show)
