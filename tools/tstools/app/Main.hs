{-# LANGUAGE OverloadedStrings, TemplateHaskell, QuasiQuotes #-}

module Main where

import qualified Control.Foldl as F
import qualified Data.Text as T
import Data.Either
import Data.FileEmbed
import Data.String.Interpolate.IsString
import Prelude hiding(FilePath)
import System.Environment
import Turtle

filterExt :: Text -> [FilePath] -> [FilePath]
filterExt ext = filter (`hasExtension` ext)

xsltStyle :: Text
xsltStyle = $(embedStringFile "transform.xsl")

mkGenerated :: MonadIO io => [FilePath] -> io [FilePath]
mkGenerated allFiles
  | [settingFile] <- filter (("settings.xml" `T.isSuffixOf`) . fromRight "" . toText) allFiles = do
      liftIO $ runManaged $ do
        xsltFile <- mktempfile "." "xxxxxx.xslt"
        liftIO $ writeTextFile xsltFile xsltStyle
        let basename = toText' $ dropExtension settingFile
        let xsltprocArgs = ["--stringparam", "filename", basename, toText' xsltFile, toText' settingFile]
        output "dummy.cpp" $ grep (has "QT_TRANSL") $ inproc "xsltproc" xsltprocArgs empty
      pure ["dummy.cpp"]
  | otherwise = pure []
  where
    toText' = fromRight undefined . toText

main :: IO ()
main = do
  [path, lang] <- getArgs
  cd $ fromString path

  files <- lsif (\path -> pure $ path /= "plugins") "." `fold` F.list
  let sources = filterExt "cpp" files <> filterExt "uis" files
  generated <- mkGenerated files

  pure ()
