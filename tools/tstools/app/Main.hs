{-# LANGUAGE OverloadedStrings, TemplateHaskell, QuasiQuotes #-}
{-# LANGUAGE DataKinds, TypeOperators, DeriveGeneric, FlexibleInstances, RecordWildCards #-}

module Main where

import qualified Control.Foldl as F
import qualified Data.Text as T
import Control.Monad
import Data.Either
import Data.FileEmbed
import Data.Functor
import Data.String.Interpolate.IsString
import Options.Generic
import Prelude hiding(FilePath)
import Turtle

xsltStyle :: Text
xsltStyle = $(embedStringFile "transform.xsl")

mkGenerated :: MonadIO io => [FilePath] -> io [FilePath]
mkGenerated allFiles
  | null settingsFiles = pure []
  | otherwise = do
      liftIO $ runManaged $ do
        xsltFile <- mktempfile "." "xxxxxx.xslt"
        liftIO $ writeTextFile xsltFile xsltStyle
        output "dummy.cpp" $ cat $ procFile xsltFile <$> settingsFiles
      pure ["dummy.cpp"]
  where
    toText' = fromRight undefined . toText
    settingsFiles = filter (settingsFilter . fromRight "" . toText) allFiles
    settingsFilter str = any (`T.isSuffixOf` str) ["settings.xml", ".qml.settings"]
    procFile xsltFile settingFile = sed ("__FILENAME__" $> toText' (basename settingFile))
                                  $ grep (has "QT_TRANSL")
                                  $ inproc "xsltproc" [toText' xsltFile, toText' settingFile] empty

guessTsBase :: FilePath -> FilePath
guessTsBase fullPath
  | ["plugins", plugin, "plugins", subplugin] <- components = [i|leechcraft_#{plugin}_#{subplugin}|]
  | ["plugins", plugin] <- components = [i|leechcraft_#{plugin}|]
  | otherwise = [i|leechcraft|]
  where
    components = tail $ dropWhile (/= "src") $ T.takeWhile (/= '/') . toTextHR <$> splitDirectories fullPath

data Options w = Options
  { path :: w ::: Maybe String <?> "Path to the plugin directory"
  , languages :: w ::: [String] <?> "List of languages to generate or update translations for (update all if empty)"
  , dropObsolete :: w ::: Bool <?> "Drop obsolete translations"
  } deriving (Generic)

instance ParseRecord (Options Wrapped)

main :: IO ()
main = do
  Options { .. } <- unwrapRecord "tstools"

  let noobsoleteArg | dropObsolete = ["-noobsolete"]
                    | otherwise = []

  case path of
       Just path' -> cd $ fromString path'
       Nothing -> pure ()

  files <- lsif (\subpath -> pure $ subpath /= "plugins") "." `fold` F.list
  let sources = filter (\file -> any (file `hasExtension`) ["cpp", "ui", "qml"]) files
  generated <- mkGenerated files

  tsFiles <- case languages of
                  [] -> pure $ filter (`hasExtension` "ts") files
                  _ -> do
                        tsBase <- guessTsBase <$> pwd
                        pure $ (\lang -> [i|#{toTextHR tsBase}_#{lang}.ts|]) <$> languages
  forM_ tsFiles $ \tsFile -> do
    let lupdateArgs = noobsoleteArg <> fmap toTextHR (sources <> generated) <> ["-ts", toTextHR tsFile]
    view $ inproc "lupdate" lupdateArgs empty

  mapM_ rm generated

toTextHR :: FilePath -> Text
toTextHR = either id id . toText
