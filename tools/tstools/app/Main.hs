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
  | [settingFile] <- filter (("settings.xml" `T.isSuffixOf`) . fromRight "" . toText) allFiles = do
      liftIO $ runManaged $ do
        xsltFile <- mktempfile "." "xxxxxx.xslt"
        liftIO $ writeTextFile xsltFile xsltStyle
        output "dummy.cpp"
          $ sed ("__FILENAME__" $> toText' (basename settingFile))
          $ grep (has "QT_TRANSL")
          $ inproc "xsltproc" [toText' xsltFile, toText' settingFile] empty
      pure ["dummy.cpp"]
  | otherwise = pure []
  where toText' = fromRight undefined . toText

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
  } deriving (Generic)

instance ParseRecord (Options Wrapped)

main :: IO ()
main = do
  Options { .. } <- unwrapRecord "tstools"

  case path of
       Just path' -> cd $ fromString path'
       Nothing -> pure ()

  files <- lsif (\subpath -> pure $ subpath /= "plugins") "." `fold` F.list
  let sources = filter (\file -> file `hasExtension` "cpp" || file `hasExtension` "ui") files
  generated <- mkGenerated files

  tsFiles <- case languages of
                  [] -> pure $ filter (`hasExtension` "ts") files
                  _ -> do
                        tsBase <- guessTsBase <$> pwd
                        pure $ (\lang -> [i|#{toTextHR tsBase}_#{lang}.ts|]) <$> languages
  forM_ tsFiles $ \tsFile -> do
    let lupdateArgs = ["-noobsolete"] <> fmap toTextHR (sources <> generated) <> ["-ts", toTextHR tsFile]
    view $ inproc "lupdate" lupdateArgs empty

  mapM_ rm generated

toTextHR :: FilePath -> Text
toTextHR = either id id . toText
