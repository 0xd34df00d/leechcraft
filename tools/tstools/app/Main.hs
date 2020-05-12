{-# LANGUAGE OverloadedStrings, TemplateHaskell, QuasiQuotes #-}

module Main where

import qualified Control.Foldl as F
import qualified Data.Text as T
import Data.Either
import Data.FileEmbed
import Data.Functor
import Data.String.Interpolate.IsString
import Prelude hiding(FilePath)
import System.Environment
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

main :: IO ()
main = do
  [path, lang] <- getArgs
  cd $ fromString path

  files <- lsif (\subpath -> pure $ subpath /= "plugins") "." `fold` F.list
  let sources = filter (\file -> file `hasExtension` "cpp" || file `hasExtension` "ui") files
  generated <- mkGenerated files

  tsBase <- guessTsBase <$> pwd
  let lupdateArgs = ["-noobsolete"] <> fmap toTextHR (sources <> generated) <> ["-ts", [i|#{toTextHR tsBase}_#{lang}.ts|]]
  view $ inproc "lupdate" lupdateArgs empty

  mapM_ rm generated

toTextHR :: FilePath -> Text
toTextHR = either id id . toText
